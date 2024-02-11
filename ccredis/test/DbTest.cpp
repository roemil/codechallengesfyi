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
