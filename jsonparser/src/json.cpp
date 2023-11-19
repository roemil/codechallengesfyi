#include "json.h"

#include "lexer.h"

#include <cstdio>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <stack>
#include <memory>
#include <vector>

namespace
{
    bool isValidString(const std::string& str) noexcept
    {
        const auto first = str.find('"');
        const auto second = str.find_last_of('"');
        if(first == second || first == std::string::npos || second == std::string::npos)
        {
            return false;
        }
        return true;
    }
    unsigned validateArray(const std::vector<std::string>& tokens, unsigned index)
    {
        if(tokens[index] == "," && tokens[index+1] == "]")
        {
            throw std::invalid_argument{"Expecting new object but got empty"};
        }
        const auto startingIndex = index;
        std::cout << "Validating array val " << tokens[index] << std::endl;
        if(tokens[++index] == ",")
        {
            return (index - startingIndex) + validateArray(tokens, index);
        }
        return index-startingIndex;

    }
    unsigned validateObject(const std::vector<std::string>& tokens, unsigned index)
    {
        if(tokens[index] == "," && tokens[index+1] == "}")
        {
            throw std::invalid_argument{"Expecting new object but got empty"};
        }
        if(tokens[index] == ",")
        {
            ++index;
        }
        const auto startingIndex = index;
        std::cout << "Validating key " << tokens[index] << std::endl;

        const auto& token = tokens[index];
        if(token == "}")
        {
            ++index;
            return index - startingIndex;
        }
        if(!isValidString(token))
        {
            throw std::invalid_argument{"Key is not a string"};
        }
        const auto separator = tokens[++index];
        if(separator != ":")
        {
            throw std::invalid_argument{"No key/value combo. Got: " + separator};
        }

        //TODO: Here I need to validate value, eg if it is a list,
        // is it valid?
        ++index; // eat value -> value should be verified in lexer

        if(tokens[++index] == ",")
        {
            return (index - startingIndex) + validateObject(tokens, index);
        }
        return (index - startingIndex);
    }
}

namespace json
{
    void parser::isValidJson(const std::string& filename) const
    {
        Lexer lexer{filename};

        auto tokens = lexer.getTokens();
        if(!tokens)
        {
            throw std::invalid_argument{"No tokens?"};
        }
        for(unsigned index = 0; index < tokens->size(); ++index)
        {
            const auto& token = tokens->at(index);
            if(token == "{")
            {
                const auto increment = validateObject(*tokens, ++index);
                index += increment;
            }
            else if(token == "[")
            {
                const auto increment = validateArray(*tokens, ++index);
                index += increment;
            }
        }

    }
}


#if 0
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
        // TODO: use regexp...
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
        // TODO: Use regexp...
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

bool isValidArr(std::string& str)
{
    if(!isBracketsValid(str))
    {
        return false;
    }

    // strip [ and ]
    str = str.substr(1);
    str = str.substr(0, str.length()-1);
    if(str.empty())
    {
        return true;
    }
    if(!isValidValue(str))
    {
        return false;
    }
    
    return true;
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

    // strip { and }
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
        else if(value.find("[") != std::string::npos)
        {
            // TODO: validate array
            if(!isValidArr(value))
            {
                std::cout << "invalid arr: " << value << std::endl;
                return false;
            }
        }
        // Normal element to validate
        else if(!isValidValue(value))
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
#endif