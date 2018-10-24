#ifndef XDK_LTEMPLATE_LTEMPLATE_H
#define XDK_LTEMPLATE_LTEMPLATE_H

#include "xdk/lua/lua.hpp"

namespace xdk {
namespace ltemplate {

// Expects a table on the stack, leaves it there.
//
// Returns LUA_OK if success.  Result is pushed on stack.
//
// In case of error, pushes the error message.
int dostring(lua_State *L, const char *data, size_t size, const char *name);

} // namespace ltemplate
} // namespace xdk

#endif
