#pragma once

#include <optional>
#include <string_view>
#include <map>
#include <iostream>
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

    friend bool operator==(const RedisRespRes& lhs, const RedisRespRes& rhs)
    {
        return lhs.integer_ == rhs.integer_ && lhs.string_ == rhs.string_ && lhs.error_ == rhs.error_ && lhs.array_ == rhs.array_;
    }

    friend std::ostream& operator<<(std::ostream& os, const RedisRespRes& resp)
    {
        os << "Int: " << resp.integer_.value_or(-1) << "\n";
        os << "String: " << resp.string_.value_or("") << "\n";
        os << "Error: " << resp.error_.value_or("") << "\n";
        if (resp.array_.has_value()) {
            for (const auto& elem : resp.array_.value()) {
                os << elem;
            }
        }
        return os;
    }
};