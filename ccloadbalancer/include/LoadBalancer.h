#pragma once

#include <future>
#include <map>
#include <mutex>
#include <optional>
#include <poll.h>
#include <string_view>
#include <thread>
#include <vector>
#include <expected>

struct Client {
    Client() = default;
    // TODO: Read/Write locks perhaps
    mutable std::mutex mutex {};
    pollfd pollFd {};

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

enum class ForwardResult {
    Success,
    Failure
};

class LoadBalancer {
public:
    LoadBalancer();
    ~LoadBalancer();
    void start(const std::string_view port);

    static ForwardResult forwardToBackend(Client& client, std::string data, int port);
    std::optional<std::future<ForwardResult>> handleClient(Client& client);
    void registerFileDescriptor(int fd, short flags);

    void addBackend(int port);

private:
    std::expected<int, std::string_view> getNextPort();
    bool isReady_ = false;
    int getNextPortIndex();
    void checkAllBackends();
    void startHealthChecker();
    std::thread healthCheckerThread;

    std::map<int, Client> clients_ {};
    std::vector<pollfd> fds_ {};

    int numForwards {};
    std::mutex beMutex {};
    std::vector<std::pair<int, bool>> backendServers {};
};