#include "xdk/ltemplate/ltemplate.h"

#include <string>

#include "xdk/lua/matchers.h"
#include "xdk/lua/stack.h"
#include "xdk/lua/state.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace xdk {
namespace ltemplate {
namespace {

using lua::HasField;
using lua::IsNil;
using lua::IsString;
using lua::Stack;
using ::testing::_;
using ::testing::StrEq;

class LTemplateTest : public ::testing::Test {
protected:
  lua::State L;
};

TEST_F(LTemplateTest, ExpressionsWork) {
  lua_newtable(L);

  std::string source = R"LT(this is {{2+1, "(three)"}} words)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("this is 3(three) words")));
}

TEST_F(LTemplateTest, StatementsWork) {
  lua_newtable(L);

  std::string source = R"LT({% x=3 %}the number {{ x }}.)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1), HasField("_", IsString("the number 3.")));
}

TEST_F(LTemplateTest, EvaluationIsSandboxed) {
  lua_newtable(L);
  lua_pushinteger(L, 5);
  lua_setfield(L, -2, "x");

  std::string source = R"LT({% y=x-2 %}the number {{ y }}.)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1), HasField("_", IsString("the number 3.")));
  EXPECT_THAT(Stack::Element(L, -2), HasField("y", IsNil()));
}

TEST_F(LTemplateTest, NamedBlocksWork) {
  lua_newtable(L);
  std::string source = R"LT(
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
)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("Some main text.some more text.")));
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("head", IsString("this is the header.\n")));
  EXPECT_THAT(Stack::Element(L, -1),
              HasField("css", IsString("some css.\nsome more css.\n")));
}

} // namespace
} // namespace ltemplate
} // namespace xdk
