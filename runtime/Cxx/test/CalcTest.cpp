#include <gtest/gtest.h>
#include "generated/CalcLexer.hpp"
#include "generated/CalcParser.hpp"
#include "generated/CalcASTLexer.hpp"
#include "generated/CalcASTParser.hpp"
#include "generated/EvalAST.hpp"

namespace {

typedef std::string (*ParseFunc)(char const * data, std::uint32_t size);

std::string parse(char const * data, std::uint32_t size)
{
    auto nullDeleter = [](std::uint8_t const *) {};
    auto inputStream = std::make_shared<antlr3::ByteCharStream>(data, size, nullDeleter, ANTLR3_T(""));
    auto lexer = std::make_shared<CalcLexer>(inputStream);
    auto tokenStream = std::make_shared<antlr3::CommonTokenStream>(lexer);
    CalcParser parser(tokenStream);

    parser_context ctx;
    parser.c = &ctx;
    parser.prog();
    parser.c = NULL;

    return ctx.ss.str();
}

std::string parseWithAST(char const * data, std::uint32_t size)
{
    auto nullDeleter = [](std::uint8_t const *) {};
    auto inputStream = std::make_shared<antlr3::ByteCharStream>(data, size, nullDeleter, ANTLR3_T(""));
    auto lexer = std::make_shared<CalcLexer>(inputStream);
    auto tokenStream = std::make_shared<antlr3::CommonTokenStream>(lexer);
    CalcASTParser parser(tokenStream);

    CalcASTParser_prog_return r = parser.prog();

    auto treeStream = std::make_shared<antlr3::CommonTreeNodeStream>(r.tree);
    EvalAST treeParser(treeStream);

    parser_context2 ctx;
    treeParser.c = &ctx;
    treeParser.prog();
    treeParser.c = NULL;
    
    return ctx.ss.str();
}

class CalcTest : public testing::TestWithParam<ParseFunc>
{
public:
    template<unsigned N>
    std::string parse(char const (& text)[N])
    {
        ParseFunc f = GetParam();
        return (*f)(text, N - 1);
    }
};

} // namespace

TEST_P(CalcTest, TestIt)
{
    EXPECT_EQ(parse("4+5*2\n"),"14\n");
    EXPECT_EQ(parse("(4+5)*2\n"),"18\n");
    EXPECT_EQ(parse("a=4+5*2\na-6\n"),"8\n");
}

TEST_P(CalcTest, TestLexerError)
{
    EXPECT_EQ(parse("4+@\n"),"4\n");
}

TEST_P(CalcTest, TestParserError)
{
    EXPECT_EQ(parse("(3\n"),"3\n");
}

TEST_P(CalcTest, TestUnknownVariable)
{
    EXPECT_EQ(parse("a=5\nb - 7\n"),"ERROR: Unknown variable \"b\"\n-7\n");
}

INSTANTIATE_TEST_CASE_P(CalcTestInstance, CalcTest, ::testing::Values(&parse, &parseWithAST));
