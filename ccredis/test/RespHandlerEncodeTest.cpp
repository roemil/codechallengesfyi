#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>
#include <vector>

#include "RespHandler.h"

namespace {
void appendCRLF(std::vector<char>& vec)
{
    vec.push_back(static_cast<char>(0x0d));
    vec.push_back(static_cast<char>(0x0A));
}

void appendChars(std::vector<char>& output, std::string_view input)
{
    for (const auto& c : input) {
        output.push_back(c);
    }
}

} // namespace

/*

Test list

For Simple Strings, the first byte of the reply is "+"
For Errors, the first byte of the reply is "-"
For Integers, the first byte of the reply is ":"
For Bulk Strings, the first byte of the reply is "$"
For Arrays, the first byte of the reply is "*"


*/

class RespHandlerTest : public testing::Test {
protected:
    RespHandler rh {};
};

TEST_F(RespHandlerTest, CRLFOnly)
{
    rh.appendSimpeString("");
    std::vector<char> result { '+' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerTest, SimpleString)
{
    rh.appendSimpeString("OK");
    std::vector<char> result { '+', 'O', 'K' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerTest, SimpleStringIllegalCR)
{
    EXPECT_THROW(rh.appendSimpeString("OK\r"), std::invalid_argument);
}

TEST_F(RespHandlerTest, Error)
{
    constexpr std::string_view error { "Some Error Message" };
    rh.appendError(error);
    std::vector<char> result { '-' };
    appendChars(result, error);
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerTest, SmallInteger)
{
    constexpr std::string_view n = "5";
    rh.appendInt(n);
    std::vector<char> result { ':' };
    appendChars(result, n);
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerTest, LargeInteger)
{
    constexpr std::string_view n = "1000";
    rh.appendInt(n);
    std::vector<char> result { ':' };
    result.push_back('1');
    result.push_back('0');
    result.push_back('0');
    result.push_back('0');
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}