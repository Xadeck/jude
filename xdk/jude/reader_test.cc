#include "xdk/jude/reader.h"

#include "absl/strings/str_cat.h"
#include "xdk/lua/read.h"
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
    return lua::Read(Reader::Read, L, &reader);
  }

  xdk::lua::State L;
};

TEST_F(ReaderTest, EmptyStringWorks) { ASSERT_EQ(Read(""), ""); }

TEST_F(ReaderTest, SingleCharWorks) {
  ASSERT_EQ(Read("x"), "_o([[\nx]])");
  ASSERT_EQ(Read("\n"), "_o([[\n\n]])");
}

TEST_F(ReaderTest, StrayRBracesArePreserved) {
  ASSERT_EQ(Read(R"(some }} in a text)"), "_o([[\nsome }} in a text]])");
}

TEST_F(ReaderTest, StrayRPercentBraceArePreserved) {
  ASSERT_EQ(Read(R"(some %} in a text)"), "_o([[\nsome %} in a text]])");
}

TEST_F(ReaderTest, EscapedLBracesArePreserved) {
  ASSERT_EQ(Read(R"(some \{{ in a text)"), "_o([[\nsome \\{{ in a text]])");
}

TEST_F(ReaderTest, ExpressionsWork) {
  ASSERT_EQ(Read(R"(some {{3+4}} expression)"),
            "_o([[\nsome ]])_o(3+4)_o([[\n expression]])");
  ASSERT_EQ(Read(R"({{expression}} at start)"),
            "_o(expression)_o([[\n at start]])");
  ASSERT_EQ(Read(R"(expression {{at end}})"),
            "_o([[\nexpression ]])_o(at end)");
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
            "_o([[\nsome ]]) 3+4 _o([[\n statement]])");
  ASSERT_EQ(Read(R"({%statement%} at start)"),
            " statement _o([[\n at start]])");
  ASSERT_EQ(Read(R"(statement {%at end%})"), "_o([[\nstatement ]]) at end ");
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

  second line]])  x=2  _o([[
  third line]]))LUA");
}

TEST_F(ReaderTest, WhitespacesStrippingCornerCases) {
  EXPECT_EQ(Read("{%--%}"), "  ");
  EXPECT_EQ(Read("\n{%--%}"), "  ");
  EXPECT_EQ(Read("{%--%}\n"), "  ");
}

TEST_F(ReaderTest, UnfinishedExpressionIsClosed) {
  EXPECT_EQ(Read(R"(unfinished {{expression)"),
            "_o([[\nunfinished ]])_o(expression)");
  EXPECT_EQ(Read(R"(unfinished {{expression with "string)"),
            "_o([[\nunfinished ]])_o(expression with \"string");
  EXPECT_EQ(Read(R"(unfinished {{expression with "string\")"),
            R"LUA(_o([[
unfinished ]])_o(expression with "string\")LUA");
}

TEST_F(ReaderTest, UnfinishedStatementIsClosed) {
  EXPECT_EQ(Read(R"(unfinished {%statement)"),
            "_o([[\nunfinished ]]) statement ");
  EXPECT_EQ(Read(R"(unfinished {%statement with "string)"),
            R"LUA(_o([[
unfinished ]]) statement with "string)LUA");
  EXPECT_EQ(Read(R"(unfinished {%statement with "string\")"),
            R"LUA(_o([[
unfinished ]]) statement with "string\")LUA");
}

TEST_F(ReaderTest, LongStringWorks) {
  // Check that ]] in regular text gets escaped.
  EXPECT_EQ(Read(R"(some [[text]] in double brackets)"),
            "_o([[\nsome [[text]])_o(']]')_o([[\n in double brackets]])");
  // Check that long strings work in expression.
  EXPECT_EQ(Read("{{ [[text]] }}"), "_o( [[text]] )");
  // Check that long strings work in statement.
  EXPECT_EQ(Read("{% x=[[text]] %}"), "  x=[[text]]  ");
}

TEST_F(ReaderTest, NewlinesWork) {
  EXPECT_EQ(Read(R"(

    this is a text
    without any expression
    or statement.)"),
            R"(_o([[


    this is a text
    without any expression
    or statement.]]))");
}

TEST_F(ReaderTest, LinesEndingWithExpressionWork) {
  // Test the case of line ending with an expression.
  EXPECT_EQ(Read(R"(
    line ending with {{expression}}
    other line.)"),
            R"(_o([[

    line ending with ]])_o(expression)_o([[

    other line.]]))");
}

} // namespace
} // namespace jude
} // namespace xdk
