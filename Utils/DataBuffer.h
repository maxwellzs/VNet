//
// Created by maxwellzs on 2023/11/29.
//

#ifndef VNET_DATABUFFER_H
#define VNET_DATABUFFER_H

#include <winsock2.h>
#include <cstddef>
#include <string>
#include <cstring>
#include <exception>
#include <vector>

namespace VNet {


#define ALL_BITS 0b11111111
#define GET_BIT(x) (uint8_t)((val >> (8 * (x-1))) & ALL_BITS)
#define BIT(x) ((uint64_t)(((uint8_t*)base)[readPos++]) << (8 * (x-1)))

    class BufferReceiveException : public std::exception {
    private:
        std::string reason;
        std::string msg;
        int wsaError;
    public:
        BufferReceiveException(const std::string &reason);

        int getWsaError() const;

        const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    };

    class ReadOutOfBoundException : public std::exception {
    public:
        const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    };

    class DataBuffer {
    private:
        char *base = nullptr;  // the base pointer of the inner data
        size_t capacity;        // how much number of bytes this buffer can currently hold
        size_t writePos = 0;    // how much number of bytes have benn writen
        size_t readPos = 0;

        /**
         * enlarge the base buffer size
         * @param newSize the new size
         */
        void grow(size_t newSize);

        /**
         * to check whether the read operation is out of bound
         * @param want the number of bytes wanted
         */
        void validateReadOperation(size_t want);

    public:

        ~DataBuffer();

        /**
         * construct the data buffer with given size
         * @param size the size of the data buffer
         */
        DataBuffer(size_t size);

        DataBuffer() : DataBuffer(512) {};

        /**
         * write the certain amount of bytes into this buffer
         * @param pData the pointer pointing to the data
         * @param lData the length of the data
         */
        void write(const void *pData, size_t lData);

        /**
         * write another DataBuffer into this one
         * @param another another DataBuffer object
         */
        void write(DataBuffer& another);

        /**
         * write a string and the length of it
         * @param s the given string
         */
        void writeVarString(const std::string& s);
        /**
         * read out a variable string from the buffer
         * the first two byte will be read as string length
         * @return the string
         */
        std::string readVarString();

        /**
         * receive a packet and write into the buffer
         * @param rSock the receive socket
         * @param length the max packet size
         * @return the address of the income packet
         */
        sockaddr_in write(SOCKET rSock, size_t length);

        /**
         * get how much number of bytes are there in the buffer
         * @return the number of bytes in this buffer
         */
        size_t getBufferSize();

        uint8_t getUint8();
        void putUint8(const uint8_t &val);
        void putBytes(const std::vector<uint8_t>& bytes);
        uint16_t getUint16();
        void putUint16(const uint16_t &val);
        uint32_t getUint32();
        void putUint32(const uint32_t &val);
        uint64_t getUint64();
        void putUint64(const uint64_t &val);

        /**
         * redirect the write pointer
         * @param newPos the new pointer
         * @return the oldPos
         */
        size_t setWritePos(size_t newPos);

        /**
         * redirect the read pointer
         * @param newPos the new read position
         * @return the old position
         */
        size_t setReadPos(size_t newPos);

        /**
         * get the base pointer of this buffer
         * @return the base pointer, read only
         */
        const void *getBaseBuffer();

        std::string toString();
    };

};

#endif //VNET_DATABUFFER_H
