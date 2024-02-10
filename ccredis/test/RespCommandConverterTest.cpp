#include "Commands.h"
#include "RespDecoder.h"
#include "Resp.h"

#include <optional>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

class RespCommandConverterTest : public testing::Test {
protected:
    RespDecoder rh {};
};

TEST_F(RespCommandConverterTest, UnknownCommand)
{
    RedisRespRes rawCommand { .string_ = "SomeCmd" };
    EXPECT_NO_THROW(std::get<CommandUnknown>(rh.convertToCommands(rawCommand)[0]));
}

TEST_F(RespCommandConverterTest, Ping)
{
    RedisRespRes rawCommand { .string_ = "PING" };
    EXPECT_NO_THROW(std::get<CommandPing>(rh.convertToCommands(rawCommand)[0]));
}

TEST_F(RespCommandConverterTest, ArrayCommandWithPayload)
{
    RedisRespRes rawCommand { .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "HELLO" } }, RedisRespRes { .string_ { "3" } } } };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ(1, commands.size());
    EXPECT_EQ("3", std::get<CommandHello>(commands[0]).version_);
}

TEST_F(RespCommandConverterTest, SET)
{
    RedisRespRes rawCommand { .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "SET" } }, RedisRespRes { .string_ { "key" } }, RedisRespRes { .string_ { "value" } } } };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("key", std::get<CommandSet>(commands[0]).key_);
    EXPECT_EQ("value", std::get<CommandSet>(commands[0]).value_);
}