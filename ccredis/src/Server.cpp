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
#include <variant>
#include <vector>

// #define PORT "7676"
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
    } else if (static_cast<size_t>(res) != buffer.size()) {
        std::cout << "[INFO] Sent not all bytes " + std::to_string(res) + " \n";
    } else {
        std::cout << "[INFO] Sent " + std::to_string(res) + " amount of bytes.\n";
    }
}

/*
TODO:
Handle Command function
Build a command queue (for arrays)
Build a response and send that out
*/

void HandleCommand::operator()(const CommandUnknown&) const{
    rh_.appendError("Unknown command. Prefix did not match any expected prefixes");
}
void HandleCommand::operator()(const CommandInvalid&) const{
    rh_.appendError("Invalid Command");
}
void HandleCommand::operator()(const CommandPing&) const{
    rh_.appendBulkstring("PONG");
}
void HandleCommand::operator()(const CommandHello& cmd) const{
    if(cmd.version_ != "3"){
        std::string error {"Version not supported: "};
        rh_.appendError(error + cmd.version_.data());
        return;
    }
    else if(cmd.version_.empty()){
        rh_.appendError("Missing version");
        return;
    }
    rh_.beginMap(3);
    rh_.appendKV("server", "redis");
    rh_.appendKV("version", "0.0.1");
    rh_.appendKV("proto", 3);
}
void HandleCommand::operator()(const CommandSet&) const{
    rh_.appendError("Unsupported command");
}
void HandleCommand::operator()(const CommandGet&) const{
    rh_.appendError("Unsupported command");
}

void handleInput(int clientFd, const std::string_view str)
{
    RespHandler rh {};
    try {
        const auto rawCmd = rh.decode(str);
        const auto cmds = rh.convertToCommands(rawCmd.second);
        // for(const auto& cmd : cmds)
        // {
        //     std::cout << "[INFO]: Cmd: " << cmd << "\n";
        // }
        for (const auto& cmd : cmds) {
            std::visit(HandleCommand{rh}, cmd);
        }
        sendData(clientFd, rh.getBuffer());
    } catch (const std::invalid_argument& e) {
        std::cout << "[INFO] Invalid argument: " << e.what() << "\n";
    }
}

void handleClient(const int clientFd)
{
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