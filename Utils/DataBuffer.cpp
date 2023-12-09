//
// Created by maxwellzs on 2023/11/29.
//

#include "DataBuffer.h"

VNet::DataBuffer::DataBuffer(size_t size) {
    this->base = new char[size];
    capacity = size;
}

void VNet::DataBuffer::grow(size_t newSize) {
    if (newSize <= capacity) return;
    char *prev = base;
    base = new char[newSize];
    memcpy(base, prev, writePos);
    delete[] prev;
}

void VNet::DataBuffer::write(const void *pData, size_t lData) {
    if (writePos + lData >= capacity) grow(2 * (capacity + lData));
    memcpy(this->base + writePos, pData, lData);
    writePos += lData;
}

size_t VNet::DataBuffer::getBufferSize() {
    return writePos;
}

const void *VNet::DataBuffer::getBaseBuffer() {
    return base;
}

VNet::DataBuffer::~DataBuffer() {
    delete[] base;
}

sockaddr_in VNet::DataBuffer::write(SOCKET rSock, size_t length) {
    if (capacity <= length) grow(length + 1);
    sockaddr_in clientAddr;
    int addrSize = sizeof(clientAddr);
    int rtCode = recvfrom(rSock,
                          base,
                          capacity,
                          0,
                          (sockaddr *) &clientAddr,
                          &addrSize);

    if (rtCode <= 0) {
        throw BufferReceiveException("recvfrom() failed");
    }

    // reset the writing position
    this->writePos = rtCode;
    return clientAddr;
}

std::string VNet::DataBuffer::toString() {
    std::string res;
    for (size_t i = 0; i < writePos; i++) {
        res += base[i];
    }
    return res;
}

void VNet::DataBuffer::validateReadOperation(size_t want) {
    if (readPos + want > writePos) throw ReadOutOfBoundException();
}

uint8_t VNet::DataBuffer::getUint8() {
    validateReadOperation(sizeof(uint8_t));
    readPos++;
    return base[readPos - 1];
}

uint16_t VNet::DataBuffer::getUint16() {
    validateReadOperation(sizeof(uint16_t));
    return BIT(2) | BIT(1);
}

void VNet::DataBuffer::putUint8(const uint8_t &val) {
    uint8_t v = val;
    write(&v, 1);
}

void VNet::DataBuffer::putUint16(const uint16_t &val) {
    putBytes({
                     GET_BIT(2),
                     GET_BIT(1)
             });
}

uint32_t VNet::DataBuffer::getUint32() {
    validateReadOperation(sizeof(uint32_t));
    return BIT(4) | BIT(3) | BIT(2) | BIT(1);
}

void VNet::DataBuffer::putUint32(const uint32_t &val) {
    putBytes({
                     GET_BIT(4),
                     GET_BIT(3),
                     GET_BIT(2),
                     GET_BIT(1)
             });
}

void VNet::DataBuffer::putBytes(const std::vector<uint8_t> &bytes) {
    for (auto i: bytes) {
        putUint8(i);
    }
}

uint64_t VNet::DataBuffer::getUint64() {
    validateReadOperation(sizeof(uint64_t));
    return BIT(8)| BIT(7)| BIT(6)| BIT(5)| BIT(4)| BIT(3)| BIT(2)| BIT(1);
}

void VNet::DataBuffer::putUint64(const uint64_t &val) {
    putBytes({
                     GET_BIT(8),
                     GET_BIT(7),
                     GET_BIT(6),
                     GET_BIT(5),
                     GET_BIT(4),
                     GET_BIT(3),
                     GET_BIT(2),
                     GET_BIT(1)
             });
}

size_t VNet::DataBuffer::setWritePos(size_t newPos) {
    size_t old = writePos;
    writePos = newPos;
    return old;
}

void VNet::DataBuffer::write(VNet::DataBuffer &another) {
    write(another.getBaseBuffer(),another.getBufferSize());
}

size_t VNet::DataBuffer::setReadPos(size_t newPos) {
    size_t old = readPos;
    readPos = newPos;
    return old;
}

void VNet::DataBuffer::writeVarString(const std::string &s) {
    uint16_t strLength = s.length();
    putUint16(strLength);
    write(s.c_str(),s.length());
}

std::string VNet::DataBuffer::readVarString() {
    std::string val;
    uint16_t strLength = getUint16();
    validateReadOperation(strLength);
    val.assign((char*)base + readPos,strLength);
    readPos += strLength;
    return val;
}

VNet::BufferReceiveException::BufferReceiveException(const std::string &reason) {
    this->reason = reason;
    wsaError = WSAGetLastError();
    this->msg = "cannot write data to buffer, reason=" + reason + ",wsa error=" + std::to_string(wsaError);
}

const char *VNet::BufferReceiveException::what() const noexcept {
    return msg.c_str();
}

int VNet::BufferReceiveException::getWsaError() const {
    return wsaError;
}

const char *VNet::ReadOutOfBoundException::what() const noexcept {
    return "read buffer out of bound";
}
