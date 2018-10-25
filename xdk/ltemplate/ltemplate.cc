#include "xdk/ltemplate/ltemplate.h"
#include <iostream>
#include <string>

#include "absl/strings/str_cat.h"
#include "xdk/ltemplate/reader.h"
#include "xdk/lua/sandbox.h"
#include "xdk/lua/stack.h"

namespace xdk {
namespace {

int _o(lua_State *L) {
  lua_pushliteral(L, "_");
  lua_rawget(L, lua_upvalueindex(1));
  lua_insert(L, 1);
  // Convert nil values to empty string.
  for (int index = 1; index <= lua_gettop(L); ++index) {
    if (lua_isnil(L, index)) {
      lua_pushstring(L, "");
      lua_replace(L, index);
    }
  }
  // Use concat to support arguments with a __concat metamethod.
  lua_pushstring(L, "");
  lua_concat(L, lua_gettop(L));
  lua_pushliteral(L, "_");
  lua_insert(L, -2);
  lua_rawset(L, lua_upvalueindex(1));
  return 0;
}
} // namespace

namespace ltemplate {

int dostring(lua_State *L, const char *data, size_t size, const char *name) {
  lua_newtable(L); // BLOCKS

  Reader reader(data, size);
  if (int error = lua_load(L, Reader::Read, &reader, name, "t")) {
    lua_pop(L, -2); // BLOCKS
    return error;
  }
  lua::Sandbox sandbox(L, -3);

  lua_pushliteral(L, "_o");
  lua_pushvalue(L, -4); // BLOCKS
  lua_pushcclosure(L, &_o, 1);
  lua_rawset(L, -3);

  lua_setupvalue(L, -2, 1);
  if (int error = lua_pcall(L, 0, 0, 0)) {
    lua_pop(L, -2); // BLOCKS
    return error;
  }
  return LUA_OK;
}

} // namespace ltemplate
} // namespace xdk
