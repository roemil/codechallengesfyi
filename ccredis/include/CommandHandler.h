#pragma once

#include "RespEncoder.h"

#include <memory>

class Db;
struct CommandUnknown;
struct CommandInvalid;
struct CommandPing;
struct CommandHello;
struct CommandSet;
struct CommandGet;
struct CommandExists;
struct CommandIncr;

class CommandHandler {
public:
    CommandHandler() = default;
    CommandHandler(RespEncoder* enc, const std::shared_ptr<Db>& db)
        : encoder_(enc)
        , db_(db)
    {
    }

    void operator()(const CommandUnknown&);
    void operator()(const CommandInvalid&);
    void operator()(const CommandPing&);
    void operator()(const CommandHello&);
    void operator()(const CommandSet&);
    void operator()(const CommandGet&);
    void operator()(const CommandExists&);
    void operator()(const CommandIncr&);

private:
    RespEncoder* encoder_;
    std::shared_ptr<Db> db_;
};