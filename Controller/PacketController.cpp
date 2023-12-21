//
// Created by maxwellzs on 2023/11/30.
//

#include "PacketController.h"
#include <utility>

VNet::SendBox::SendRecord::SendRecord(std::shared_ptr<DataBuffer> dat, size_t retryMax, sequence_t seq)
        : innerData(std::move(dat)), retryMax(retryMax) {
    this->creationTime = clock() - 1000;
    this->sequence = seq;
    this->retryCount = 0;
}

bool VNet::SendBox::SendRecord::trySend(SOCKET sock, sockaddr_in dest) {
    // this resend should be discarded
    if(retryCount == retryMax) {
        return false;
    }
    clock_t now = clock();
    if(now - creationTime >= 1000) {
        sockaddr_in destination;
        destination.sin_family = AF_INET;
        destination.sin_addr = dest.sin_addr;
        destination.sin_port = dest.sin_port;
        // reaches time out
        int rt = sendto(sock,
               (char*)innerData->getBaseBuffer(),
               innerData->getBufferSize(),
               0,
               (sockaddr*)&destination,
               sizeof(sockaddr_in));
        retryCount ++;
        creationTime = now;
    }
    return true;
}

VNet::sequence_t VNet::SendBox::SendRecord::getRecordSequence() {
    return sequence;
}

void VNet::SendBox::Destination::putRecord(std::shared_ptr<DataBuffer> dat, VNet::sequence_t seq, size_t retry) {
    this->records.push_back(std::make_shared<SendRecord>(dat,retry,seq));
}

bool VNet::SendBox::Destination::compareDestination(const sockaddr_in &addr) {
    return (this->addr.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr) &&
            (this->addr.sin_port == addr.sin_port);
}

void VNet::SendBox::Destination::deleteRecord(sequence_t seq, sockaddr_in from) {
    for(auto i = records.begin();i != records.end();i ++) {
        // record found
        if((*i)->getRecordSequence() == seq && compareDestination(from)) {
            records.erase(i);
            return;
        }
    }
}

void VNet::SendBox::Destination::iterateAndSend(SOCKET sSock) {
    for(auto i = records.begin();i != records.end(); i ++) {
        bool kept = (*i)->trySend(sSock, this->addr);
        // if the record is marked delete
        if(!kept) {
            // this destination may have failed
            failedTimes ++;
            i = records.erase(i);
        }
    }
}

sockaddr_in VNet::SendBox::Destination::getAddress() {
    return addr;
}

void VNet::SendBox::Destination::resetFailTime() {
    this->failedTimes = 0;
}

bool VNet::SendBox::Destination::isFailed() {
    return failedTimes > maxFailTime;
}

VNet::SendBox::Destination::Destination(sockaddr_in addr, size_t maxFail) : addr(addr),maxFailTime(maxFail) {

}

void VNet::SendBox::broadcastToAll(std::shared_ptr<DataBuffer> dat, sequence_t seq, size_t retry) {
    for(auto i : routeList) {
        // skip local address
        if(isAddressLocal(i->getAddress())) continue;
        i->putRecord(dat, seq, retry);
    }
}

void VNet::SendBox::flush(SOCKET sSock) {
    for(auto i = routeList.begin();i != routeList.end();i ++) {
        // check whether the destination has failed
        if((*i)->isFailed()) {
            sockaddr_in failedAddress = (*i)->getAddress();
            std::cerr << "[!] the destination " << inet_ntoa(failedAddress.sin_addr)  << ":" << ntohs(failedAddress.sin_port) << " failed too many times" << std::endl;
            i = routeList.erase(i);
            continue;
        }
        (*i)->iterateAndSend(sSock);
    }
}

void VNet::SendBox::cancelPacket(sequence_t seq, sockaddr_in from) {
    for(auto dest : routeList) {
        dest->deleteRecord(seq, from);
    }
}

void VNet::SendBox::registerDestination(sockaddr_in addr, bool reset) {
    for(auto dest : routeList) {
        if(dest->compareDestination(addr)) {
            if(reset) dest->resetFailTime();
            return;
        }
    }
    // seems don't exist, just add
    routeList.push_back(std::make_shared<Destination>(addr));
}

std::vector<sockaddr_in> VNet::SendBox::enumDestination() {
    std::vector<sockaddr_in> allAddr;
    for(auto each : routeList) {
        allAddr.push_back(each->getAddress());
    }
    return allAddr;
}

void
VNet::SendBox::broadcastExcept(std::shared_ptr<DataBuffer> dat, VNet::sequence_t seq, sockaddr_in addr, size_t retry) {
    for(auto i : routeList) {
        if(!i->compareDestination(addr) && !isAddressLocal(i->getAddress())) i->putRecord(dat,seq,retry);
    }
}

