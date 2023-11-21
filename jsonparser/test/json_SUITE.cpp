#include "json.h"

#include <gtest/gtest.h>
#include <stdexcept>

using namespace json;

struct JsonParser : ::testing::Test
{
    parser parser{};
};

TEST_F(JsonParser, invalidEmptyFile)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step1/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step1Valid)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step1/valid.json"));
}

TEST_F(JsonParser, step2Invalid)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step2/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step2Invalid2)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step2/invalid2.json"), std::invalid_argument);
}

TEST_F(JsonParser, step2Valid)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step2/valid.json"));
}

TEST_F(JsonParser, step2Valid2)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step2/valid2.json"));
}

TEST_F(JsonParser, step3invalid)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step3/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step3valid)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step3/valid.json"));
}

TEST_F(JsonParser, step4valid)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step4/valid.json"));
}

TEST_F(JsonParser, step4valid2)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step4/valid2.json"));
}

TEST_F(JsonParser, step4valid3)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step4/valid3.json"));
}

TEST_F(JsonParser, step4valid4)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/step4/valid4.json"));
}

TEST_F(JsonParser, step4invalid)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step4/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step4invalid2)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step4/invalid2.json"), std::invalid_argument);
}

TEST_F(JsonParser, step4invalid3)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step4/invalid3.json"), std::invalid_argument);
}

TEST_F(JsonParser, step4invalid4)
{
    EXPECT_THROW(this->parser.isValidJson("../data/step4/invalid4.json"), std::invalid_argument);
}
