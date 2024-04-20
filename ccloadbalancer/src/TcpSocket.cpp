#include <TcpSocket.h>

#include <cerrno>
#include <errno.h>
#include <expected>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <array>
#include <iostream>

TcpSocket::TcpSocket(int port)
{
    struct sockaddr_in serv_addr;
    if ((clientFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::invalid_argument { "Socket creation error " + std::to_string(errno) };
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        throw std::invalid_argument { "Invalid address/ Address not supported" + std::to_string(errno) };
    }

    if ((::connect(clientFd, (struct sockaddr*)&serv_addr,
            sizeof(serv_addr)))
        < 0) {
        throw std::invalid_argument { "Connection Failed. errno: " + std::to_string(errno) };
    }
}

int TcpSocket::send(const std::string_view data) const noexcept
{
    return ::send(clientFd, data.data(), data.length(), 0);
}

TcpSocket::RecvValue TcpSocket::recv()
{
    std::array<char, 1024> buffer { 0 };
    auto bytesRead = ::recv(clientFd, buffer.data(),
        buffer.size() - 1, 0); // subtract 1 for the null
                               // terminator at the end
    std::cout << "Read " << bytesRead << " number of bytes\n";
    buffer[bytesRead] = '\0';
    std::cout << "received: " << buffer.data() << '\n';
    return RecvValue { buffer, clientFd };
}

std::expected<TcpSocket::RecvValue, int> TcpSocket::recvNonBlocking()
{
    std::array<char, 1024> buffer { 0 };
    auto bytesRead = ::recv(clientFd, buffer.data(),
        buffer.size() - 1, MSG_DONTWAIT); // subtract 1 for the null
                                          // terminator at the end
    if (bytesRead <= 0) {
        return std::unexpected { errno };
    }
    std::cout << "Read " << bytesRead << " number of bytes\n";
    buffer[bytesRead] = '\0';
    std::cout << "received: " << buffer.data() << '\n';
    return RecvValue { buffer, clientFd };
}

std::expected<TcpSocket::RecvValue, int> TcpSocket::recvWithError()
{
    std::array<char, 1024> buffer { 0 };
    auto bytesRead = ::recv(clientFd, buffer.data(),
        buffer.size() - 1, 0); // subtract 1 for the null
                               // terminator at the end
    std::cout << "Read " << bytesRead << " number of bytes\n";
    if (bytesRead <= 0) {
        return std::unexpected { bytesRead };
    }
    buffer[bytesRead] = '\0';
    std::cout << "received: " << buffer.data() << '\n';
    return RecvValue { buffer, clientFd };
}

int TcpSocket::getFd() const noexcept
{
    return clientFd;
}

TcpSocket::~TcpSocket()
{
    close(clientFd);
}