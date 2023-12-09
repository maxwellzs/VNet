//
// Created by maxwellzs on 2023/12/8.
//

#include "TextMessageHandler.h"
#include "Packet/TextMessagePacket.h"

void VNet::TextMessageHandler::onPacketReceiveCallback(VNet::PacketController &parent, std::shared_ptr<Packet> packet,
                                                       sockaddr_in from) {
    if(packet.get() && packet->getPacketID() == 0x3) {
        auto textPack = std::static_pointer_cast<TextMessagePacket>(packet);
        printTextMessage(textPack->getNickName(),textPack->getTextUuid(),textPack->getMessage());
    }
}

void VNet::TextMessageHandler::printTextMessage(const std::string &name, const std::string &uuid, const std::string &msg) {
    std::cout << std::endl << "[" << name << "]::{" << uuid << "}" << std::endl
    << " >>\t" << msg << std::endl;
}
