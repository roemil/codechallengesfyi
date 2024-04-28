#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace json {
    enum class JsonElement
    {
        Invalid,
        True,
        False,
        Null
    };

    constexpr std::string_view toString(const JsonElement val)
    {
        switch (val) {
            case JsonElement::Invalid:
                return "Invalid!";
            case JsonElement::True:
                return "True";
            case JsonElement::False:
                return "False";
            case JsonElement::Null:
                return "Null";
        }
        return "Unexpected Json value";
    }
    struct ParseResult {
        std::vector<ParseResult> array_{};
        std::map<std::string, ParseResult> map_{};
        std::string val_{};
        JsonElement jsonValue{};
        //int integer{};
        std::string integer;

        friend bool operator==(const ParseResult& lhs, const ParseResult& rhs)
        {
            return lhs.array_ == rhs.array_ && lhs.map_ == rhs.map_ && lhs.val_ == rhs.val_;
        }
        friend std::ostream& operator<<(std::ostream& os, const ParseResult& value)
        {
            if(!value.array_.empty())
            {
                os << "\nArray: ";
                for(const auto& val : value.array_){
                    os << val << "\n";
                }
            }
            if(!value.map_.empty())
            {
                os << "\nMap: ";
                for(const auto& [key, val] : value.map_){
                    os << "key: " << key << " val " << val << "\n";
                }
            }
            if(!value.val_.empty())
            {
                os << "\nVal: " << value.val_ << "\n";
            }
            //if(value.integer != 0)
            if(!value.integer.empty())
            {
                // This will not print integers with value 0...
                os << "\nInteger: " << value.integer << "\n";
            }
            if(value.jsonValue != JsonElement::Invalid){
                os << "JsonValue: " << toString(value.jsonValue) << "\n";
            }


            return os;
        }
    };
    struct parser
    {
        ParseResult parse(const std::string& filename);

        std::pair<ParseResult, unsigned> parseArray(const std::vector<std::string>& tokens, unsigned index);
        std::pair<ParseResult, unsigned> parseObject(const std::vector<std::string>& tokens, unsigned index);
        std::tuple<std::string, ParseResult, unsigned> parseKeyValuePair(const std::vector<std::string>& tokens, unsigned index);
        std::pair<ParseResult, unsigned> parseValue(const std::vector<std::string>& tokens, unsigned index, const std::string_view value_raw);
        int depth{};
    };
}