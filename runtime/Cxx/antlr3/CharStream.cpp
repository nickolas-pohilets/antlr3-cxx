/// \file
/// Base functions to initialize and manipulate any input stream
///

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

#include <antlr3/CharStream.hpp>
#include <antlr3/ConvertUTF.hpp>

namespace antlr3 {

ItemPtr CharStream::itemFromChar(Char c) {
    size_t n = static_cast<size_t>(c);
    void* p = reinterpret_cast<void*>(n);
    return ItemPtr(p, [](void*){});
}

Char CharStream::charFromItem(ItemPtr const & item) {
    size_t n = reinterpret_cast<size_t>(item.get());
    return static_cast<Char>(n);
}

ItemPtr CharStream::LI(std::int32_t i)
{
    return itemFromChar(LA(i));
}

template<class CodeUnit>
BasicCharStream<CodeUnit>::BasicCharStream(DataRef data, String name)
    : CharStream()
    , streamName_(std::move(name))
    , data_(std::move(data))
    , lastPos_(data_.begin())
    , currentPos_(data_.begin())
    , lines_({data_.begin()})
    , newlineChar_('\n')
{
}

template<class CodeUnit>
BasicCharStream<CodeUnit>::~BasicCharStream()
{
}

template<class CodeUnit>
String BasicCharStream<CodeUnit>::sourceName()
{
    return streamName_;
}

template<class CodeUnit>
void BasicCharStream<CodeUnit>::consume()
{
    if (currentPos_ != data_.end())
    {
        std::uint32_t c = read(currentPos_);
        ++currentPos_;
        if(currentPos_ > lastPos_) {
            lastPos_ = currentPos_;
            if (c == newlineChar_) {
                lines_.push_back(currentPos_);
            }
        }
    }
}

template<class CodeUnit>
std::uint32_t BasicCharStream<CodeUnit>::LA(std::int32_t i)
{
    if (i > 0)
    {
        CodeUnit const * ptr = currentPos_ + (i - 1);
        if (ptr < data_.end())
        {
            return read(ptr);
        }
        return CharstreamEof;
    }
    else if(i < 0)
    {
        CodeUnit const * ptr = currentPos_ + i;
        if (ptr >= data_.begin())
        {
            return read(ptr);
        }
        return CharstreamEof;
    }
    else
    {
        assert(false);
        return CharstreamEof;
    }
}

template<class CodeUnit>
MarkerPtr BasicCharStream<CodeUnit>::mark()
{
    return std::make_shared<CharStreamMarker>(currentPos_, this->shared_from_this());
}

template<class CodeUnit>
Index BasicCharStream<CodeUnit>::index()
{
    return currentPos_ - data_.begin();
}

template<class CodeUnit>
void BasicCharStream<CodeUnit>::seek(Index index)
{
    assert(index >= BasicCharStream<CodeUnit>::index());
    auto p = data_.begin() + index;
    currentPos_ = std::min(lastPos_, p);
    while (currentPos_ < p) {
        consume();
    }
}

// CharStream

template<class CodeUnit>
Location BasicCharStream<CodeUnit>::location(Index index)
{
    CodeUnit const * ptr = data_.begin() + index;
    if (ptr < data_.begin() || ptr > data_.end()) {
        assert(false);
        return location(data_.end() - data_.begin());
    }
    if (ptr > lastPos_) {
        assert(false && "Should not access locations in not read area");
        auto tmp = currentPos_;
        currentPos_ = lastPos_;
        while (lastPos_ < ptr) {
            consume();
        }
        currentPos_ = tmp;
    }
    auto it = std::lower_bound(lines_.begin(), lines_.end(), ptr);
    if (it == lines_.end() || *it > ptr) {
        assert(it > lines_.begin());
        --it;
    }
    size_t line = it - lines_.begin();
    size_t charPos = ptr - *it;
    return Location(std::uint32_t(1 + line), std::uint32_t(1 + charPos));
}

template<class CodeUnit>
String BasicCharStream<CodeUnit>::substr(Index start, Index stop)
{
    CodeUnit const * b = data_.begin() + start;
    CodeUnit const * e = data_.begin() + stop;
    return String(b, e);
}

template<class CodeUnit>
void BasicCharStream<CodeUnit>::reset()
{
    currentPos_ = data_.begin();
}

template<class CodeUnit>
std::uint32_t BasicCharStream<CodeUnit>::size()
{
    return (std::uint32_t)data_.size();
}

template<class CodeUnit>
std::uint8_t BasicCharStream<CodeUnit>::newLineChar() const
{
    return newlineChar_;
}

template<class CodeUnit>
void BasicCharStream<CodeUnit>::setNewLineChar(std::uint8_t newLineChar)
{
    newlineChar_ = newLineChar;
}

ByteCharStream::ByteCharStream(DataRef data, String name)
    : BasicCharStream(std::move(data), std::move(name))
{}

ByteCharStream::ByteCharStream(void const * data, std::uint32_t size, String name)
    : BasicCharStream(DataRef(reinterpret_cast<std::uint8_t const *>(data), size), std::move(name))
{}

ByteCharStream::ByteCharStream(void const * data, std::uint32_t size, Deleter deleter, String name)
    : BasicCharStream(DataRef(reinterpret_cast<std::uint8_t const *>(data), size, deleter), std::move(name))
{}

ByteCharStream::~ByteCharStream() {}

UnicodeCharStream::UnicodeCharStream(void const * data, std::uint32_t size, String name, TextEncoding encoding)
    : BasicCharStream(decodeData(data, size, encoding), std::move(name))
{}

UnicodeCharStream::~UnicodeCharStream() {}

UnicodeCharStream::DataRef UnicodeCharStream::decodeData(void const * data, std::uint32_t size, TextEncoding encoding)
{
    switch (encoding) {
    case TextEncoding::UTF8:
        return decode<utf::UTF8, utf::LE>(data, size);
    case TextEncoding::UTF16LE:
        return decode<utf::UTF16, utf::LE>(data, size);
    case TextEncoding::UTF16BE:
        return decode<utf::UTF16, utf::BE>(data, size);
    case TextEncoding::UTF32LE:
        return decode<utf::UTF32, utf::LE>(data, size);
    case TextEncoding::UTF32BE:
        return decode<utf::UTF32, utf::BE>(data, size);
    default:
        assert(false && !"Unknown encoding");
        return DataRef(ANTLR3_T(""), std::size_t(0), [](CharType const *) {});
    }
}
    
template<class UTF, class ByteOrder, class OutIterator>
void UnicodeCharStream::decode(void const * data, std::uint32_t size, OutIterator& begin, OutIterator end)
{
    std::uint32_t k = size % sizeof(UTF);
    auto dataStart = reinterpret_cast<std::uint8_t const *>(data);
    auto dataEnd = dataStart + size - k;
    
    typedef utf::CodeUnitForChar<CharType>::type DestUTF;
    
    utf::CodeUnitIterator<decltype(dataStart), UTF, ByteOrder> srcStart(dataStart), srcEnd(dataEnd);
    utf::ConversionResult r = utf::Convert<UTF, DestUTF>(srcStart, srcEnd, begin, end, utf::ConversionFlags::LenientConversion);
    assert(r == utf::ConversionResult::ConversionOK || r == utf::ConversionResult::SourceExhausted);
    if (r == utf::ConversionResult::SourceExhausted || k > 0) {
        r = utf::Traits<DestUTF>::Write(begin, end, utf::UNI_REPLACEMENT_CHAR);
        assert(r == utf::ConversionResult::ConversionOK);
    }
}

template<class UTF, class ByteOrder>
UnicodeCharStream::DataRef UnicodeCharStream::decode(void const * data, std::uint32_t size)
{
    typedef utf::CodeUnitForChar<CharType>::type DestUTF;
    utf::DummyWriteIterator<DestUTF> lenStart(0), lenEnd;
    decode<UTF, ByteOrder>(data, size, lenStart, lenEnd);

    std::size_t len = lenStart.pos();
    Deleter del = [](CharType const * d) { delete[] d; };
    CharType * ptr = new CharType[len];
    DataRef retVal(ptr, len, std::move(del));
    decode<UTF, ByteOrder>(data, size, ptr, ptr + len);
    return std::move(retVal);
}

} // namespace antlr3
