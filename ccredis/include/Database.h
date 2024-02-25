#pragma once

#include <map>
#include <string_view>
#include <string>
#include <type_traits>
#include <optional>
#include <iostream>
#include <functional>

using ValueT = std::string_view;
using KeyT = std::string_view;
class Db{
    public:
    Db() {
        std::cout << "[INFO]: New Db is created.\n";
    }
     void set(const KeyT key, const ValueT& value){
        static_assert(std::is_same_v<ValueT, std::string_view>, "Only support string_view as value as of now.");
        std::cout << "[INFO]: Storing key: " << key << " val: " << value << "\n";
        const auto key_ = std::string{key.data(), key.length()};
        const auto val_ = std::string{value.data(), value.length()};
        map_[key_] = val_;
     }
     std::optional<ValueT> get(const KeyT& key) {
        std::cout << "[INFO]: Fetching key: " << key << "\n";
        const auto key_ = std::string{key.data(), key.length()};
        if(map_.contains(key_)){
            return map_[key_];
        }
        return std::nullopt;
     }
    
    private:
        std::map<std::string, std::string> map_{}; 

};