//
// Created by maxwellzs on 2023/12/7.
//

#include "IPSyncHandler.h"

void VNet::IPSyncHandler::onPacketReceiveCallback(VNet::PacketController &parent, std::shared_ptr<Packet> packet,
                                                  sockaddr_in from) {
    if(packet.get() && packet->getPacketID() == 0x2) {
        // ip sync pack
        auto vec = std::static_pointer_cast<IPSyncPacket>(packet)->getAddress();
        for(auto i : vec) {
            parent.registerBroadcastDestination(i, true);
        }
    }
}

// synchronize every 10 seconds
VNet::IntervalSyncHandler::IntervalSyncHandler(clock_t interval) : IntervalEventHandler(interval) {
}

void VNet::IntervalSyncHandler::onInterval(VNet::PacketController &parent) {
    auto allDestinations = parent.enumDestination();
    for(auto each : allDestinations) {
        std::shared_ptr<IPSyncPacket> out = std::make_shared<IPSyncPacket>();
        out->putAddress(each);
        parent.broadcastPacket(out);
    }
    // every sync packet only have 3 retry
}
