#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>
#include <vector>

#include "RespHandler.h"

class RespHandlerDecodeTest : public testing::Test {
protected:
    RespHandler rh {};
};

TEST_F(RespHandlerDecodeTest, SimpleString)
{
    constexpr std::string_view simpleString = { "+OK\r\n" };
    EXPECT_EQ(std::string_view { "OK" }, rh.decode(simpleString).second);
}

TEST_F(RespHandlerDecodeTest, PingString)
{
    constexpr std::string_view simpleString = { "+PING\r\n" };
    EXPECT_EQ(std::string_view { "PING" }, rh.decode(simpleString).second);
}

TEST_F(RespHandlerDecodeTest, IllegalString)
{
    constexpr std::string_view simpleString = { "+OK" };
    EXPECT_THROW(static_cast<void>(rh.decode(simpleString)), std::invalid_argument);
}

TEST_F(RespHandlerDecodeTest, Error)
{
    EXPECT_EQ(std::string_view { "My Error" }, rh.decode("-My Error\r\n").second);
}

TEST_F(RespHandlerDecodeTest, IntegerNoSign)
{
    EXPECT_EQ(std::string_view { "1000" }, rh.decode(":1000\r\n").second);
}

TEST_F(RespHandlerDecodeTest, IntegerNegative)
{
    EXPECT_EQ(std::string_view { "-1000" }, rh.decode(":-1000\r\n").second);
}

TEST_F(RespHandlerDecodeTest, IntegerPos)
{
    EXPECT_EQ(std::string_view { "+1000" }, rh.decode(":+1000\r\n").second);
}

TEST_F(RespHandlerDecodeTest, BulkString)
{
    EXPECT_EQ(std::string_view { "ping" }, rh.decode("$4\r\nping\r\n").second);
}

TEST_F(RespHandlerDecodeTest, BulkStringLonger)
{
    EXPECT_EQ(std::string_view { "hello world" }, rh.decode("$11\r\nhello world\r\n").second);
}

TEST_F(RespHandlerDecodeTest, Null)
{
    EXPECT_EQ(std::string_view { "null" }, rh.decode("$-1\r\n").second);
}

TEST_F(RespHandlerDecodeTest, ArraySingleInt)
{
    EXPECT_EQ(std::string_view { ";1" }, rh.decode("*1\r\n:1\r\n").second);
}

TEST_F(RespHandlerDecodeTest, ArrayTwoInts)
{
    EXPECT_EQ(std::string_view { ";1;2" }, rh.decode("*2\r\n:1\r\n:2\r\n").second);
}

TEST_F(RespHandlerDecodeTest, ArrayTwoIntsBulkString)
{
    EXPECT_EQ(std::string_view { ";1;PING;2" }, rh.decode("*3\r\n:1\r\n$4\r\nPING\r\n:2\r\n").second);
}

TEST_F(RespHandlerDecodeTest, ArrayTwoIntsLongBulkString)
{
    EXPECT_EQ(std::string_view { ";1;hello world;2" }, rh.decode("*3\r\n:1\r\n$11\r\nhello world\r\n:2\r\n").second);
}

TEST_F(RespHandlerDecodeTest, NestedArr)
{
    EXPECT_EQ(std::string_view { ";;2" }, rh.decode("*1\r\n*1\r\n:2\r\n\r\n").second);
}