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
using lua::IsString;
using lua::Stack;
using ::testing::StrEq;

TEST(LTemplateTest, Works) {
  lua::State L;
  lua_newtable(L);

  std::string source = R"LT(this is {{2+1, "(three)"}} words)LT";
  ASSERT_EQ(dostring(L, source.data(), source.size(), "test"), LUA_OK)
      << Stack(L);
  ASSERT_THAT(Stack::Element(L, -1),
              HasField("_", IsString("this is 3(three) words")));
}

} // namespace
} // namespace ltemplate
} // namespace xdk
