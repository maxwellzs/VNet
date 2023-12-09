//
// Created by maxwellzs on 2023/12/3.
//

#ifndef VNET_TESTPAYLOADPACKET_H
#define VNET_TESTPAYLOADPACKET_H

#include "Packet/Packet.h"
#include "Packet/PacketBuilder.h"

namespace VNet {

    class TestPayloadPacket : public Packet {
    public:
        PACKET_ID_DES(TestPayloadPacket,0x1,"[test payload]")

        std::shared_ptr<Packet> unMarshal(std::shared_ptr<DataBuffer> inBuf, sockaddr_in sender) override;

        void marshal(std::shared_ptr<DataBuffer> outBuf) override;

    };

    REG_PACKET_PROTOTYPE(TestPayloadPacket)

};

#endif //VNET_TESTPAYLOADPACKET_H
