cmake_minimum_required(VERSION 3.24)
project(VNet)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

add_library(VNetBase SHARED
        Utils/DataBuffer.cpp
        Utils/DataBuffer.h
        Packet/Packet.h
        Controller/UDPController.cpp
        Controller/UDPController.h
        Packet/PacketBuilder.cpp
        Packet/PacketBuilder.h
        Controller/PacketController.cpp
        Controller/PacketController.h
        Packet/TestPayloadPacket.cpp
        Packet/TestPayloadPacket.h
        Packet/IPSyncPacket.cpp
        Packet/IPSyncPacket.h
        Packet/TextMessagePacket.cpp
        Packet/TextMessagePacket.h
        Controller/Handlers/IPSyncHandler.cpp
        Controller/Handlers/IPSyncHandler.h
        Utils/NetworkUtils.cpp
        Utils/NetworkUtils.h
        Controller/Handlers/CommandHandler.cpp
        Controller/Handlers/CommandHandler.h
        Controller/Handlers/TextMessageHandler.cpp
        Controller/Handlers/TextMessageHandler.h)
add_executable(VNet main.cpp)
target_link_libraries(VNetBase ws2_32)
target_link_libraries(VNet VNetBase)
