#include "lexer.h"

#include <_ctype.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <fstream>
#include <memory>
#include <iostream>
#include <regex>
#include <array>
#include <cctype>

namespace json
{
Lexer::Lexer(const std::string& filename)
{
    std::ifstream file(filename);
    std::string str = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    //str.erase(std::remove(str.begin(), str.end(), '\n'), str.cend());
    lex(str);
}

bool isNumber(const char input)
{
    // Regular expression to match valid numbers
    std::regex numberPattern("^[-+]?(0|([1-9]\\d*\\.?\\d*)|0?\\.\\d+)([eE][-+]?\\d+)?$");

    // Check if the input matches the pattern
    return std::regex_match(std::string{input}, numberPattern);
}

bool isValidEscapeCharacterForJSON(const char ch)
{
    constexpr std::array<char, 8> validChars{'"', '\\', '/', 'b', 'f', 'n', 'r', 't'};
    return (std::find(validChars.cbegin(), validChars.cend(), ch) != validChars.cend());
}

bool isValidString2(const std::string& str)
{
    for(std::string::size_type i = 0; i < str.length(); ++i)
    {
        if(str[i] == '\\')
        {
            ++i;
            if(i > str.size())
            {
                throw std::invalid_argument{"Invalid string. Escape character must follow character"};
            }
            // Check if the next character is a valid escape character
            if (isValidEscapeCharacterForJSON(str[i]))
            {
                continue;
            }
            else if (str[i] == 'u')
            {
                // Handle Unicode escape sequence (\uXXXX)
                // Check for next 4 hexadecimal digits (\uXXXX)
                for (int j = 0; j < 4; j++)
                {
                    i++;
                    // Check if the next character doesn't exists
                    // or if it's not a hexadecimal character
                    if (i >= str.length() || !isxdigit(str[i]))
                    {
                        return false;
                    }
                }
            }
            else
            {
                // Any other character following a backslash is invalid
                return false;
            }
        }
    }
    return true;
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
        {
            const auto c = str[i];
            if(isdigit(c) || c == '+' || c == '-')
            {
                std::string intString{};
                while(isdigit(str[i]) || str[i] == 'x' || str[i] == '+' || str[i] == '-' || str[i] == 'e' || str[i] == '.')
                {
                    intString += str[i];
                    ++i;
                }
                if(!isNumber(intString))
                {
                    throw std::invalid_argument{"Cannot have leading 0s"};
                }
                tokens->emplace_back(intString);
                if(str[i] == ',' || str[i] == ']' || str[i] == '}')
                {
                    tokens->emplace_back(std::string{str[i]});
                }
                continue;
            }
        }
        const auto c = str[i];
        switch(c)
        {
            case ' ':
            case '\n':
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
                while(true)
                {
                    if(str[i] == '\\')
                    {
                        innerStr += str[i++];
                        if(i < str.length())
                        {
                            // Add character after escape
                            innerStr += str[i++];
                        }
                        continue;
                    }
                    else if (std::iscntrl(str[i]))
                    {
                        innerStr += str[i]; // add control character
                        break;
                    }
                    if(str[i] == '"')
                    {
                        innerStr += str[i];
                        break;
                    }
                    innerStr += str[i++];
                }
                if(!isValidString2(innerStr))
                {
                    throw std::invalid_argument{"Invalid string"};
                }
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
                    i+=4;
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
                    i+=3;
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
                    i+=3;
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