#include "json.h"

#include <gtest/gtest.h>
#include <stdexcept>

struct JsonParser : ::testing::Test
{
    json::parser parser{};
};

TEST_F(JsonParser, invalidEmptyFile)
{
    EXPECT_THROW(this->parser.parse("../data/step1/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step1Valid)
{
    json::ParseResult expected{};
    EXPECT_EQ(this->parser.parse("../data/step1/valid.json"), expected);
}

TEST_F(JsonParser, step2Invalid)
{
    EXPECT_THROW(this->parser.parse("../data/step2/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step2Invalid2)
{
    EXPECT_THROW(this->parser.parse("../data/step2/invalid2.json"), std::invalid_argument);
}

TEST_F(JsonParser, step2Valid)
{
    json::ParseResult expected{};
    expected.map_["\"key\""] = json::ParseResult{.val_ = "\"value\""};
    EXPECT_EQ(this->parser.parse("../data/step2/valid.json"), expected);
}

TEST_F(JsonParser, step2Valid2)
{
    json::ParseResult expected{};
    expected.map_["\"key\""] = json::ParseResult{.val_ = "\"value\""};
    expected.map_["\"key2\""] = json::ParseResult{.val_ = "\"value\""};
    EXPECT_EQ(this->parser.parse("../data/step2/valid2.json"), expected);
}

TEST_F(JsonParser, step3invalid)
{
    EXPECT_THROW(this->parser.parse("../data/step3/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step3valid)
{
    json::ParseResult expected{};
    expected.map_["\"key1\""] = json::ParseResult{.jsonValue = json::JsonElement::True};
    expected.map_["\"key2\""] = json::ParseResult{.jsonValue = json::JsonElement::False};
    expected.map_["\"key3\""] = json::ParseResult{.jsonValue = json::JsonElement::Null};
    expected.map_["\"key4\""] = json::ParseResult{.val_ = "\"value\""};
    expected.map_["\"key5\""] = json::ParseResult{.integer = "101"};
    EXPECT_EQ(this->parser.parse("../data/step3/valid.json"), expected);
}

TEST_F(JsonParser, step4valid)
{
    json::ParseResult expected{};
    expected.map_["\"key\""] = json::ParseResult{.jsonValue = json::JsonElement::True};
    expected.map_["\"key-n\""] = json::ParseResult{.integer = "101"};
    expected.map_["\"key-o\""] = json::ParseResult{};
    expected.map_["\"key-l\""] = json::ParseResult{};
    EXPECT_NO_THROW(this->parser.parse("../data/step4/valid.json"));
}

TEST_F(JsonParser, step4valid2)
{
    json::ParseResult expected{};
    expected.map_["\"key\""] = json::ParseResult{.val_ = "\"value\""};
    expected.map_["\"key-n\""] = json::ParseResult{.integer = "101"};
    json::ParseResult innerObject{};
    innerObject.map_["\"inner key\""] = json::ParseResult{.val_ = "\"inner value\""};
    expected.map_["\"key-o\""] = innerObject;
    json::ParseResult innerArray{};
    innerArray.array_.push_back(json::ParseResult{.val_ = "\"list value\""});
    expected.map_["\"key-l\""] = innerArray;
    EXPECT_EQ(this->parser.parse("../data/step4/valid2.json"), expected);
}

TEST_F(JsonParser, step4valid3)
{
    json::ParseResult expected{};
    expected.map_["\"key\""] = json::ParseResult{.val_ = "\"value\""};
    expected.map_["\"key-n\""] = json::ParseResult{.integer = "101"};
    json::ParseResult innerObject{};
    innerObject.map_["\"inner key\""] = json::ParseResult{.val_ = "\"inner value\""};
    expected.map_["\"key-o\""] = innerObject;
    json::ParseResult innerArray{};
    innerArray.array_.push_back(json::ParseResult{.val_ = "\"list value1\""});
    innerArray.array_.push_back(json::ParseResult{.val_ = "\"list value2\""});
    expected.map_["\"key-l\""] = innerArray;
    EXPECT_EQ(this->parser.parse("../data/step4/valid3.json"), expected);
}

TEST_F(JsonParser, step4valid4)
{
    json::ParseResult expected{};
    expected.map_["\"key\""] = json::ParseResult{.val_ = "\"value\""};
    expected.map_["\"key-n\""] = json::ParseResult{.integer = "101"};

    json::ParseResult innerObject{};
    innerObject.map_["\"inner key\""] = json::ParseResult{.val_ = "\"inner value\""};
    std::vector<json::ParseResult> inner_array{json::ParseResult{.val_="\"inner list1\""}};
    inner_array.push_back(json::ParseResult{.val_="\"inner list2\""});
    innerObject.map_["\"inner key2\""] = json::ParseResult{.array_ = inner_array};
    json::ParseResult nestedInnerObject{};
    nestedInnerObject.map_["\"nested key\""] = json::ParseResult{.val_="\"some nested object\""};
    innerObject.map_["\"inner key3\""] = nestedInnerObject;
    expected.map_["\"key-o\""] = innerObject;


    json::ParseResult innerArray{};
    innerArray.array_.push_back(json::ParseResult{.val_ = "\"list value\""});
    innerArray.array_.push_back(json::ParseResult{.val_ = "\"list value\""});
    expected.map_["\"key-l\""] = innerArray;
    EXPECT_EQ(this->parser.parse("../data/step4/valid4.json"), expected);
}

TEST_F(JsonParser, step4invalid)
{
    EXPECT_THROW(this->parser.parse("../data/step4/invalid.json"), std::invalid_argument);
}

TEST_F(JsonParser, step4invalid2)
{
    EXPECT_THROW(this->parser.parse("../data/step4/invalid2.json"), std::invalid_argument);
}

TEST_F(JsonParser, step4invalid3)
{
    EXPECT_THROW(this->parser.parse("../data/step4/invalid3.json"), std::invalid_argument);
}

TEST_F(JsonParser, step4invalid4)
{
    EXPECT_THROW(this->parser.parse("../data/step4/invalid4.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail1)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail1.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail2)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail2.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail3)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail3.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail4)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail4.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail5)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail5.json"), std::invalid_argument);
}


TEST_F(JsonParser, testFail6)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail6.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail7)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail7.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail8)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail8.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail9)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail9.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail10)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail10.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail11)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail11.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail12)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail12.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail13)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail13.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail14)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail14.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail15)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail15.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail16)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail16.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail17)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail17.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail18)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail18.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail19)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail19.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail20)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail20.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail21)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail21.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail22)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail22.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail23)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail23.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail24)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail24.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail25)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail25.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail26)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail26.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail27)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail27.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail28)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail28.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail29)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail29.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail30)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail30.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail31)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail31.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail32)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail32.json"), std::invalid_argument);
}

TEST_F(JsonParser, testFail33)
{
    EXPECT_THROW(this->parser.parse("../data/test/fail33.json"), std::invalid_argument);
}

TEST_F(JsonParser, testPass1)
{
    EXPECT_NO_THROW(this->parser.parse("../data/test/pass1.json"));
}

TEST_F(JsonParser, testPass2)
{
    EXPECT_NO_THROW(this->parser.parse("../data/test/pass2.json"));
}

TEST_F(JsonParser, testPass3)
{
    json::ParseResult expected{};
    json::ParseResult innerMap{};
    innerMap.map_["\"The outermost value\""] = json::ParseResult{.val_ = "\"must be an object or array.\""};
    innerMap.map_["\"In this test\""] = json::ParseResult{.val_ = "\"It is an object.\""};

    expected.map_["\"JSON Test Pattern pass3\""] = innerMap;
    
    EXPECT_EQ(this->parser.parse("../data/test/pass3.json"), expected);
}