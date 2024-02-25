#include "Database.h"

#include <gtest/gtest.h>

class DbTest : public testing::Test {
protected:
    Db db {};
};

TEST_F(DbTest, Basic)
{
    db.set("key", "value");
    EXPECT_EQ("value", db.get("key").value());
}


TEST_F(DbTest, NotExisting)
{
    db.set("key", "value");
    EXPECT_EQ("value", db.get("key").value());

    EXPECT_FALSE(db.get("otherkey").has_value());

    EXPECT_EQ("value", db.get("key").value());
}
