//
// Created by maxwellzs on 2023/12/8.
//

#include <sstream>
#include "CommandHandler.h"
#include "Packet/TextMessagePacket.h"

void VNet::CommandHandler::sendCommand(std::string &cmd) {
    std::unique_lock<std::mutex> lock(mtx);
    auto wordList = commandToWords(cmd);
    for(auto i = wordList.begin();i != wordList.end();i ++) {

        if(*i == "quit") {
            // user quit
            return;
        } else if(*i == "connect") {
            // user try to connect to other clients
            if(wordList.size() != 3) {
                throw InvalidCommandException(cmd, "connect <ip> <port>");
            }
            std::string textIP = *(++i);
            std::string textPort = *(++i);
            auto ip = inet_addr(textIP.c_str());

            if(ip == INADDR_ANY) {
                throw InvalidCommandException(cmd, "connect <ip> <port>");
            }

            addr.sin_addr.S_un.S_addr = ip;
            addr.sin_family = AF_INET;
            try {
                addr.sin_port = htons(std::stoi(textPort));
            } catch (std::invalid_argument& i) {
                throw InvalidCommandException(cmd, "connect <ip> <port>");
            }

            tryConnect = true;
            break;
        } else if(*i == "msg") {

            // user input messages
            if(wordList.size() == 1) {
                throw InvalidCommandException(cmd, "msg <message strings>");
            }
            i++;
            std::string all;
            for(;i!=wordList.end();i++) {
                all += *i;
                all += " ";
            }
            messageList.push_back(all);
            break;
        } else if(*i == "nickname") {
            if(wordList.size() == 1) {
                throw InvalidCommandException(cmd, "nickname <new nickname>");
            }
            nickName = *(++i);
        }else if(*i == "enum") {
            auto list = controller->listListeningAddress();
            for(auto each : list) {
                std::cout << "listening to : " << inet_ntoa(each.sin_addr) << ":" << ntohs(each.sin_port) << std::endl;
            }
        } else {
            throw InvalidCommandException(cmd, "");
        }
    }
}

void VNet::CommandHandler::onInterval(VNet::PacketController &parent) {
    if(tryConnect) {
        std::cout << "[!] added destination : " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << std::endl;
        parent.registerBroadcastDestination(addr, true);
        std::shared_ptr<TextMessagePacket> connectionInfo = std::make_shared<TextMessagePacket>();
        connectionInfo->setUUIDString(uuidString);
        connectionInfo->setNickName("SYSTEM");
        connectionInfo->setMessageBody("a client has connected to you");
        parent.broadcastPacket(connectionInfo);
        tryConnect = false;
    }
    std::unique_lock<std::mutex> lock(mtx);
    if(!messageList.empty()) {
        // has messages
        for(auto each = messageList.begin();each != messageList.end();each ++) {
            std::shared_ptr<TextMessagePacket> pack = std::make_shared<TextMessagePacket>();
            if(!nickName.empty()) pack->setNickName(nickName);
            pack->setUUIDString(uuidString);
            pack->setMessageBody(*each);
            parent.broadcastPacket(pack,2);
        }
        messageList.clear();
    }
}

VNet::CommandHandler::CommandHandler(UDPController *parent, clock_t interval) : IntervalEventHandler(interval) {
    this->controller = parent;
    uuidString = NetworkUtils::generateUUID();
    resetTimer();
}

std::vector<std::string> VNet::CommandHandler::commandToWords(std::string &cmd) {
    std::istringstream i(cmd);
    std::string word;
    std::vector<std::string> wordList;
    while(std::getline(i,word,' ')) {
        wordList.push_back(word);
    }
    return wordList;
}

VNet::InvalidCommandException::InvalidCommandException(const std::string &input, const std::string &example) {
    msg = "malformed command \"" + input + "\", usage : " + example;
}

const char *VNet::InvalidCommandException::what() const noexcept {
    return msg.c_str();
}
