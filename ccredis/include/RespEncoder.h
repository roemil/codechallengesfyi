#pragma once

#include <string_view>
#include <vector>

struct CommandUnknown;
struct CommandInvalid;
struct CommandPing;
struct CommandHello;
struct CommandSet;
struct CommandGet;

class RespEncoder {
public:
    void appendSimpleString(const std::string_view str);
    void appendError(const std::string_view str);
    void appendInt(const std::string_view n);
    void appendBulkstring(const std::string_view str);
    void appendNull();
    void beginArray(const unsigned numElements);
    void beginMap(const unsigned numElements);
    void appendKV(const std::string_view key, const std::string_view val);
    void appendKV(const std::string_view key, const int val);

    const std::vector<char>& getBuffer() const;
    void operator()(const CommandUnknown&);
    void operator()(const CommandInvalid&);
    void operator()(const CommandPing&);
    void operator()(const CommandHello&);
    void operator()(const CommandSet&);
    void operator()(const CommandGet&);

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    std::vector<char> buffer {};
};