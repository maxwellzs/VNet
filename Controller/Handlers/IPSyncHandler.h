//
// Created by maxwellzs on 2023/12/7.
//

#pragma once

#ifndef VNET_IPSYNCHANDLER_H
#define VNET_IPSYNCHANDLER_H

#include "Controller/PacketController.h"
#include "Packet/IPSyncPacket.h"

namespace VNet {

    class IPSyncHandler : public EventHandler {
    public:
        void
        onPacketReceiveCallback(PacketController &parent, std::shared_ptr<Packet> packet, sockaddr_in from) override;

    };

    class IntervalSyncHandler : public IntervalEventHandler {
    public:
        IntervalSyncHandler(clock_t interval = 10000);

        void onInterval(PacketController &parent) override;
    };

};

#endif //VNET_IPSYNCHANDLER_H
