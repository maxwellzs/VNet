//
// Created by maxwellzs on 2023/12/7.
//

#include "IPSyncPacket.h"

std::shared_ptr<VNet::Packet> VNet::IPSyncPacket::unMarshal(std::shared_ptr<DataBuffer> inBuf, sockaddr_in sender) {
    std::shared_ptr<VNet::IPSyncPacket> p = std::make_shared<VNet::IPSyncPacket>();
    uint8_t pack_id = inBuf->getUint8();
    if(pack_id != getPacketID()) return nullptr;
    uint16_t IPCount = inBuf->getUint16();
    for(uint16_t i = 0;i < IPCount;i ++) {
        uint32_t ip = inBuf->getUint32();
        uint16_t port = inBuf->getUint16();
        sockaddr_in addr;
        addr.sin_addr.S_un.S_addr = ip;
        addr.sin_port = port;
        p->addrList.push_back(addr);
    }
    return p;
}

void VNet::IPSyncPacket::marshal(std::shared_ptr<DataBuffer> outBuf) {
    Packet::marshal(outBuf);
    uint16_t listSize = addrList.size();
    outBuf->putUint16(listSize);
    for(auto each : addrList) {
        outBuf->putUint32(each.sin_addr.S_un.S_addr);
        outBuf->putUint16(each.sin_port);
    }
}

std::vector<sockaddr_in> VNet::IPSyncPacket::getAddress() {
    return addrList;
}

void VNet::IPSyncPacket::putAddress(sockaddr_in addr) {
    addrList.push_back(addr);
}
