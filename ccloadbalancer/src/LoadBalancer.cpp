#include "LoadBalancer.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

#include <thread>

#include "TcpSocket.h"

#define PORT "8080"

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

void LoadBalancer::forwardToBackend(pollfd& event, const std::string_view str)
{
    TcpSocket backend{8081};
    logInfo("Forwarding FD: " + std::to_string(backend.getFd()));
    backend.send(str);
    auto response = backend.recv();
    ::send(event.fd, response.data(), response.size(), 0);
}

void LoadBalancer::handleClient(pollfd& event)
{
    std::array<char, 1024> buf {};
    const auto clientFd = event.fd;

        int n = recv(clientFd, buf.data(), 1024, 0);
        if (n < 0) {
            logInfo("Received " + std::to_string(n) + " . There is an error");
            logInfo("errno: " + std::to_string(errno));
            // Remove client.
            event.fd = -1;
            return;
        }
        logInfo("Received " + std::to_string(n) + " amount of bytes");
        logInfo("Received msg: " + std::string { buf.data(), static_cast<size_t>(n) });
        // To avoid copies the data could be moved to a unique_ptr/shared_ptr
        std::thread t(forwardToBackend, std::ref(event), std::string { buf.data(), static_cast<size_t>(n) });
        // TODO: Now we dont care if there are any issues while forwarding/waiting for the request.
        // Not the best... Should probably not detach but try to join.
        t.detach();
}

int acceptNewClient(int listener)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int clientFd = accept(listener, (struct sockaddr*)&their_addr, &addr_size);
    return clientFd;
}

void LoadBalancer::registerFileDescriptor(int fd, short flags)
{
    fds_.emplace_back(pollfd { .fd = fd, .events = flags, .revents = 0 });
}

namespace
{
    void purgeInvalidFds(std::vector<pollfd>& fds)
    {
        
        fds.erase(std::remove_if(fds.begin(), fds.end(), [](const pollfd fdElem) {
        return fdElem.fd == -1;
    }),
        fds.end());
    }
}

void LoadBalancer::start(const std::string_view port)
{
    // servinfo now points to a linked list of 1 or more struct addrinfos
    const auto servinfo = getAddrInfo(port);
    int listener = createListener(*servinfo);
    logInfo("Listener: " + std::to_string(listener));

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
    registerFileDescriptor(listener, POLL_IN);

    std::vector<std::thread> threads;

    while (true) {
        logInfo("Waiting for poll...");
        int pollCount = poll(fds_.data(), fds_.size(), -1);
        logInfo("Polled: " + std::to_string(pollCount));
        if (pollCount == -1) {
            perror("poll");
            exit(1);
        }

        for (auto& pollFd : fds_) {
            logInfo("fd: " + std::to_string(pollFd.fd));
            logInfo(std::to_string(pollFd.revents));
            if (pollFd.revents & POLL_IN)
            {
                if (pollFd.fd == listener)
                {
                    int clientFd = acceptNewClient(listener);
                    logInfo("Client connected. Fd= " + std::to_string(clientFd));
                    registerFileDescriptor(clientFd, POLL_IN | POLL_OUT);
                }
                else
                {
                    int n = recv(pollFd.fd, nullptr, 1024, MSG_PEEK);
                    if(n == 0)
                    {
                        // client disconnected - mark as invalid
                        pollFd.fd = -1;
                        continue;
                    }
                    // TODO: join thread when we are ready to pollout?
                    handleClient(pollFd);
                }
            }
            else if(pollFd.revents & POLL_OUT)
            {
                logInfo("Got revent pollout for: " + std::to_string(pollFd.fd));
            }
            else if(pollFd.events & POLL_OUT)
            {
                logInfo("Got event pollout for: " + std::to_string(pollFd.fd));
            }
        }
        purgeInvalidFds(fds_);
    }
}

int main()
{
    LoadBalancer server {};
    server.start(PORT);
}
