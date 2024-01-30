
#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>
#include <sys/types.h>
#include <vector>

#include "RespHandler.h"

namespace {
void appendCRLF(std::vector<char>& vec)
{
    vec.push_back('\r');
    vec.push_back('\n');
}

void appendChars(std::vector<char>& output, std::string_view input)
{
    for (const auto& c : input) {
        output.push_back(c);
    }
}

} // namespace

class RespHandlerEncodeTest : public testing::Test {
protected:
    RespHandler rh {};
};

TEST_F(RespHandlerEncodeTest, CRLFOnly)
{
    rh.appendSimpleString("");
    std::vector<char> result { '+' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, SimpleString)
{
    rh.appendSimpleString("OK");
    std::vector<char> result { '+', 'O', 'K' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, BulkString)
{
    rh.appendBulkstring("OK");
    std::vector<char> result { '$', '2', '\r', '\n', 'O', 'K' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, null)
{
    rh.appendNull();
    std::vector<char> result { '$', static_cast<char>(-1) };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, SimpleStringIllegalCR)
{
    EXPECT_THROW(rh.appendSimpleString("OK\r"), std::invalid_argument);
}

TEST_F(RespHandlerEncodeTest, Error)
{
    constexpr std::string_view error { "Some Error Message" };
    rh.appendError(error);
    std::vector<char> result { '-' };
    appendChars(result, error);
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, SmallInteger)
{
    constexpr std::string_view n = "5";
    rh.appendInt(n);
    std::vector<char> result { ':' };
    appendChars(result, n);
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, LargeInteger)
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

TEST_F(RespHandlerEncodeTest, ArraySingleInt)
{
    constexpr std::string_view n = "1";
    rh.beginArray(1);
    rh.appendInt(n);
    std::vector<char> result { '*', '1', '\r', '\n', ':', '1' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, ArraySingleSimpleString)
{
    rh.beginArray(1);
    rh.appendSimpleString("OK");
    std::vector<char> result { '*', '1', '\r', '\n', '+', 'O', 'K' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, ArraySingleBulkString)
{
    rh.beginArray(1);
    rh.appendBulkstring("OK");
    std::vector<char> result { '*', '1', '\r', '\n', '$', '2', '\r', '\n', 'O', 'K' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, EmptyArray)
{
    rh.beginArray(0);
    std::vector<char> result { '*', '0', '\r', '\n' };
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, map)
{
    rh.beginMap(2);
    rh.appendKV("first", 1);
    rh.appendKV("second", 2);
    std::vector<char> result { '%', '2', '\r', '\n'};
    appendChars(result, "$5\r\nfirst");
    appendCRLF(result);
    result.push_back(':');
    result.push_back('1');
    appendCRLF(result);

    appendChars(result, "$6\r\nsecond");
    appendCRLF(result);
    result.push_back(':');
    result.push_back('2');
    appendCRLF(result);

    EXPECT_EQ(result, rh.getBuffer());
}