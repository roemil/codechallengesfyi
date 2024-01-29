#pragma once

#include <string_view>
#include <vector>
#include <utility>
#include <string>

enum class Prefix : char {
    SIMPLE_STRING = '+',
    ERROR = '-',
    INTEGER = ':',
    BULK_STRING = '$',
    ARRAY = '*'
};

// TODO: use this :)
// struct RedisRespRes{
//     int i;
//     std::vector<RedisRespRes> vec;
// };

class RespHandler {
public:
    void appendSimpleString(const std::string_view str);
    void appendError(const std::string_view str);
    void appendInt(const std::string_view n);
    void appendBulkstring(const std::string_view str);
    void appendNull();
    void beginArray(const unsigned numElements);

    [[nodiscard]] std::pair<size_t, std::string_view> decode(const std::string_view str);

    const std::vector<uint8_t>& getBuffer() const;

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    static std::pair<size_t, std::string_view> decodeSimpleString(const std::string_view str);
    static std::pair<size_t, std::string_view> decodeError(const std::string_view str);
    static std::pair<size_t, std::string_view> decodeInt(const std::string_view str);
    static std::pair<size_t, std::string_view> decodeBulkString(const std::string_view str);
    std::pair<size_t, std::string_view> decodeArray(const std::string_view str);
    // TODO: Replace with a output stringstream?
    std::vector<uint8_t> buffer {};
    std::string res{};
};