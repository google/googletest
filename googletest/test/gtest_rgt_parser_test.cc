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

// Test for the RgtParser class.

#include "gtest/internal/gtest-rgt-parser.h"
#include "gtest/gtest.h"

#if GTEST_HAS_RGT

using ::testing::internal::RgtParser;
using ::testing::internal::RgtTestInfo;
using ::testing::internal::LineRange;
using ::testing::internal::StringRef;

namespace {

// Lexing tests.

TEST(LexerTest, TokenizeEmpty) {
  const char* const empty_input = "";
  const char* const empty_output = "<EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(empty_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(empty_output, output);
}

TEST(LexerTest, TokenizeSimple) {
  const char* const simple_input =
    "123456789 0x0123456789abcdef \n"
    "(),{} #error\n";

  const char* const simple_output =
    "Line 1: 123456789 0 x0123456789abcdef \n"
    "Line 2: ( ) , { } other error <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(simple_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(simple_output, output);
}

TEST(LexerTest, TokenizeStringLit) {
  const char* const string_lit_input =
    "\"StringLit1\" \"StringLit2\" 'c'\n";

  const char* const string_lit_output =
    "Line 1: \"StringLit1\" \"StringLit2\" 'c' <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(string_lit_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(string_lit_output, output);
}

TEST(LexerTest, TokenizeContinuedStringLit) {
  const char* const cont_string_lit_input =
    "\"StringLit1\\\nStringLit2\\\rStringLit3\\\r\nStringLit4\" 'c'\n";

  const char* const cont_string_lit_output =
    "Line 1: \"StringLit1\\\nStringLit2\\\rStringLit3\\\r\nStringLit4\" \n"
    "Line 4: 'c' <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(cont_string_lit_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(cont_string_lit_output, output);
}

TEST(LexerTest, TokenizeRawStringLitEmpty) {
  const char* const raw_string_empty_input =
    "R\"()\"";
  const char* const raw_string_empty_output =
    "Line 1: RawLitIntro(R) \"()\" <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(raw_string_empty_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(raw_string_empty_output, output);
}

TEST(LexerTest, TokenizeRawStringLitSimple) {
  const char* const raw_string_simple_input =
    "R\"(stuff))\"";
  const char* const raw_string_simple_output =
    "Line 1: RawLitIntro(R) \"(stuff))\" <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(raw_string_simple_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(raw_string_simple_output, output);
}

TEST(LexerTest, TokenizeRawStringLitMultiline) {
  const char* const raw_string_multiline_input =
    "R\"(stuff line 1\nstuff line 2)\" \n word";
  const char* const raw_string_multiline_output =
    "Line 1: RawLitIntro(R) \"(stuff line 1\nstuff line 2)\" \n"
    "Line 3: word <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(raw_string_multiline_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(raw_string_multiline_output, output);
}

TEST(LexerTest, TokenizeRawStringLitDelimited) {
  const char* const raw_string_delimited_input =
    "R\"delim(stuff))more stuff) delim\" )delim\"";
  const char* const raw_string_delimited_output =
    "Line 1: RawLitIntro(R) \"delim(stuff))more stuff) delim\" )delim\" <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(raw_string_delimited_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(raw_string_delimited_output, output);
}

TEST(LexerTest, TokenizeComments) {
  // The tokenizer eats comments, particularly ignoring non-lex-able
  // things like un-paired single or double quotes, or /*, or whatever.
  const char* const comment_input =
    "start // comment /* \n"
    "// comment with crlf \r\n"
    "/* C comment ' \" */\n"
    "/**/ part 1\n"
    "/*/ multi-line comment with tricky slash, \n"
    "   still counts lines correctly */\n"
    "part 2\n"
    "/*\n"
    "  newline right after /* works\r\n"
    "  embedded crlf also work\r\n"
    "*/ trailer\n"
    "end\n";

  const char* const comment_output =
    "Line 1: start \n"
    "Line 4: part 1 \n"
    "Line 7: part 2 \n"
    "Line 11: trailer \n"
    "Line 12: end <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(comment_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(comment_output, output);
}

TEST(LexerTest, TokenizeDirectives) {
  // The tokenizer eats preprocessor directives so the parser doesn't
  // have to deal with them. Conditional directives affect what's saved,
  // other directives are ignored.
  const char* const directive_input =
    "#if something\n"
    "visible tokens\n"
    "#error oops\n"
    "#else\n"
    "hidden tokens\n"
    "#endif\n"
    "# if 0\n"
    "if-d out tokens\n"
    "#\telse\n"
    "more visible tokens\n"
    "#endif\n"
    " #if x\n"
    "visible1\n"
    "#elif y\n"
    "invisible1\n"
    "#else\n"
    "hidden 2\n"
    "#endif\n"
    "#if 0\n"
    "inivisible2\n"
    "#elif 1\n"
    "visible2\n"
    "#endif";

  const char* const directive_output =
    "Line 2: visible tokens \n"
    "Line 10: more visible tokens \n"
    "Line 13: visible1 \n"
    "Line 22: visible2 <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(directive_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(directive_output, output);
}

TEST(LexerTest, TokenizeIfdOutBlocks) {
  // Text inside an #if'd out block isn't necessarily lex-able or parse-able.
  // In particular it might have un-paired single or double quotes, or
  // a /*, or something else we have to keep from interfering with lexing.
  // Basically we need to treat all of it like a comment.
  const char* const ifout_directive_input =
    "#if 0\n"
    " if'd out tokens\n"
    "#else\n"
    "visible tokens\n"
    "#endif\n"
    "#if ok\n"
    "more visible tokens\n"
    "#elif\n"
    "quote\"\n"
    "#else\n"
    " /* \n"
    "#endif\n";

  const char* const ifout_directive_output =
    "Line 4: visible tokens \n"
    "Line 7: more visible tokens <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(ifout_directive_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(ifout_directive_output, output);
}

TEST(LexerTest, TokenizeTestMacros) {
  // The tokenizer prints these with distinct punctuation so we can tell
  // they are tokenized specially. Case must match to count as special.
  const char* const macro_input =
    "TEST TEST_F TEST_P TYPED_TEST TYPED_TEST_P\n"
    "test test_f test_p typed_test typed_test_p\n"
    "EXPECT_FATAL_FAILURE EXPECT_NONFATAL_FAILURE\n"
    "expect_fatal_failure expect_nonfatal_failure\n";

  const char* const macro_output =
    "Line 1: *TEST* *TEST_F* *TEST_P* *TYPED_TEST* *TYPED_TEST_P* \n"
    "Line 2: test test_f test_p typed_test typed_test_p \n"
    "Line 3: !EXPECT_FATAL_FAILURE! !EXPECT_NONFATAL_FAILURE! \n"
    "Line 4: expect_fatal_failure expect_nonfatal_failure <EOF> \n";

  RgtParser parser;
  std::string output;

  parser.ParseString(macro_input);
  ASSERT_TRUE(parser.Parsed());
  parser.PrintTokens(output);
  EXPECT_EQ(macro_output, output);
}

TEST(ParserTest, ParseEmpty) {
  const char* const empty_input =
    "";

  RgtParser parser;
  parser.ParseString(empty_input);
  ASSERT_TRUE(parser.Parsed());

  EXPECT_EQ(nullptr, parser.FindTest(1));
  EXPECT_EQ(1, parser.GetAssertEndLine(1));
}

TEST(ParserTest, ParseBasic) {
  const char* const basic_input =
    "TEST(MyTest, /* embedded comment ignored */ One) {\n"
    " TEST(Illegal, Ignored)\n"
    "}\n";

  RgtParser parser;
  parser.ParseString(basic_input);
  ASSERT_TRUE(parser.Parsed());

  EXPECT_EQ(nullptr, parser.FindTest(0));
  const RgtTestInfo *info1 = parser.FindTest(1);
  const RgtTestInfo *info2 = parser.FindTest(2);
  ASSERT_NE(nullptr, info1);
  EXPECT_EQ(info1, info2);
  EXPECT_TRUE(info1->test_kind.Equals("TEST"));
  EXPECT_EQ(1, info1->lines.start);
  EXPECT_EQ(3, info1->lines.end);
}

TEST(ParserTest, ParseOneLine) {
  const char* const oneline_input =
    "TEST(MyTest, One) { TEST(Illegal, Ignored); }\n"
    "\n"
    "TEST_F(MyTest, Two) { EXPECT_EQ(0, 0); }\n";

  RgtParser parser;
  parser.ParseString(oneline_input);
  ASSERT_TRUE(parser.Parsed());

  const RgtTestInfo *info1 = parser.FindTest(1);
  const RgtTestInfo *info2 = parser.FindTest(2);
  const RgtTestInfo *info3 = parser.FindTest(3);
  ASSERT_NE(nullptr, info1);
  EXPECT_TRUE(info1->test_kind.Equals("TEST"));
  EXPECT_EQ(1, info1->lines.start);
  EXPECT_EQ(1, info1->lines.end);
  EXPECT_EQ(nullptr, info2);
  ASSERT_NE(nullptr, info3);
  EXPECT_TRUE(info3->test_kind.Equals("TEST_F"));
  EXPECT_EQ(3, info3->lines.start);
  EXPECT_EQ(3, info3->lines.end);
}

TEST(ParserTest, ParseEachKind) {
  const char* const each_kind_input =
    "TEST(MyTest, One) {\n"
    "}\n"
    "TEST_F(MyTest, Two) {\n"
    "}\n"
    "TEST_P(MyTest, Three) {\n"
    "}\n"
    "TYPED_TEST(MyTest, Four) {\n"
    "}\n"
    "TYPED_TEST_P(MyTest, Five) {\n"
    "}\n"
    "GTEST_TEST(MyTest, Six) {\n"
    "}\n"
    "GTEST_TEST_F(MyTest, Seven) {\n"
    "}\n";

  RgtParser parser;
  parser.ParseString(each_kind_input);
  ASSERT_TRUE(parser.Parsed());

  const RgtTestInfo *info;
  info = parser.FindTest(1);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(1, info->lines.start);
  EXPECT_EQ(2, info->lines.end);

  info = parser.FindTest(3);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST_F"));
  EXPECT_EQ(3, info->lines.start);
  EXPECT_EQ(4, info->lines.end);

  info = parser.FindTest(5);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST_P"));
  EXPECT_EQ(5, info->lines.start);
  EXPECT_EQ(6, info->lines.end);

  info = parser.FindTest(7);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TYPED_TEST"));
  EXPECT_EQ(7, info->lines.start);
  EXPECT_EQ(8, info->lines.end);

  info = parser.FindTest(9);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TYPED_TEST_P"));
  EXPECT_EQ(9, info->lines.start);
  EXPECT_EQ(10, info->lines.end);

  info = parser.FindTest(11);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("GTEST_TEST"));
  EXPECT_EQ(11, info->lines.start);
  EXPECT_EQ(12, info->lines.end);

