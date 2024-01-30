#include "Server.h"

#include "RespHandler.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

//#define PORT "7676"
#define PORT "6379"

using servInfo = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;

std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> getAddrInfo()
{
    struct addrinfo hints;
    struct addrinfo* servinfo;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    int status;
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> servinfoPtr(servinfo, freeaddrinfo); // will point to the results
    return servinfoPtr;
}

int getSocket(const addrinfo& addrI)
{
    int sockfd = socket(addrI.ai_family, addrI.ai_socktype, addrI.ai_protocol);
    // lose the pesky "Address already in use" error message
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    }
    return sockfd;
}

void sendData(int clientFd, std::vector<char> buffer)
{
    int res = send(clientFd, buffer.data(), buffer.size(), 0);
    if (res == -1) {
        perror("send");
    } else if(static_cast<size_t>(res) != buffer.size())
    {
        std::cout << "[INFO] Sent not all bytes " + std::to_string(res) + " \n";
    } else {
        std::cout << "[INFO] Sent " + std::to_string(res) + " amount of bytes.\n";
    }
}

void handleInput(int clientFd, const std::string_view str)
{
    RespHandler rh {};
    // TODO handle client handshake
    try {
        const auto cmd = rh.decode(str);
        std::cout << "[INFO] CMD: " << cmd.second << "\n";
        std::cout << "[INFO] CMD: " << cmd.second.simpleString_ << "\n";
        if (cmd.second.simpleString_ == "PING") {
            rh.appendBulkstring("PONG");
            sendData(clientFd, rh.getBuffer());
        }else if (cmd.second.array_.size() > 0 && cmd.second.array_[0].bulkString_ == "PING") {
            rh.appendBulkstring("PONG");
            sendData(clientFd, rh.getBuffer());
        } else if (cmd.second.array_[0].bulkString_ == "HELLO" && cmd.second.array_[1].bulkString_ == "3") {
            rh.beginMap(3);
            rh.appendKV("server", "redis");
            rh.appendKV("version", "0.0.1");
            rh.appendKV("proto", 3);
            // rh.appendKV("id", 1);
            // rh.appendKV("mode", "standalone");
            // rh.appendKV("role", "master");
            // rh.appendSimpleString("modules");
            // rh.beginArray(0);
            // rh.beginArray(14);
            // rh.appendSimpleString("server");
            // rh.appendSimpleString("redis");
            // rh.appendSimpleString("version");
            // rh.appendSimpleString("0.0.1");
            // rh.appendSimpleString("proto");
            // rh.appendInt("3");
            // rh.appendSimpleString("id");
            // rh.appendInt("1");
            // rh.appendSimpleString("mode");
            // rh.appendSimpleString("standalone");
            // rh.appendSimpleString("role");
            // rh.appendSimpleString("master");
            // rh.appendSimpleString("modules");
            // rh.beginArray(0);
            std::cout << "[INFO]: sent: ";
            for(const auto& elem: rh.getBuffer()){
                std::cout << static_cast<char>(+elem);
            }
            std::cout << "\n";
            sendData(clientFd, rh.getBuffer());
        } else {
            rh.appendSimpleString("OK");
            sendData(clientFd, rh.getBuffer());
            std::cout << "[INFO] Unknown command: " << cmd.second.simpleString_ << "\n";
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "[INFO] Invalid argument: " << e.what() << "\n";
    }
}

void handleClient(const int clientFd){
    std::array<char, 1024> buf;
    while (true) {
        int n = recv(clientFd, buf.data(), 1024, 0);
        for (int i = 0; i < n; i++) {
            std::cout << std::setfill('0') << std::setw(2) << std::hex << +static_cast<char>(buf[i]) << " ";
        }
        std::cout << "\n";
        if (n == 0) {
            // client closed the connection
            std::cout << "Client disconnected\n";
            return;
        }
        std::cout << "[INFO] Received " + std::to_string(n) + " amount of bytes.\n";
        std::cout << "[INFO] Received msg: " + std::string { buf.data(), static_cast<size_t>(n) };
        handleInput(clientFd, std::string_view { buf.data(), static_cast<size_t>(n) });
    }
}

int main()
{
    // servinfo now points to a linked list of 1 or more struct addrinfos
    const auto servinfo = getAddrInfo();
    int sockfd = getSocket(*servinfo);

    int bindResult = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (bindResult == -1) {
        perror("bind");
        exit(1);
    }

    int err = listen(sockfd, 20);
    if (err != 0) {
        perror("listen");
        exit(1);
    }
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    // TODO: Handle multiple clients
    while (true) {
    
    int clientFd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
        std::cout << "[INFO] Client connected\n";
        handleClient(clientFd);
    }
   
}