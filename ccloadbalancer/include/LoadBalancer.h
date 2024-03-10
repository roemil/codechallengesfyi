#pragma once

#include <string_view>
#include <vector>

struct pollfd;

enum class ClientState
{
    Connected,
    Disconnected
};

class LoadBalancer
{
    public:
        void start(const std::string_view port);

    void forwardToBackend(int clientFd, const std::string_view str);
    ClientState handleClient(const int clientFd);

    private:
        std::vector<pollfd> fds_;

};