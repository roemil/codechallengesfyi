#pragma once

#include <optional>
#include <string_view>
#include <map>
#include <iosfwd>
#include <vector>

enum class Prefix : char {
    SIMPLE_STRING = '+',
    ERROR = '-',
    INTEGER = ':',
    BULK_STRING = '$',
    ARRAY = '*',
    MAP = '%'
};

struct RedisRespRes {
    std::optional<int> integer_ {};
    std::optional<std::string_view> string_ {};
    std::optional<std::string_view> error_ {};
    std::optional<std::vector<RedisRespRes>> array_ {};
    std::optional<std::map<std::string_view, RedisRespRes>> map_ {};

    friend bool operator==(const RedisRespRes& lhs, const RedisRespRes& rhs);
    friend std::ostream& operator<<(std::ostream& os, const RedisRespRes& resp);
};