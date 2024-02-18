#pragma once

#include "RespDecoder.h"
#include "RespEncoder.h"
#include <algorithm>
#include <memory>
#include <string_view>
#include <vector>
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
        RedisServer() = default;
        RedisServer(std::shared_ptr<Db>&& db) : db_(std::move(db)) {
            assert(db_);
            respEncoder_ = RespEncoder{db_};
        }

    void start(std::string_view port);

    private:
    ClientState handleClient(const int clientFd);
    void handleInput(int clientFd, const std::string_view str);
    std::vector<pollfd> fds_{};
    RespDecoder respDecoder_{};
    RespEncoder respEncoder_{};
    std::shared_ptr<Db> db_{};



};