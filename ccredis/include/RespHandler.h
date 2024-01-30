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

struct RedisRespRes {
    // TODO: Make values optional
    std::string_view integer_{};
    std::string_view simpleString_{};
    std::string_view bulkString_{};
    std::string_view error_{};
    std::vector<RedisRespRes> array_;

    friend bool operator==(const RedisRespRes& lhs, const RedisRespRes& rhs){
        return lhs.integer_ == rhs.integer_ && lhs.simpleString_ == rhs.simpleString_ && lhs.bulkString_ == rhs.bulkString_ && lhs.error_ == rhs.error_ && lhs.array_ == rhs.array_;
    }
};

class RespHandler {
public:
    void appendSimpleString(const std::string_view str);
    void appendError(const std::string_view str);
    void appendInt(const std::string_view n);
    void appendBulkstring(const std::string_view str);
    void appendNull();
    void beginArray(const unsigned numElements);

    [[nodiscard]] static std::pair<size_t, RedisRespRes> decode(const std::string_view str);

    const std::vector<uint8_t>& getBuffer() const;

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeSimpleString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeError(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeInt(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeBulkString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeArray(const std::string_view str);
    // TODO: Replace with a output stringstream?
    std::vector<uint8_t> buffer {};
};