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
        if(tokens[++index] == ",")
        {
            index += validateArray(tokens, index);
        }
        return index-startingIndex;

    }
    unsigned validateObject(const std::vector<std::string>& tokens, unsigned index)
    {
        const auto startingIndex = index;
        if(tokens[index] == "}")
        {
            ++index;
            return index - startingIndex;
        }
        if(tokens[index] == ",")
        {
            ++index;
        }
        const auto& token = tokens[index];
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
        switch(value[0])
        {
            case '{':
                {
                    const auto increment = validateObject(tokens, ++index);
                    index += increment;
                    break;
                }
            case '[':
                {
                    const auto increment = validateArray(tokens, ++index);
                    index += increment;
                    break;
                }
        }

        if(tokens[++index] == ",")
        {
            switch (tokens[index+1][0])
            {
                case '{':
                    index += validateObject(tokens, ++index);
                    break;
                case '[':
                    index += validateArray(tokens, ++index);
                    break;
                case '"':
                    index += validateObject(tokens, ++index);
                    break;
                case '}':
                case ']':
                    throw std::invalid_argument{"Trailing comma"};
            }
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
                index += validateObject(*tokens, ++index);
            }
            else if(token == "[")
            {
                index += validateArray(*tokens, ++index);
            }
            else if(token == ",")
            {
                index += validateObject(*tokens, index);
            }
        }

    }
}   