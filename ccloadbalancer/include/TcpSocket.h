#pragma once

#include <array>
#include <string_view>

class TcpSocket {
public:
    TcpSocket(int port);
    ~TcpSocket();

    void connect();
    void send(std::string_view data);
    std::array<char, 1024> recv();

private:
    int client_fd {};
};
