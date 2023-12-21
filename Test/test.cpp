//
// Created by maxwellzs on 2023/12/14.
//

#include <gtest/gtest.h>
#include <winsock2.h>

__attribute__((constructor()))
static void startupWsa() {
    WSAData dat;
    WSAStartup(WINSOCK_VERSION,&dat);
}

int main(int args,char ** argv) {
    testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}

