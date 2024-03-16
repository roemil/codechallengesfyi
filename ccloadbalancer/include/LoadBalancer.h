#pragma once

#include <string_view>
#include <vector>

struct pollfd;

enum class ClientState {
    Connected,
    Disconnected
};

class LoadBalancer {
public:
    void start(const std::string_view port);

    static void forwardToBackend(pollfd& clientFd, const std::string_view str);
    static void handleClient(pollfd& clientFd);
    void registerFileDescriptor(int fd, short flags);

private:
    std::vector<pollfd> fds_;
};