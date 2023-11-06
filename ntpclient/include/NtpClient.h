#pragma once

#include <cstdint>
#include <vector>

#include <arpa/inet.h>

struct NtpClient
{
    NtpClient();
    struct NtpPacket
    {
        uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
                            // li.   Two bits.   Leap indicator.
                            // vn.   Three bits. Version number of the protocol.
                            // mode. Three bits. Client will pick mode 3 for client.

        uint8_t stratum;
        uint8_t poll;
        uint8_t precision;
        uint32_t rootDelay;
        uint32_t rootDispersion;
        uint32_t refId;
        uint32_t refTimestampSec;
        uint32_t refTimestampSecFrac;
        uint32_t origTimestampSec;
        uint32_t origTimestampSecFrac;
        uint64_t receivedTimestampSec;
        uint64_t transmitedTimestampSec;
    };

    NtpPacket ntpPacket{};
    int clientSocket{};
    sockaddr_in serAddr{};

    void getTime();
    void print();
};