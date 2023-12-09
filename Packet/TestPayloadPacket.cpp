//
// Created by maxwellzs on 2023/12/3.
//

#include "TestPayloadPacket.h"

std::shared_ptr<VNet::Packet> VNet::TestPayloadPacket::unMarshal(std::shared_ptr<DataBuffer> inBuf, sockaddr_in sender) {
    return std::shared_ptr<TestPayloadPacket>();
}

void VNet::TestPayloadPacket::marshal(std::shared_ptr<DataBuffer> outBuf) {
    Packet::marshal(outBuf);
}
