#include <TcpSocket.h>

#include <errno.h>
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
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::invalid_argument { "Socket creation error " };
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        throw std::invalid_argument { "Invalid address/ Address not supported" };
    }

    if ((::connect(client_fd, (struct sockaddr*)&serv_addr,
            sizeof(serv_addr)))
        < 0) {
        throw std::invalid_argument { "Connection Failed" };
    }
}

void TcpSocket::send(const std::string_view data)
{
    ::send(client_fd, data.data(), data.length(), 0);
}

std::array<char, 1024> TcpSocket::recv()
{
    std::array<char, 1024> buffer { 0 };
    auto valread = ::read(client_fd, buffer.data(),
        buffer.size() - 1); // subtract 1 for the null
                            // terminator at the end
    std::cout << "Read " << valread << " number of bytes\n";
    buffer[valread] = '\0';
    std::cout << "received: " << buffer.data() << '\n';
    return buffer;
}

TcpSocket::~TcpSocket()
{
    close(client_fd);
}