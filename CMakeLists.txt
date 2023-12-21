cmake_minimum_required(VERSION 3.24)
project(VNet)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://mirror.ghproxy.com/https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
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
        Controller/Handlers/TextMessageHandler.h
)
add_executable(VNet main.cpp)
target_link_libraries(VNetBase ws2_32)
target_link_libraries(VNet VNetBase)
add_executable(VNetTest Test/test.cpp)
target_link_libraries(VNetTest ws2_32)
target_link_libraries(VNetTest VNetBase)
target_link_libraries(VNetTest gtest_main)

include(GoogleTest)
gtest_discover_tests(VNetTest)