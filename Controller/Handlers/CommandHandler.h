//
// Created by maxwellzs on 2023/12/8.
//

#ifndef VNET_COMMANDHANDLER_H
#define VNET_COMMANDHANDLER_H

#include <mutex>
#include <winsock2.h>
#include "Controller/UDPController.h"
#include "Controller/PacketController.h"
#include "Utils/NetworkUtils.h"

namespace VNet {


    class InvalidCommandException : public std::exception {
    private:
        std::string msg;
    public:
        InvalidCommandException(const std::string &input, const std::string &example);

        const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    };

    class CommandHandler : public IntervalEventHandler {
    private:
        std::mutex mtx;

        std::string nickName = "";
        std::string uuidString;
        std::vector<std::string> messageList;
        bool quit = false;
        sockaddr_in addr;
        bool tryConnect = false;
        UDPController * controller;

        std::vector<std::string> commandToWords(std::string &cmd);

    public:
        // process command every 1s
        CommandHandler(UDPController *parent,clock_t interval = 1000);

        void sendCommand(std::string &cmd);

        void onInterval(PacketController &parent) override;
    };

};

#endif //VNET_COMMANDHANDLER_H
