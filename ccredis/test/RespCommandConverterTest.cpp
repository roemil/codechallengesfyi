#include "Commands.h"
#include "Resp.h"
#include "RespDecoder.h"

#include <chrono>
#include <memory>
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
    EXPECT_NO_THROW(
        std::get<CommandUnknown>(rh.convertToCommands(rawCommand)[0]));
}

TEST_F(RespCommandConverterTest, Ping)
{
    RedisRespRes rawCommand { .string_ = "PING" };
    EXPECT_NO_THROW(std::get<CommandPing>(rh.convertToCommands(rawCommand)[0]));
}

TEST_F(RespCommandConverterTest, PingWithMessage)
{
    RedisRespRes rawCommand { .array_ = std::vector<RedisRespRes> {
                                  RedisRespRes { .string_ { "PING" } },
                                  RedisRespRes { .string_ { "Hello world" } } } };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("Hello world", std::get<CommandPing>(commands[0]).value_);
}

TEST_F(RespCommandConverterTest, ArrayCommandWithPayload)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "HELLO" } },
            RedisRespRes { .string_ { "3" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ(1, commands.size());
    EXPECT_EQ("3", std::get<CommandHello>(commands[0]).version_);
}

TEST_F(RespCommandConverterTest, SET)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "SET" } },
            RedisRespRes { .string_ { "key" } },
            RedisRespRes { .string_ { "value" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("key", std::get<CommandSet>(commands[0]).key_);
    EXPECT_EQ("value", std::get<CommandSet>(commands[0]).value_);
}

TEST_F(RespCommandConverterTest, GET)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "GET" } },
            RedisRespRes { .string_ { "key" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("key", std::get<CommandGet>(commands[0]).key_);
}

TEST_F(RespCommandConverterTest, EXISTS)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "EXISTS" } },
            RedisRespRes { .string_ { "key" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("key", std::get<CommandExists>(commands[0]).key_);
}

TEST_F(RespCommandConverterTest, EXISTSInvalid)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "EXISTS" } },
            RedisRespRes { .string_ { "" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("Missing key", std::get<CommandInvalid>(commands[0]).errorString);
}

TEST_F(RespCommandConverterTest, SETwithEx)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> {
            RedisRespRes { .string_ { "SET" } }, RedisRespRes { .string_ { "key" } },
            RedisRespRes { .string_ { "value" } }, RedisRespRes { .string_ { "EX" } },
            RedisRespRes { .string_ { "100" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("key", std::get<CommandSet>(commands[0]).key_);
    EXPECT_EQ("value", std::get<CommandSet>(commands[0]).value_);
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
    std::chrono::time_point<std::chrono::system_clock> time {
        std::chrono::seconds { 100 } + now
    };

    // MAake sure we are at least not 1 seconds off. This is probably not stable
    // and we should mock system time.
    EXPECT_GE(time, std::get<CommandSet>(commands[0]).expire);
}

TEST_F(RespCommandConverterTest, SETwithPx)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> {
            RedisRespRes { .string_ { "SET" } }, RedisRespRes { .string_ { "key" } },
            RedisRespRes { .string_ { "value" } }, RedisRespRes { .string_ { "PX" } },
            RedisRespRes { .string_ { "100" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("key", std::get<CommandSet>(commands[0]).key_);
    EXPECT_EQ("value", std::get<CommandSet>(commands[0]).value_);
    using Ms = std::chrono::milliseconds;
    const auto now = std::chrono::duration_cast<Ms>(
        std::chrono::system_clock::now().time_since_epoch());
    std::chrono::time_point<std::chrono::system_clock> time { Ms { 100 } + now };

    // MAake sure we are at least not 1 seconds off. This is probably not stable
    // and we should mock system time.
    EXPECT_GE(time, std::get<CommandSet>(commands[0]).expire);
}

TEST_F(RespCommandConverterTest, INCR)
{
    RedisRespRes rawCommand {
        .array_ = std::vector<RedisRespRes> { RedisRespRes { .string_ { "INCR" } },
            RedisRespRes { .string_ { "key" } } }
    };
    const auto commands = rh.convertToCommands(rawCommand);
    EXPECT_EQ("key", std::get<CommandIncr>(commands[0]).key_);
}