#include "xdk/jude/do.h"

#include <string>

#include "xdk/lua/matchers.h"
#include "xdk/lua/stack.h"
#include "xdk/lua/state.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace xdk {
namespace jude {
namespace {

using lua::HasField;
using lua::IsNil;
using lua::IsString;
using lua::Stack;
using ::testing::_;
using ::testing::HasSubstr;
using ::testing::StrEq;

class DoTest : public ::testing::Test {
protected:
  lua::State L;
};

TEST_F(DoTest, EmptySourceGivesEmptyTable) {
  lua_newtable(L);

  const std::string source = "";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1), HasField("_", IsNil()));
}

TEST_F(DoTest, ExpressionsWork) {
  lua_newtable(L);

  const std::string source = R"S(this is {{2+1, "(three)"}} words)S";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("this is 3(three) words")));
}

TEST_F(DoTest, StatementsWork) {
  lua_newtable(L);

  const std::string source = R"({% x=3 %}the number {{ x }}.)";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1), HasField("_", IsString("the number 3.")));
}

TEST_F(DoTest, EvaluationIsSandboxed) {
  lua_newtable(L);
  lua_pushinteger(L, 5);
  lua_setfield(L, -2, "x");

  const std::string source = R"({% y=x-2 %}the number {{ y }}.)";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1), HasField("_", IsString("the number 3.")));
  EXPECT_THAT(Stack::Element(L, -2), HasField("y", IsNil()));
}

TEST_F(DoTest, NamedBlocksWork) {
  lua_newtable(L);
  const std::string source = R"(
{%- beginblock('head') -%}
this is the header.
{% endblock() -%}
Some main text.
{%- beginblock('css') -%}
some css.
{% endblock() -%}
some more text.
{%- beginblock('css') -%}
some more css.
{% endblock() -%}
)";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("Some main text.some more text.")));
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("head", IsString("this is the header.\n")));
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("css", IsString("some css.\nsome more css.\n")));
}

TEST_F(DoTest, LoadErrorIsReported) {
  lua_newtable(L);
  const std::string source = "{% x = foo( %}";
  int error = dostring(L, source.data(), source.size(), "test");
  ASSERT_EQ(error, LUA_ERRSYNTAX);
  EXPECT_THAT(Stack::Element(L, -1),
              IsString(HasSubstr("unexpected symbol near <eof>")));
}

TEST_F(DoTest, CallErrorIsReported) {
  lua_newtable(L);
  const std::string source = "{{ y .. 3 }}";
  int error = dostring(L, source.data(), source.size(), "test");
  ASSERT_EQ(error, LUA_ERRRUN);
  EXPECT_THAT(
      Stack::Element(L, -1),
      IsString(HasSubstr("attempt to concatenate a nil value (global \'y\')")));
}

TEST_F(DoTest, LongStringsAreHandledInText) {
  lua_newtable(L);
  const std::string source = "this is [[text]] in double brackets";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("this is [[text]] in double brackets")));
}

TEST_F(DoTest, LongStringsAreHandledInExpression) {
  lua_newtable(L);
  const std::string source = "this is {{ [[text]] }} in double brackets";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("this is text in double brackets")));
}

TEST_F(DoTest, LongStringsAreHandledInStatement) {
  lua_newtable(L);
  const std::string source = "{% x=[[text]] %}this is {{x}} in double brackets";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("this is text in double brackets")));
}

TEST_F(DoTest, NewlinesWork) {
  lua_newtable(L);
  const std::string source = R"(

    this is a text
    without any expression
    or statement.)";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1), HasField("_", IsString(source)));
}

TEST_F(DoTest, LinesEndingWithExpressionWork) {
  lua_newtable(L);
  lua_pushnumber(L, 3);
  lua_setfield(L, -2, "expression");
  const std::string source = R"(
    line ending with {{expression}}
    other line.)";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  // Lua long string eat the first newline if any.
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("\n"
                                     "    line ending with 3.0\n"
                                     "    other line.")));
}

} // namespace
} // namespace jude
} // namespace xdk
