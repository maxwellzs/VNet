//
// Created by maxwellzs on 2023/11/30.
//

#include <iostream>
#include "UDPController.h"


void VNet::UDPController::runController() {
    SOCKET lSock = createListeningSocket();

    u_long on = 1;
    ioctlsocket(lSock,FIONBIO,&on);

    controller.setEchoSocket(lSock);
    controller.registerHandler(std::make_shared<IPSyncHandler>());
    controller.registerHandler(std::make_shared<TextMessageHandler>());
    controller.registerIntervalHandler(std::make_shared<IntervalSyncHandler>(5000));

    while(!shouldStop) {
        // main loop
        controller.flushSendBox(lSock);
        try {
            controller.pullInterval();
        } catch (std::exception& e) {
            std::cerr << "internal error in handlers : " << e.what() << std::endl;
        }

        std::shared_ptr<DataBuffer> inBuf = std::make_shared<DataBuffer>(1024);
        sockaddr_in inAddr;

        try {
            inAddr = inBuf->write(lSock,1024);
        } catch(BufferReceiveException& e) {
            // no current data
            if(e.getWsaError() == WSAEWOULDBLOCK || e.getWsaError() == WSAECONNRESET) {

                // yield time slice
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            std::cerr << "[!] IO exception occurred : " << e.what() << std::endl;
            continue;
        }

        try {
            controller.handleData(inBuf,inAddr);
        } catch(PacketHandleException& e) {
            std::cerr << "[!] packet failed to be handled ! " << e.what() << std::endl;
        }

    }

    closesocket(lSock);
}

void VNet::UDPController::startupWSA() {
    WSADATA dat;
    if(WSAStartup(WINSOCK_VERSION,&dat) != 0) {
        throw ControllerStartupException("WSAStartup failed");
    }
}

SOCKET VNet::UDPController::createListeningSocket() {
    SOCKET s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(SOCKET_ERROR == s) {
        throw ControllerStartupException("create socket failed");
    }

    sockaddr_in localAddress;
    localAddress.sin_port = htons(port);
    localAddress.sin_addr.S_un.S_addr = INADDR_ANY;
    localAddress.sin_family = AF_INET;

    if(bind(s,(sockaddr *)&localAddress,sizeof(localAddress)) != 0) {
        throw ControllerStartupException("bind socket failed");
    }
    return s;
}

VNet::UDPController::UDPController(uint16_t port) : controller(port) {
    startupWSA();
    this->port = port;
}

void VNet::UDPController::signalStop() {
    shouldStop = true;
}

void VNet::UDPController::connect(sockaddr_in addr) {
    controller.registerBroadcastDestination(addr,true);
}

const char *VNet::ControllerStartupException::what() const noexcept {
    return msg.c_str();
}

VNet::ControllerStartupException::ControllerStartupException(const std::string &reason) {
    this->reason = reason;
    this->msg = "cannot initialize the controller,reason=" + reason + ",wsa error=" + std::to_string(WSAGetLastError());
}