  info = parser.FindTest(13);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("GTEST_TEST_F"));
  EXPECT_EQ(13, info->lines.start);
  EXPECT_EQ(14, info->lines.end);
}

TEST(ParserTest, ParseExpectFailure) {
  const char* const expect_fail_input =
    "TEST(MyTest, One) {\n"
    "  EXPECT_FATAL_FAILURE(all on one line);\n"
    "  EXPECT_NONFATAL_FAILURE(spanning\n"
    "                          multiple\n"
    "                          lines);\n"
    "}\n";

  RgtParser parser;
  parser.ParseString(expect_fail_input);
  ASSERT_TRUE(parser.Parsed());

  const RgtTestInfo *info = parser.FindTest(1);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(1, info->lines.start);
  EXPECT_EQ(6, info->lines.end);

  EXPECT_EQ(2, parser.GetAssertEndLine(2));
  EXPECT_EQ(5, parser.GetAssertEndLine(3));
}

// Per preprocessor rules, if a macro is defined with params, using the
// macro name without params isn't a macro invocation.
TEST(ParserTest, MacroParamNames) {
  const char* const macro_input =
    "TEST(TEST, TEST_F) {\n"
    "}\n"
    "TEST(TEST_P, TYPED_TEST) {\n"
    "}\n"
    "TEST(TYPED_TEST_P, EXPECT_EQ) {\n"
    "}\n"
    "TEST(EXPECT_FATAL_FAILURE, EXPECT_NONFATAL_FAILURE) {\n"
    "}\n";

  RgtParser parser;
  parser.ParseString(macro_input);
  ASSERT_TRUE(parser.Parsed());

  const RgtTestInfo *info;

  info = parser.FindTest(1);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(1, info->lines.start);
  EXPECT_EQ(2, info->lines.end);
  info = parser.FindTest(3);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(3, info->lines.start);
  EXPECT_EQ(4, info->lines.end);
  info = parser.FindTest(5);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(5, info->lines.start);
  EXPECT_EQ(6, info->lines.end);
  info = parser.FindTest(7);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(7, info->lines.start);
  EXPECT_EQ(8, info->lines.end);
}