void VNet::SendBox::excludeLocalAddress(std::vector<uint32_t> ipList, uint16_t port) {
    this->port = port;
    this->localIP = ipList;
}

bool VNet::SendBox::isAddressLocal(sockaddr_in addr) {
    uint32_t ip = addr.sin_addr.S_un.S_addr;
    for(auto i : localIP) {
        if(ip == i &&
        (ntohs(addr.sin_port) == port ||
        addr.sin_port == port)) {
            return true;
        }
    }
    return false;
}

void VNet::PacketController::registerHandler(std::shared_ptr<EventHandler> newHandle) {
    this->handles.push_back(newHandle);
}

void VNet::PacketController::broadcastPacket(std::shared_ptr<VNet::Packet> newPacket, size_t retry) {
    std::shared_ptr<DataBuffer> b = std::make_shared<DataBuffer>();
    sequence_t generated = preparePacketData(newPacket,b);
    sendBox.broadcastToAll(b,generated, retry);
}

VNet::sequence_t VNet::PacketController::generateSequence() {
    return sequenceController.fetch_add(1);
}

void VNet::PacketController::flushSendBox(SOCKET sSock) {
    sendBox.flush(sSock);
}

void VNet::PacketController::handleData(std::shared_ptr<DataBuffer> coming, sockaddr_in from) {

    /**
     * this is the core method of the controller
     * where all the coming data is handled
     */

    if(coming->getBufferSize() < META_DATA_SIZE) {
        throw PacketHandleException("corrupted meta data");
    }
    PacketMetaData meta = getMetaData(coming);
    if(coming->getBufferSize() < meta.length) {
        throw PacketHandleException("corrupted data packet, the length is smaller than declared in the meta data declared="
        + std::to_string(meta.length) + " actual=" + std::to_string(coming->getBufferSize()));
    }

    sockaddr_in actual = from;
    actual.sin_port= meta.listenPort;

    sendBox.registerDestination(actual, true);
    pending.removeTimedOutArray();

    std::shared_ptr<Packet> packet = nullptr;

    if(meta.flag & FLAG_ECHO) {
        // received an echo meta data, don't invoke handlers
        // prevent the network exchange from altering the address, especially in the shared network
        sendBox.cancelPacket(meta.thisSequence, actual);
        return;
    } else if(meta.flag & FLAG_HEAD) {
        // the first segmentation of the data
        pending.createIncomeArray(from,coming,meta.nextSequence);
    } else if(meta.flag & FLAG_PART) {
        // the body of the data
        pending.addToIncomeArray(from,coming,meta.thisSequence,meta.nextSequence);
        packet = PacketBuilder::constructPacket(pending.mergeData(),from);
    } else {
        packet = PacketBuilder::constructPacket(coming,from);
    }

    if(meta.flag & FLAG_RESEND) echoBack(from,meta.thisSequence);


    for(auto i : handles) {
        // invoke call backs
        if(packet.get()) i->onPacketReceiveCallback(*this,packet,from);
    }
}

VNet::PacketMetaData VNet::PacketController::getMetaData(std::shared_ptr<DataBuffer> in) {
    PacketMetaData meta;
    meta.flag = in->getUint8();
    meta.thisSequence = in->getUint32();
    meta.nextSequence = in->getUint32();
    meta.length = in->getUint16();
    meta.listenPort = in->getUint16();
    return meta;
}

void VNet::PacketController::writeMetaData(VNet::PacketMetaData dat, std::shared_ptr<DataBuffer> out) {
    out->putUint8(dat.flag);
    out->putUint32(dat.thisSequence);
    out->putUint32(dat.nextSequence);
    out->putUint16(dat.length);
    out->putUint16(dat.listenPort);
}

void VNet::PacketController::setEchoSocket(SOCKET s) {
    sock = s;
}

void VNet::PacketController::echoBack(sockaddr_in from, VNet::sequence_t which) {
    sockaddr_in addr = from;
    DataBuffer temp(16);
    temp.putUint8(FLAG_ECHO);
    temp.putUint32(which);
    temp.putUint32(0);
    temp.putUint16(META_DATA_SIZE);
    temp.putUint16(htons(this->port));
    sendto(sock,
           (char *)temp.getBaseBuffer(),
           temp.getBufferSize(), 0,
           reinterpret_cast<const sockaddr *>(&addr),
           sizeof(sockaddr_in));
}

void VNet::PacketController::registerBroadcastDestination(sockaddr_in newAddress, bool reset) {
    this->sendBox.registerDestination(newAddress, reset);
}

void VNet::PacketController::registerIntervalHandler(std::shared_ptr<IntervalEventHandler> newHandle) {
    newHandle->resetTimer();
    intervals.push_back(newHandle);
}

void VNet::PacketController::pullInterval() {
    for(auto each : intervals) {
        if(each->isTimedOut()) {
            each->onInterval(*this);
            each->resetTimer();
        }
    }
}

