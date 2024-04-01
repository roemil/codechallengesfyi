#include "LoadBalancer.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <netdb.h>
#include <optional>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/poll.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <expected>

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
    std::cout << "[INFO LB] " << str << "\n";
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

LoadBalancer::LoadBalancer()
    : healthCheckerThread { [this]() { this->startHealthChecker(); } }
{
}

LoadBalancer::~LoadBalancer()
{
    healthCheckerThread.join();
}

// TODO: Error handling

int LoadBalancer::getNextPortIndex()
{
    std::lock_guard<std::mutex> lock { beMutex };
    int nextPortIndex = 0;
    std::string nextString { std::to_string(nextPortIndex) };
    logInfo("Next port " + nextString);
    nextPortIndex = numForwards % backendServers.size();
    ++numForwards;
    return nextPortIndex;
}

std::expected<int, std::string_view> LoadBalancer::getNextPort()
{
    int nextPortIndex = 0;
    int maxTries = 100;
    while(!backendServers[nextPortIndex].second && maxTries > 0)
    {
        --maxTries;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        nextPortIndex = getNextPortIndex();
    }

    if(maxTries <= 0)
    {
        return std::unexpected{"No backend available"};
    }

    // TODO: Fix implicit copy
    return backendServers[nextPortIndex].first;
}

ForwardResult LoadBalancer::forwardToBackend(Client& client, const std::string data, int port)
{
    std::lock_guard<std::mutex> lock(client.mutex);
    int maxRetries = 3;
    while (true) {
        try {
            TcpSocket backend { port };

            logInfo("Forwarding FD: " + std::to_string(backend.getFd()));
            auto sendRes = backend.send(data);
            if (sendRes < 0) {
                client.pollFd.fd = -1;
                return ForwardResult::Failure;
            }
            auto response = backend.recv();
            auto sendBackToClientRes = ::send(client.pollFd.fd, response.data(), response.size(), 0);
            if (sendBackToClientRes < 0) {
                client.pollFd.fd = -1;
                return ForwardResult::Failure;
            }
            return ForwardResult::Success;

        } catch (const std::invalid_argument& e) {
            logInfo(e.what());
            maxRetries--;
            if (maxRetries == 0) {
                client.pollFd.fd = -1;
                return ForwardResult::Failure;
            }
        }
    }
    // Unreachable code
    return ForwardResult::Failure;
}

// TODO: std::expected perhaps
std::optional<std::future<ForwardResult>> LoadBalancer::handleClient(Client& client)
{
    std::array<char, 1024> buf {};
    std::lock_guard<std::mutex> lock(client.mutex);
    const auto clientFd = client.pollFd.fd;

    int n = recv(clientFd, buf.data(), 1024, 0);
    if (n < 0) {
        logInfo("Received " + std::to_string(n) + " . There is an error");
        logInfo("errno: " + std::to_string(errno));
        // Remove client.
        client.pollFd.fd = -1;
        return std::nullopt;
    }
    logInfo("Received " + std::to_string(n) + " amount of bytes");
    logInfo("Received msg: " + std::string { buf.data(), static_cast<size_t>(n) });

    const auto nextPort = getNextPort();

    if(!nextPort.has_value())
    {
        logInfo("Failed to get next port: " + std::string{nextPort.error()});
        // Remove client.
        client.pollFd.fd = -1;
        return std::nullopt;
    }

    // To avoid copies the data could be moved to a unique_ptr/shared_ptr
    return std::async(forwardToBackend, std::ref(client), std::string { buf.data(), static_cast<size_t>(n) }, nextPort.value());
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
    Client client {};
    client.pollFd = pollfd { .fd = fd, .events = flags, .revents = 0 };
    clients_[fd] = client;
    fds_.push_back(client.pollFd);
}

namespace {
void purgeInvalidFds(std::vector<pollfd>& clients)
{

    clients.erase(std::remove_if(clients.begin(), clients.end(), [](const pollfd client) {
        return client.fd == -1;
    }),
        clients.end());
}
void purgeInvalidClients(std::map<int, Client>& clients)
{

    auto itr = clients.begin();
    while (itr != clients.end()) {
        if ((*itr).second.pollFd.fd == -1) {
            itr = clients.erase(itr);
        } else {
            itr++;
        }
    }
}

void markFdsForRemoval(std::vector<pollfd>& fds, const std::map<int, Client>& clients)
{
    for (auto& [fd, client] : clients) {
        std::lock_guard<std::mutex> lock(client.mutex);
        if (client.pollFd.fd == -1) {
            auto iter = std::find_if(fds.begin(), fds.end(), [fd](pollfd pfd) {
                return fd == pfd.fd;
            });
            if (iter == fds.end()) {
                continue;
            }
            iter->fd = -1;
        }
    }
}
}

void LoadBalancer::checkAllBackends()
{
    std::lock_guard<std::mutex> lock { beMutex };
    for (auto& port : backendServers) {
        // Try all ports, if port succeeds we mark it as alive,
        // otherwise we mark port as dead.
        try {
            TcpSocket socket { port.first };
            port.second = true;
        } catch (const std::invalid_argument& e) {
            logInfo("Server " + std::to_string(port.first) + " is down. Error: " + e.what());
            port.second = false;
        }
    }
}

void LoadBalancer::startHealthChecker()
{
    while (true) {
        logInfo("Checking backends");
        checkAllBackends();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10000ms);
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

    registerFileDescriptor(listener, POLL_IN);

    while (true) {
        logInfo("Waiting for poll...");
        int pollCount = ::poll(fds_.data(), fds_.size(), -1);
        logInfo("Polled: " + std::to_string(pollCount));
        if (pollCount == -1) {
            perror("poll");
            exit(1);
        }

        std::vector<std::future<ForwardResult>> futureResults;
        std::vector<int> fdsToRegister {};

        for (const auto& pollFd : fds_) {
            if (pollFd.revents & POLL_IN) {
                if (pollFd.fd == listener) {
                    auto fd = fdsToRegister.emplace_back(acceptNewClient(listener));
                    logInfo("Client connected. Fd= " + std::to_string(fd));
                } else {
                    logInfo("Got pollin for fd=" + std::to_string(pollFd.fd));
                    int n = recv(pollFd.fd, nullptr, 1024, MSG_PEEK);
                    if (n == 0) {
                        // client disconnected - mark as invalid
                        std::lock_guard<std::mutex> lock(clients_.at(pollFd.fd).mutex);
                        clients_.at(pollFd.fd).pollFd.fd = -1;
                        continue;
                    }
                    auto res = handleClient(clients_.at(pollFd.fd));
                    if (res) {
                        futureResults.push_back(std::move(res.value()));
                    } else {
                        close(pollFd.fd);
                    }
                }
            } else if (pollFd.revents & POLL_OUT) {
                logInfo("Got revent pollout for: " + std::to_string(pollFd.fd));
            } else if (pollFd.events & POLL_OUT) {
                logInfo("Got event pollout for: " + std::to_string(pollFd.fd));
            }
        }

        for (const auto fd : fdsToRegister) {
            registerFileDescriptor(fd, POLL_IN);
        }

        for (auto& fut : futureResults) {
            if (!fut.valid()) {
                fut.wait();
            }
            auto res = fut.get();
            if (res == ForwardResult::Failure) {
                // TODO: Close client connection here
                logInfo("Failed to forward request");
            }
        }

        markFdsForRemoval(fds_, clients_);
        purgeInvalidFds(fds_);
        purgeInvalidClients(clients_);
    }
}

void LoadBalancer::addBackend(int port)
{
    // Assume port is alive as default
    backendServers.emplace_back(port, true);
}
