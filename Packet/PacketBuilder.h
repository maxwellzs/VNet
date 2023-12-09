//
// Created by maxwellzs on 2023/11/30.
//

#ifndef VNET_PACKETBUILDER_H
#define VNET_PACKETBUILDER_H

#include <vector>
#include <memory>
#include <iostream>
#include "Packet.h"
#include "Utils/DataBuffer.h"

namespace VNet {

/**
 * use this macro to register prototype
 */
#define REG_PACKET_PROTOTYPE(name) __attribute__((constructor(102)))              \
    static void name##_reg_function() {                                                 \
        VNet::PacketBuilder::addPacketPrototype(std::make_shared<VNet::name>()); \
    }

    class PacketBuilder {
    private:
        static std::vector<std::shared_ptr<Packet>> * prototypes;
    public:
        PacketBuilder() = delete;
        /**
         * register a prototype into this builder
         * @param newProto new prototype
         */
        static void addPacketPrototype(std::shared_ptr<Packet> newProto);

        /**
         * construct the data buffer with the given buffer
         * the data buffer will pass down to every sections
         * @param buf the buffer
         * @return pointer of the new packet if the construction succeeded
         */
        static std::shared_ptr<Packet> constructPacket(std::shared_ptr<DataBuffer> buf, sockaddr_in clientAddr);

        inline static void cleanPrototypes() {
            delete PacketBuilder::prototypes;
        }

        inline static void setupPrototypes() {
            prototypes = new std::vector<std::shared_ptr<Packet>>;
        }
    };

    __attribute__((destructor))
    void destroy_prototypes();

    __attribute__((constructor(101)))
    void init_prototypes();

};

#endif //VNET_PACKETBUILDER_H
