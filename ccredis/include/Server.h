#pragma once

#include "RespDecoder.h"
#include "RespEncoder.h"
#include "CommandHandler.h"
#include <algorithm>
#include <memory>
#include <string_view>
#include <vector>
#include <cassert>

struct pollfd;

enum class ClientState{
    Disconnected,
    Connected
};

class RespDecoder;
class RespEncoder;
class Db;

class RedisServer
{
    public:
        RedisServer(std::shared_ptr<Db>&& db) : db_(std::move(db)) {
            assert(db_);
            commandHandler_ = CommandHandler{&respEncoder_, db_};
        }

    void start(std::string_view port);

    private:
    ClientState handleClient(const int clientFd);
    void handleInput(int clientFd, const std::string_view str);
    std::vector<pollfd> fds_{};
    RespDecoder respDecoder_{};
    RespEncoder respEncoder_{};
    CommandHandler commandHandler_;
    std::shared_ptr<Db> db_{};



};
