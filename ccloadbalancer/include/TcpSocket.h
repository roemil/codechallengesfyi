#pragma once

#include <string_view>
#include <array>

class TcpSocket
{
    public:
    TcpSocket(int port);
    ~TcpSocket();

    void connect();
    void send(std::string_view data);
    std::array<char, 1024> recv();

    private:
        int client_fd{};

};
