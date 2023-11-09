#include "json.h"

#include <gtest/gtest.h>

TEST(JsonParser, invalidEmptyFile)
{
    Json json{};
    json.readFile("../data/step1/invalid.json");
    EXPECT_FALSE(json.isValidJson());
}

TEST(JsonParser, step1Valid)
{
    Json json{};
    json.readFile("../data/step1/valid.json");
    EXPECT_TRUE(json.isValidJson());
}

TEST(JsonParser, step2Invalid)
{
    Json json{};
    json.readFile("../data/step2/invalid.json");
    EXPECT_FALSE(json.isValidJson());
}

TEST(JsonParser, step2Invalid2)
{
    Json json{};
    json.readFile("../data/step2/invalid2.json");
    EXPECT_FALSE(json.isValidJson());
}

TEST(JsonParser, step2Valid)
{
    Json json{};
    json.readFile("../data/step2/valid.json");
    EXPECT_TRUE(json.isValidJson());
}

TEST(JsonParser, step2Valid2)
{
    Json json{};
    json.readFile("../data/step2/valid2.json");
    EXPECT_TRUE(json.isValidJson());
}

TEST(JsonParser, step3invalid)
{
    Json json{};
    json.readFile("../data/step3/invalid.json");
    EXPECT_FALSE(json.isValidJson());
}

TEST(JsonParser, step3valid)
{
    Json json{};
    json.readFile("../data/step3/valid.json");
    EXPECT_TRUE(json.isValidJson());
}

TEST(JsonParser, step4valid)
{
    Json json{};
    json.readFile("../data/step4/valid.json");
    EXPECT_TRUE(json.isValidJson());
}

TEST(JsonParser, step4valid2)
{
    Json json{};
    json.readFile("../data/step4/valid2.json");
    EXPECT_TRUE(json.isValidJson());
}

TEST(JsonParser, step4invalid)
{
    Json json{};
    json.readFile("../data/step4/invalid.json");
    EXPECT_FALSE(json.isValidJson());
}