#ifndef _ANTLR3_STL_UTF16_HPP
#define _ANTLR3_STL_UTF16_HPP

// [The "BSD licence"]
// Copyright (c) 20013-2015 Nickolas Pohilets
// Copyright (c) 2005-2009 Jim Idle, Temporal Wave LLC
// http://www.temporal-wave.com
// http://www.linkedin.com/in/jimidle
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <antlr3/antlr3.cpp>
#include <antlr3/StringLiteral.hpp>

namespace antlr3ex {

class StdUTF16StringTraits {
public:
    /// Class of the mutable string
    typedef std::u16string String;
    /// Class of the element of String or StringLiteral
    typedef String::value_type Char;
    /// Thin wrapper around string literal that can be used together with String.
    typedef StringLiteralRef<char16_t> StringLiteral;
    /// Stream object that can be used to construct String
    typedef std::basic_stringstream<char16_t> StringStream;
    
    template<size_t N8, size_t N16, size_t N32>
    static StringLiteral literal(
        char const (&)[N8],
        char16_t const (& s)[N16],
        char32_t const (&)[N32]
    ) { return StringLiteral(s, N16); }
    
    static String string(std::uint8_t const * b, std::uint8_t const * e) { return String(b, e); }
    static String string(Char const * b, Char const * e) { return String(b, e); }
    
    static String toString(int val)                { return fromLatin1(std::to_string(val)); }
    static String toString(long val)               { return fromLatin1(std::to_string(val)); }
    static String toString(long long val)          { return fromLatin1(std::to_string(val)); }
    static String toString(unsigned val)           { return fromLatin1(std::to_string(val)); }
    static String toString(unsigned long val)      { return fromLatin1(std::to_string(val)); }
    static String toString(unsigned long long val) { return fromLatin1(std::to_string(val)); }
    static String toString(float val)              { return fromLatin1(std::to_string(val)); }
    static String toString(double val)             { return fromLatin1(std::to_string(val)); }
    static String toString(long double val)        { return fromLatin1(std::to_string(val)); }
    
    static std::string toUTF8(String const & s);
    static std::string toUTF8(StringLiteral s);
    static String fromUTF8(std::string const & s);
    static String fromUTF8(char const * s);

    static std::string& appendToUTF8(std::string& s8, String const & s);
    static std::string& appendToUTF8(std::string& s8, StringLiteral s);
    static String& appendUTF8(String& s, std::string const & s8);
    static String& appendUTF8(String& s, char const * s8);
private:
    static String fromLatin1(std::string const & s)
    {
        return String(s.begin(), s.end());
    }
};

} // namespace antlr3ex

namespace std {
std::ostream& operator<<(std::ostream& s, antlr3ex::StdUTF16StringTraits::String const & str);
std::ostream& operator<<(std::ostream& s, antlr3ex::StdUTF16StringTraits::StringLiteral str);
} // namespace std {

extern template class antlr3<antlr3ex::StdUTF16StringTraits>;

#endif //_ANTLR3_STL_UTF16_HPP
