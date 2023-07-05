// (c)2023 Sony Interactive Entertainment Inc
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Sony Interactive Entertainment Inc. nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Lexer/parser implementation for Rotten Green Test detection.
// It is as simple as possible, because we care only about finding the
// line ranges of certain constructs, which are easily identified.
// We can take shortcuts because we know a-priori that the file
// is a syntactically valid C++ googletest-using source file.

#include "gtest/internal/gtest-rgt-parser.h"

#if GTEST_HAS_RGT

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <vector>

namespace testing {
namespace internal {

// We recognize the fewest number of tokens that will let us determine
// the source ranges of the constructs we care about.
// For a TEST (etc) function that means we need to be able to parse
//   TEST(id, id) {
//     contents mostly irrelevant
//   }
// without getting confused by comments, conditional compilation, or
// string literals (in all their marvelous variety).
//
// We also need to be able to recognize
//   EXPECT_[NON]FATAL_FAILURE(some sort of test);
// because if "some sort of test" contains an assertion, it will not
// be flagged as executed, and we need to compensate for that.

enum TokenKind {
  kTokenStringLit,      // Don't get confused by text in a string literal.
  kTokenDigitString,    // Save some memory by tokenizing digit strings.
  kTokenLParen,         // Parens delimit arguments to macros and functions.
  kTokenRParen,
  kTokenComma,          // Comma separates arguments we care about.
  kTokenLBrace,         // Braces delimit the source range of a function.
  kTokenRBrace,
  kTokenHashIf,         // #if (but not #if 0), #ifdef, #ifndef
  kTokenHashIf0,        // #if 0
  kTokenHashElif,       // #elif, etc
  kTokenHashElse,       // #else
  kTokenHashEndif,      // #endif
  kTokenTest,           // One of the TEST macro words.
  kTokenExpectFailure,  // EXPECT_[NON]FATAL_FAILURE
  kTokenRawLitIntro,    // Raw-literal introducer.
  kTokenWord,           // Some identifier.
  kTokenOther,          // Everything else.
  kTokenEof             // Special end-of-file token.
};

// Describe one lexed token. id_ will point to the text.
class Token {
  TokenKind kind_;
  int line_;
  StringRef id_;
public:
  Token(TokenKind k, StringRef s, int line) :
        kind_(k), line_(line), id_(s) {}
  TokenKind Kind() const { return kind_; }
  int Line() const { return line_; }
  StringRef Id() const { return id_; }
  void Print(std::string &out) const;
};

// The lexer builds an array of tokens, for the parser to look at.
// The lexer figures out #if blocks so the parser doesn't have to.
class TokenVector : public std::vector<Token> {
  using RawVector = std::vector<Token>;

  // Prevent accidental use of push_back.
  void push_back(const Token &);

  // Keep track of #if state. We need to remember whether we are
  // saving or skipping tokens in the current chunk, and where the
  // most recent real #if is on the stack (as opposed to #elif).
  struct HashIfContext {
    bool saving;
    bool real_if;
  };
  // Stack of #if contexts.
  std::vector<HashIfContext> contexts;

  // Figure out what the new "saving" state should be.
  bool getNewHashIfState() const;
public:
  // Start out with an implicit "#if 1" state.
  TokenVector() { contexts.push_back({true, true}); }

  // Save the token, or handle a conditional directive.
  void Save(const Token &token);

  // Let the lexer know whether we're in an #if-d out block.
  bool Skipping() { return !contexts.back().saving; }

