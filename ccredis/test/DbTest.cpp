#include "Database.h"

#include <gtest/gtest.h>

class DbTest : public testing::Test {
protected:
    Db db {};
};

TEST_F(DbTest, Basic)
{
    db.set("key", "value");
    EXPECT_EQ(ValueType { "value" }, db.get("key").value());
}

TEST_F(DbTest, NotExisting)
{
    db.set("key", "value");
    EXPECT_EQ(ValueType { "value" }, db.get("key").value());

    EXPECT_FALSE(db.get("otherkey").has_value());

    EXPECT_EQ(ValueType { "value" }, db.get("key").value());
}

TEST_F(DbTest, ExistWithTime)
{
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count();
    db.set("key", "value", now+100);
    EXPECT_EQ(ValueType { "value" }, db.get("key").value());
}

TEST_F(DbTest, TimeExpires)
{
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count();
    db.set("key", "value", now-100);
    EXPECT_FALSE(db.get("key").has_value());
}