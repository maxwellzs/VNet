//
// Created by maxwellzs on 2023/11/30.
//

#ifndef VNET_PACKETCONTROLLER_H
#define VNET_PACKETCONTROLLER_H

#include <memory>
#include <atomic>
#include <ctime>
#include <map>
#include <list>
#include "Utils/DataBuffer.h"
#include "Utils/NetworkUtils.h"
#include "Packet/Packet.h"
#include "Packet/PacketBuilder.h"

namespace VNet {

    typedef uint32_t sequence_t;

    const uint8_t FLAG_RESEND = 0b1;
    const uint8_t FLAG_ECHO = 0b1 << 1;
    const uint8_t FLAG_PART = 0b1 << 2;
    const uint8_t FLAG_HEAD = 0b1 << 3;

    class PacketHandleException : public std::exception {
    private:
        std::string msg;
    public:
        PacketHandleException(const std::string &msg) {
            this->msg += "failed to handle the incoming payload, reason : " + msg;
        }

        const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    };

    typedef struct {
        uint8_t flag;
        sequence_t thisSequence;
        sequence_t nextSequence;
        uint16_t length;
        uint16_t listenPort;
    } PacketMetaData;

#define META_DATA_SIZE (sizeof(uint8_t) + 2 * sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t))


    /**
     * this class holds the packet to be send out
     */
    class SendBox {

        class SendRecord {
        private:
            clock_t creationTime;   // the packet will be resent every 1 second
            size_t retryMax;        // the max count of this packet being sent out
            size_t retryCount = 0;
            sequence_t sequence;
            std::shared_ptr<DataBuffer> innerData; // the data registered to this record
        public:
            SendRecord(std::shared_ptr<DataBuffer> dat, size_t retryMax, sequence_t seq);

            /**
             * try to send the inner data to the address
             * this operation will succeed if this record reaches time out
             * @param sock the out socket
             * @param dest the wanted destination
             * @return true if this record should be kept
             */
            bool trySend(SOCKET sock, sockaddr_in dest);

            sequence_t getRecordSequence();
        };

        class Destination {
        private:
            std::list<std::shared_ptr<SendRecord>> records;
            sockaddr_in addr;
            size_t failedTimes = 0;
            size_t maxFailTime;
        public:
            Destination(sockaddr_in addr, size_t maxFail = 5);

            /**
             * put in a buffer to be send out
             * @param dat the data
             * @param seq the sequence number of this buffer
             * @param retry the number of retries
             */
            void putRecord(std::shared_ptr<DataBuffer> dat, sequence_t seq, size_t retry);

            /**
             * compare the given address with the address in this destination
             * @param addr the given destination
             * @return true if the address is identical
             */
            bool compareDestination(const sockaddr_in &addr);

            /**
             * delete the record with the given sequence
             * @param seq the sequence number
             * @param from where the packet is from
             */
            void deleteRecord(sequence_t seq, sockaddr_in from);

            /**
             * iterate all records and try to send
             * @param sSock the socket use to send data
             */
            void iterateAndSend(SOCKET sSock);

            sockaddr_in getAddress();

            bool isFailed();

            void resetFailTime();
        };

    private:
        std::list<std::shared_ptr<Destination>> routeList; // store all destination to be broadcast
        std::vector<uint32_t> localIP;
        uint16_t port;

        /**
        * check if the address is a local address
        * @param addr
        * @return
        */
        bool isAddressLocal(sockaddr_in addr);

    public:
        /**
         * broad cast a series of data to all registered clients
         * @param dat the data
         * @param seq the sequence of the current data buffer
         * @param retry the max retry time
         */
        void broadcastToAll(std::shared_ptr<DataBuffer> dat, sequence_t seq, size_t retry = 5);

        /**
         * broadcast the packet to all destination except the given one
         * @param dat the data
         * @param seq the sequence of the packet
         * @param addr the address won't send
         * @param retry the max retry times
         */
        void broadcastExcept(std::shared_ptr<DataBuffer> dat, sequence_t seq, sockaddr_in addr, size_t retry = 5);

        /**
         * flush all records
         * @param sSock the socket used to send packet
         */
        void flush(SOCKET sSock);

        /**
         * cancel the sending of the packet marked with the given sequence number
         * @param seq the targeted sequence
         * @param from where the echo is from
         */
        void cancelPacket(sequence_t seq, sockaddr_in from);

        /**
         * register the audience to this
         * @param addr the address, if the address has existed already then nothing will happen
         * @param reset whether to reset the destination fail time
         */
        void registerDestination(sockaddr_in addr, bool reset);

        std::vector<sockaddr_in> enumDestination();

