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
    EXPECT_EQ(std::string_view { "OK" }, rh.decode(simpleString).second.simpleString_);
}

TEST_F(RespHandlerDecodeTest, PingString)
{
    constexpr std::string_view simpleString = { "+PING\r\n" };
    EXPECT_EQ(std::string_view { "PING" }, rh.decode(simpleString).second.simpleString_);
}

TEST_F(RespHandlerDecodeTest, IllegalString)
{
    constexpr std::string_view simpleString = { "+OK" };
    EXPECT_THROW(static_cast<void>(rh.decode(simpleString)), std::invalid_argument);
}

TEST_F(RespHandlerDecodeTest, Error)
{
    EXPECT_EQ(std::string_view { "My Error" }, rh.decode("-My Error\r\n").second.error_);
}

TEST_F(RespHandlerDecodeTest, IntegerNoSign)
{
    EXPECT_EQ(std::string_view { "1000" }, rh.decode(":1000\r\n").second.integer_);
}

TEST_F(RespHandlerDecodeTest, IntegerNegative)
{
    EXPECT_EQ(std::string_view { "-1000" }, rh.decode(":-1000\r\n").second.integer_);
}

TEST_F(RespHandlerDecodeTest, IntegerPos)
{
    EXPECT_EQ(std::string_view { "+1000" }, rh.decode(":+1000\r\n").second.integer_);
}

TEST_F(RespHandlerDecodeTest, BulkString)
{
    EXPECT_EQ(std::string_view { "ping" }, rh.decode("$4\r\nping\r\n").second.bulkString_);
}

TEST_F(RespHandlerDecodeTest, BulkStringLonger)
{
    EXPECT_EQ(std::string_view { "hello world" }, rh.decode("$11\r\nhello world\r\n").second.bulkString_);
}

TEST_F(RespHandlerDecodeTest, Null)
{
    EXPECT_EQ(std::string_view { "null" }, rh.decode("$-1\r\n").second.bulkString_);
}

TEST_F(RespHandlerDecodeTest, ArraySingleInt)
{
    EXPECT_EQ(RedisRespRes{.array_ = {RedisRespRes{.integer_ = "1"}}}, rh.decode("*1\r\n:1\r\n").second);
}

TEST_F(RespHandlerDecodeTest, ArrayTwoInts)
{
    RedisRespRes redisRespInt1 = RedisRespRes{.integer_ = "1"};
    RedisRespRes redisRespInt2 = RedisRespRes{.integer_ = "2"};
    RedisRespRes redisRespArr = RedisRespRes{.array_ = {redisRespInt1, redisRespInt2}};
    EXPECT_EQ(redisRespArr, rh.decode("*2\r\n:1\r\n:2\r\n").second);
}


TEST_F(RespHandlerDecodeTest, ArrayTwoIntsBulkString)
{
    RedisRespRes redisRespInt1 = RedisRespRes{.integer_ = "1"};
    RedisRespRes redisRespInt2 = RedisRespRes{.integer_ = "2"};
    RedisRespRes redisRespBulkStr = RedisRespRes{.bulkString_ = "PING"};
    RedisRespRes redisRespArr = RedisRespRes{.array_ = {redisRespInt1, redisRespBulkStr, redisRespInt2}};
    EXPECT_EQ(redisRespArr, rh.decode("*3\r\n:1\r\n$4\r\nPING\r\n:2\r\n").second);
}

TEST_F(RespHandlerDecodeTest, ArrayTwoIntsLongBulkString)
{
    RedisRespRes redisRespInt1 = RedisRespRes{.integer_ = "1"};
    RedisRespRes redisRespInt2 = RedisRespRes{.integer_ = "2"};
    RedisRespRes redisRespBulkStr = RedisRespRes{.bulkString_ = "hello world"};
    RedisRespRes redisRespArr = RedisRespRes{.array_ = {redisRespInt1, redisRespBulkStr, redisRespInt2}};
    EXPECT_EQ(redisRespArr, rh.decode("*3\r\n:1\r\n$11\r\nhello world\r\n:2\r\n").second);
}

TEST_F(RespHandlerDecodeTest, NestedArr)
{
    RedisRespRes redisRespInt = RedisRespRes{.integer_ = "2"};
    RedisRespRes redisRespArr = RedisRespRes{.array_ = {RedisRespRes{.array_ = {redisRespInt}}}};
    EXPECT_EQ(redisRespArr, rh.decode("*1\r\n*1\r\n:2\r\n\r\n").second);
}