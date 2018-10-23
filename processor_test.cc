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

TEST_F(ProcessorTest, EmptyStringWorks) { ASSERT_EQ(Process(""), ""); }

TEST_F(ProcessorTest, StrayRBracesArePreserved) {
  ASSERT_EQ(Process(R"LT(some }} in a text)LT"),
            R"LUA(_s([[some }} in a text]]))LUA");
}

TEST_F(ProcessorTest, StrayRPercentBraceArePreserved) {
  ASSERT_EQ(Process(R"LT(some %} in a text)LT"),
            R"LUA(_s([[some %} in a text]]))LUA");
}

TEST_F(ProcessorTest, ExpressionsWork) {
  ASSERT_EQ(Process(R"LT(some {{3+4}} expression)LT"),
            R"LUA(_s([[some ]])_e(3+4)_s([[ expression]]))LUA");
  ASSERT_EQ(Process(R"LT({{expression}} at start)LT"),
            R"LUA(_e(expression)_s([[ at start]]))LUA");
  ASSERT_EQ(Process(R"LT(expression {{at end}})LT"),
            R"LUA(_s([[expression ]])_e(at end))LUA");
}

TEST_F(ProcessorTest, StringsInExpressionsWork) {
  ASSERT_EQ(Process(R"LT({{with "string \" }}" expression}})LT"),
            R"LUA(_e(with "string \" }}" expression))LUA");
  ASSERT_EQ(Process(R"LT({{with 'string \' }}' expression}})LT"),
            R"LUA(_e(with 'string \' }}' expression))LUA");
}

TEST_F(ProcessorTest, StatementsWork) {
  ASSERT_EQ(Process(R"LT(some {%3+4%} statement)LT"),
            R"LUA(_s([[some ]]) 3+4 _s([[ statement]]))LUA");
  ASSERT_EQ(Process(R"LT({%statement%} at start)LT"),
            R"LUA( statement _s([[ at start]]))LUA");
  ASSERT_EQ(Process(R"LT(statement {%at end%})LT"),
            R"LUA(_s([[statement ]]) at end )LUA");
}

TEST_F(ProcessorTest, StringsInStatementsWork) {
  ASSERT_EQ(Process(R"LT({%with "string \" %}" statement%})LT"),
            R"LUA( with "string \" %}" statement )LUA");
  ASSERT_EQ(Process(R"LT({%with 'string \' %}' statement%})LT"),
            R"LUA( with 'string \' %}' statement )LUA");
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

TEST_F(ProcessorTest, UnfinishedExpressionIsClosed) {
  EXPECT_EQ(Process(R"(unfinished {{expression)"),
            R"LUA(_s([[unfinished ]])_e(expression))LUA");
  EXPECT_EQ(Process(R"(unfinished {{expression with "string)"),
            R"LUA(_s([[unfinished ]])_e(expression with "string)LUA");
  EXPECT_EQ(Process(R"(unfinished {{expression with "string\")"),
            R"LUA(_s([[unfinished ]])_e(expression with "string\")LUA");
}

TEST_F(ProcessorTest, UnfinishedStatementIsClosed) {
  EXPECT_EQ(Process(R"(unfinished {%statement)"),
            R"LUA(_s([[unfinished ]]) statement )LUA");
  EXPECT_EQ(Process(R"(unfinished {%statement with "string)"),
            R"LUA(_s([[unfinished ]]) statement with "string)LUA");
  EXPECT_EQ(Process(R"(unfinished {%statement with "string\")"),
            R"LUA(_s([[unfinished ]]) statement with "string\")LUA");
}

} // namespace
} // namespace ltemplate
} // namespace xdk
