#include "xdk/jude/do.h"

#include <iostream>
#include <string>

#include "xdk/jude/reader.h"
#include "xdk/lua/back.h"
#include "xdk/lua/sandbox.h"

namespace xdk {
namespace {
constexpr char kUnnamed[] = "_";

int _o(lua_State *L) {
  // Get current block name.
  lua::getback(L, lua_upvalueindex(2));
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_pushstring(L, kUnnamed);
  }
  // Get value of that block and push it before any argument
  // with which o_ was invoked.
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
  // Get current block name and save result to that block.
  lua::getback(L, lua_upvalueindex(2));
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_pushstring(L, kUnnamed);
  }
  lua_insert(L, -2);
  lua_rawset(L, lua_upvalueindex(1));
  return 0;
}

int beginblock(lua_State *L) {
  if (lua_gettop(L) != 1) {
    lua_pushfstring(L, "beginblock() expects 1 argument, got %d",
                    lua_gettop(L));
    return lua_error(L);
  }
  lua::pushback(L, lua_upvalueindex(1));
  return 0;
}

int endblock(lua_State *L) {
  if (lua_gettop(L) != 0) {
    lua_pushfstring(L, "endblock() expects 0 arguments, got %d", lua_gettop(L));
    return lua_error(L);
  }
  lua::popback(L, lua_upvalueindex(1));
  return 0;
}

} // namespace

namespace jude {

int dostring(lua_State *L, const char *data, size_t size,
             const char *name) noexcept {
  lua_newtable(L); // BLOCKS
  lua_newtable(L); // BLOCKS STACK

  Reader reader(data, size);
  if (int error = lua_load(L, Reader::Read, &reader, name, "t")) {
    lua_remove(L, -3); // BLOCKS
    lua_remove(L, -2); // BLOCKS STACK
    return error;
  }
  lua::newsandbox(L, -4);
  {
    lua_pushliteral(L, "_o");
    lua_pushvalue(L, -5); // BLOCKS
    lua_pushvalue(L, -5); // BLOCKS STACK
    lua_pushcclosure(L, &_o, 2);
    lua_rawset(L, -3);
  }
  {
    lua_pushliteral(L, "beginblock");
    lua_pushvalue(L, -4); // BLOCKS STACK
    lua_pushcclosure(L, &beginblock, 1);
    lua_rawset(L, -3);
  }
  {
    lua_pushliteral(L, "endblock");
    lua_pushvalue(L, -4); // BLOCKS STACK
    lua_pushcclosure(L, &endblock, 1);
    lua_rawset(L, -3);
  }

  lua_setupvalue(L, -2, 1);
  if (int error = lua_pcall(L, 0, 0, 0)) {
    lua_remove(L, -3); // BLOCKS
    lua_remove(L, -2); // BLOCKS STACK
    return error;
  }
  lua_pop(L, 1); // BLOCKS STACK
  return LUA_OK;
}

} // namespace jude
} // namespace xdk
