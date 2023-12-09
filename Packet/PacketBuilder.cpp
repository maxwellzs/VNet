//
// Created by maxwellzs on 2023/11/30.
//

#include "PacketBuilder.h"
#include "Controller/PacketController.h"

std::vector<std::shared_ptr<VNet::Packet>> * VNet::PacketBuilder::prototypes = VNet::PacketBuilder::prototypes;

void VNet::PacketBuilder::addPacketPrototype(std::shared_ptr<Packet> newProto) {
    PacketBuilder::prototypes->push_back(newProto);
}

std::shared_ptr<VNet::Packet> VNet::PacketBuilder::constructPacket(std::shared_ptr<DataBuffer> buf, sockaddr_in clientAddr) {
    if(!buf.get()) return nullptr;
    for(const auto& each : (*PacketBuilder::prototypes)) {
        buf->setReadPos(META_DATA_SIZE); // resume the read pos for next parse
        std::shared_ptr<Packet> construction = nullptr;
        try {
            construction = each->unMarshal(buf,clientAddr);
        } catch (std::exception& e) {
            construction = nullptr;
        }
        if(construction.get()) return construction;
    }
    return nullptr;
}

void VNet::destroy_prototypes() {
    PacketBuilder::cleanPrototypes();
}

void VNet::init_prototypes() {
    PacketBuilder::setupPrototypes();
}
