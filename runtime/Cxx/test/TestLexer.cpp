#include <gtest/gtest.h>
#include "BaseTest.hpp"
#include "P1Lexer.hpp"
#include "P1Parser.hpp"
#include "P2Lexer.hpp"
#include "P2Parser.hpp"
#include "P3Lexer.hpp"
#include "P3Parser.hpp"
#include "P4Lexer.hpp"
#include "P4Parser.hpp"
using namespace antlr3;

TEST(TestLexer, testLocation)
{
    auto inputStream = makeCharStream("34-31 22\n0 1\n14");
    auto lexer = std::make_shared<P2Lexer>(inputStream);
    static struct { size_t offset, line, charPos; } const data[] = {
        {  0, 0, 0 }, // 34
        {  2, 0, 2 }, // -31
        {  5, 0, 5 }, // <WS>
        {  6, 0, 6 }, // 22
        {  8, 0, 8 }, // \n
        {  9, 1, 0 }, // 0
        { 10, 1, 1 }, // <WS>
        { 11, 1, 2 }, // 1
        { 12, 1, 3 }, // \n
        { 13, 2, 0 }, // 14
        { 15, 2, 2 }, // EOF
        { 15, 2, 2 }
    };
    static size_t n = std::end(data) - std::begin(data);
    for (size_t i = 1; i < n; ++i) {
        CommonTokenPtr t = lexer->nextToken();
        antlr3::Location start = t->startLocation();
        antlr3::Location stop = t->stopLocation();
        ASSERT_EQ(t->startIndex(), data[i-1].offset);
        ASSERT_EQ(start.line(), data[i-1].line);
        ASSERT_EQ(start.charPositionInLine(), data[i-1].charPos);
        ASSERT_EQ(t->stopIndex(), data[i].offset);
        ASSERT_EQ(stop.line(), data[i].line);
        ASSERT_EQ(stop.charPositionInLine(), data[i].charPos);
    }
}

TEST(TestLexer, testSet)
{
    String s = execParser<P1Parser, P1Lexer>(&P1Parser::a, "\\t");
    ASSERT_EQ(s, ANTLR3_T("\t"));
}

TEST(TestLexer, testRefToRuleDoesNotSetTokenNorEmitAnother)
{
    String s = execParser<P2Parser, P2Lexer>(&P2Parser::a, "-34");
    ASSERT_EQ(s, ANTLR3_T("-34"));
}

//TEST(TestLexer, testTokenText)
//{
//    String s = execParser<P3Parser, P3Lexer>(&P3Parser::decl, "int a;");
//    ASSERT_EQ(s, ANTLR3_T("var a:int"));
//}

TEST(TestLexer, testTokenList)
{
    String s = execParser<P4Parser, P4Lexer>(&P4Parser::idList, "a,b,c");
    ASSERT_EQ(s, ANTLR3_T("3"));
}
