#ifndef XDK_JUDE_READER_H
#define XDK_JUDE_READER_H

#include "absl/strings/string_view.h"
#include "xdk/lua/lua.hpp"

namespace xdk {
namespace jude {

class Reader final {
public:
  Reader(const char *data, size_t size);

  static const char *Read(lua_State *L, void *data, size_t *size);

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
