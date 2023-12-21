//
// Created by maxwellzs on 2023/11/30.
//

#ifndef VNET_UDPCONTROLLER_H
#define VNET_UDPCONTROLLER_H

#include <thread>
#include <exception>
#include <winsock2.h>
#include "Packet/Packet.h"
#include "Packet/PacketBuilder.h"
#include "Controller/PacketController.h"
#include "Controller/Handlers/IPSyncHandler.h"
#include "Controller/Handlers/TextMessageHandler.h"

namespace VNet {


    class PacketListener {
    public:
        /**
         * called by the controller when the given packet is received
         * @param inPack the packet coming in
         */
        virtual void onReceive(std::shared_ptr<Packet> inPack) = 0;
    };

    class ControllerStartupException : public std::exception {
    private:
        std::string reason;
        std::string msg = "Controller init error, reason=";
    public:
        ControllerStartupException(const std::string &reason);
        const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;

    };

    class UDPController {
    private:
        void startupWSA();
        /**
         * create a socket binding to the given port
         * @return the created socket
         */
        SOCKET createListeningSocket();
        uint16_t port;
        bool shouldStop = false;

        PacketController controller;
    public:
        /**
         * tell this controller to stop
         */
        void signalStop();
        /**
         * create the controller at the given port
         * @param port the port
         */
        UDPController(uint16_t port);
        /**
         * the execute thread of the udp controller
         */
        void runController();

        /**
         * connect to a given address
         * @param addr the address
         */
        void connect(sockaddr_in addr);

        inline void addHandler(std::shared_ptr<EventHandler> handler) {
            controller.registerHandler(handler);
        }

        inline void addInterval(std::shared_ptr<IntervalEventHandler> handler) {
            controller.registerIntervalHandler(handler);
        }

        inline std::vector<sockaddr_in> listListeningAddress() {
            return controller.enumDestination();
        }
    };

};

#endif //VNET_UDPCONTROLLER_H
