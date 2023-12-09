//
// Created by maxwellzs on 2023/12/7.
//

#ifndef VNET_IPSYNCPACKET_H
#define VNET_IPSYNCPACKET_H

#include "Packet/Packet.h"
#include "Packet/PacketBuilder.h"
#include <vector>

namespace VNet {

    class IPSyncPacket : public Packet {
    private:
        std::vector<sockaddr_in> addrList;
    public:
        PACKET_ID_DES(IPSyncPacket,0x2,"[synchronize ip address packet]");

        std::vector<sockaddr_in> getAddress();

        std::shared_ptr<Packet> unMarshal(std::shared_ptr<DataBuffer> inBuf, sockaddr_in sender) override;

        void marshal(std::shared_ptr<DataBuffer> outBuf) override;

        void putAddress(sockaddr_in addr);

    };

    REG_PACKET_PROTOTYPE(IPSyncPacket)


};

#endif //VNET_IPSYNCPACKET_H
