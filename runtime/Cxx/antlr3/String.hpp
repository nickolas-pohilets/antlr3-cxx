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

template<class T>
class antlr3_defs::StringLiteralRef {
public:
    typedef T                                       value_type;
    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef T const *                               pointer;
    typedef T const *                               const_pointer;
    typedef const_pointer                           iterator;
    typedef const_pointer                           const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    
    StringLiteralRef(T const * data, size_t len) : data_(data), len_(len) {}
    
    bool empty() const { return len_ == 0; }
    size_t size() const { return len_; }
    
    const_iterator begin() const { return data_; }
    const_iterator end() const { return data_ + len_; }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
private:
    T const * data_;
    size_t len_;
};
    
class antlr3_defs::StdUTF16StringTraits {
public:
    /// Class of the mutable string
    typedef std::u16string String;
    /// Class of the element of String or StringLiteral
    typedef String::value_type Char;
    /// Thin wrapper around string literal.
    typedef StringLiteralRef<char16_t> StringLiteral;
    /// Stream object that can be used to construct String
    typedef std::basic_stringstream<char16_t> StringStream;
    
    template<size_t N8, size_t N16, size_t N32>
    static StringLiteral literal(
        char const (&)[N8],
        char16_t const (& s)[N16],
        char32_t const (&)[N32]
    ) { return StringLiteralRef<char16_t>(s, N16); }
    
    String string(std::uint8_t const * b, std::uint8_t const * e) { return String(b, e); }
    String string(Char const * b, Char const * e) { return String(b, e); }
    
    static String toString(int val)                { return fromLatin1(std::to_string(val)); }
    static String toString(long val)               { return fromLatin1(std::to_string(val)); }
    static String toString(long long val)          { return fromLatin1(std::to_string(val)); }
    static String toString(unsigned val)           { return fromLatin1(std::to_string(val)); }
    static String toString(unsigned long val)      { return fromLatin1(std::to_string(val)); }
    static String toString(unsigned long long val) { return fromLatin1(std::to_string(val)); }
    static String toString(float val)              { return fromLatin1(std::to_string(val)); }
    static String toString(double val)             { return fromLatin1(std::to_string(val)); }
    static String toString(long double val)        { return fromLatin1(std::to_string(val)); }
private:
    static String fromLatin1(std::string const & s)
    {
        return String(s.begin(), s.end());
    }
};
    
class antlr3_defs::StdUTF8StringTraits {
public:
    typedef std::string String;
    typedef String::value_type Char;
    typedef StringLiteralRef<char> StringLiteral;
    typedef std::stringstream StringStream;
    
    template<size_t N8, size_t N16, size_t N32>
    static StringLiteral literal(
        char const (&)[N8],
        char16_t const (& s)[N16],
        char32_t const (&)[N32]
    ) { return StringLiteral(s, N16); }
    
    String string(std::uint8_t const * b, std::uint8_t const * e) { return String(b, e); }
    String string(Char const * b, Char const * e) { return String(b, e); }
    
    static String toString(int val)                { return std::to_string(val); }
    static String toString(long val)               { return std::to_string(val); }
    static String toString(long long val)          { return std::to_string(val); }
    static String toString(unsigned val)           { return std::to_string(val); }
    static String toString(unsigned long val)      { return std::to_string(val); }
    static String toString(unsigned long long val) { return std::to_string(val); }
    static String toString(float val)              { return std::to_string(val); }
    static String toString(double val)             { return std::to_string(val); }
    static String toString(long double val)        { return std::to_string(val); }
};

#define ANTLR3_T(X) StringTraits::literal(X, u##X, U##X)

//std::string toUTF8(String const & s);
//std::string toUTF8(StringLiteral s);
//String fromUTF8(std::string const & s);
//String fromUTF8(char const * s);
//
//std::string& appendToUTF8(std::string& s8, String const & s);
//std::string& appendToUTF8(std::string& s8, StringLiteral s);
//String& appendUTF8(String& s, std::string const & s8);
//String& appendUTF8(String& s, char const * s8);
//
//std::ostream& operator<<(std::ostream& s, const String& str);
//std::ostream& operator<<(std::ostream& s, StringLiteral str);


template<class StringTraits>
class antlr3<StringTraits>::StringUtils {
public:
    typedef typename StringTraits::String String;
    typedef typename StringTraits::Char Char;
    typedef typename StringTraits::StringLiteral StringLiteral;
    
    static String& appendEscape(String& dest, String const & src) {
        for (Char c : src)
        {
            appendEscape(dest, c);
        }
        return dest;
    }
    
    static String& appendEscape(String& dest, StringLiteral const & src) {
        for (Char c : src)
        {
            appendEscape(dest, c);
        }
        return dest;
    }
    
    static String& appendEscape(String& dest, Char src) {
        switch (src)
        {
            case '\"':
                dest += ANTLR3_T("\\\"");
                break;
            case '\n':
                dest += ANTLR3_T("\\n");
                break;
            case '\r':
                dest += ANTLR3_T("\\r");
                break;
            default:
                dest += src;
                break;
        }
        return dest;
    }
    
    String& appendEscape(String& dest, std::uint32_t src) {
        if (src == CharstreamEof) {
            return dest += ANTLR3_T("<EOF>");
        }
        std::uint32_t maxChar = std::uint32_t(1) << (CHAR_BIT * sizeof(Char));
        if (src < maxChar) {
            return appendEscape(dest, (Char)src);
        }
        dest += "\\u";
        char buffer[16];
        sprintf(buffer, "%04X", (unsigned)src);
        dest.append(buffer, buffer + strlen(buffer));
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