  // For testing; print all saved tokens.
  void PrintTokens(std::string &out) const;
};

// Figure out the new saving/skipping state for #else/#elif cases.
// In general, the new saving state will be false, except: If we were in
// an #if 0, then the first #elif, or #else without #elif, will be true.
// (An #if 0 is a real_if with saving set to false.)
// Or more simply: If the previous state was "#if 0" then we start saving.
// Otherwise we don't.
bool TokenVector::getNewHashIfState() const {
  assert(!contexts.empty());
  const HashIfContext &top = contexts.back();
  return !top.saving && top.real_if;
}

// Conditionally save the token, based on the current #if block state.
// In general we pretend each #if evaluates to true; the only exception
// is for "#if 0" which is quite common and might comment out syntax
// that would confuse the parser. If "#if 0" has an #else, we use that.
void TokenVector::Save(const Token &token) {
  switch (token.Kind()) {
  case kTokenHashIf:
    // For most #if clauses, take the first conditional chunk.
    contexts.push_back({true, true});
    break;
  case kTokenHashIf0:
    // For "#if 0" skip the first conditional chunk.
    contexts.push_back({false, true});
    break;
  case kTokenHashElif:
    // Compute new state like this was #else #if.
    // That way successif #elif's will compute the state correctly.
    contexts.push_back({getNewHashIfState(), false});
    break;
  case kTokenHashElse:
    // Compute new state but this is not a new #if level.
    contexts.back().saving = getNewHashIfState();
    break;
  case kTokenHashEndif:
    // Pop state up to and including the most recent real #if.
    while (!contexts.back().real_if)
      contexts.pop_back();
    contexts.pop_back();
    break;
  default:
    // Not a preprocessing conditional directive.
    assert(!contexts.empty());
    if (contexts.back().saving)
      RawVector::push_back(token);
  }
}

// Pretty-print a Token. For testing.
void Token::Print(std::string &out) const {
  switch (Kind()) {
  case kTokenOther:
    out.append("other");
    break;
  case kTokenEof:
    out.append("<EOF>");
    break;
  case kTokenRawLitIntro:
    out.append("RawLitIntro(");
    out.append(Id().Data(), Id().Length());
    out.append(")");
    break;
  case kTokenTest:
    out.append("*");
    out.append(Id().Data(), Id().Length());
    out.append("*");
    break;
  case kTokenExpectFailure:
    out.append("!");
    out.append(Id().Data(), Id().Length());
    out.append("!");
    break;
  case kTokenHashIf0:
    out.append("#if 0");
    break;
  case kTokenHashIf:
  case kTokenHashElif:
  case kTokenHashElse:
  case kTokenHashEndif:
    out.append("#");
    // FALLTHROUGH
  default:
    out.append(Id().Data(), Id().Length());
    break;
  }
  out.append(" ");
}

// Pretty-print all the Tokens. For testing.
void TokenVector::PrintTokens(std::string &out) const {
  int prev_line = 0;
  for (auto t = this->cbegin(); t != this->cend(); ++t) {
    if (t->Line() > prev_line) {
      // Max chars needed is max digits of an int (10), plus 8, plus nul.
      char buf[19];
      snprintf(buf, 19, "%sLine %d: ", prev_line ? "\n" : "", t->Line());
      out.append(buf);
      prev_line = t->Line();
    }
    t->Print(out);
  }
  out.append("\n");
}

// Lexer helper functions used to scan C++ code. It's not a preprocessor;
// it minimally understands #if conditions and does no macro substitution.
// It's as simple as possible and still find test functions and report
// their line numbers.

// Report whether this character starts an identifier (or keyword).
static bool IsIdStart(char c) {
  return c == '_' || isalpha(c);
}

// Lex an identifier or keyword starting at buf_ptr.
// Returns a StringRef for the identifier and updates buf_ptr to point to
// the last character of the identifier.
static StringRef LexWord(const char **buf_ptr) {
  const char *p = *buf_ptr;
  while (*p == '_' || isalnum(*p))
    ++p;
  StringRef id(*buf_ptr, size_t(p - *buf_ptr));
  // Back up to the last character of the identifier.
  *buf_ptr = p - 1;
  return id;
}

// Decide whether this identifier is a "keyword" that we care about.
// Mostly we care about identifying certain googletest macros, but we
// also have to look out for the introducers for a raw literal.
static TokenKind LexWordToKeyword(StringRef id) {
  switch (id.Data()[0]) {
  case 'E':
    if (id.Equals("EXPECT_FATAL_FAILURE") ||
        id.Equals("EXPECT_NONFATAL_FAILURE") ||
        id.Equals("EXPECT_FATAL_FAILURE_ON_ALL_THREADS") ||
        id.Equals("EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS"))
      return kTokenExpectFailure;
    break;
  case 'G':
    // Only these aliases currently exist, for the keywords we care about.
    if (id.Equals("GTEST_TEST") || id.Equals("GTEST_TEST_F"))
      return kTokenTest;
    break;
  case 'T':
    if (id.Equals("TEST") || id.Equals("TEST_F") || id.Equals("TEST_P") ||
        id.Equals("TYPED_TEST") || id.Equals("TYPED_TEST_P"))
      return kTokenTest;
    break;
  default:
    // A raw literal starts with a possibly prefixed R.
    // The caller is responsible for lexing the actual raw string.
    // TODO: support wide, UTF-16, UTF-32 strings.
    if (id.Equals("R") || id.Equals("u8R"))
      return kTokenRawLitIntro;
    break;
  }
  return kTokenWord;
}

// In a context where we don't care about the keywords, is this a word?
static bool IsAnyWord(TokenKind kind) {
  return kind == kTokenWord || kind == kTokenTest ||
      kind == kTokenExpectFailure || kind == kTokenRawLitIntro;
}

// Decide whether this identifier is a preprocessor directive we care about.
static TokenKind LexWordToDirective(StringRef id) {
  switch (id.Data()[0]) {
  case 'i':
    if (id.Equals("if") || id.Equals("ifdef") || id.Equals("ifndef"))
      return kTokenHashIf;
    break;
  case 'e':
    if (id.Equals("elif") || id.Equals("elifdef") || id.Equals("elifndef"))
      return kTokenHashElif;
    if (id.Equals("else"))
      return kTokenHashElse;
    if (id.Equals("endif"))
      return kTokenHashEndif;
    break;
  default:
    break;
  }
  return kTokenWord;
}

// Lex a delimited character or normal string literal. buf_ptr points
// to the beginning delimiter. Understands backslash quoting.
// Understands backslash continuation for the purpose of line counting,
// although it will not remove those backslashes from the final string.
// Returns a StringRef for the quoted string, and updates buf_ptr to
// point to the closing delimiter.
static StringRef LexStringLit(const char **buf_ptr, int &line) {
  const char *p = *buf_ptr;
  char delim = *p;
  do {
    if (*p == '\\') {
      ++p;
      // Handle continuations.
      if (p[0] == '\r') {
        if (p[1] == '\n')
          ++p;
        line += 1;
      } else if (p[0] == '\n')
        line += 1;
    }
    ++p;
  } while (delim != *p);
  // p points to the delimiter but the token needs to include it.
  StringRef lit(*buf_ptr, size_t(p - *buf_ptr) + 1);
  *buf_ptr = p;
  return lit;
}

// Lex a raw string literal. Caller has found the introducer sequence,
// buf_ptr points to the beginning double-quote. A raw literal has one
// of two forms:
//   R"(arbitrary characters except not right-paren)"
//   R"delim(arbitrary characters except not right-paren followed
//           by "delim" followed by double-quote)delim"
// where "delim" is anything except whitespace, parens, or backslash.
static StringRef LexRawStringLit(const char **buf_ptr, int &line) {
  const char *p = *buf_ptr;
  const char *delim_start = ++p;
  size_t delim_len = 0;
  if (*p != '(') {
    p = strchr(p, '(');
    delim_len = size_t(p - delim_start);
  }
  ++p;
  for (;;) {
    // Look for the closing sequence, and count lines.
    p = strpbrk(p, "\r\n)");
    if (*p == ')') {
      ++p;
      if (strncmp(p, delim_start, delim_len) == 0)
        if (p[delim_len] == '"')
          break;
    } else {
      // *p == \r or \n.
      if (p[0] == '\r' && p[1] == '\n')
        ++p;
      ++p;
      line += 1;
    }
  }
  // p points to the delim, make it point to the double-quote.
  p += delim_len;
  StringRef lit(*buf_ptr, size_t(p - *buf_ptr) + 1);
  *buf_ptr = p;
  return lit;
}

// Lex a digit string as one token. This is strictly to reduce memory
// consumption; otherwise each digit would be tokenized separately as
// "other."  We are not super fussy about this lexing, and particularly
// don't care that '0x12345' gets lexed as '0' followed by the "word" x12345.
static StringRef LexDigitString(const char **buf_ptr) {
  const char *p = *buf_ptr;
  while (isdigit(*p))
    ++p;
  // We're pointing to the first non-digit.
  StringRef lit(*buf_ptr, size_t(p - *buf_ptr));
  *buf_ptr = --p;
  return lit;
}

// Skip characters up to the next newline; returns a pointer to the newline.
// Returns immediately if buf_ptr already points to a newline.
// Note: backslash at the end of a // line should continue the comment.
// The function currently does not look for this, on the theory that
// nobody actually does that in practice.
static const char *SkipToEol(const char *buf_ptr) {
  size_t index = 0;
  // Stop at a LF or bare CR or null.
  // FIXME: use strpbrk?
  while (!(buf_ptr[index] == '\0' || buf_ptr[index] == '\n' ||
           (buf_ptr[index] == '\r' && buf_ptr[index + 1] != '\n')))
    ++index;
  return &buf_ptr[index - 1];
}

// Skip characters up to the next newline; if this line is continued,
// repeat the process. Increment line when that happens.
static const char *SkipToContinuedEol(const char *buf_ptr, int &line) {
  while (1) {
    buf_ptr = SkipToEol(buf_ptr);
    if (buf_ptr[0] == '\0' || buf_ptr[-1] != '\\')
      break;
    // Count the line and skip the EOL marker.
    ++line;
    if (buf_ptr[0] == '\r' && buf_ptr[1] == '\n')
      ++buf_ptr;
    ++buf_ptr;
  }
  return buf_ptr;
}

// Skip to the end of a /* comment. buf_ptr points to the initial slash.
// Returns a pointer to the terminating slash of the */. Counts lines.
static const char *LexCComment(const char *buf_ptr, int &line) {
  assert(buf_ptr[0] == '/' && buf_ptr[1] == '*');

  // It's common to do lines of asterisks, so look for the slash first.
  // Skip over the initial /*, but don't get tripped up by /*/.
  int index = buf_ptr[2] == '/' ? 2 : 1;
  do {
    // Skip the character that isn't the * of a final */.
    ++index;
    // Find the next slash.
    while (buf_ptr[index] != '/') {
      // Count lines.
      if (buf_ptr[index] == '\r') {
        if (buf_ptr[index + 1] == '\n')
          ++index;
        ++line;
      } else if (buf_ptr[index] == '\n')
        ++line;
      ++index;
    }
  } while (buf_ptr[index - 1] != '*');
  return &buf_ptr[index];
}

// Lexer/parser implementation class.
// It's as simple as possible and still find the constructs we need and
// report their line numbers.

class RgtParserImpl {
  char *file_buf_ = nullptr;
  TokenVector tokens_;
  std::vector<RgtTestInfo> test_ranges_;
  std::vector<LineRange> expect_ranges_;
public:
  ~RgtParserImpl() {
    delete[] file_buf_;
  }

