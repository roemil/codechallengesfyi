#pragma once

#include <string_view>
#include <vector>

struct pollfd;

enum class ClientState
{
    Connected,
    Disconnected
};

class EchoServer
{
    public:
        void start(const std::string_view port);

    void echoReply(int clientFd, const std::string_view str);
    ClientState handleClient(const int clientFd);

    private:
        std::vector<pollfd> fds_;

};