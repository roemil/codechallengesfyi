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



    bool isValidObjectMember(std::string& str)
    {
        //std::cout << "parsing str: " << str << std::endl;
        const auto pos = str.find(":");
        if(pos == std::string::npos)
        {
            // No key - value -> invalid object
            return false;
        }

        const std::string key = str.substr(0, pos);
        if(!isValidString(key))
        {
            // key must be string
            return false;
        }

        // consume key
        str = str.substr(pos+1);

        auto endPos = str.find(",");
        if(endPos == std::string::npos)
        {
            if(!isValidValue(str))
            {
                return false;
            }
            // Consume str to break the outer loop.
            str = str.substr(str.length());
            return true;
        }
        const std::string value = str.substr(0, endPos);
        if(!isValidValue(value))
        {
            return false;
        }
        // consume value
        str = str.substr(value.length());
        // only one comma left from the parsed string -> errounous trailing comma
        if(str == ",")
        {
            return false;
        }
        str = str.substr(1); // update str to point to the next member by consuming the comma
        return true;
    }
}

bool Json::isValidJson() const
{
    if(content->empty())
    {
        return false;
    }
    std::stack<char> stack{};
    for(const auto& c : *content)
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
        else if(c == ']' && stack.top() == '{')
        {
            stack.pop();
        }
    }
    if(!stack.empty())
    {
        return false;
    }

    // strip { and }
    *content = content->substr(1);
    *content = content->substr(0, content->length()-1);

    while(!content->empty())
    {
        //TODO:
        // Here I can get the key and validate key
        // Then I get the value and check if value is
        // object or array or element. Then I can
        // validate value based on that info.

        if(!isValidObjectMember(*content))
        {
            return false;
        }
    }
    return true;
    

    // parse object - begins with { and ends with }, can be comma separated k:v pair.

    


}