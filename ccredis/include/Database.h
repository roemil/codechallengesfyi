#pragma once

#include <chrono>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>

using KeyT = std::string_view;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
struct ValueType {
  ValueType() = default;
  explicit ValueType(const std::string_view value)
      : value_(std::string{value.data(), value.length()}) {}
  ValueType(const std::string_view value, const TimePoint& expire)
      : value_(std::string{value.data(), value.length()}), expire_(expire) {}
  std::string value_{};
  std::optional<TimePoint> expire_{};

  operator std::string_view() { return value_; }
  operator std::string_view() const { return value_; }

  friend bool operator==(const ValueType &lhs, const ValueType &rhs) {
    return lhs.value_ == rhs.value_;
  }
};

class Db {
public:

  Db() { std::cout << "[INFO]: New Db is created.\n"; }
  void set(const KeyT key, const std::string_view &value) {
    std::cout << "[INFO]: Storing key: " << key << " val: " << value << "\n";
    const auto key_ = std::string{key.data(), key.length()};
    const auto val_ = ValueType{value};
    map_[key_] = val_;
  }
  void set(const KeyT key, const std::string_view &value, const TimePoint& expire) {
  using namespace std::chrono;
   const auto now =std::chrono::system_clock::now();
    std::cout << "[INFO]: Storing key: " << key << " val: " << value
              << " Expires in: " << std::chrono::duration_cast<std::chrono::seconds>(expire.time_since_epoch() - now.time_since_epoch()) << "\n";
    const auto key_ = std::string{key.data(), key.length()};
    const auto val_ = ValueType{value, expire};
    map_[key_] = val_;
  }
  std::optional<ValueType> get(const KeyT &key) {
    std::cout << "[INFO]: Fetching key: " << key << "\n";
    const auto key_ = std::string{key.data(), key.length()};

    if (map_.contains(key_)) {
      const auto &value_ = map_[key_];
      if (value_.expire_.has_value()) {

        const auto now =std::chrono::system_clock::now();
        const auto expires = value_.expire_.value();
        // std::cout << "[INFO]: Key expires in: " << std::to_string(expires - now)
        //           << "\n";
        using Ms = std::chrono::milliseconds;
        const auto timeElapsed = std::chrono::duration_cast<Ms> ( expires - now);
        if (timeElapsed < Ms{0}) {
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
  std::map<std::string, ValueType> map_{};
};