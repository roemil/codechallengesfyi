#include "Server.h"

#include "CommandHandler.h"
#include "Resp.h"
#include "RespDecoder.h"
#include "RespEncoder.h"
#include "Database.h"

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
#include <string>
#include <string_view>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <variant>
#include <vector>
#include <memory>
#include <poll.h>

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

void handleInput(int clientFd, const std::string_view str, std::shared_ptr<Db>& db)
{
    RespDecoder rd {};
    RespEncoder re {db};
    try {
        const auto rawCmd = rd.decode(str);
        const auto cmds = rd.convertToCommands(rawCmd.second);
        for (const auto& cmd : cmds) {
            std::visit(re, cmd);
        }
        sendData(clientFd, re.getBuffer());
    } catch (const std::invalid_argument& e) {
        std::cout << "[INFO] Invalid argument: " << e.what() << "\n";
    }
}

enum class ClientState{
    Disconnected,
    Connected
};

void logInfo(const std::string_view str)
{
    std::cout << "[INFO] " << str << "\n";
}
ClientState handleClient(const int clientFd, std::shared_ptr<Db>& db)
{
    std::array<char, 1024> buf;
    while (true) {
        int n = recv(clientFd, buf.data(), 1024, 0);
        if (n == 0) {
            // client closed the connection
            logInfo("Client disconnected");
            return ClientState::Disconnected;
        }
        logInfo("Received " + std::to_string(n) + " amount of bytes");
        std::cout << "[INFO] Received msg: " + std::string { buf.data(), static_cast<size_t>(n) };
        handleInput(clientFd, std::string_view { buf.data(), static_cast<size_t>(n) }, db);
        return ClientState::Connected;
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
    auto db = std::make_shared<Db>();

    std::vector<pollfd> fdArray{};
    fdArray.reserve(5);
    fdArray.emplace_back(pollfd{.fd = sockfd, .events = POLL_IN});

    while (true) {
        logInfo("Waiting for poll...");
        int pollCount = poll(fdArray.data(), fdArray.size(), -1);
        logInfo("Polled: " + std::to_string(pollCount));
        if (pollCount == -1){
            perror("poll");
            exit(1);
        }

        for(const auto pollFd : fdArray){
            if(pollFd.revents & POLL_IN){
                if(pollFd.fd == sockfd){
                    logInfo("Client connected. Fd= " + std::to_string(pollFd.fd));
                    struct sockaddr_storage their_addr;
                    socklen_t addr_size = sizeof their_addr;
                    int clientFd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
                    fdArray.emplace_back(pollfd{.fd = clientFd, .events = POLL_IN});
                }
                else {
                    const auto state = handleClient(pollFd.fd, db);
                    if (state == ClientState::Disconnected){
                        auto fdToRemove = pollFd.fd;
                        fdArray.erase(std::remove_if(fdArray.begin(), fdArray.end(), [fdToRemove](pollfd fdElem){
                            return fdElem.fd == fdToRemove;
                        }), fdArray.end());
                    }
                }
            }
        }

    }
}