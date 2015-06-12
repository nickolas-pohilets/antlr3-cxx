#ifndef _ANTLR3_STL_UTF8_HPP
#define _ANTLR3_STL_UTF8_HPP

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

class StdUTF8StringTraits {
public:
    typedef std::string String;
    typedef String::value_type Char;
    typedef StringLiteralRef<char, String> StringLiteral;
    typedef std::stringstream StringStream;
    
    template<class T> static antlr3_defs::TryNextStringLiteral selectLiteral(T);
    template<size_t N>
    static StringLiteral selectLiteral(char const (&s)[N]) { return StringLiteral(s, N - 1); }
    
    static String string(std::uint8_t const * b, std::uint8_t const * e) { return String(b, e); }
    static String string(Char const * b, Char const * e) { return String(b, e); }
    
    static String toString(int val)                { return std::to_string(val); }
    static String toString(long val)               { return std::to_string(val); }
    static String toString(long long val)          { return std::to_string(val); }
    static String toString(unsigned val)           { return std::to_string(val); }
    static String toString(unsigned long val)      { return std::to_string(val); }
    static String toString(unsigned long long val) { return std::to_string(val); }
    static String toString(float val)              { return std::to_string(val); }
    static String toString(double val)             { return std::to_string(val); }
    static String toString(long double val)        { return std::to_string(val); }
    
    static std::string toUTF8(String s) { return std::move(s); }
    static StringLiteral toUTF8(StringLiteral s) { return s; }
    static String fromUTF8(std::string s) { return std::move(s); }
    static StringLiteral fromUTF8(char const * s) { return StringLiteral(s, strlen(s)); }

    static std::string& appendToUTF8(std::string& s8, String const & s) { return s8 += s; }
    static std::string& appendToUTF8(std::string& s8, StringLiteral s) { return s8 += s; }
    static String& appendUTF8(String& s, std::string const & s8) { return s += s8; }
    static String& appendUTF8(String& s, char const * s8) { return s += s8; }
};

} // namespace antlr3ex

extern template class antlr3<antlr3ex::StdUTF8StringTraits>;

#endif //_ANTLR3_STL_UTF8_HPP
