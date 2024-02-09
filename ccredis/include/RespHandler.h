#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <map>

enum class Prefix : char {
    SIMPLE_STRING = '+',
    ERROR = '-',
    INTEGER = ':',
    BULK_STRING = '$',
    ARRAY = '*',
    MAP = '%'
};

struct RedisRespRes {
    // TODO: Make values optional
    std::optional<int> integer_ {};
    std::optional<std::string_view> simpleString_ {};
    std::optional<std::string_view> bulkString_ {};
    std::optional<std::string_view> error_ {};
    std::optional<std::vector<RedisRespRes>> array_{};
    std::optional<std::map<std::string_view, RedisRespRes>> map_{};

    friend bool operator==(const RedisRespRes& lhs, const RedisRespRes& rhs)
    {
        return lhs.integer_ == rhs.integer_ && lhs.simpleString_ == rhs.simpleString_ && lhs.bulkString_ == rhs.bulkString_ && lhs.error_ == rhs.error_ && lhs.array_ == rhs.array_;
    }

    friend std::ostream& operator<<(std::ostream& os, const RedisRespRes& resp){
        os << "Int: " << resp.integer_.value_or(-1) << "\n";
        os << "SimpleString: " << resp.simpleString_.value_or("") << "\n";
        os << "BulkString: " << resp.bulkString_.value_or("") << "\n";
        os << "Error: " << resp.error_.value_or("") << "\n";
        if(resp.array_.has_value())
        {
            for(const auto& elem : resp.array_.value()){
                os << elem;
            }
        }
        return os;
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
    void beginMap(const unsigned numElements);
    void appendKV(const std::string_view key, const std::string_view val);
    void appendKV(const std::string_view key, const int val);

    [[nodiscard]] static std::pair<size_t, RedisRespRes> decode(const std::string_view str);

    const std::vector<char>& getBuffer() const;

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeSimpleString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeError(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeInt(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeBulkString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeArray(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeMap(const std::string_view str);
    // TODO: Replace with a output stringstream?
    std::vector<char> buffer {};
};