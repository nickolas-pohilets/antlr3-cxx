#ifndef _ANTLR3_UTF_CONVERTION_UTILS_HPP
#define _ANTLR3_UTF_CONVERTION_UTILS_HPP

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

#include <antlr3/ConvertUTF.hpp>

namespace antlr3ex {

template<class Char> struct UTFType;
template<> struct UTFType<char> { typedef utf::UTF8 t; };
template<> struct UTFType<char16_t> { typedef utf::UTF16 t; };

template<class SrcChar, class DstChar>
struct ConvertionFunc {
    template<class SrcIt, class DstIt>
    static void convert(SrcIt& sourceStart, SrcIt sourceEnd, DstIt& targetStart, DstIt targetEnd) {
        typedef typename UTFType<SrcChar>::t SrcT;
        typedef typename UTFType<DstChar>::t DstT;
        utf::ConversionResult r = utf::Convert<SrcT, DstT>(sourceStart, sourceEnd, targetStart, targetEnd, utf::ConversionFlags::LenientConversion);
        assert(r == utf::ConversionResult::ConversionOK);
    }
};

template<class SrcChar, class DstString>
DstString& appendUTF(DstString& dst, SrcChar const * src, size_t len)
{
    typedef typename DstString::value_type DstChar;
    typedef typename UTFType<SrcChar>::t SrcT;
    typedef typename UTFType<DstChar>::t DstT;

    size_t oldLen = dst.size();

    // 1 - Dry run - determine needed dst size
    {
        static_assert(sizeof(SrcT) == sizeof(SrcChar), "Size of source UTF code unit should match with size of SrcChar");
        auto srcStart = reinterpret_cast<SrcT const *>(src);
        auto srcEnd = srcStart + len;
        utf::DummyWriteIterator<DstT> dstStart(0), dstEnd;
        ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
        size_t dstCapacity = dst.length() + dstStart.pos();
        dst.resize((typename DstString::size_type)dstCapacity);
    }

    // 2 - Actual conversion
    {
        static_assert(sizeof(SrcT) == sizeof(SrcChar), "Size of source UTF code unit should match with size of SrcChar");
        auto srcStart = reinterpret_cast<SrcT const *>(src);
        auto srcEnd = srcStart + len;
        auto dstStart = dst.begin() + oldLen;
        auto dstEnd = dstStart - oldLen + dst.capacity();
        ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
    }
    return dst;
}

template <class T>
class OutStreamIterator
    : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
    typedef T char_type;
    typedef std::char_traits<T> traits_type;
    typedef std::basic_ostream<T> ostream_type;
private:
    ostream_type* stream_;
public:
    OutStreamIterator() : stream_() {}
    OutStreamIterator(ostream_type& __s) : stream_(&__s) {}
    
    OutStreamIterator& operator=(const T& __value_) {
        *stream_ << __value_;
        return *this;
    }

    OutStreamIterator& operator*()     {return *this;}
    OutStreamIterator& operator++()    {return *this;}
    OutStreamIterator& operator++(int) {return *this;}
    
    bool operator==(OutStreamIterator) const { return false; }
    bool operator<(OutStreamIterator) const { return true; }
};

template<class SrcChar, class DstChar>
std::basic_ostream<DstChar> & outputUTF(std::basic_ostream<DstChar> & out, SrcChar const * src, size_t len) {
    typedef typename UTFType<SrcChar>::t SrcT;
    typedef typename UTFType<DstChar>::t DstT;
    auto srcStart = reinterpret_cast<SrcT const *>(src);
    auto srcEnd = srcStart + len;
    OutStreamIterator<DstChar> dstStart(out), dstEnd;
    ConvertionFunc<SrcChar, DstChar>::convert(srcStart, srcEnd, dstStart, dstEnd);
    return out;
}

} // namespace antlr3ex


#endif // _ANTLR3_UTF_CONVERTION_UTILS_HPP
