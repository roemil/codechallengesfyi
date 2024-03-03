#pragma once

#include <memory>
#include <string_view>
#include <vector>

class Db;

class RespEncoder {
public:
    RespEncoder() = default;
    // RespEncoder(const std::shared_ptr<Db>& db) : db_(db) {}
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
    void clearBuffer() { buffer.clear(); }

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    std::vector<char> buffer {};
};