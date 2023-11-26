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

TEST_F(JsonParser, testFail1)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail1.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail2)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail2.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail3)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail3.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail4)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail4.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail5)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail5.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail6)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail6.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail7)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail7.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail8)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail8.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail9)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail9.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail10)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail10.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail11)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail11.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail12)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail12.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail13)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail13.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail14)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail14.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail15)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail15.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail16)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail16.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail17)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail17.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail18)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail18.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail19)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail19.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail20)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail20.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail21)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail21.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail22)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail22.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail23)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail23.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail24)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail24.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail25)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail25.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail26)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail26.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail27)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail27.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail28)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail28.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail29)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail29.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail30)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail30.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail31)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail31.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail32)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail32.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail33)
{
    EXPECT_THROW(this->parser.isValidJson("../data/test/fail33.json"), std::invalid_argument);
}

TEST_F(JsonParser, testPass1)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/test/pass1.json"));
}

TEST_F(JsonParser, testPass2)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/test/pass2.json"));
}

TEST_F(JsonParser, testPass3)
{
    EXPECT_NO_THROW(this->parser.isValidJson("../data/test/pass3.json"));
}