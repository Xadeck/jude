#ifndef XDK_LTEMPLATE_PROCESSOR_H
#define XDK_LTEMPLATE_PROCESSOR_H

#include "absl/strings/string_view.h"
#include "re2/re2.h"
#include <lua.hpp>

namespace xdk {
namespace ltemplate {

class Processor final {
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
  bool TryConsume(const char prefix[]);
  bool TryConsume(const LazyRE2 &re);
  bool Match(size_t size, const char prefix[]) const;
  bool Match(size_t size, const LazyRE2 &re, absl::string_view *match = nullptr,
             int n_match = 0) const;
  const char *Consume(size_t size);

  const char *Read(lua_State *L, size_t *size);
  void ReadCharOrString(size_t *size) const;

  absl::string_view source_;
  Mode mode_ = Mode::BEGIN;
};

} // namespace ltemplate
} // namespace xdk

#endif