TEST(ParserTest, ParseConditional) {
  const char* const conditional_input =
    "#if condition\n"
    "TEST(MyTest, true) {\n"
    "#else\n"
    "TEST_F(MyTest, false) {\n"
    "#endif\n"
    "}\n"
    "#if 0\n"
    "TYPED_TEST(MyTest, zero)\n"
    "#else\n"
    "TYPED_TEST_P(MyTest, one)\n"
    "#endif\n"
    "{ }\n";

  RgtParser parser;
  parser.ParseString(conditional_input);
  ASSERT_TRUE(parser.Parsed());

  const RgtTestInfo *info;

  info = parser.FindTest(2);
  ASSERT_NE(nullptr, info);
  EXPECT_EQ(info, parser.FindTest(4));
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(2, info->lines.start);
  EXPECT_EQ(6, info->lines.end);

  EXPECT_EQ(nullptr, parser.FindTest(8));
  EXPECT_EQ(nullptr, parser.FindTest(10));

  info = parser.FindTest(12);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TYPED_TEST_P"));
  EXPECT_EQ(12, info->lines.start);
  EXPECT_EQ(12, info->lines.end);
}

TEST(ParserTest, ParseWithEmbeddedDirective) {
  const char* const embedded_input =
    "TEST(MyTest, \n"
    "#warning ) {\n"
    "    RealName) {\n"
    "}\n";

  RgtParser parser;
  parser.ParseString(embedded_input);
  ASSERT_TRUE(parser.Parsed());

  const RgtTestInfo *info;

  info = parser.FindTest(3);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(3, info->lines.start);
  EXPECT_EQ(4, info->lines.end);
}

