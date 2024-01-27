#include <_types/_uint8_t.h>
#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>
#include <sys/types.h>
#include <vector>

#include "RespHandler.h"

namespace {
void appendCRLF(std::vector<uint8_t>& vec)
{
    vec.push_back(static_cast<uint8_t>(0x0d));
    vec.push_back(static_cast<uint8_t>(0x0A));
}

void appendChars(std::vector<uint8_t>& output, std::string_view input)
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

class RespHandlerEncodeTest : public testing::Test {
protected:
    RespHandler rh {};
};

TEST_F(RespHandlerEncodeTest, CRLFOnly)
{
    rh.appendSimpleString("");
    std::vector<uint8_t> result { '+' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, SimpleString)
{
    rh.appendSimpleString("OK");
    std::vector<uint8_t> result { '+', 'O', 'K' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, BulkString)
{
    rh.appendBulkstring("OK");
    std::vector<uint8_t> result { '$', static_cast<uint8_t>(2) , '\r', '\n' , 'O', 'K' };
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, null)
{
    rh.appendNull();
    std::vector<uint8_t> result { '$', static_cast<uint8_t>(-1) };
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
    std::vector<uint8_t> result { '-' };
    appendChars(result, error);
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, SmallInteger)
{
    constexpr std::string_view n = "5";
    rh.appendInt(n);
    std::vector<uint8_t> result { ':' };
    appendChars(result, n);
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, LargeInteger)
{
    constexpr std::string_view n = "1000";
    rh.appendInt(n);
    std::vector<uint8_t> result { ':' };
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
    std::vector<uint8_t> result { '*' , 1, '\r' , '\n' , ':', '1'};
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, ArraySingleSimpleString)
{
    rh.beginArray(1);
    rh.appendSimpleString("OK");
    std::vector<uint8_t> result { '*' , 1, '\r' , '\n' , '+', 'O', 'K'};
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}

TEST_F(RespHandlerEncodeTest, ArraySingleBulkString)
{
    rh.beginArray(1);
    rh.appendBulkstring("OK");
    std::vector<uint8_t> result { '*' , 1, '\r' , '\n' , '$', 2, '\r' , '\n'  ,'O', 'K'};
    appendCRLF(result);
    EXPECT_EQ(result, rh.getBuffer());
}