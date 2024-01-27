#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>
#include <vector>

#include "RespHandler.h"

// namespace {
// void appendCRLF(std::vector<char>& vec)
// {
//     vec.push_back(static_cast<char>(0x0d));
//     vec.push_back(static_cast<char>(0x0A));
// }

// void appendChars(std::vector<char>& output, std::string_view input)
// {
//     for (const auto& c : input) {
//         output.push_back(c);
//     }
// }

// } // namespace

/*

Test list

For Simple Strings, the first byte of the reply is "+"
For Errors, the first byte of the reply is "-"
For Integers, the first byte of the reply is ":"
For Bulk Strings, the first byte of the reply is "$"
For Arrays, the first byte of the reply is "*"


*/

class RespHandlerDecodeTest : public testing::Test {
protected:
    RespHandler rh {};
};

TEST_F(RespHandlerDecodeTest, SimpleString)
{
    constexpr std::string_view simpleString = { "+OK\r\n" };
    EXPECT_EQ(std::string_view { "OK" }, RespHandler::decode(simpleString));
}

TEST_F(RespHandlerDecodeTest, IllegalString)
{
    constexpr std::string_view simpleString = { "+OK" };
    EXPECT_THROW(static_cast<void>(RespHandler::decode(simpleString)), std::invalid_argument);
}

TEST_F(RespHandlerDecodeTest, Error)
{
    EXPECT_EQ(std::string_view { "My Error" }, rh.decode("-My Error\r\n"));
}

TEST_F(RespHandlerDecodeTest, IntegerNoSign)
{
    EXPECT_EQ(std::string_view { "1000" }, rh.decode(":1000\r\n"));
}

TEST_F(RespHandlerDecodeTest, IntegerNegative)
{
    EXPECT_EQ(std::string_view { "-1000" }, rh.decode(":-1000\r\n"));
}

TEST_F(RespHandlerDecodeTest, IntegerPos)
{
    EXPECT_EQ(std::string_view { "+1000" }, rh.decode(":+1000\r\n"));
}

TEST_F(RespHandlerDecodeTest, BulkString)
{
    EXPECT_EQ(std::string_view { "ping" }, rh.decode("$4\r\nping\r\n"));
}

TEST_F(RespHandlerDecodeTest, BulkStringLonger)
{
    EXPECT_EQ(std::string_view { "hello world" }, rh.decode("$11\r\nhello world\r\n"));
}

TEST_F(RespHandlerDecodeTest, Null)
{
    EXPECT_EQ(std::string_view { "null" }, rh.decode("$-1\r\n"));
}

TEST_F(RespHandlerDecodeTest, ArraySingleInt)
{
    EXPECT_EQ(std::string_view { "1" }, rh.decode("*1\r\n:1\r\n"));
}