#if GTEST_HAS_FILE_SYSTEM
// Read and parse this very source file. Checking a lot of it would
// make the test excessively fragile, but we can readily verify that
// this test method was parsed correctly. Among other things, it's
// inside a namespace which shows we recognize tests even if they're
// not at file level.
TEST(ParserTest, ParseFile) {
  int start_line = __LINE__ - 1; // Obviously must be the first line.

  // __FILE__ might be relative, and the cwd when the test is run might
  // not be the same as the compilation directory. So, verify a source of
  // the same name is available, and if it is, assume it's the correct
  // source file.
  testing::internal::FilePath filepath(__FILE__);
  if (!filepath.FileOrDirectoryExists())
    GTEST_SKIP();

  RgtParser parser;
  parser.Parse(__FILE__);
  ASSERT_TRUE(parser.Parsed());

  const RgtTestInfo *info = parser.FindTest(start_line);
  ASSERT_NE(nullptr, info);
  EXPECT_TRUE(info->test_kind.Equals("TEST"));
  EXPECT_EQ(start_line, info->lines.start);
  // Be careful here!
  int end_line = __LINE__ + 2;
  EXPECT_EQ(end_line, info->lines.end);
}
#endif // GTEST_HAS_FILE_SYSTEM

} // namespace

#endif // GTEST_HAS_RGT
