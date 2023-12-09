#include <iostream>
#include "Controller/UDPController.h"
#include "Controller/PacketController.h"
#include "Packet/TestPayloadPacket.h"
#include "Packet/IPSyncPacket.h"
#include "Utils/NetworkUtils.h"
#include "Controller/UDPController.h"
#include "Controller/Handlers/CommandHandler.h"

class TestHandler : public VNet::EventHandler {
public:
    void
    onPacketReceiveCallback(VNet::PacketController &parent, std::shared_ptr<VNet::Packet> packet, sockaddr_in from) override {
        if(packet.get()) {
            switch (packet->getPacketID()) {
                case 0x2:
                    auto v = std::static_pointer_cast<VNet::IPSyncPacket>(packet)->getAddress();
                    for(auto ip : v) {
                        std::cout << "received incoming ip=" << inet_ntoa(ip.sin_addr) << " port=" << ip.sin_port << std::endl;
                    }
                    parent.broadcastPacket(std::make_shared<VNet::TestPayloadPacket>(),4);
                    break;
            }
        }
    }
};

int main(int args,char ** argv) {
    system("chcp 65001");
    std::cout << "Hello, World!" << std::endl;
    if(args == 1) {
        std::cerr << "usage : VNet.exe <port>" << std::endl;
        return -1;
    }
    std::string txtPort = argv[1];
    uint16_t port = std::stoi(txtPort);
    using namespace VNet;

    UDPController c(port);
    NetworkUtils::getLocalIPList();
    auto cmdHandler = std::make_shared<CommandHandler>(&c,1000);
    c.addInterval(cmdHandler);
    std::thread worker([&c]{
        c.runController();
    });
    std::string cmd;
    while(cmd != "quit") {
        std::getline(std::cin,cmd);
        try {
            cmdHandler->sendCommand(cmd);
        } catch (VNet::InvalidCommandException& i) {
            std::cerr << i.what() << std::endl;
        }
        cmd.clear();
    }
    c.signalStop();
    worker.join();

    return 0;
}
