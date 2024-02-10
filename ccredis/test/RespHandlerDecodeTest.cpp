#include <gtest/gtest.h>

#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "Resp.h"
#include "RespDecoder.h"

class RespDecoderTest : public testing::Test {
protected:
    RespDecoder rh {};
};

TEST_F(RespDecoderTest, SimpleString)
{
    constexpr std::string_view simpleString = { "+OK\r\n" };
    EXPECT_EQ(std::string_view { "OK" }, rh.decode(simpleString).second.string_);
}

TEST_F(RespDecoderTest, PingString)
{
    constexpr std::string_view simpleString = { "+PING\r\n" };
    EXPECT_EQ(std::string_view { "PING" }, rh.decode(simpleString).second.string_);
}

TEST_F(RespDecoderTest, IllegalString)
{
    constexpr std::string_view simpleString = { "+OK" };
    EXPECT_THROW(static_cast<void>(rh.decode(simpleString)), std::invalid_argument);
}

TEST_F(RespDecoderTest, Error)
{
    EXPECT_EQ(std::string_view { "My Error" }, rh.decode("-My Error\r\n").second.error_);
}

TEST_F(RespDecoderTest, IntegerNoSign)
{
    EXPECT_EQ(1000, rh.decode(":1000\r\n").second.integer_);
}

TEST_F(RespDecoderTest, IntegerNegative)
{
    EXPECT_EQ(-1000, rh.decode(":-1000\r\n").second.integer_);
}

TEST_F(RespDecoderTest, IntegerPos)
{
    EXPECT_EQ(1000, rh.decode(":+1000\r\n").second.integer_);
}

TEST_F(RespDecoderTest, BulkString)
{
    EXPECT_EQ(std::string_view { "ping" }, rh.decode("$4\r\nping\r\n").second.string_);
}

TEST_F(RespDecoderTest, BulkStringLonger)
{
    EXPECT_EQ(std::string_view { "hello world" }, rh.decode("$11\r\nhello world\r\n").second.string_);
}

TEST_F(RespDecoderTest, Null)
{
    EXPECT_EQ(std::string_view { "null" }, rh.decode("$-1\r\n").second.string_);
}

TEST_F(RespDecoderTest, ArraySingleInt)
{
    EXPECT_EQ(RedisRespRes { .array_ = { std::vector<RedisRespRes> { RedisRespRes { .integer_ = 1 } } } }, rh.decode("*1\r\n:1\r\n").second);
}

TEST_F(RespDecoderTest, ArrayTwoInts)
{
    RedisRespRes redisRespInt1 = RedisRespRes { .integer_ = 1 };
    RedisRespRes redisRespInt2 = RedisRespRes { .integer_ = 2 };
    RedisRespRes redisRespArr = RedisRespRes { .array_ = std::vector<RedisRespRes> { redisRespInt1, redisRespInt2 } };
    EXPECT_EQ(redisRespArr, rh.decode("*2\r\n:1\r\n:2\r\n").second);
}

TEST_F(RespDecoderTest, ArrayTwoIntsBulkString)
{
    RedisRespRes redisRespInt1 = RedisRespRes { .integer_ = 1 };
    RedisRespRes redisRespInt2 = RedisRespRes { .integer_ = 2 };
    RedisRespRes redisRespBulkStr = RedisRespRes { .string_ = "PING" };
    RedisRespRes redisRespArr = RedisRespRes { .array_ = std::vector<RedisRespRes> { redisRespInt1, redisRespBulkStr, redisRespInt2 } };
    EXPECT_EQ(redisRespArr, rh.decode("*3\r\n:1\r\n$4\r\nPING\r\n:2\r\n").second);
}

TEST_F(RespDecoderTest, ArrayTwoIntsLongBulkString)
{
    RedisRespRes redisRespInt1 = RedisRespRes { .integer_ = 1 };
    RedisRespRes redisRespInt2 = RedisRespRes { .integer_ = 2 };
    RedisRespRes redisRespBulkStr = RedisRespRes { .string_ = "hello world" };
    RedisRespRes redisRespArr = RedisRespRes { .array_ = std::vector<RedisRespRes> { redisRespInt1, redisRespBulkStr, redisRespInt2 } };
    EXPECT_EQ(redisRespArr, rh.decode("*3\r\n:1\r\n$11\r\nhello world\r\n:2\r\n").second);
}

TEST_F(RespDecoderTest, NestedArr)
{
    RedisRespRes redisRespInt = RedisRespRes { .integer_ = 2 };
    RedisRespRes redisRespArr = RedisRespRes { .array_ = std::vector<RedisRespRes> { RedisRespRes { .array_ = std::vector<RedisRespRes> { redisRespInt } } } };
    EXPECT_EQ(redisRespArr, rh.decode("*1\r\n*1\r\n:2\r\n\r\n").second);
}

TEST_F(RespDecoderTest, Map)
{
    RedisRespRes redisRespInt = RedisRespRes { .integer_ = 2 };
    std::map<std::string_view, RedisRespRes> map {};
    map["key"] = redisRespInt;
    RedisRespRes redisRespMap = RedisRespRes { .map_ = map };
    EXPECT_EQ(redisRespMap, rh.decode("%1\r\n+key\r\n:2\r\n\r\n").second);
}

TEST_F(RespDecoderTest, HelloHandshake)
{
    RedisRespRes redisRespInt = RedisRespRes { .integer_ = 3 };
    RedisRespRes redisRespHello = RedisRespRes { .string_ = "HELLO" };
    RedisRespRes redisRespArr = RedisRespRes { .array_ = std::vector<RedisRespRes> { redisRespHello, redisRespInt } };
    EXPECT_EQ(redisRespArr, rh.decode("*2\r\n$5\r\nHELLO\r\n:3\r\n\r\n").second);
}