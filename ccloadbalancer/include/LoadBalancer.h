#pragma once

#include <map>
#include <mutex>
#include <poll.h>
#include <string_view>
#include <vector>

struct Client {
    Client() = default;
    // TODO: Read/Write locks perhaps
    mutable std::mutex mutex;
    pollfd pollFd;

    Client(const Client& other)
    {
        pollFd = other.pollFd;
        std::lock(mutex, other.mutex);
        std::lock_guard<std::mutex> lhs_lk(mutex, std::adopt_lock);
        std::lock_guard<std::mutex> rhs_lk(other.mutex, std::adopt_lock);
    }

    Client& operator=(const Client& other)
    {
        if (this != &other) {
            pollFd = other.pollFd;
            std::lock(mutex, other.mutex);
            std::lock_guard<std::mutex> lhs_lk(mutex, std::adopt_lock);
            std::lock_guard<std::mutex> rhs_lk(other.mutex, std::adopt_lock);
        }

        return *this;
    }
};

class LoadBalancer {
public:
    void start(const std::string_view port);

    static void forwardToBackend(Client& client, const std::string_view data);
    static void handleClient(Client& client);
    void registerFileDescriptor(int fd, short flags);

private:
    std::map<int, Client> clients_ {};
    std::vector<pollfd> fds_;
};