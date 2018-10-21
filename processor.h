#ifndef XDK_LTEMPLATE_PROCESSOR_H
#define XDK_LTEMPLATE_PROCESSOR_H

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include <lua.hpp>

namespace xdk {
namespace ltemplate {

class Processor {
public:
  explicit Processor(absl::string_view source);

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
  };
  const char *Consumed(size_t size);
  void To(Mode mode);
  const char *Read(lua_State *L, size_t *size);

  absl::string_view source_;
  Mode mode_ = Mode::BEGIN;
  absl::optional<char> string_delimiter_;
};

} // namespace ltemplate
} // namespace xdk

#endif
