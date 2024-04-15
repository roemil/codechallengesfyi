#pragma once

#include "IIrcClient.h"
#include <string>
#include <string_view>
#include <vector>

class IIrcClient;

class Tui{
public:
    Tui(IIrcClient* client) : ircClient(client) {}
    void start();

    void addChannel(const std::string_view ch){
        channels.emplace_back(ch);
    }

    void removeChannel(const std::string_view ch){
        channels.erase(std::remove(channels.begin(), channels.end(), ch), channels.end());
    }

    void updateChat(const std::string_view data){
        chat.emplace_back(data);
    }

    void addUser(const std::string_view user) {
        users_.emplace_back(user);
    }


private:
  std::vector<std::string> chat;
  std::vector<std::string> channels;
  std::vector<std::string> users_{}; // TODO: Do not hardcode :)
  std::string input;
  std::string name_;
  IIrcClient* ircClient = nullptr;
};