        /**
         * prevent this box to send packet to local address
         * @param ipList the list of the ip address
         * @param port listening port
         */
        void excludeLocalAddress(std::vector<uint32_t> ipList, uint16_t port);
    };

    class PacketIncomingQueue {
    private:

        class PacketArray {
        private:
            sockaddr_in from;
            sequence_t last;
            std::vector<std::shared_ptr<DataBuffer>> dataList;
            clock_t creationTime;
            clock_t maxReceiveTime;
        public:
            /**
             * register a packet pending to receive
             * @param from where the first packet is from
             * @param first the first packet
             * @param next the next expected sequence
             */
            PacketArray(sockaddr_in from, std::shared_ptr<DataBuffer> first, sequence_t next, clock_t timeMax = 100000);

            /**
             * put this packet to the array, if the current meet the last
             * and the address is identical then returns true, else false
             * @param from where the packet is from
             * @param packet the packet data
             * @param current the sequence attached to this data
             * @param next the next expected sequence
             * @return true if the address and sequence are identical with the attribute on this array
             */
            bool attachPacket(sockaddr_in from, std::shared_ptr<DataBuffer> data, sequence_t current, sequence_t next);

            /**
             * merge all the data in this array if the packet has ended
             * @return nullptr if still pending
             */
            std::shared_ptr<DataBuffer> mergeIfDataEnds();

            /**
             * whether this array should be discarded
             * @return true if should be discarded
             */
            bool isTimedOut();

        };

        std::list<std::shared_ptr<PacketArray>> queue;

    public:
        void createIncomeArray(sockaddr_in from, std::shared_ptr<DataBuffer> first, sequence_t next,
                               clock_t timeMax = 100000);

        void addToIncomeArray(sockaddr_in from, std::shared_ptr<DataBuffer> dat, sequence_t current, sequence_t next);

        void removeTimedOutArray();

        std::shared_ptr<DataBuffer> mergeData();
    };

    class PacketController;

    class EventHandler {
    public:
        virtual void
        onPacketReceiveCallback(PacketController &parent, std::shared_ptr<Packet> packet, sockaddr_in from) = 0;
    };

    class IntervalEventHandler {
    protected:
        clock_t interval;
        clock_t lastPull;
    public:
        IntervalEventHandler(clock_t interval);

        bool isTimedOut();

        void resetTimer();

        virtual void onInterval(PacketController &parent) = 0;
    };

    /**
     * the util class to control the packet transmission
     * a packet can be marked resend. when the controller receive such a packet,
     * an echo packet will be send back to confirm the packet receive
     * If the packet is too big, then the controller will split the packet
     */
    class PacketController {
    private:
        std::vector<std::shared_ptr<EventHandler>> handles;
        std::vector<std::shared_ptr<IntervalEventHandler>> intervals;
        std::atomic_int64_t sequenceController;
        SendBox sendBox;
        PacketIncomingQueue pending;

        uint16_t port;
        SOCKET sock;

        sequence_t generateSequence();

        PacketMetaData getMetaData(std::shared_ptr<DataBuffer> in);

        void writeMetaData(PacketMetaData dat, std::shared_ptr<DataBuffer> out);

        void echoBack(sockaddr_in from, sequence_t which);

        /**
         * serialize the given data packet into the given buffer
         * @param packet
         * @param buf
         * @return the generated sequence number
         */
        sequence_t preparePacketData(std::shared_ptr<Packet> packet, std::shared_ptr<DataBuffer> buf);

    public:
        /**
         * create a controller on the given listen port
         * @param port the port the program is listening on
         */
        PacketController(uint16_t port);

        void registerBroadcastDestination(sockaddr_in newAddress, bool reset);

        void setEchoSocket(SOCKET s);

        void registerHandler(std::shared_ptr<EventHandler> newHandle);

        inline std::vector<sockaddr_in> enumDestination() {
            return sendBox.enumDestination();
        }

        void registerIntervalHandler(std::shared_ptr<IntervalEventHandler> newHandle);

        void broadcastPacket(std::shared_ptr<VNet::Packet> newPacket, size_t retry = 5);

        void broadcastPacketExcept(std::shared_ptr<VNet::Packet> newPacket, sockaddr_in addr, size_t retry = 5);

        /**
         * flush out the existing records in the send box
         * @param sSock the socket used to send
         */
        void flushSendBox(SOCKET sSock);

        void pullInterval();

        /**
         * handle the income payload and read the packet metadata
         * @param coming the coming packet
         * @param from where the packet comes from
         */
        void handleData(std::shared_ptr<DataBuffer> coming, sockaddr_in from);
    };

};

#endif //VNET_PACKETCONTROLLER_H
