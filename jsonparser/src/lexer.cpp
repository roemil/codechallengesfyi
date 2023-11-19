#include "lexer.h"

#include <_ctype.h>
#include <stdexcept>
#include <string>
#include <fstream>
#include <memory>
#include <iostream>
#include <regex>

namespace json
{
Lexer::Lexer(const std::string& filename)
{
    std::ifstream file(filename);
    std::string str = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.cend());
    str.erase(std::remove_if(str.begin(), str.end(), isspace), str.cend());
    lex(str);
}

bool isNumber(const char input)
{
    // Regular expression to match valid numbers
    std::regex numberPattern("^[-+]?(0|([1-9]\\d*\\.?\\d*)|0?\\.\\d+)([eE][-+]?\\d+)?$");

    // Check if the input matches the pattern
    return std::regex_match(std::string{input}, numberPattern);
}



void Lexer::lex(const std::string& str)
{
    std::cout << "str: " << str << std::endl;
    if(str.empty())
    {
        throw std::invalid_argument{"Empty json"};
    }
    tokens = std::make_unique<std::vector<std::string>>();
    for(std::size_t i = 0; i < str.length(); ++i)
    {
        const auto c = str[i];
        if(isNumber(c))
        {
            std::string intString{};
            while(isNumber(str[i]))
            {
                intString += str[i];
                ++i;
            }
            tokens->emplace_back(intString);
            continue;
        }
        switch(c)
        {
            case ' ':
                break;
            case '{':
            case '}':
            case '[':
            case ']':
            case ',':
            case ':':
                tokens->emplace_back(std::string{c});
                break;
            case '"':
            {
                std::string innerStr{};
                innerStr += str[i];
                ++i;
                while(str[i] != '"' && i < str.length())
                {
                    innerStr += str[i++];
                }
                innerStr += '"';
                // TODO: Deal with invalid strings
                tokens->emplace_back(innerStr);
                //++i; // Increment for last quote mark
                break;
            }
            case 'f':
            {
                std::string expectedString {"false"};
                const auto actualString = str.substr(i, 5);
                if(actualString == expectedString)
                {
                    tokens->emplace_back(actualString);
                    i+=5;
                }
                else
                {
                    throw std::invalid_argument{"wrong string starting with f: " + actualString};
                }
                break;
            }
            case 't':
            {
                std::string expectedString {"true"};
                const auto actualString = str.substr(i, 4);
                if(actualString == expectedString)
                {
                    tokens->emplace_back(actualString);
                    i+=4;
                }
                else
                {
                    throw std::invalid_argument{"wrong string starting with t: " + actualString};
                }
                
                break;
            }
            case 'n':
            {
                std::string expectedString {"null"};
                const auto actualString = str.substr(i, 4);
                if(actualString == expectedString)
                {
                    tokens->emplace_back(actualString);
                    i+=4;
                }
                else
                {
                    throw std::invalid_argument{"wrong string starting with n: " + actualString};
                }
                break;
            }
            default:
            {
                throw std::invalid_argument{"Unexpected value: " + std::string{str[i]}};
            }
        }
    }
}

std::unique_ptr<std::vector<std::string>> Lexer::getTokens()
{
    return std::move(tokens);
}

bool Lexer::isValid()
{
    return isValidJson;
}

}