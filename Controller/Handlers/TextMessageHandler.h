//
// Created by maxwellzs on 2023/12/8.
//

#ifndef VNET_TEXTMESSAGEHANDLER_H
#define VNET_TEXTMESSAGEHANDLER_H

#include "Controller/PacketController.h"

namespace VNet {

    class TextMessageHandler : public EventHandler {
    private:
        void printTextMessage(const std::string& name, const std::string& uuid, const std::string& msg);
    public:
        void
        onPacketReceiveCallback(PacketController &parent, std::shared_ptr<Packet> packet, sockaddr_in from) override;

    };

};

#endif //VNET_TEXTMESSAGEHANDLER_H
