#include "NtpClient.h"

#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#define PORT 123
#define MAXLINE 1024
#define NTP_TIMESTAMP_DELTA 2208988800ull

NtpClient::NtpClient()
{
     // create a socket
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation error...\n");
        exit(-1);
    }

    // server socket address
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(PORT);
    serAddr.sin_addr.s_addr = inet_addr("162.159.200.1");   
}

void NtpClient::print()
{
    const auto todayInSec = (time_t)(ntohl(ntpPacket.transmitedTimestampSec) - NTP_TIMESTAMP_DELTA);
    printf("Time: %s", ctime((const time_t *)&todayInSec));
}

void NtpClient::getTime()
{
    ntpPacket.li_vn_mode = 0x1b;
    if (sendto(clientSocket, (char *)&ntpPacket, sizeof(NtpPacket), 0, (struct sockaddr *)&serAddr, sizeof(serAddr)) < 0)
    {
        perror("sending error...\n");
        close(clientSocket);
        exit(-1);
    }

    socklen_t serAddrLen = sizeof(serAddr);
    int readStatus = recvfrom(clientSocket, &ntpPacket, sizeof(NtpPacket), 0, (struct sockaddr *)&serAddr, &serAddrLen);
    if (readStatus < 0)
    {
        perror("reading error...\n");
        close(clientSocket);
        exit(-1);
    }
}

int main()
{
    int cliSockDes, readStatus;

    NtpClient ntp{};
    ntp.getTime();
    ntp.print();

    close(cliSockDes);

    return 0;
}