  int ReadFile(const char *file_path);
  void LexFile();
  const char *LexDirective(const char *buf, int &line);
  void ParseTokens();
  void ParseTest(TokenVector::const_iterator &current);
  LineRange ParseTestBody(TokenVector::const_iterator &current);
  void ParseExpectFailure(TokenVector::const_iterator &current);
  const RgtTestInfo *FindTest(int line) const;
  int GetAssertEndLine(int line) const;

  // Methods for internal testing only.
  void UseBuffer(StringRef buf);
  void PrintTokens(std::string &out) const { tokens_.PrintTokens(out); }
};

// Read the specified file into an internal buffer. Returns errno.
int RgtParserImpl::ReadFile(const char *file_path) {
  // Allow reuse of parsers.
  delete[] file_buf_;

  errno = 0;
  FILE *file = posix::FOpen(file_path, "r");
  if (!file)
    return errno;
  posix::StatStruct info;
  int stat_ret = posix::Stat(file_path, &info);
  if (stat_ret < 0) {
    posix::FClose(file);
    return errno;
  }
  size_t fsize = (size_t)info.st_size;
  file_buf_ = new char[fsize + 1];
  if (!file_buf_) {
    posix::FClose(file);
    return ENOMEM;
  }
  size_t bytes_read = fread(file_buf_, 1, fsize, file);
  if (bytes_read != fsize) {
    delete[] file_buf_;
    posix::FClose(file);
    return errno ? errno : ERANGE;
  }
  file_buf_[bytes_read] = 0;
  posix::FClose(file);
  return 0;
}

// Use the specified buffer, instead of reading a file. For testing.
void RgtParserImpl::UseBuffer(StringRef buf) {
  // We need to copy buf, because the dtor wil delete file_buf_.
  delete[] file_buf_;
  file_buf_ = new char[buf.Length() + 1];
  GTEST_DISABLE_MSC_WARNINGS_PUSH_(4996 /* deprecated function */)
  strncpy(file_buf_, buf.Data(), buf.Length());
  GTEST_DISABLE_MSC_WARNINGS_POP_()
  file_buf_[buf.Length()] = 0;
}

// Preprocessor directives occupy one full (possibly continued) line.
// Primarily we care about classifying the directive, and then skipping
// ahead to the end of the line.
const char *RgtParserImpl::LexDirective(const char *buf, int &line) {
  assert (*buf == '#');

  // Skip horizontal whitespace.
  do {
    ++buf;
  } while (*buf == ' ' || *buf == '\t');

  if (IsIdStart(*buf)) {
    // We've found a directive word. It might be one we care about.
    StringRef word = LexWord(&buf);
    TokenKind kind = LexWordToDirective(word);
    // We treat "#if 0" specially.
    if (kind == kTokenHashIf && word.Equals("if")) {
      // LexWord left buf pointing at the 'f' of 'if', so we need to
      // step past that before checking for whitespace.
      do {
        ++buf;
      } while (*buf == ' ' || *buf == '\t');
      if (buf[0] == '0' && std::isspace(buf[1]))
        kind = kTokenHashIf0;
    }
    // Tell the token buffer about conditional directives; ignore others.
    if (kind != kTokenWord)
      tokens_.Save({kind, word, line});
  }
  // Skip to the (possibly continued) EOL and return the updated pointer.
  return SkipToContinuedEol(buf, line);
}

// Tokenize the file/buffer.
void RgtParserImpl::LexFile() {
  tokens_.clear();
  const char *buf = file_buf_;
  if (!buf)
    return;

  // Detect the preprocessor # only at the start of a line.
  bool hash_ok = true;

  for (int line = 1; *buf; ++buf) {
    // We always look at whitespace, even if we're skipping.
    // Lines need counting, and leading whitespace before '#' of a
    // directive needs to be stepped over.
    if (std::isspace(*buf)) {
      switch (*buf) {
      case ' ':
      case '\t':
        // Non-newline whitespace gets ignored.
        break;
      case '\r':
        // CRLF increments line when it sees the LF.
        // Handle a bare CR as if it had a LF after it.
        if (buf[1] != '\n') {
          ++line;
          hash_ok = true;
        }
        break;
      case '\n':
        // Bare LF or second half of a CRLF.
        ++line;
        hash_ok = true;
        break;
      default:
        // Form feed and vertical tab.
        break;
      }
      // Nothing else to do on this iteration.
      continue;
    }

    // We've reached non-whitespace. If we're in an if-d out block,
    // we need to skip the rest of the line, unless this is a #directive.
    if (tokens_.Skipping() && *buf != '#') {
      buf = SkipToEol(buf);
      continue;
    }

    // Get here if we find non-whitespace and we're not skipping.
    // Or, we are skipping, but we have to look at a #directive.
    switch (*buf) {
    case '/':
      // Comments don't turn into a token, but standalone slash does.
      if (buf[1] == '/') {
        buf = SkipToEol(&buf[2]);
      } else if (buf[1] == '*') {
        buf = LexCComment(buf, line);
      } else {
        // This is a standalone slash.
        tokens_.Save({kTokenOther, {buf, 1}, line});
        hash_ok = false;
      }
      break;
    case '{':
      tokens_.Save({kTokenLBrace, {buf, 1}, line});
      hash_ok = false;
      break;
    case '}':
      tokens_.Save({kTokenRBrace, {buf, 1}, line});
      hash_ok = false;
      break;
    case '(':
      tokens_.Save({kTokenLParen, {buf, 1}, line});
      hash_ok = false;
      break;
    case ')':
      tokens_.Save({kTokenRParen, {buf, 1}, line});
      hash_ok = false;
      break;
    case ',':
      tokens_.Save({kTokenComma, {buf, 1}, line});
      hash_ok = false;
      break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      {
        StringRef lit = LexDigitString(&buf);
        tokens_.Save({kTokenDigitString, lit, line});
        hash_ok = false;
      }
      break;
    case '\'':
    case '"':
      {
        // Allow continuation lines; assign the start line to the token.
        int start_line = line;
        StringRef lit = LexStringLit(&buf, line);
        tokens_.Save({kTokenStringLit, lit, start_line});
        hash_ok = false;
      }
      break;
    case '#':
      if (hash_ok) {
        // This handles an entire line, so hash_ok remains true.
        buf = LexDirective(buf, line);
      } else {
        tokens_.Save({kTokenOther, {buf, 1}, line});
        // hash_ok = false;
      }
      break;
    default:
      if (!IsIdStart(*buf)) {
        tokens_.Save({kTokenOther, {buf, 1}, line});
      } else {
        StringRef word = LexWord(&buf);
        TokenKind kind = LexWordToKeyword(word);
        // Don't allow whitespace before the " of a raw literal.
        if (kind == kTokenRawLitIntro) {
          if (buf[1] == '"') {
            tokens_.Save({kind, word, line});
            ++buf;
            int start_line = line;
            StringRef lit = LexRawStringLit(&buf, line);
            tokens_.Save({kTokenStringLit, lit, start_line});
          } else {
            // No double-quote so this is just a normal word.
            tokens_.Save({kTokenWord, word, line});
          }
        } else {
          tokens_.Save({kind, word, line});
        }
      }
      hash_ok = false;
      break;
    }
  }
  tokens_.Save({kTokenEof, {buf, 0}, 0});
}

// Parse EXPECT_[NON]FATAL_FAILURE(contents) and save the line range so
// we can exclude the (failing) contents from being reported as rotten.
// On entry, current should be the macro keyword; on exit, it is one past
// the closing paren. The contents must have correctly nested parens.
void RgtParserImpl::ParseExpectFailure(TokenVector::const_iterator &current) {
  assert(current->Kind() == kTokenExpectFailure);
  int start_line = current->Line();
  int end_line = 0;
  ++current;
  // If we're not looking at (contents) then someone has decided to use
  // a macro name in some other context.
  if (current->Kind() != kTokenLParen)
    return;
  ++current;
  for (unsigned nesting = 1; nesting; ++current) {
    switch (current->Kind()) {
    case kTokenLParen:
      ++nesting;
      break;
    case kTokenRParen:
      end_line = current->Line();
      --nesting;
      break;
    default:
      break;
    }
  }
  expect_ranges_.push_back({start_line, end_line});
}

// Parse a test method body, i.e., a correctly nested set of braces.
// On entry, current should be the opening brace; on exit, it is one token
// past the matching close brace.
// Return value is a pair of line numbers for the open/close brace.
// Because the input is known to be a legal C++ source, we don't worry about
// running off the end.
LineRange RgtParserImpl::ParseTestBody(TokenVector::const_iterator &current) {
  if (current->Kind() != kTokenLBrace)
    return {0, 0};
  int start_line = current->Line();
  int end_line = 0;
  ++current;
  for (unsigned nesting = 1; nesting; ++current) {
    switch (current->Kind()) {
    case kTokenLBrace:
      ++nesting;
      break;
    case kTokenRBrace:
      end_line = current->Line();
      --nesting;
      break;
    case kTokenExpectFailure:
      ParseExpectFailure(current);
      break;
    default:
      break;
    }
  }
  return {start_line, end_line};
}

// Parse the parameters to a TEST(id, id) invocation, and return
// success/failure.
static bool ParseTestParams(TokenVector::const_iterator &current) {
  if (current->Kind() != kTokenLParen)
    return false;
  ++current;
  if (!IsAnyWord(current->Kind()))
    return false;
  ++current;
  if (current->Kind() != kTokenComma)
    return false;
  ++current;
  if (!IsAnyWord(current->Kind()))
    return false;
  ++current;
  if (current->Kind() != kTokenRParen)
    return false;
  ++current;
  return true;
}

// Parse one TEST(id, id) declaration, and its test body.
void RgtParserImpl::ParseTest(TokenVector::const_iterator &current) {
  assert(current->Kind() == kTokenTest);
  StringRef test_kind = current->Id();
  ++current;
  if (!ParseTestParams(current))
    return;
  LineRange lines = ParseTestBody(current);
  // Parsing mishaps indicated by returning {0, 0}.
  if (lines.start != 0)
    test_ranges_.push_back({test_kind, lines});
}

// Parse the whole file.
void RgtParserImpl::ParseTokens() {
  // Find all the TEST macros.
  TokenVector::const_iterator current = tokens_.cbegin();
  TokenVector::const_iterator end = tokens_.cend();
  while (current != end) {
    if (current->Kind() == kTokenTest)
      ParseTest(current);
    else
      ++current;
  }
}

// Look up a Test from a line number that it contains.
const RgtTestInfo *RgtParserImpl::FindTest(int line) const {
  // RgtTestInfo entries naturally do not overlap and are sorted by
  // line number, which makes binary search algorithms the thing.
  // lower_bound will give us the first Test whose range ends
  // after the requested line.
  auto it = std::lower_bound(test_ranges_.begin(), test_ranges_.end(),
                             line,
                   [](const RgtTestInfo &lhs, int rhs) {
                     return lhs.lines.end < rhs; });
  if (it == test_ranges_.end() || it->lines.start > line)
    return nullptr;
  return &*it;
}

// Get the end of the EXPECT_[NON]FATAL_FAILURE range, if this line is
// part of one; otherwise return the input line.
// We don't do anything fancy because we expect expect_ranges_ to be
// small or empty.
int RgtParserImpl::GetAssertEndLine(int line) const {
  // Ranges don't overlap so this is relatively simple.
  for (auto entry : expect_ranges_) {
    if (entry.start <= line && line <= entry.end)
      return entry.end;
  }
  return line;
}

// The parser interface used by the RGT client.

RgtParser::RgtParser() : impl_(new RgtParserImpl), parsed_(false) {}

RgtParser::~RgtParser() {
  delete impl_;
}

void RgtParser::Parse(const char *file_path) {
  // Any inability to read the file simply means we have
  // no information to report.
  // Note: Any changes here probably need to be reflected in ParseString.
  parsed_ = false;
  if (impl_->ReadFile(file_path))
    return;
  impl_->LexFile();
  impl_->ParseTokens();
  parsed_ = true;
}

const RgtTestInfo *RgtParser::FindTest(int line) const {
  if (!parsed_)
    return nullptr;
  return impl_->FindTest(line);
}

int RgtParser::GetAssertEndLine(int line) const {
  return impl_->GetAssertEndLine(line);
}

// Internal testing methods.

void RgtParser::ParseString(StringRef buffer) {
  // Note: Any changes here probably need to be reflected in Parse.
  parsed_ = false;
  impl_->UseBuffer(buffer);
  impl_->LexFile();
  impl_->ParseTokens();
  parsed_ = true;
}

void RgtParser::PrintTokens(std::string &out) const {
  impl_->PrintTokens(out);
}

} // end namespace internal
} // end namespace testing

#endif // GTEST_HAS_RGT
