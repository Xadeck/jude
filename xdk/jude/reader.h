#ifndef XDK_JUDE_READER_H
#define XDK_JUDE_READER_H

#include "absl/strings/string_view.h"
#include "xdk/lua/lua.hpp"

namespace xdk {
namespace jude {

// Performs on the fly transformation of a Jude template into it's Lua program
// equivalent. Used as a lua_Reader to load that program:
//
//   absl::string_view tpl = "some text {{x}}";
//   Reader reader(tpl.data(), tpl.size());
//   lua_load(L, Reader::Read, &reader, "tpl", "t"));
//
// This is equivalent to:
//
//   luaL_loadstring(L, "_o([[\nsome text ]])_o(x)");
//
// Caller can then lua_pcall the loaded chunks with whatever definition of the
// _o function it wants, and setting up whatever environment it wants.
class Reader final {
public:
  // Data must stay valid as long as the reader is being used.
  Reader(const char *data, size_t size) noexcept;

  static const char *Read(lua_State *L, void *data, size_t *size) noexcept;

private:
  enum class Mode {
    BEGIN = 0,
    TEXT = 1,
    TEXT_END = 2,
    EXPRESSION = 3,
    EXPRESSION_END = 4,
    STATEMENT = 5,
    STATEMENT_END = 6,
    STRING = 7,
  };
  bool TryConsume(const char prefix[]);
  bool TryConsumeOpeningStatement();
  bool TryConsumeClosingStatement();
  bool Match(size_t size, const char prefix[]) const;
  bool MatchOpeningStatement(size_t size) const;
  bool MatchClosingStatement(size_t size) const;
  bool MatchClosingLongString(size_t size) const;
  const char *Consume(size_t size);

  const char *Read(lua_State *L, size_t *size);

  absl::string_view source_;
  Mode mode_ = Mode::BEGIN;
  char delimiter_ = 0;
  Mode from_ = Mode::BEGIN;
};

} // namespace jude
} // namespace xdk

#endif
