#include "xdk/jude/reader.h"
#include "absl/base/macros.h"
#include "absl/strings/match.h"
#include "absl/strings/strip.h"
#include <iostream>

namespace xdk {
namespace jude {
namespace {

constexpr char kOpeningExpression[] = {"{{"};
constexpr char kClosingExpression[] = {"}}"};

template <size_t N>
const char *Produce(const char (&literal)[N], size_t *size) {
  *size = N - 1;
  return literal;
}

bool IsQuote(char c) { return c == '"' || c == '\''; }

} // namespace

Reader::Reader(const char *data, size_t size) : source_(data, size) {}

bool Reader::Match(size_t size, const char prefix[]) const {
  return size >= source_.size() ||
         absl::StartsWith(source_.substr(size), prefix);
}

bool Reader::MatchOpeningStatement(size_t size) const {
  if (Match(size, "\n")) {
    while (++size < source_.size() && source_[size] == ' ')
      ;
    return Match(size, "{%-");
  }
  return Match(size, "{%-") || Match(size, "{%");
}

bool Reader::MatchClosingStatement(size_t size) const {
  return Match(size, "-%}") || Match(size, "%}");
}

bool Reader::TryConsume(const char prefix[]) {
  return absl::ConsumePrefix(&source_, prefix);
}

bool Reader::TryConsumeOpeningStatement() {
  int size = 0;
  if (absl::StartsWith(source_.substr(size), "\n")) {
    while (++size < source_.size() && source_[size] == ' ')
      ;
    if (absl::StartsWith(source_.substr(size), "{%-")) {
      source_ = source_.substr(size + 3);
      return true;
    };
    return false;
  }
  return TryConsume("{%-") || TryConsume("{%");
}

bool Reader::TryConsumeClosingStatement() {
  int size = 0;
  if (absl::StartsWith(source_.substr(size), "-%}")) {
    size += 3;
    while (size < source_.size() && source_[size] == ' ')
      ;
    if (size < source_.size() && source_[size] == '\n') {
      ++size;
    }
    source_ = source_.substr(size);
    return true;
  }
  return TryConsume("%}");
}

const char *Reader::Read(lua_State *L, void *data, size_t *size) {
  return reinterpret_cast<Reader *>(data)->Read(L, size);
}

const char *Reader::Consume(size_t size) {
  const char *read = source_.data();
  source_.remove_prefix(size);
  return read;
}
const char *Reader::Read(lua_State *L, size_t *size) {
  switch (mode_) {
  case Mode::BEGIN:
    if (TryConsume(kOpeningExpression)) {
      return mode_ = Mode::EXPRESSION, Produce("_o(", size);
    }
    if (TryConsumeOpeningStatement()) {
      return mode_ = Mode::STATEMENT, Produce(" ", size);
    }
    if (!source_.empty()) {
      return mode_ = Mode::TEXT, Produce("_o([[", size);
    }
    return nullptr;
  case Mode::TEXT:
    for (*size = 0;
         !Match(*size, kOpeningExpression) && !MatchOpeningStatement(*size);
         ++*size) {
      if (source_[*size] == '\\' && *size + 1 < source_.size()) {
        ++*size;
      }
    }
    return mode_ = Mode::TEXT_END, Consume(*size);
  case Mode::TEXT_END:
    return mode_ = Mode::BEGIN, Produce("]])", size);
  case Mode::EXPRESSION:
    for (*size = 0; !Match(*size, kClosingExpression); ++*size) {
      if (IsQuote(delimiter_ = source_[*size])) {
        return mode_ = Mode::STRING,     //
               from_ = Mode::EXPRESSION, //
               Consume(++*size);
      }
    }
    if (*size || source_.empty()) {
      return mode_ = Mode::EXPRESSION_END, Consume(*size);
    }
    ABSL_FALLTHROUGH_INTENDED;
  case Mode::EXPRESSION_END:
    TryConsume(kClosingExpression);
    return mode_ = Mode::BEGIN, Produce(")", size);
  case Mode::STATEMENT:
    for (*size = 0; !MatchClosingStatement(*size); ++*size) {
      if (IsQuote(delimiter_ = source_[*size])) {
        return mode_ = Mode::STRING,    //
               from_ = Mode::STATEMENT, //
               Consume(++*size);
      }
    }
    if (*size || source_.empty()) {
      return mode_ = Mode::STATEMENT_END, Consume(*size);
    }
    ABSL_FALLTHROUGH_INTENDED;
  case Mode::STATEMENT_END:
    TryConsumeClosingStatement();
    return mode_ = Mode::BEGIN, Produce(" ", size);
  case Mode::STRING:
    for (*size = 0; *size < source_.size(); ++*size) {
      if (source_[*size] == delimiter_) {
        return mode_ = from_, Consume(++*size);
      }
      if (source_[*size] == '\\' && *size + 1 < source_.size()) {
        ++*size;
      }
    }
    return mode_ = from_, Consume(*size);
  }
}

} // namespace jude
} // namespace xdk
