#include "xdk/ltemplate/ltemplate.h"

#include <string>

#include "xdk/lua/stack.h"
#include "xdk/lua/state.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace xdk {
namespace ltemplate {
namespace {

using ::testing::StrEq;

TEST(LTemplateTest, Works) {
  lua::State L;
  lua_newtable(L);

  std::string source = R"LT(this is {{2+1, "(three)"}} words)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << lua::Stack(L);
  ASSERT_EQ(lua_gettop(L), 2);
  ASSERT_TRUE(lua_istable(L, -1));
  lua_getfield(L, 2, "_");
  ASSERT_THAT(lua_tostring(L, -1), StrEq("this is 3(three) words"));
}

} // namespace
} // namespace ltemplate
} // namespace xdk
