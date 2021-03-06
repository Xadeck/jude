#ifndef XDK_jude_DO_H
#define XDK_jude_DO_H

#include "xdk/lua/lua.hpp"

namespace xdk {
namespace jude {

// Expects a table on the stack, leaves it there.
//
// Returns LUA_OK if success.  Result is pushed on stack.
//
// In case of error, pushes the error message.
int dostring(lua_State *L, const char *data, size_t size,
             const char *name) noexcept;

} // namespace jude
} // namespace xdk

#endif
