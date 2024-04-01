#pragma once

#include <array>
#include <string_view>
#include <expected>

class TcpSocket {
public:
    TcpSocket() = default;
    TcpSocket(int port);
    ~TcpSocket();

    int send(std::string_view data) const noexcept;
    std::array<char, 1024> recv();
    std::expected<std::array<char, 1024>, int> recvWithError();

    int getFd() const noexcept;

private:
    int clientFd {};
};
