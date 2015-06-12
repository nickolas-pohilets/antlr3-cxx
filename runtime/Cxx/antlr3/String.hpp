/** \file
 * Simple string interface allows indiscriminate allocation of strings
 * such that they can be allocated all over the place and released in 
 * one chunk via a string factory - saves lots of hassle in remembering what
 * strings were allocated where.
 */
#ifndef _ANTLR3_STRING_HPP
#define _ANTLR3_STRING_HPP

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

#include <antlr3/Defs.hpp>
#include <string>
#include <sstream>

namespace antlr3_detail {

template<class T> T const & make();

template<class StringTraits, class T, class... Args> class StringLiteralHelper;

template<class StringTraits, class T, class X, class... Args> class StringLiteralHelper<StringTraits, T, X, Args...> {
public:
    typedef T CompilerLiteral;
    
    static T literal(X const & x, Args const & ... args) {
        return StringTraits::selectLiteral(x);
    }
};

template<class StringTraits, class X, class Y, class... Args> class StringLiteralHelper<StringTraits, antlr3_defs::TryNextStringLiteral, X, Y, Args...> {
public:
    typedef StringLiteralHelper<StringTraits, decltype(StringTraits::selectLiteral(make<Y>())), Y, Args...> Next;
    typedef typename Next::CompilerLiteral CompilerLiteral;
    static CompilerLiteral literal(X const & x, Y const & y, Args const & ... args) {
        return Next::literal(y, args...);
    }
};

template<class StringTraits, class X> class StringLiteralHelper<StringTraits, antlr3_defs::TryNextStringLiteral, X> {
private:
    typedef void CompilerLiteral;
};

template<class StringTraits, class X, class... Args>
typename StringLiteralHelper<StringTraits, decltype(StringTraits::selectLiteral(make<X>())), X, Args...>::CompilerLiteral
    selectLiteral(X const & x, Args const &... args) {
    return StringLiteralHelper<StringTraits, decltype(StringTraits::selectLiteral(make<X>())), X, Args...>::literal(x, args...);
}

} // namespace antlr3_detail

#ifdef __OBJC__
#define ANTLR3_COMPILER_STRING_LITERALS(X) X, u##X, @X
#else
#define ANTLR3_COMPILER_STRING_LITERALS(X) X, u##X
#endif

#define ANTLR3_T(X) antlr3_detail::selectLiteral<StringTraits>(ANTLR3_COMPILER_STRING_LITERALS(X))

//std::string toUTF8(String const & s);
//std::string toUTF8(StringLiteral s);
//String fromUTF8(std::string const & s);
//String fromUTF8(char const * s);
//
//std::string& appendToUTF8(std::string& s8, String const & s);
//std::string& appendToUTF8(std::string& s8, StringLiteral s);
//String& appendUTF8(String& s, std::string const & s8);
//String& appendUTF8(String& s, char const * s8);


template<class StringTraits>
class antlr3<StringTraits>::StringUtils {
public:
    typedef typename StringTraits::String String;
    typedef typename StringTraits::Char Char;
    typedef typename StringTraits::StringLiteral StringLiteral;
    
    template<class T>
    static String& appendEscape(String& dest, T const & src) {
        for (T c : src)
        {
            appendEscape(dest, c);
        }
        return dest;
    }
    
    static String& appendEscape(String& dest, Char src) {
        if (src == '\"') {
            dest += ANTLR3_T("\\\"");
        } else if (src == '\n') {
            dest += ANTLR3_T("\\n");
        } else if (src == '\r') {
            dest += ANTLR3_T("\\r");
        } else {
            dest += src;
        }
        return dest;
    }
    
    static String& appendEscape(String& dest, std::uint32_t src) {
        if (src == CharstreamEof) {
            return dest += ANTLR3_T("<EOF>");
        }
        std::uint32_t maxChar = std::uint32_t(1) << (CHAR_BIT * sizeof(Char));
        if (src < maxChar) {
            return appendEscape(dest, (Char)src);
        }
        dest += ANTLR3_T("\\u");
        char buffer[16];
        sprintf(buffer, "%04X", (unsigned)src);
        StringTraits::appendUTF8(dest, buffer);
        return dest;
    }
    
    template<class T>
    static String escape(T const & x) {
        String retVal;
        appendEscape(retVal, x);
        return std::move(retVal);
    }
};

#endif