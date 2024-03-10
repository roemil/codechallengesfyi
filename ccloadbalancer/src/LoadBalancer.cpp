#include "LoadBalancer.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <array>
#include <algorithm>

#include "TcpSocket.h"

#define PORT "80"

using servInfo = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;

std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> getAddrInfo(const std::string_view port)
{
    struct addrinfo hints;
    struct addrinfo* servinfo;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    int status;
    if ((status = getaddrinfo(NULL, port.data(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> servinfoPtr(servinfo, freeaddrinfo); // will point to the results
    return servinfoPtr;
}

int createListener(const addrinfo& addrInfo)
{
    int sockfd = socket(addrInfo.ai_family, addrInfo.ai_socktype, addrInfo.ai_protocol);
    // lose the pesky "Address already in use" error message
    constexpr int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    }
    return sockfd;
}

void logInfo(const std::string_view str)
{
    std::cout << "[INFO] " << str << "\n";
}

void sendData(int clientFd, const std::vector<char>& buffer)
{
    int res = send(clientFd, buffer.data(), buffer.size(), 0);
    if (res == -1) {
        perror("send");
    } else if (static_cast<size_t>(res) != buffer.size()) {
        logInfo("All bytes were not sent " + std::to_string(res));
    } else {
        logInfo("Sent " + std::to_string(res) + " amount of bytes.");
    }
}


void LoadBalancer::forwardToBackend(int clientFd, const std::string_view str)
{
    TcpSocket tcpSocket{8080};
    tcpSocket.send(str);
    auto response = tcpSocket.recv();
    ::send(clientFd, response.data(), response.size(), 0);
}

ClientState LoadBalancer::handleClient(const int clientFd)
{
    std::array<char, 1024> buf {};
    while (true) {
        int n = recv(clientFd, buf.data(), 1024, 0);
        if (n == 0) {
            // client closed the connection
            logInfo("Client disconnected");
            return ClientState::Disconnected;
        }
        if (n < 0) {
            logInfo("Received " + std::to_string(n) + " . There is an error");
            logInfo("errno: " + std::to_string(errno));
            // Remove client.
            return ClientState::Disconnected;
        }
        logInfo("Received " + std::to_string(n) + " amount of bytes");
        logInfo("Received msg: " + std::string { buf.data(), static_cast<size_t>(n) });
        forwardToBackend(clientFd, std::string_view { buf.data(), static_cast<size_t>(n) });
        return ClientState::Connected;
    }
}

int acceptNewClient(int listener)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int clientFd = accept(listener, (struct sockaddr*)&their_addr, &addr_size);
    return clientFd;
}

void LoadBalancer::start(const std::string_view port)
{
    // servinfo now points to a linked list of 1 or more struct addrinfos
    const auto servinfo = getAddrInfo(port);
    int listener = createListener(*servinfo);

    int bindResult = bind(listener, servinfo->ai_addr, servinfo->ai_addrlen);
    if (bindResult == -1) {
        perror("bind");
        exit(1);
    }

    constexpr int maxClients = 50;
    int err = listen(listener, maxClients);
    if (err != 0) {
        perror("listen");
        exit(1);
    }

    fds_.reserve(maxClients);
    fds_.emplace_back(pollfd { .fd = listener, .events = POLL_IN, .revents = 0 });

    while (true) {
        logInfo("Waiting for poll...");
        int pollCount = poll(fds_.data(), fds_.size(), -1);
        logInfo("Polled: " + std::to_string(pollCount));
        if (pollCount == -1) {
            perror("poll");
            exit(1);
        }

        for (const auto pollFd : fds_) {
            if (pollFd.revents & POLL_IN) {
                if (pollFd.fd == listener) {
                    logInfo("Client connected. Fd= " + std::to_string(pollFd.fd));
                    int clientFd = acceptNewClient(listener);
                    fds_.emplace_back(pollfd { .fd = clientFd, .events = POLL_IN, .revents = 0 });
                } else {
                    const auto state = handleClient(pollFd.fd);
                    if (state == ClientState::Disconnected) {
                        auto fdToRemove = pollFd.fd;
                        fds_.erase(std::remove_if(fds_.begin(), fds_.end(), [fdToRemove](pollfd fdElem) {
                            return fdElem.fd == fdToRemove;
                        }),
                            fds_.end());
                    }
                }
            }
        }
    }
}

int main()
{
    LoadBalancer server {};
    server.start(PORT);
}
