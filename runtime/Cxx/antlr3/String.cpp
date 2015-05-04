/** \file
 * Implementation of the ANTLR3 string and string factory classes
 */

// [The "BSD licence"]
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
#include <antlr3/ConvertUTF.hpp>
#include <antlr3/CommonToken.hpp>

namespace antlr3 {

#if ANTLR3_UTF16

namespace {

    template<class SrcChar, class DstChar> struct ConvertionFunc;
    template<> struct ConvertionFunc<char, char16_t> {
        template<class SrcIt, class DstIt>
        static void convert(SrcIt& sourceStart, SrcIt sourceEnd, DstIt& targetStart, DstIt targetEnd) {
            utf::ConversionResult r = utf::Convert<utf::UTF8, utf::UTF16>(sourceStart, sourceEnd, targetStart, targetEnd, utf::ConversionFlags::LenientConversion);
            assert(r == utf::ConversionResult::ConversionOK);
        }
    };
    template<> struct ConvertionFunc<char16_t, char> {
        template<class SrcIt, class DstIt>
        static void convert(SrcIt& sourceStart, SrcIt sourceEnd, DstIt& targetStart, DstIt targetEnd) {
            utf::ConversionResult r = utf::Convert<utf::UTF16, utf::UTF8>(sourceStart, sourceEnd, targetStart, targetEnd, utf::ConversionFlags::LenientConversion);
            assert(r == utf::ConversionResult::ConversionOK);
        }
    };

    template<class Char> struct UTFType;
    template<> struct UTFType<char> { typedef utf::UTF8 t; };
    template<> struct UTFType<char16_t> { typedef utf::UTF16 t; };

    template<class SrcChar, class DstChar>
    std::basic_string<DstChar>& appendUTF(std::basic_string<DstChar>& dst, SrcChar const * src, size_t len)
    {
        typedef typename UTFType<SrcChar>::t SrcT;
        typedef typename UTFType<DstChar>::t DstT;

        size_t oldLen = dst.size();

        // 1 - Dry run - determine needed dst size
        {
            auto srcStart = reinterpret_cast<SrcT const *>(src);
            auto srcEnd = srcStart + len;
            utf::DummyWriteIterator<DstT> dstStart(0), dstEnd;
            ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
            size_t dstCapacity = dst.length() + dstStart.pos();
            dst.resize(dstCapacity);
        }

        // 2 - Actual conversion
        {
            auto srcStart = reinterpret_cast<SrcT const *>(src);
            auto srcEnd = srcStart + len;
            auto dstStart = reinterpret_cast<DstT *>(&dst[0] + oldLen);
            auto dstEnd = dstStart - oldLen + dst.capacity();
            ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
        }
        return dst;
    }

    String fromLatin1(std::string const & s)
    {
        return String(s.begin(), s.end());
    }
}

std::string toUTF8(String const & s)
{
    std::string retVal;
    appendToUTF8(retVal, s);
    return std::move(retVal);
}

std::string toUTF8(ConstString s)
{
    std::string retVal;
    appendToUTF8(retVal, s);
    return std::move(retVal);
}

String fromUTF8(std::string const & s)
{
    String retVal;
    appendUTF8(retVal, s);
    return std::move(retVal);
}

String fromUTF8(char const * s)
{
    String retVal;
    appendUTF8(retVal, s);
    return std::move(retVal);
}

std::string& appendToUTF8(std::string& s8, String const & s)
{
    return appendUTF(s8, s.c_str(), s.length());
}

std::string& appendToUTF8(std::string& s8, ConstString s)
{
    return appendUTF(s8, s, std::char_traits<char16_t>::length(s));
}

String& appendUTF8(String& s, std::string const & s8)
{
    return appendUTF(s, s8.c_str(), s8.length());
}

String& appendUTF8(String& s, char const * s8)
{
    return appendUTF(s, s8, s8 ? strlen(s8) : 0);
}

String toString(int val)                { return fromLatin1(std::to_string(val)); }
String toString(long val)               { return fromLatin1(std::to_string(val)); }
String toString(long long val)          { return fromLatin1(std::to_string(val)); }
String toString(unsigned val)           { return fromLatin1(std::to_string(val)); }
String toString(unsigned long val)      { return fromLatin1(std::to_string(val)); }
String toString(unsigned long long val) { return fromLatin1(std::to_string(val)); }
String toString(float val)              { return fromLatin1(std::to_string(val)); }
String toString(double val)             { return fromLatin1(std::to_string(val)); }
String toString(long double val)        { return fromLatin1(std::to_string(val)); }

std::ostream& operator<<(std::ostream& s, const String& str) { return s << toUTF8(str); }
std::ostream& operator<<(std::ostream& s, ConstString str) { return s << toUTF8(str); }

#else

String toString(int val)                { return std::to_string(val); }
String toString(long val)               { return std::to_string(val); }
String toString(long long val)          { return std::to_string(val); }
String toString(unsigned val)           { return std::to_string(val); }
String toString(unsigned long val)      { return std::to_string(val); }
String toString(unsigned long long val) { return std::to_string(val); }
String toString(float val)              { return std::to_string(val); }
String toString(double val)             { return std::to_string(val); }
String toString(long double val)        { return std::to_string(val); }

#endif

String escape(String const & str)
{
    String retVal;
    appendEscape(retVal, str);
    return std::move(retVal);
}

String& appendEscape(String& dest, String const & src)
{
    for (String::value_type c : src)
    {
        appendEscape(dest, c);
    }
    return dest;
}

String& appendEscape(String& dest, String::value_type src) {
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

String& appendEscape(String& dest, Char src) {
    Char maxChar = Char(1) << (8 * sizeof(String::value_type));
    if (src < maxChar) {
        return appendEscape(dest, (String::value_type)src);
    }
    if (src == TokenEof) {
        return dest += ANTLR3_T("<EOF>");
    }
    dest += "\\u";
    char buffer[16];
    sprintf(buffer, "%04X", (unsigned)src);
    dest.append(buffer, buffer + strlen(buffer));
    return dest;
}

} // namespace antlr3