#pragma once

#include "Commands.h"

#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

using PayloadT = std::variant<int, std::string_view>;
struct RedisRespRes;

class RespDecoder {
public:
    [[nodiscard]] static std::pair<size_t, RedisRespRes> decode(const std::string_view str);

    static std::vector<CommandVariant> convertToCommands(const RedisRespRes& rawCommands);

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeSimpleString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeError(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeInt(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeBulkString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeArray(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeMap(const std::string_view str);

    static CommandVariant parseRawCommand(const std::string_view rawCommand);
    static CommandVariant parseRawArrayCommands(const std::vector<RedisRespRes>& commandArray);
};