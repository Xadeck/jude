#include "xdk/ltemplate/processor.h"

#include "absl/strings/str_cat.h"
#include "xdk/lua/stack.h"
#include "xdk/lua/state.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <string>

namespace xdk {
namespace ltemplate {
namespace {

using ::testing::ElementsAre;

class ProcessorTest : public ::testing::Test {
protected:
  std::string Process(absl::string_view source) {
    Processor processor(source);
    return Read(Processor::Read, L, &processor);
  }

  xdk::lua::State L;

private:
  static std::string Read(lua_Reader reader, lua_State *L, void *data) {
    std::string result;
    size_t size;
    while (const char *read = reader(L, data, &size)) {
      absl::StrAppend(&result, absl::string_view(read, size));
      if (!size)
        break;
    }
    return result;
  }
};

TEST_F(ProcessorTest, Text) {
  ASSERT_EQ(Process(R"LT(some text)LT"), R"LUA(_s([[some text]]))LUA");
}

TEST_F(ProcessorTest, Expression) {
  ASSERT_EQ(Process(R"LT(some {{3+4}} expression)LT"),
            R"LUA(_s([[some ]])_e(3+4)_s([[ expression]]))LUA");
}

TEST_F(ProcessorTest, Statement) {
  ASSERT_EQ(Process(R"LT(some {%x=4%} statement)LT"),
            R"LUA(_s([[some ]]) x=4 _s([[ statement]]))LUA");
}

TEST_F(ProcessorTest, WhitespacesAreStripped) {
  EXPECT_EQ(Process(R"(
  first line
  {%  x=1  %}
  second line
  {%- x=2 -%}
  third line)"),
            R"LUA(_s([[
  first line
  ]])   x=1   _s([[
  second line]])  x=2  _s([[  third line]]))LUA");
}

} // namespace
} // namespace ltemplate
} // namespace xdk
