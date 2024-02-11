#pragma once

#include <functional>
#include <map>
#include <string_view>
#include <type_traits>
#include <optional>

using ValueT = std::string_view;
using KeyT = std::string_view;
class Db{
    public:
     void set(const KeyT key, const ValueT& value){
        static_assert(std::is_same_v<ValueT, std::string_view>, "Only support string_view as value as of now.");
        map_[key] = value;
     }
     std::optional<ValueT> get(const KeyT& key) {
        if(map_.contains(key)){
            return map_[key];
        }
        return std::nullopt;
     }
    
    private:
        std::map<KeyT, ValueT> map_{}; 

};