//
// Created by maxwellzs on 2023/12/7.
//

#ifndef VNET_TEXTMESSAGEPACKET_H
#define VNET_TEXTMESSAGEPACKET_H

#include "PacketBuilder.h"

namespace VNet {

    // should send no larger than 1000 words at once
    const size_t MAX_MESSAGE_LENGTH = 1000;

    class TextMessageException : public std::exception {
    private:
        std::string msg;
    public:
        TextMessageException(size_t provided);
        const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    };

    class TextMessagePacket : public VNet::Packet {
    private:
        std::string message; // the text message
        std::string textUUID; // the text identifier
        std::string nickName = "Anonymous"; // the given username, if not set then it is Anonymous
    public:
        void setMessageBody(const std::string &val);
        void setNickName(const std::string& nickName);
        void setUUIDString(const std::string& uuid);

        const std::string &getMessage() const;

        const std::string &getTextUuid() const;

        const std::string &getNickName() const;


        PACKET_ID_DES(TextMessagePacket, 0x3, "[raw text message]");

        std::shared_ptr<Packet> unMarshal(std::shared_ptr<DataBuffer> inBuf, sockaddr_in sender) override;

        void marshal(std::shared_ptr<DataBuffer> outBuf) override;

    };

    REG_PACKET_PROTOTYPE(TextMessagePacket)

};

#endif //VNET_TEXTMESSAGEPACKET_H