void
VNet::PacketController::broadcastPacketExcept(std::shared_ptr<VNet::Packet> newPacket, sockaddr_in addr, size_t retry) {
    std::shared_ptr<DataBuffer> newBuffer = std::make_shared<DataBuffer>();
    sequence_t generated = preparePacketData(newPacket,newBuffer);
    sendBox.broadcastExcept(newBuffer,generated,addr,retry);
}

VNet::sequence_t
VNet::PacketController::preparePacketData(std::shared_ptr<Packet> packet, std::shared_ptr<DataBuffer> buf) {
    PacketMetaData meta;
    meta.thisSequence = generateSequence();
    meta.nextSequence = 1 << 31; // preserved
    meta.flag = FLAG_RESEND;
    meta.listenPort = htons(this->port);

    writeMetaData(meta,buf);

    // attach the packet length data
    packet->marshal(buf);
    uint16_t old = buf->setWritePos(sizeof(uint8_t ) + sizeof(uint32_t ) * 2);
    buf->putUint16(old);
    buf->setWritePos(old); // resume the old writing pos
    return meta.thisSequence;
}

VNet::PacketController::PacketController(uint16_t port) : port(port) {
    this->sendBox.excludeLocalAddress(NetworkUtils::getLocalIPList(),port);
}

const char *VNet::PacketHandleException::what() const noexcept {
    return msg.c_str();
}

VNet::PacketIncomingQueue::PacketArray::PacketArray(sockaddr_in from, std::shared_ptr<DataBuffer> first,
                                                    sequence_t next, clock_t timeMax) : from(from), last(next), maxReceiveTime(timeMax) {
    creationTime = clock();
    dataList.push_back(first);
}

bool VNet::PacketIncomingQueue::PacketArray::attachPacket(sockaddr_in from, std::shared_ptr<DataBuffer> data,
                                                          VNet::sequence_t current, VNet::sequence_t next) {
    if(this->from.sin_port != from.sin_port
    || this->from.sin_addr.S_un.S_addr != from.sin_addr.S_un.S_addr) return false;
    if(current != last) return false;
    dataList.push_back(data);
    last = next;
    return true;
}

std::shared_ptr<VNet::DataBuffer> VNet::PacketIncomingQueue::PacketArray::mergeIfDataEnds() {
    if(last != 0) return nullptr; // not ended yet !
    std::shared_ptr<DataBuffer> allData = dataList[0];
    for(auto i = ++dataList.begin();i != dataList.end();i ++) {
        void * dat = (char *)(*i)->getBaseBuffer() + META_DATA_SIZE;
        size_t transferSize = (*i)->getBufferSize() - META_DATA_SIZE;
        allData->write(dat,transferSize);
    }
    allData->setReadPos(META_DATA_SIZE);
    return allData;
}

bool VNet::PacketIncomingQueue::PacketArray::isTimedOut() {
    return (clock() - creationTime) > maxReceiveTime;
}

void
VNet::PacketIncomingQueue::createIncomeArray(sockaddr_in from, std::shared_ptr<DataBuffer> first, VNet::sequence_t next,
                                             clock_t timeMax) {
    queue.push_back(std::make_shared<PacketArray>(from,first,next,timeMax));
}

void
VNet::PacketIncomingQueue::addToIncomeArray(sockaddr_in from, std::shared_ptr<DataBuffer> dat,VNet::sequence_t current,VNet::sequence_t next) {
    std::cout << "attaching income array for sequence = "  << current << std::endl;
    for(auto i : queue) {
        // try to attach to each destination
        // this operation may fail
        if(i->attachPacket(from,dat,current,next)) return;
    }
}

void VNet::PacketIncomingQueue::removeTimedOutArray() {
    // remove timed out array
    for(auto i = queue.begin();i != queue.end();i ++) {
        if((*i)->isTimedOut()) {
            std::cout << "time out" << std::endl;
            i = queue.erase(i);
        }
    }
}

std::shared_ptr<VNet::DataBuffer> VNet::PacketIncomingQueue::mergeData() {
    for(auto i = queue.begin();i != queue.end();i ++) {
        auto ptr = (*i)->mergeIfDataEnds();
        if(ptr.get()) {
            // found data, remove from the pending list
            queue.erase(i);
            return ptr;
        }
    }
    // not found
    return nullptr;
}

VNet::IntervalEventHandler::IntervalEventHandler(clock_t interval) : interval(interval) {
    lastPull = clock() - interval;
}

bool VNet::IntervalEventHandler::isTimedOut() {
    clock_t current = clock();
    if(current - lastPull > interval) {
        return true;
    }
    return false;
}

void VNet::IntervalEventHandler::resetTimer() {
    lastPull = clock();
}
