#ifndef _ANTLR3_QT5_HPP
#define _ANTLR3_QT5_HPP

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
#include <QtCore/QString>
#include <QtCore/QTextStream>

namespace antlr3ex {

class Qt5String : public QString
{
public:
    using QString::QString;
    Qt5String() {}
    Qt5String(QString const & other) : QString(other) {}
    Qt5String(QString && other) : QString(std::move(other)) {}
    
    bool empty() const { return isEmpty(); }
};

template<>
class StringLiteralHelper<char16_t, Qt5String> {
public:
    static Qt5String reserve(size_t n) {
        Qt5String retVal;
        retVal.reserve((int)n);
        return std::move(retVal);
    }
    static Qt5String make(char16_t const * b, char16_t const * e) {
        static_assert(sizeof(QChar) == sizeof(char16_t), "Size of QChar and char16_t should match");
        return Qt5String(reinterpret_cast<QChar const *>(b), (int)(e - b));
    }
    static Qt5String & append(Qt5String & s, char16_t const * b, char16_t const * e) {
        static_assert(sizeof(QChar) == sizeof(char16_t), "Size of QChar and char16_t should match");
        s.append(reinterpret_cast<QChar const *>(b), (int)(e - b));
        return s;
    }
};

class Qt5StringTraits {
public:
    /// Class of the mutable string
    typedef Qt5String String;
    /// Class of the element of String or StringLiteral
    typedef char16_t Char;
    /// Thin wrapper around string literal that can be used together with String.
    typedef StringLiteralRef<char16_t, String> StringLiteral;
    /// Stream object that can be used to construct String
    typedef QTextStream StringStream;
    
    template<class T> static antlr3_defs::TryNextStringLiteral selectLiteral(T);
    template<size_t N>
    static StringLiteral selectLiteral(char16_t const (&s)[N]) { return StringLiteral(s, N - 1); }
    
    static String string(std::uint8_t const * b, std::uint8_t const * e) {
        return QLatin1String(reinterpret_cast<char const *>(b), (int)(e - b));
    }
    static String string(Char const * b, Char const * e) {
        static_assert(sizeof(Char) == sizeof(ushort), "Sizes of QChar and ushort should match");
        return QString::fromUtf16(reinterpret_cast<ushort const *>(b), (int)(e - b));
    }
    
    static String toString(int val)                { return QString::number(val); }
    static String toString(long val)               { return QString::number(val); }
    static String toString(long long val)          { return QString::number(val); }
    static String toString(unsigned val)           { return QString::number(val); }
    static String toString(unsigned long val)      { return QString::number(val); }
    static String toString(unsigned long long val) { return QString::number(val); }
    static String toString(float val)              { return QString::number(val); }
    static String toString(double val)             { return QString::number(val); }
    static String toString(long double val)        { return QString::number((double)val); }
    
    static std::string toUTF8(String const & s);
    static std::string toUTF8(StringLiteral s);
    static String fromUTF8(std::string const & s);
    static String fromUTF8(char const * s);

    static std::string& appendToUTF8(std::string& s8, String const & s);
    static std::string& appendToUTF8(std::string& s8, StringLiteral s);
    static String& appendUTF8(String& s, std::string const & s8);
    static String& appendUTF8(String& s, char const * s8);
    
    static String& appendEscape(String& dest, String const & src);
    static String& appendEscape(String& dest, StringLiteral const & src);
    static String& appendEscape(String& dest, Char src);
    static String& appendEscape(String& dest, std::uint32_t src);
    
    template<class T>
    static String escape(T const & x) {
        String retVal;
        appendEscape(retVal, x);
        return std::move(retVal);
    }
};

} // namespace antlr3ex

namespace std {
std::ostream& operator<<(std::ostream& s, antlr3ex::Qt5String const & str);
} // namespace std

extern template class antlr3<antlr3ex::Qt5StringTraits>;

#endif //_ANTLR3_QT5_HPP
