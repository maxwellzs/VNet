//
// Created by maxwellzs on 2023/12/8.
//

#include <iostream>
#include "NetworkUtils.h"

std::string VNet::NetworkUtils::generateUUID() {
    std::stringstream stm;
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 engine(seed);
    std::uniform_int_distribution<size_t> gen;
    stm << std::hex << gen(engine);
    return stm.str();
}

std::vector<uint32_t> VNet::NetworkUtils::getLocalIPList() {
    WSADATA wsaData;
    WSAStartup(WINSOCK_VERSION, &wsaData);
    std::vector<uint32_t> local;
    char name[256];
    PHOSTENT info;
    if (gethostname(name, sizeof(name)) == 0) {
        if ((info = gethostbyname(name)) != nullptr) {
            while(*(info->h_addr_list)!=nullptr) {
                in_addr addr = (*(struct in_addr *) *info->h_addr_list);
                local.push_back(addr.S_un.S_addr);
                info->h_addr_list ++;
            }
        }
    }
    return local;
}
