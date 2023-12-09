//
// Created by maxwellzs on 2023/12/7.
//

#include "TextMessagePacket.h"

std::shared_ptr<VNet::Packet> VNet::TextMessagePacket::unMarshal(std::shared_ptr<DataBuffer> inBuf, sockaddr_in sender) {
    std::shared_ptr<VNet::TextMessagePacket> packet = std::make_shared<TextMessagePacket>();
    uint8_t pack_id = inBuf->getUint8();
    if(pack_id != getPacketID()) return nullptr;
    packet->setUUIDString(inBuf->readVarString());
    packet->setNickName(inBuf->readVarString());
    packet->setMessageBody(inBuf->readVarString());
    return packet;
}

void VNet::TextMessagePacket::marshal(std::shared_ptr<DataBuffer> outBuf) {
    Packet::marshal(outBuf);
    outBuf->writeVarString(textUUID);
    outBuf->writeVarString(nickName);
    outBuf->writeVarString(message);
}

void VNet::TextMessagePacket::setMessageBody(const std::string &val) {
    if(val.length() > MAX_MESSAGE_LENGTH) throw TextMessageException(val.length());
    message = val;
}

void VNet::TextMessagePacket::setNickName(const std::string &nickName) {
    this->nickName = nickName;
}

void VNet::TextMessagePacket::setUUIDString(const std::string &uuid) {
    this->textUUID = uuid;
}

const std::string &VNet::TextMessagePacket::getMessage() const {
    return message;
}

const std::string &VNet::TextMessagePacket::getTextUuid() const {
    return textUUID;
}

const std::string &VNet::TextMessagePacket::getNickName() const {
    return nickName;
}

const char *VNet::TextMessageException::what() const noexcept {
    return msg.c_str();
}

VNet::TextMessageException::TextMessageException(size_t provided) {
    msg = "The text message should not be larger than " + std::to_string(MAX_MESSAGE_LENGTH)
            + ", provided "  + std::to_string(provided) + " words";
}
