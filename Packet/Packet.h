//
// Created by maxwellzs on 2023/11/29.
//

#ifndef VNET_PACKET_H
#define VNET_PACKET_H

#include <winsock2.h>
#include "Utils/DataBuffer.h"
#include <memory>

namespace VNet {

#define PACKET_ID(name,id) explicit name() : VNet::Packet(id) {};
#define PACKET_ID_DES(name,id,des) explicit name() : VNet::Packet(id,des) {};

    /**
     * the abstract packet class
     * a packet is a sequence of data
     */
    class Packet {
    protected:
        uint8_t     packet_id;      // the identifier of the packet
        std::string packet_description;
        sockaddr_in sender;         // the sender's address of this packet
    public:
        /**
         * construct the packet with at least a packet id
         * @param pack_id the id of this packet
         */
        explicit Packet(uint8_t pack_id,const std::string& pack_des = "[unnamed generic packet]") : packet_id(pack_id),packet_description(pack_des) {};
        /**
         * unpack this buffer from the given buffer
         * @param inBuf the input buffer
         * @param sender the sender of this packet
         * @return the new packet, if the packet is malformed, return nullptr
         */
        virtual std::shared_ptr<Packet> unMarshal(std::shared_ptr<DataBuffer> inBuf,sockaddr_in sender) = 0;

        /**
         * serialize the package into the given buffer
         * @param outBuf the output buffer
         */
        virtual void marshal(std::shared_ptr<DataBuffer> outBuf) {
            outBuf->putUint8(packet_id);
        };

        inline uint8_t getPacketID() {
            return packet_id;
        }

        inline u_long getPacketIPSource() {
            return sender.sin_addr.S_un.S_addr;
        }

        inline const std::string& getPacketDescription() {
            return packet_description;
        }
    };

};

#endif //VNET_PACKET_H
