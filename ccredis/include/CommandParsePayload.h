#pragma once

#include <string_view>
#include <variant>

struct CommandUnknown;
struct CommandInvalid;
struct CommandPing;
struct CommandHello;
struct CommandSet;
struct CommandGet;
struct CommandExists;

struct RedisRespRes;
class RespEncoder;

struct ParsePayload {
    ParsePayload(const RedisRespRes& resp)
        : resp_(resp)
    {
    }
    const RedisRespRes& resp_;

    ParsePayload(const ParsePayload&) = delete;
    ParsePayload operator=(const ParsePayload&) = delete;

    void operator()(CommandUnknown&);
    void operator()(CommandInvalid&);
    void operator()(CommandPing&);
    void operator()(CommandHello&);
    void operator()(CommandSet&);
    void operator()(CommandGet&);
    void operator()(CommandExists&);
};
