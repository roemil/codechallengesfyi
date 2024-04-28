#include "json.h"

#include "lexer.h"

#include <charconv>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace json
{
    bool isValidString(const std::string_view str) noexcept
    {
        const auto first = str.find('"');
        const auto second = str.find_last_of('"');
        if(first == second || first == std::string_view::npos || second == std::string_view::npos)
        {
            return false;
        }
        return true;
    }

    int toNumber(const std::string_view numberString)
    {
        int value{};
        auto [ptr, ec] = std::from_chars(numberString.data(), numberString.data() + numberString.size(), value);
        if(ec != std::errc()){
            throw std::invalid_argument{"Not a number: " + std::string{numberString}};
        }
        
        return value;
    }

    std::pair<ParseResult, unsigned> parser::parseValue(const std::vector<std::string>& tokens, unsigned index, const std::string_view value_raw)
    {
        ParseResult value{};
        switch (value_raw[0])
        {
            case 't':
            {
                value.jsonValue = JsonElement::True;
                ++index;
                break;
            }
            case 'f':
            {
                value.jsonValue = JsonElement::False;
                ++index;
                break;
            }
            case 'n':
            {
                value.jsonValue = JsonElement::Null;
                ++index;
                break;
            }
            case '\"':
            {
                value.val_ = value_raw;
                ++index;
                break;
            }
            case '{':
            {
                const auto object = parseObject(tokens, index);
                index = object.second;
                value = object.first;
                break;
            }
            case '[':
            {
                const auto parsedArray = parseArray(tokens, index);
                value = parsedArray.first;
                index = parsedArray.second;
                break;
            }

            default:
            {
                if(Lexer::isNumber(std::string{value_raw}))
                {
                    //value.integer = toNumber(value_raw);
                    value.integer = value_raw;
                    ++index;
                    break;
                }
                throw std::invalid_argument{"Unexpected value: " + std::string{value_raw} + "!"};
            }
        }
        return {value, index};
    }

    std::tuple<std::string, ParseResult, unsigned> parser::parseKeyValuePair(const std::vector<std::string>& tokens, unsigned index)
    {
        const std::string_view key = tokens[index];
        if(!isValidString(key))
        {
            throw std::invalid_argument{"Key is not a string: " + std::string{key}};
        }

        const std::string_view separator = tokens[++index];
        if(separator != ":")
        {
            throw std::invalid_argument{"No key/value combo. Got: " + std::string{separator}};
        }
        const std::string_view value_raw = tokens[++index];
        const auto parsedValue = parseValue(tokens, index, value_raw);
        ParseResult value = parsedValue.first;
        return {std::string{key}, value, parsedValue.second};

    }
    std::pair<ParseResult, unsigned> parser::parseArray(const std::vector<std::string>& tokens, unsigned index)
    {
        ++depth;
        if(depth >= 20)
        {
            throw std::invalid_argument{"Too deep"};
        }

        if(tokens[index] == "[")
        {
            ++index;
        }
        else
        {
            throw std::invalid_argument{"Arrays must begin with [ or ,"};
        }
        if(tokens[index] == ",")
        {
            throw std::invalid_argument{"Missing value"};
        }
        if(tokens[index] == "]")
        {
            return {ParseResult{}, ++index};
        }
        ParseResult result{};
        while(tokens[index] != "]")
        {
            const std::string_view token = tokens[index];
            if(token == ",")
            {
                if(index + 1 < tokens.size() && tokens[index+1] == "]"){
                    throw std::invalid_argument{"Missing value in array"};
                }
                ++index;
                continue;
            }
            const auto parsedValue = parseValue(tokens, index, token);
            result.array_.push_back(parsedValue.first);
            index = parsedValue.second;
            if(index > tokens.size())
            {
                throw std::invalid_argument{"Out of bounds when parsing array"};
            }
        }
        --depth;
        ++index; // Consume last bracket
        
        return {result, index};

    }
    std::pair<ParseResult, unsigned> parser::parseObject(const std::vector<std::string>& tokens, unsigned index)
    {
        if(tokens[index] == "{")
        {
            ++index;
        }
        else
        {
            throw std::invalid_argument{"Object must begin with {"};
        }
        if(tokens[index] == "}")
        {
            //++index;
            return {ParseResult{}, ++index};
        }
        const std::string_view key = tokens[index];
        if(!isValidString(key))
        {
            throw std::invalid_argument{"Key is not a string"};
        }
        const auto separator = tokens[++index];
        if(separator != ":")
        {
            throw std::invalid_argument{"No key/value combo. Got: " + separator};
        }

        ++index;
        ParseResult result{};
        while(tokens[index] != "}")
        {
            const std::string_view value = tokens[index];
            if(value.starts_with(",")){
                const auto parsedValue = parseKeyValuePair(tokens, index+1);
                result.map_[std::get<0>(parsedValue)] = std::get<1>(parsedValue);
                index = std::get<2>(parsedValue);
            }
            else
            {
                const auto parsedValue = parseValue(tokens, index, value);
                result.map_[std::string{key}] = parsedValue.first;
                index = parsedValue.second;
            }
            if(index > tokens.size()) 
            {
                throw std::invalid_argument{"Trying to access token out of bounds"};    
            }
        }

        return {result, ++index}; // Consume last bracket
    }
}

namespace json
{
    ParseResult parser::parse(const std::string& filename)
    {
        Lexer lexer{filename};

        auto tokens = lexer.getTokens();
        if(!tokens)
        {
            throw std::invalid_argument{"No tokens?"};
        }
        if(tokens->at(0) != "{" && tokens->at(0) != "[")
        {
            throw std::invalid_argument{"JSON must begin as object or array"};
        }
        const auto lastIndex = tokens->size()-1;
        if(tokens->at(lastIndex) != "}" && tokens->at(lastIndex) != "]")
        {
            throw std::invalid_argument{"JSON must end as object or array"};
        }
        ParseResult parsedData{};
        const auto& token = tokens->at(0);
        if(token == "{"){
            const auto parsedJson = parseObject(*tokens, 0);
            if(parsedJson.second < tokens->size())
            {
                throw std::invalid_argument{"Unparsed tokens: " + tokens->at(parsedJson.second)};
            }
            return parsedJson.first;
        }
        else if(token == "[")
        {
            const auto parsedJson = parseArray(*tokens, 0);
            if(parsedJson.second < tokens->size())
            {
                throw std::invalid_argument{"Unparsed tokens: " + tokens->at(parsedJson.second)};
            }
            return parsedJson.first;
        }
        
        throw std::invalid_argument{"JSON must start with { or ["};
    }
}