#include <gtest/gtest.h>
#include "generated/Filter.hpp"


TEST(FilterTest, TestIt)
{
    auto data = u8",.,.abc=== !!!";
    auto size = strlen(data);
    auto nullDeleter = [](std::uint8_t const *) {};
    auto inputStream = std::make_shared<antlr3::ByteCharStream>(data, size, nullDeleter, ANTLR3_T(""));
    auto lexer = std::make_shared<Filter>(inputStream);
    
    static uint32_t const tokens[] = {
        Filter::ID, Filter::WS, antlr3::TokenEof
    };
    static size_t const n = std::end(tokens) - std::begin(tokens);
    for (size_t i = 0; i < n; ++i) {
        auto tok = lexer->nextToken();
        EXPECT_EQ(tokens[i], tok->type());
    }
}
