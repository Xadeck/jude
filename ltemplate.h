#ifndef XDK_LTEMPLATE_LTEMPLATE_H
#define XDK_LTEMPLATE_LTEMPLATE_H

#include <lua.hpp>

namespace xdk {
namespace ltemplate {

// Expects a table on the stack, leaves it there.
//
// Returns LUA_OK if success.  Result is pushed on stack.
//
// In case of error, pushes the error message.
int dostring(lua_State *L, const char *data, size_t size);

} // namespace ltemplate
} // namespace xdk

#endif
