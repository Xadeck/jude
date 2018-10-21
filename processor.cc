#include "xdk/ltemplate/processor.h"
#include "absl/strings/match.h"
#include "absl/strings/strip.h"
#include "re2/re2.h"
#include <iostream>

namespace xdk {
namespace ltemplate {
namespace {

constexpr char kOpeningExpression[] = {"{{"};
constexpr char kClosingExpression[] = {"}}"};
LazyRE2 kOpeningStatement = {R"RE({%-?|\n *{%-)RE"};
LazyRE2 kClosingStatement = {R"RE(-%} *\n|-?%})RE"};
LazyRE2 kOpeningExpressionOrStatement = {R"RE({{|{%-?|\n *{%-)RE"};
LazyRE2 kClosingExpressionOrStatement = {R"RE(}}|-%} *\n|-?%})RE"};

bool Consume(absl::string_view *source, const char prefix[]) {
  return absl::ConsumePrefix(source, prefix);
}

bool Consume(absl::string_view *source, const LazyRE2 &re) {
  return RE2::Consume(source, *re);
}

bool Match(const char prefix[], absl::string_view source, size_t size) {
  return absl::StartsWith(source.substr(size), prefix);
}

bool Match(const LazyRE2 &re, absl::string_view source, size_t size) {
  return re->Match(source, size, source.size(), RE2::ANCHOR_START, nullptr, 0);
}

template <bool accept_string, typename T>
void ConsumeUntil(absl::string_view source, size_t *size, const T &condition) {
  *size = 0;
  while (*size < source.size() && !Match(condition, source, *size)) {
    if (accept_string) {
      const char delimiter = source[*size];
      if (delimiter == '\'' || delimiter == '"') {
        while (++*size < source.size() && source[*size] != delimiter) {
          if (source[*size] == '\\' &&     //
              *size + 1 < source.size() && //
              source[*size + 1] == delimiter) {
            *size += 2;
          }
        }
      }
    }
    ++*size;
  }
}

template <size_t N>
const char *Literal(const char (&literal)[N], size_t *size) {
  *size = N - 1;
  return literal;
}

} // namespace

Processor::Processor(absl::string_view source) : source_(source) {}

const char *Processor::Read(lua_State *L, void *data, size_t *size) {
  return reinterpret_cast<Processor *>(data)->Read(L, size);
}

void Processor::To(Mode mode) { mode_ = mode; }

const char *Processor::Consumed(size_t size) {
  const char *read = source_.data();
  source_.remove_prefix(size);
  return read;
}
const char *Processor::Read(lua_State *L, size_t *size) {
  switch (mode_) {
  case Mode::BEGIN:
    while (!source_.empty()) {
      if (Consume(&source_, kOpeningExpression)) {
        return To(Mode::EXPRESSION), Literal("_e(", size);
      }
      if (Consume(&source_, kOpeningStatement)) {
        return To(Mode::STATEMENT), Literal(" ", size);
      }
      if (Consume(&source_, kClosingExpressionOrStatement)) {
        continue;
      }
      return To(Mode::TEXT), Literal("_s([[", size);
    }
    return nullptr;
  case Mode::TEXT:
    ConsumeUntil<false>(source_, size, kOpeningExpressionOrStatement);
    return To(Mode::TEXT_END), Consumed(*size);
  case Mode::TEXT_END:
    return To(Mode::BEGIN), Literal("]])", size);
  case Mode::EXPRESSION:
    ConsumeUntil<true>(source_, size, kClosingExpression);
    return To(Mode::EXPRESSION_END), Consumed(*size);
  case Mode::EXPRESSION_END:
    return To(Mode::BEGIN), Literal(")", size);
  case Mode::STATEMENT:
    ConsumeUntil<true>(source_, size, kClosingStatement);
    return To(Mode::STATEMENT_END), Consumed(*size);
  case Mode::STATEMENT_END:
    return To(Mode::BEGIN), Literal(" ", size);
  }
}

} // namespace ltemplate
} // namespace xdk
