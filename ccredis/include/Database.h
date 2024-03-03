#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <chrono>

using KeyT = std::string_view;

struct ValueType {
    ValueType() = default;
    explicit ValueType(const std::string_view value)
        : value_(std::string { value.data(), value.length() })
    {
    }
    ValueType(const std::string_view value, const int expire)
        : value_(std::string { value.data(), value.length() }), expire_(expire)
    {
    }
    std::string value_ {};
    std::optional<int> expire_{};

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
    void set(const KeyT key, const std::string_view& value, const int expire)
    {
         const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::cout << "[INFO]: Storing key: " << key << " val: " << value << " Expires in: " << std::to_string(expire-now) << "\n";
        const auto key_ = std::string { key.data(), key.length() };
        const auto val_ = ValueType { value, expire };
        map_[key_] = val_;
    }
    std::optional<ValueType> get(const KeyT& key)
    {
        std::cout << "[INFO]: Fetching key: " << key << "\n";
        const auto key_ = std::string { key.data(), key.length() };

        if (map_.contains(key_)) {
            const auto& value_ = map_[key_];
            if (value_.expire_.has_value())
            {

                const auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                const auto expires = value_.expire_.value();
                std::cout << "[INFO]: Key expires in: " << std::to_string(expires-now) << "\n";
                if(value_.expire_.value() - now < 0)
                {
                    std::cout << "[INFO]: Key expired\n";
                    return std::nullopt;
                }
            }
            return map_[key_];
        }
        std::cout << "[INFO]: Key not found\n";
        return std::nullopt;
    }

private:
    std::map<std::string, ValueType> map_ {};
};