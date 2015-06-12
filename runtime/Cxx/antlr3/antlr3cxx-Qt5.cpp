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

#include <antlr3/antlr3cxx-Qt5.hpp>
#include <antlr3/UTFConvertionUtils.hpp>
#include <antlr3/EscapingUtils.hpp>

typedef antlr3ex::Qt5StringTraits StringTraits;
typedef StringTraits::String String;
typedef StringTraits::StringLiteral StringLiteral;

template class antlr3<StringTraits>;

namespace antlr3ex {

template<> struct UTFType<QChar> { typedef utf::UTF16 t; };

std::string StringTraits::toUTF8(String const & s) {
    return s.toStdString();
}

std::string StringTraits::toUTF8(StringLiteral s) {
    std::string retVal;
    appendToUTF8(retVal, s);
    return std::move(retVal);
}

String StringTraits::fromUTF8(std::string const & s) {
    return String::fromStdString(s);
}

String StringTraits::fromUTF8(char const * s) {
    return String::fromUtf8(s);
}

std::string& StringTraits::appendToUTF8(std::string& s8, String const & s) {
    return appendUTF(s8, reinterpret_cast<char16_t const*>(s.data()), s.size());
}

std::string& StringTraits::appendToUTF8(std::string& s8, StringLiteral s) {
    return appendUTF(s8, s.data(), s.size());
}

String& StringTraits::appendUTF8(String& s, std::string const & s8) {
    return appendUTF(s, s8.c_str(), s8.size());
}

String& StringTraits::appendUTF8(String& s, char const * s8) {
    return appendUTF(s, s8, s8 ? strlen(s8) : 0);
}

String& StringTraits::appendEscape(String& dest, String const & src) {
    return EscapingUtils<StringTraits>::appendEscapedString(dest, src);
}

String& StringTraits::appendEscape(String& dest, StringLiteral const & src) {
    return EscapingUtils<StringTraits>::appendEscapedString(dest, src);
}

String& StringTraits::appendEscape(String& dest, Char src) {
    return EscapingUtils<StringTraits>::appendEscapedChar(dest, src);
}

String& StringTraits::appendEscape(String& dest, std::uint32_t src) {
    return EscapingUtils<StringTraits>::appendEscapedANTLRChar(dest, src);
}

} // namespace antlr3ex

namespace std {

std::ostream& operator<<(std::ostream& s, String const & str) {
    return antlr3ex::outputUTF(s, reinterpret_cast<char16_t const*>(str.data()), str.size());
}

std::ostream& operator<<(std::ostream& s, StringLiteral str) {
    return antlr3ex::outputUTF(s, str.data(), str.size());
}

} // namespace std

