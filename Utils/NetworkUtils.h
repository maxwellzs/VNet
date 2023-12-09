//
// Created by maxwellzs on 2023/12/8.
//

#ifndef VNET_NETWORKUTILS_H
#define VNET_NETWORKUTILS_H

#include <string>
#include <sstream>
#include <chrono>
#include <random>
#include <winsock2.h>

namespace VNet {

    class NetworkUtils {
    public:
        NetworkUtils() = delete;

        static std::string generateUUID();

        static std::vector<uint32_t> getLocalIPList();
    };

};

#endif //VNET_NETWORKUTILS_H
