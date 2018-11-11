#include "xdk/jude/reader.h"

#include "absl/strings/str_cat.h"
#include "xdk/lua/stack.h"
#include "xdk/lua/state.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <string>

namespace xdk {
namespace jude {
namespace {

using ::testing::ElementsAre;

class ReaderTest : public ::testing::Test {
protected:
  std::string Read(absl::string_view source) {
    Reader reader(source.data(), source.size());
    return Read(Reader::Read, L, &reader);
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

TEST_F(ReaderTest, EmptyStringWorks) { ASSERT_EQ(Read(""), ""); }

TEST_F(ReaderTest, SingleCharWorks) {
  ASSERT_EQ(Read("x"), "_o([[x]])");
  ASSERT_EQ(Read("\n"), "_o([[\n]])");
}

TEST_F(ReaderTest, StrayRBracesArePreserved) {
  ASSERT_EQ(Read(R"(some }} in a text)"), R"LUA(_o([[some }} in a text]]))LUA");
}

TEST_F(ReaderTest, StrayRPercentBraceArePreserved) {
  ASSERT_EQ(Read(R"(some %} in a text)"), R"LUA(_o([[some %} in a text]]))LUA");
}

TEST_F(ReaderTest, EscapedLBracesArePreserved) {
  ASSERT_EQ(Read(R"(some \{{ in a text)"),
            R"LUA(_o([[some \{{ in a text]]))LUA");
}

TEST_F(ReaderTest, ExpressionsWork) {
  ASSERT_EQ(Read(R"(some {{3+4}} expression)"),
            R"LUA(_o([[some ]])_o(3+4)_o([[ expression]]))LUA");
  ASSERT_EQ(Read(R"({{expression}} at start)"),
            R"LUA(_o(expression)_o([[ at start]]))LUA");
  ASSERT_EQ(Read(R"(expression {{at end}})"),
            R"LUA(_o([[expression ]])_o(at end))LUA");
}

TEST_F(ReaderTest, StringsInExpressionsWork) {
  // Inside a double quoted string, }} does not stop expression.
  // An escaped double quote does not stop the string.
  // An escaped backslash does not escape the following double quote.
  ASSERT_EQ(Read(R"({{with "string \" }}\\" expression}})"),
            R"LUA(_o(with "string \" }}\\" expression))LUA");
  // Double quoted string can be at begin/end of expression.
  ASSERT_EQ(Read(R"({{ "string" }})"), R"LUA(_o( "string" ))LUA");
  ASSERT_EQ(Read(R"({{"string"}})"), R"LUA(_o("string"))LUA");
  // Same thing with single quoted string.
  ASSERT_EQ(Read(R"({{with 'string \' }}' expression}})"),
            R"LUA(_o(with 'string \' }}' expression))LUA");
  ASSERT_EQ(Read(R"({{ 'string' }})"), R"LUA(_o( 'string' ))LUA");
  ASSERT_EQ(Read(R"({{'string'}})"), R"LUA(_o('string'))LUA");
}

TEST_F(ReaderTest, StatementsWork) {
  ASSERT_EQ(Read(R"(some {%3+4%} statement)"),
            R"LUA(_o([[some ]]) 3+4 _o([[ statement]]))LUA");
  ASSERT_EQ(Read(R"({%statement%} at start)"),
            R"LUA( statement _o([[ at start]]))LUA");
  ASSERT_EQ(Read(R"(statement {%at end%})"),
            R"LUA(_o([[statement ]]) at end )LUA");
}

TEST_F(ReaderTest, StringsInStatementsWork) {
  // Inside a double quoted string, %} does not stop statement.
  // An escaped double quote does not stop the string.
  // An escaped backslash does not escape the following double quote.
  ASSERT_EQ(Read(R"({%with "string \" %}\\" statement%})"),
            R"LUA( with "string \" %}\\" statement )LUA");
  // Single quoted string can be at begin/end of statement.
  ASSERT_EQ(Read(R"({% "string" %})"), R"LUA(  "string"  )LUA");
  ASSERT_EQ(Read(R"({%"string"%})"), R"LUA( "string" )LUA");
  // Same thing with single quoted string.
  ASSERT_EQ(Read(R"({%with 'string \' %}\\' statement%})"),
            R"LUA( with 'string \' %}\\' statement )LUA");
  ASSERT_EQ(Read(R"({% 'string' %})"), R"LUA(  'string'  )LUA");
  ASSERT_EQ(Read(R"({%'string'%})"), R"LUA( 'string' )LUA");
}

TEST_F(ReaderTest, WhitespacesAreStripped) {
  EXPECT_EQ(Read(R"(
  first line
  {%  x=1  %}
  second line
  {%- x=2 -%}
  third line)"),
            R"LUA(_o([[
  first line
  ]])   x=1   _o([[
  second line]])  x=2  _o([[  third line]]))LUA");
}

TEST_F(ReaderTest, WhitespacesStrippingCornerCases) {
  EXPECT_EQ(Read("{%--%}"), "  ");
  EXPECT_EQ(Read("\n{%--%}"), "  ");
  EXPECT_EQ(Read("{%--%}\n"), "  ");
}

TEST_F(ReaderTest, UnfinishedExpressionIsClosed) {
  EXPECT_EQ(Read(R"(unfinished {{expression)"),
            R"LUA(_o([[unfinished ]])_o(expression))LUA");
  EXPECT_EQ(Read(R"(unfinished {{expression with "string)"),
            R"LUA(_o([[unfinished ]])_o(expression with "string)LUA");
  EXPECT_EQ(Read(R"(unfinished {{expression with "string\")"),
            R"LUA(_o([[unfinished ]])_o(expression with "string\")LUA");
}

TEST_F(ReaderTest, UnfinishedStatementIsClosed) {
  EXPECT_EQ(Read(R"(unfinished {%statement)"),
            R"LUA(_o([[unfinished ]]) statement )LUA");
  EXPECT_EQ(Read(R"(unfinished {%statement with "string)"),
            R"LUA(_o([[unfinished ]]) statement with "string)LUA");
  EXPECT_EQ(Read(R"(unfinished {%statement with "string\")"),
            R"LUA(_o([[unfinished ]]) statement with "string\")LUA");
}

} // namespace
} // namespace jude
} // namespace xdk
