#pragma once

#include <array>
#include <expected>
#include <string_view>

class TcpSocket {
public:
    TcpSocket() = default;
    TcpSocket(int port);
    ~TcpSocket();

    int send(std::string_view data) const noexcept;

    using RecvValue = std::pair<std::array<char, 1024>, int>;
    RecvValue recv();
    std::expected<RecvValue, int> recvWithError();
    std::expected<RecvValue, int> recvNonBlocking();

    int getFd() const noexcept;

private:
    int clientFd {};
};
