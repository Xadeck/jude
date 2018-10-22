#include "xdk/ltemplate/processor.h"
#include "absl/strings/match.h"
#include "absl/strings/strip.h"
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
// The follow-up matches strings with escaped quotes.
// See https://regex101.com/r/tAsbx5/1 for details.
LazyRE2 kString = {R"RE(\\["']|"(?:\\"|.)*?["\n]|'(?:\\'|.)*?['\n])RE"};

template <size_t N>
const char *Literal(const char (&literal)[N], size_t *size) {
  *size = N - 1;
  return literal;
}

} // namespace

Processor::Processor(absl::string_view source) : source_(source) {}

bool Processor::Match(size_t size, const char prefix[]) const {
  return size >= source_.size() || //
         absl::StartsWith(source_.substr(size), prefix);
}

bool Processor::Match(size_t size, const LazyRE2 &re) const {
  return size >= source_.size() || //
         re->Match(source_, size, source_.size(), RE2::ANCHOR_START, nullptr,
                   0);
}

bool Processor::Consume(const char prefix[]) {
  return absl::ConsumePrefix(&source_, prefix);
}

bool Processor::Consume(const LazyRE2 &re) {
  return RE2::Consume(&source_, *re);
}

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
      if (Consume(kOpeningExpression)) {
        return To(Mode::EXPRESSION), Literal("_e(", size);
      }
      if (Consume(kOpeningStatement)) {
        return To(Mode::STATEMENT), Literal(" ", size);
      }
      if (Consume(kClosingExpressionOrStatement)) {
        continue;
      }
      return To(Mode::TEXT), Literal("_s([[", size);
    }
    return nullptr;
  case Mode::TEXT:
    for (*size = 0; !Match(*size, kOpeningExpressionOrStatement);) {
      ++*size;
    }
    return To(Mode::TEXT_END), Consumed(*size);
  case Mode::TEXT_END:
    return To(Mode::BEGIN), Literal("]])", size);
  case Mode::EXPRESSION:
    for (*size = 0; !Match(*size, kClosingExpression);) {
      ReadCharOrString(size);
    }
    return To(Mode::EXPRESSION_END), Consumed(*size);
  case Mode::EXPRESSION_END:
    return To(Mode::BEGIN), Literal(")", size);
  case Mode::STATEMENT:
    for (*size = 0; !Match(*size, kClosingStatement);) {
      ReadCharOrString(size);
    }
    return To(Mode::STATEMENT_END), Consumed(*size);
  case Mode::STATEMENT_END:
    return To(Mode::BEGIN), Literal(" ", size);
  }
}

void Processor::ReadCharOrString(size_t *size) const {
  absl::string_view match;
  if (kString->Match(source_, *size, source_.size(), RE2::ANCHOR_START, &match,
                     1)) {
    *size += match.size();
  } else {
    ++*size;
  }
}

} // namespace ltemplate
} // namespace xdk
