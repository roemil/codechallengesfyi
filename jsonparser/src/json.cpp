#include "json.h"

#include "lexer.h"

#include <cstdio>
#include <gtest/gtest.h>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <stack>
#include <memory>
#include <vector>

namespace json
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
    unsigned verifyKvPair(const std::vector<std::string>& tokens, unsigned index)
    {
        const auto startIndex = index;
        const auto& token = tokens[++index];
        if(!isValidString(token))
        {
            throw std::invalid_argument{"Key is not a string"};
        }
        const auto separator = tokens[++index];
        if(separator != ":")
        {
            throw std::invalid_argument{"No key/value combo. Got: " + separator};
        }
        ++index;
        return index - startIndex;
    }
    unsigned parser::validateArray(const std::vector<std::string>& tokens, unsigned index)
    {
        ++depth;
        if(depth >= 20)
        {
            throw std::invalid_argument{"Too deep"};
        }
        // TODO: Should only start with {
        const auto startingIndex = index;
        if(tokens[index] == "," || tokens[index] == "[")
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
            return (++index - startingIndex);
        }
        while(tokens[index] != "]")
        {
            const auto& token = tokens[index];
            switch (token[0])
            {
                case '{':
                {
                    index += validateObject(tokens, index);
                    break;
                }
                case '[':
                {
                    index += validateArray(tokens, index);
                }
                case '"':
                {
                    ++index; // Normal value - validated in lexer.
                    break;
                }
                case ',':
                {
                    if(tokens[index+1] == "]")
                    {
                        throw std::invalid_argument{"Trailing comma in array"};
                    }
                    ++index;
                    break;
                }
                case 't':
                case 'f':
                case 'n':
                    ++index;
                    break;
                default:
                    if(Lexer::isNumber(tokens[index]))
                    {
                        ++index;
                        continue;
                    }
                    else if(index >= tokens.size())
                    {
                        return (index - startingIndex);
                    }
                    else
                    {
                        throw std::invalid_argument{"Invalid value in arr: " + token};
                    }
            }
        }

        if(tokens[index] != "]")
        {
            throw std::invalid_argument{"Expected closing bracket " + tokens[index]};
        }
        ++index; // Consume last bracket
        --depth;
        return index-startingIndex;

    }
    unsigned parser::validateObject(const std::vector<std::string>& tokens, unsigned index)
    {
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

        // TODO: Refactor to use verifyKvPair
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
        ++index;
        while(tokens[index] != "}")
        {
            const auto value = tokens[index];
 
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
                case ',':
                {
                    if(tokens[index+1] == "]" || tokens[index+1] == "}")
                    {
                        throw std::invalid_argument{"Trailing comma"};
                    }
                    else if(tokens[index+1] == "{")
                    {
                        index += validateObject(tokens, index);
                        break;
                    }
                    index += verifyKvPair(tokens, index);
                    break;
                }
                case '"':
                case 't':
                case 'f':
                case 'n':
                    {
                        ++index;
                        break;
                    }
                default:
                    if(Lexer::isNumber(value))
                    {
                        ++index;
                        continue;
                    }
                    else
                    {
                            throw std::invalid_argument{"Invalid value: " + value };
                    }
            }
            assert(index < tokens.size());

        }

        if(tokens[index] == "}")
        {
            ++index;
        }
        else {
            throw std::invalid_argument{"Expected closing }"};
        }
        return (index - startingIndex);
    }
}

namespace json
{
    void parser::isValidJson(const std::string& filename)
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
            if((index < tokens->size() && tokens->at(index) == "]"))
            {
                throw std::invalid_argument{"Trailing ]"};
            }
        }
    }
}   