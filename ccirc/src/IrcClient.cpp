
#include "IrcClient.h"

#include <stdexcept>
#include <string_view>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <cassert>
#include <poll.h>

#include <iostream>
#include <array>
#include <string>

#include "IrcHandler.h"
#include "IrcParser.h"

void IrcClient::handleServerReply() {
    std::array<char, 5*1024> buf{};
    const auto bytes = read(socket_, buf.data(), buf.size());
        if(bytes < 0) {
            std::cout << "bytes = " << bytes << "\n";
            throw std::runtime_error{"Error receiving: " + std::to_string(errno)};
        }
        buf[bytes] = '\0';
        const std::string_view receivedData {buf.data(), static_cast<std::size_t>(bytes)};

        const auto lines = IrcParser::splitByCrlf(receivedData);
        for(const auto line : lines){
            try {
                const auto incomingCmd = IrcHandler::parse(line);
                const auto response = execute(incomingCmd, *this);
                const auto isPing = response.find("PONG ") != std::string::npos;
                if(!response.empty() && isPing) {
                    const auto sent = send(socket_, response.data(), response.length(), 0);
                    if(sent < 0) {
                        throw std::runtime_error{"Error sending: " + std::to_string(errno)};
                    }
                    assert(static_cast<unsigned long>(sent) == response.length());
                }
            } catch (const std::invalid_argument& e) {
                std::cout << "[DEBUG]: Failed to parse data: " << e.what() << "\n";
            }

        }
}

void IrcClient::uiCmd(const std::string_view cmd){
    if(cmd.empty()) {
        return;
    }
    const auto ircCmd = IrcParser::createIrcCommand(cmd, currentChannel.name);
    const auto sent = send(socket_, ircCmd.data(), ircCmd.length(), 0);
    assert(sent > 0);
    assert(static_cast<std::size_t>(sent) == ircCmd.length());
}

void IrcClient::startPoll()
{
    pollfd stdinFd{};
    stdinFd.fd = STDIN_FILENO;
    stdinFd.events = POLLIN;
    fds_.push_back(stdinFd);
    pollfd ircSocketFd{};
    ircSocketFd.fd = socket_;
    ircSocketFd.events = POLLIN;
    fds_.push_back(ircSocketFd);
    while (true) {
        poll(fds_.data(), fds_.size(), 0);

        for(const auto pollFd : fds_) {
            // if(pollFd.fd == STDIN_FILENO && pollFd.revents & POLLIN){
            //     std::string indata{};
            //     std::getline(std::cin, indata);
            //     if(indata.empty()) {
            //         continue;
            //     }
            //     //std::cout << "[DEBUG] Indata= " << indata << ".\n";
            //     const auto cmd = IrcParser::createIrcCommand(indata, currentChannel.name);
            //     //std::cout << "[DEBUG] IRC Command= " << cmd << ".\n";
            //     const auto sent = send(socket_, cmd.data(), cmd.length(), 0);
            //     assert(sent > 0);
            //     assert(static_cast<std::size_t>(sent) == cmd.length());
            // }
            if (pollFd.revents & POLLIN) {
                //std::cout << "[DEBUG] Got server reply triggered\n";
                handleServerReply();
            }
        }
        

    }
}

void IrcClient::connect(const std::string_view serverAddress)
{
    struct sockaddr_in server_addr;     // set server addr and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    const auto server = gethostbyname(serverAddress.data());
    assert(server);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*server->h_addr_list));
    server_addr.sin_port = htons(6667);  // server default port


    socket_ = socket(AF_INET,SOCK_STREAM, 0);
    if (socket_ < 0) {
        throw std::runtime_error{"Could not create socket"};
    }

    //connect server, return 0 with success, return -1 with error
    //std::cout << "Connecting...\n";
    if (::connect(socket_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        throw std::runtime_error{"Could not connect"};
    }

    char server_ip[INET_ADDRSTRLEN]="";
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
    printf("connected server(%s:%d). \n", server_ip, ntohs(server_addr.sin_port));

    std::array<char, 1024*5> buf{'\0'};
    auto receivedBytes = read(socket_, buf.data(), buf.size());
    // std::cout << "Received bytes: " << receivedBytes << std::endl;
    // std::cout << "Received data: " << buf.data() << std::endl;

    // Register
    //std::cout << "Registration!\n";
    const std::string nickCmd = "NICK " + nick_ + "\r\n";
    std::size_t sent = send(socket_, nickCmd.data(), nickCmd.length(), 0);
    assert(sent == nickCmd.length());

    const std::string realnameCmd = "USER guest 0 * :" + realname + "\r\n";
    sent = send(socket_, realnameCmd.data(), realnameCmd.length(), 0);
    assert(sent == realnameCmd.length());

    std::string_view receivedData = "";
    int maxTries = 5;
    while(maxTries > 0){
        //std::cout << "Max tries left: " << maxTries << "\n";
        receivedBytes = read(socket_, buf.data(), buf.size());
        receivedData = {buf.data(), static_cast<std::size_t>(receivedBytes)};
        if(receivedData.find("PING :") != std::string_view::npos){
            break;
        }
        --maxTries;
    }
    //std::cout << "Ping msg: " << receivedBytes << "\n";
    auto delimiterPos = receivedData.find(":");
    auto pingMsg = receivedData.substr(delimiterPos+1);
    //std::cout << "Ping msg: " << pingMsg << "\n";
    assert(pingMsg.length() < static_cast<std::size_t>(receivedBytes));
    assert(pingMsg.find("\r\n") < pingMsg.length());
    const std::string pong = "PONG " + std::string{pingMsg.data(), pingMsg.size()};
    sent = send(socket_, pong.data(), pong.length(), 0);
    assert(sent == pong.length());

    startPoll();
}

void IrcClient::incPingCounter(){
    ++counters.ping;
}
void IrcClient::incUnknownCounter(){
    ++counters.unknown;
}
void IrcClient::incJoinCounter(){
    ++counters.join;
}
void IrcClient::incPartCounter(){
    ++counters.part;
}
void IrcClient::incOtherCounter(){
    ++counters.other;
}
void IrcClient::incModeCounter(){
    ++counters.mode;
}
void IrcClient::incNoticeCounter(){
    ++counters.notice;
}
void IrcClient::incNickCounter(){
    ++counters.nick;
}

void IrcClient::incPrivMsgCounter(){
    ++counters.privMsg;
}

void IrcClient::setNick(const std::string_view nick) {
    nick_ = std::string{nick};
}

void IrcClient::setChannelName(const std::string_view ch) {
    currentChannel.name = std::string{ch};
}

void IrcClient::addUserToUi(const std::string_view name) {
    assert(tui_);
    tui_->addUser(name);
}