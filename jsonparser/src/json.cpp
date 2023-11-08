#include "json.h"

#include <cstdio>
#include <iterator>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <stack>
#include <memory>
#include <vector>

void Json::readFile(const std::string& filename)
{
    std::ifstream file(filename);
    content = std::make_unique<std::string>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    content->erase(std::remove(content->begin(), content->end(), '\n'), content->cend());
}

namespace
{
    bool isValidString(const std::string& str)
    {
        const auto first = str.find("\"");
        if(first == std::string::npos)
        {
            return false;
        }
        const auto end = str.find("\"", first+1);
        if(end == std::string::npos)
        {
            return false;
        }
        return true;
    }

    bool isValidDigit(const std::string& str)
    {
        try
        {
            const int TheInt = std::stoi(str);
        } catch (...)
        {
            return false;
        }
        return true;
    }

    bool isValidValue(const std::string& str)
    {
        if(isValidString(str))
        {
            return true;
        }
        if(str == "true")
        {
            return true;
        }
        if(str == "false")
        {
            return true;
        }
        if(str == "null")
        {
            return true;
        }
        if(isValidDigit(str))
        {
            return true;
        }
        return false;
    }

std::string extractKey(std::string& str)
{
    const auto separator = str.find(":");
    if(separator == std::string::npos)
    {
        return "";
    }
    const std::string key = str.substr(0, separator);
    str = str.substr(key.length());
    return key;
}

std::string extractValue(std::string& str)
{
    const auto commaPos = str.find(",");
    if(commaPos == std::string::npos)
    {
        const std::string value = str;
        str = str.substr(value.length());
        return value;
    }
    const std::string value = str.substr(0, commaPos);
    str = str.substr(value.length());
    return value;
}

bool isBracketsValid(const std::string& str)
{
    std::stack<char> stack{};
    for(const auto& c : str)
    {
        if(c == '{')
        {
            stack.push(c);
        }
        else if(c == '}' && stack.top() == '{')
        {
            stack.pop();
        }
        if(c == '[')
        {
            stack.push(c);
        }
        else if(c == ']' && stack.top() == '[')
        {
            stack.pop();
        }
    }
    return stack.empty();
}

bool isValidObject(std::string& str)
{
    std::cout << "str: " << str << std::endl;
    if(str.empty())
    {
        return false;
    }
    if(!isBracketsValid(str))
    {
        return false;
    }

    // strip {/[ and }/]
    str = str.substr(1);
    str = str.substr(0, str.length()-1);

    while(!str.empty())
    {
        // get key
        const auto key = extractKey(str);
        if(!isValidString(key))
        {
            return false;
        }
        if(str.empty())
        {
            // We parsed just a key but missing value
            return false;
        }
        // consume ":" + space
        str = str.substr(2);

        auto value = extractValue(str);

        // Check type of value
        if(value.find("{") != std::string::npos)
        {
            if(!isValidObject(value))
            {
                return false;
            }
            // consume the comma
            assert(str[0] == ',');
            str.substr(1);
        }

        if(value.find("[") != std::string::npos)
        {
            // TODO: validate array
        }

        // Normal element to validate
        if(!isValidValue(value))
        {
            return false;
        }
    }
    return true;
}


}


bool Json::isValidJson() const
{
    if(content->empty())
    {
        return false;
    }
    return isValidObject(*content);
}