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

TEST(LTemplateTest, ExpressionsWork) {
  lua::State L;
  lua_newtable(L);

  std::string source = R"LT(this is {{2+1, "(three)"}} words)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  ASSERT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("this is 3(three) words")));
}

TEST(LTemplateTest, StatementsWork) {
  lua::State L;
  lua_newtable(L);

  std::string source = R"LT({% x=3 %}the number {{ x }}.)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  ASSERT_THAT(Stack::Element(L, -1), HasField("_", IsString("the number 3.")));
}

TEST(LTemplateTest, EvaluationIsSandboxed) {
  lua::State L;
  lua_newtable(L);
  lua_pushinteger(L, 5);
  lua_setfield(L, -2, "x");

  std::string source = R"LT({% y=x-2 %}the number {{ y }}.)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  ASSERT_THAT(lua_gettop(L), 2);
  ASSERT_THAT(Stack::Element(L, -1), HasField("_", IsString("the number 3.")));
  ASSERT_THAT(Stack::Element(L, -2), HasField("y", IsNil()));
}
} // namespace
} // namespace ltemplate
} // namespace xdk
