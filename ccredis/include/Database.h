#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

using KeyT = std::string_view;

struct ValueType {
    ValueType() = default;
    ValueType(const std::string_view value)
        : value_(std::string { value.data(), value.length() })
    {
    }
    std::string value_ {};
    // time when to expire

    operator std::string_view() { return value_; }
    operator std::string_view() const { return value_; }

    friend bool operator==(const ValueType& lhs, const ValueType& rhs)
    {
        return lhs.value_ == rhs.value_;
    }
};

class Db {
public:
    Db()
    {
        std::cout << "[INFO]: New Db is created.\n";
    }
    void set(const KeyT key, const std::string_view& value)
    {
        std::cout << "[INFO]: Storing key: " << key << " val: " << value << "\n";
        const auto key_ = std::string { key.data(), key.length() };
        const auto val_ = ValueType { value };
        map_[key_] = val_;
    }
    std::optional<ValueType> get(const KeyT& key)
    {
        std::cout << "[INFO]: Fetching key: " << key << "\n";
        const auto key_ = std::string { key.data(), key.length() };
        if (map_.contains(key_)) {
            return map_[key_];
        }
        return std::nullopt;
    }

private:
    std::map<std::string, ValueType> map_ {};
};