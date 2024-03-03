#include "Database.h"

#include <chrono>

#include <gtest/gtest.h>

class DbTest : public testing::Test {
protected:
  Db db{};
};

TEST_F(DbTest, Basic) {
  db.set("key", "value");
  EXPECT_EQ(ValueType{"value"}, db.get("key").value());
}

TEST_F(DbTest, NotExisting) {
  db.set("key", "value");
  EXPECT_EQ(ValueType{"value"}, db.get("key").value());

  EXPECT_FALSE(db.get("otherkey").has_value());

  EXPECT_EQ(ValueType{"value"}, db.get("key").value());
}

TEST_F(DbTest, ExistWithTime) {

const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
 std::chrono::time_point<std::chrono::system_clock> expires{std::chrono::seconds{100} + now};
       
  db.set("key", "value", expires);
  EXPECT_EQ(ValueType{"value"}, db.get("key").value());
}

TEST_F(DbTest, TimeExpires) {
const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
 std::chrono::time_point<std::chrono::system_clock> expires{now - std::chrono::milliseconds{100}};
  db.set("key", "value", expires);
  EXPECT_FALSE(db.get("key").has_value());
}