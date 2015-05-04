#ifndef _ANTLR3_TEST_BASE_TEST_HPP_
#define _ANTLR3_TEST_BASE_TEST_HPP_

#include <antlr3/antlr3.hpp>

antlr3::CharStreamPtr makeCharStream(char const * text) {
    return std::make_shared<antlr3::UnicodeCharStream>(
       text, (std::uint32_t)strlen(text),
       "test_input", antlr3::TextEncoding::UTF8
    );
}

template<class ParserT, class LexerT>
antlr3::String execParser(void (ParserT::*method)(), char const * text) {
    auto inputStream = makeCharStream(text);
    auto lexer = std::make_shared<LexerT>(inputStream);
    antlr3::CommonTokenStreamPtr tokenStream(new antlr3::CommonTokenStream(lexer));
    ParserT parser(tokenStream);
    (parser.*method)();
    return parser.c.str();
}


#endif // _ANTLR3_TEST_BASE_TEST_HPP_
