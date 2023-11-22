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
        if(tokens[index] != "[")
        {
            throw std::invalid_argument{"Arrays must begin with ["};
        }
        ++index;
        const auto startingIndex = index;
        if(tokens[index] == ",")
        {
            throw std::invalid_argument{"Missing value"};
        }
        if(tokens[index] == "]")
        {
            return ++index;
        }
        ++index;
        while(tokens[index] == ",")
        {
            if(tokens[index+1] == "]")
            {
                throw std::invalid_argument{"Trailing comma"};
            }
            if(isValidString(tokens[index+1]))
            {
                ++index;
                continue;
            }
            throw std::invalid_argument{"Expected valid string in array"};
        }
        if(isValidString(tokens[index]))
        {
            index += 2; // Consume last value and bracket
            return index - startingIndex;
        }
        else if(tokens[index] != "]")
        {
            throw std::invalid_argument{"Expected closing bracket"};
        }
        ++index; // Consume last bracket
        return index-startingIndex;

    }
    unsigned validateObject(const std::vector<std::string>& tokens, unsigned index)
    {
        std::cout << "object: " << tokens[index] << std::endl;
        const auto startingIndex = index;
        // TODO: Should only start with {
        if(tokens[index] == "," || tokens[index] == "{")
        {
            ++index;
        }
        else
        {
            throw std::invalid_argument{"Object must begin with { or ,"};
        }
        if(tokens[index] == "}")
        {
            ++index;
            return index - startingIndex;
        }

        const auto& token = tokens[index];
        std::cout << "key: " << token << std::endl;
        if(!isValidString(token))
        {
            throw std::invalid_argument{"Key is not a string"};
        }
        const auto separator = tokens[++index];
        if(separator != ":")
        {
            throw std::invalid_argument{"No key/value combo. Got: " + separator};
        }

        const auto value = tokens[++index];
        std::cout << "value " << value << std::endl;
        switch(value[0])
        {
            case '{':
                {
                    index += validateObject(tokens, index);
                    break;
                }
            case '[':
                {
                    index += validateArray(tokens, index);
                    break;
                }
        }
        if(isValidString(value) || tokens[index] == "}"
        || value == "true" || value == "false")
        {
            // Consume the current value or the closing bracket.
            ++index;
        }
        std::cout << "value2 " << tokens[index] << std::endl;

        if(tokens[index] == ",")
        {
            switch (tokens[index+1][0])
            {
                case '{':
                    std::cout << "Parsing nested object\n";
                    index += validateObject(tokens, index);
                    break;
                case '[':
                    index += validateArray(tokens, index);
                    break;
                case '"':
                    index += validateObject(tokens, index);
                    break;
                case '}':
                case ']':
                case ',':
                    throw std::invalid_argument{"Trailing comma"};
            }
        }
        if(tokens[index] == "}")
        {
            ++index;
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
        if(tokens->at(0) != "{" && tokens->at(0) != "[")
        {
            throw std::invalid_argument{"JSON must begin as object or array"};
        }
        const auto lastIndex = tokens->size()-1;
        if(tokens->at(lastIndex) != "}" && tokens->at(lastIndex) != "]")
        {
            throw std::invalid_argument{"JSON must end as object or array"};
        }
        for(unsigned index = 0; index < tokens->size(); ++index)
        {
            const auto& token = tokens->at(index);
            std::cout << "token: " << token << std::endl;
            if(token == "{")
            {
                index += validateObject(*tokens, index);
            }
            else if(token == "[")
            {
                index += validateArray(*tokens, index);
            }
            else if(token == ",")
            {
                index += validateObject(*tokens, index);
            }
            else if(token == "]")
            {
                throw std::invalid_argument{"Trailing ]"};
            }
        }

    }
}   