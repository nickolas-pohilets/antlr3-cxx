#ifndef _ANTLR3_ESCAPING_UTILS_HPP
#define _ANTLR3_ESCAPING_UTILS_HPP

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

#include <antlr3/String.hpp>

namespace antlr3ex {

template<class StringTraits>
class EscapingUtils {
public:
    typedef typename StringTraits::String String;
    typedef typename StringTraits::Char Char;
    typedef typename StringTraits::StringLiteral StringLiteral;
    
    template<class T>
    static String& appendEscapedString(String& dest, T const & src) {
        for (auto c : src)
        {
            appendEscapedChar(dest, c);
        }
        return dest;
    }
    
    template<class T>
    static String& appendEscapedChar(String& dest, T src) {
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
    
    static String& appendEscapedANTLRChar(String& dest, std::uint32_t src) {
        if (src == antlr3_defs::CharstreamEof) {
            return dest += ANTLR3_T("<EOF>");
        }
        std::uint32_t maxChar = std::uint32_t(1) << (CHAR_BIT * sizeof(Char));
        if (src < maxChar) {
            return appendEscapedChar(dest, (Char)src);
        }
        dest += ANTLR3_T("\\u");
        char buffer[16];
        sprintf(buffer, "%04X", (unsigned)src);
        StringTraits::appendUTF8(dest, buffer);
        return dest;
    }
};

} // namespace antlr3ex


#endif // _ANTLR3_ESCAPING_UTILS_HPP
