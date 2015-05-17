/// \file
/// Base functions to initialize and manipulate any input stream
///

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

#include <antlr3/CharStream.hpp>
#include <antlr3/ConvertUTF.hpp>

template<class StringTraits>
antlr3_defs::ItemPtr antlr3<StringTraits>::CharStream::itemFromChar(std::uint32_t c) {
    size_t n = static_cast<size_t>(c);
    void* p = reinterpret_cast<void*>(n);
    return ItemPtr(p, [](void*){});
}

template<class StringTraits>
std::uint32_t antlr3<StringTraits>::CharStream::charFromItem(ItemPtr const & item) {
    size_t n = reinterpret_cast<size_t>(item.get());
    return static_cast<std::uint32_t>(n);
}

template<class StringTraits>
antlr3_defs::ItemPtr antlr3<StringTraits>::CharStream::LI(std::int32_t i)
{
    return itemFromChar(this->LA(i));
}

template<class StringTraits>
template<class CodeUnit>
antlr3<StringTraits>::BasicCharStream<CodeUnit>::BasicCharStream(DataRef data, String name)
    : CharStream()
    , streamName_(std::move(name))
    , data_(std::move(data))
    , lastPos_(data_.begin())
    , currentPos_(data_.begin())
    , lines_({data_.begin()})
    , newlineChar_('\n')
{
}

template<class StringTraits>
template<class CodeUnit>
antlr3<StringTraits>::BasicCharStream<CodeUnit>::~BasicCharStream()
{
}

template<class StringTraits>
template<class CodeUnit>
typename antlr3<StringTraits>::String antlr3<StringTraits>::BasicCharStream<CodeUnit>::sourceName()
{
    return streamName_;
}

template<class StringTraits>
template<class CodeUnit>
void antlr3<StringTraits>::BasicCharStream<CodeUnit>::consume()
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

template<class StringTraits>
template<class CodeUnit>
std::uint32_t antlr3<StringTraits>::BasicCharStream<CodeUnit>::LA(std::int32_t i)
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

template<class StringTraits>
template<class CodeUnit>
antlr3_defs::MarkerPtr antlr3<StringTraits>::BasicCharStream<CodeUnit>::mark()
{
    return std::make_shared<CharStreamMarker>(currentPos_, this->shared_from_this());
}

template<class StringTraits>
template<class CodeUnit>
typename antlr3<StringTraits>::Index
    antlr3<StringTraits>::BasicCharStream<CodeUnit>::index()
{
    return currentPos_ - data_.begin();
}

template<class StringTraits>
template<class CodeUnit>
void antlr3<StringTraits>::BasicCharStream<CodeUnit>::seek(Index index)
{
    assert(index >= BasicCharStream<CodeUnit>::index());
    auto p = data_.begin() + index;
    currentPos_ = std::min(lastPos_, p);
    while (currentPos_ < p) {
        consume();
    }
}

// CharStream

template<class StringTraits>
template<class CodeUnit>
antlr3_defs::Location antlr3<StringTraits>::BasicCharStream<CodeUnit>::location(Index index)
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

template<class StringTraits>
template<class CodeUnit>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::BasicCharStream<CodeUnit>::substr(Index start, Index stop)
{
    CodeUnit const * b = data_.begin() + start;
    CodeUnit const * e = data_.begin() + stop;
    return StringTraits::string(b, e);
}

template<class StringTraits>
template<class CodeUnit>
void antlr3<StringTraits>::BasicCharStream<CodeUnit>::reset()
{
    currentPos_ = data_.begin();
}

template<class StringTraits>
template<class CodeUnit>
std::uint32_t antlr3<StringTraits>::BasicCharStream<CodeUnit>::size()
{
    return (std::uint32_t)data_.size();
}

template<class StringTraits>
template<class CodeUnit>
std::uint8_t antlr3<StringTraits>::BasicCharStream<CodeUnit>::newLineChar() const
{
    return newlineChar_;
}

template<class StringTraits>
template<class CodeUnit>
void antlr3<StringTraits>::BasicCharStream<CodeUnit>::setNewLineChar(std::uint8_t newLineChar)
{
    newlineChar_ = newLineChar;
}

template<class StringTraits>
antlr3<StringTraits>::ByteCharStream::ByteCharStream(DataRef data, String name)
    : Base(std::move(data), std::move(name))
{}

template<class StringTraits>
antlr3<StringTraits>::ByteCharStream::ByteCharStream(void const * data, std::uint32_t size, String name)
    : Base(DataRef(reinterpret_cast<std::uint8_t const *>(data), size), std::move(name))
{}

template<class StringTraits>
antlr3<StringTraits>::ByteCharStream::ByteCharStream(void const * data, std::uint32_t size, Deleter deleter, String name)
    : Base(DataRef(reinterpret_cast<std::uint8_t const *>(data), size, deleter), std::move(name))
{}

template<class StringTraits>
antlr3<StringTraits>::ByteCharStream::~ByteCharStream() {}

template<class StringTraits>
antlr3<StringTraits>::UnicodeCharStream::UnicodeCharStream(void const * data, std::uint32_t size, String name, TextEncoding encoding)
    : Base(decodeData(data, size, encoding), std::move(name))
{}

template<class StringTraits>
antlr3<StringTraits>::UnicodeCharStream::~UnicodeCharStream() {}

template<class StringTraits>
typename antlr3<StringTraits>::UnicodeCharStream::DataRef
    antlr3<StringTraits>::UnicodeCharStream::decodeData(void const * data, std::uint32_t size, TextEncoding encoding)
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
        return DataRef(ANTLR3_T(""), std::size_t(0), [](Char const *) {});
    }
}

template<class StringTraits>
template<class UTF, class ByteOrder, class OutIterator>
void antlr3<StringTraits>::UnicodeCharStream::decode(void const * data, std::uint32_t size, OutIterator& begin, OutIterator end)
{
    std::uint32_t k = size % sizeof(UTF);
    auto dataStart = reinterpret_cast<std::uint8_t const *>(data);
    auto dataEnd = dataStart + size - k;
    
    typedef typename utf::CodeUnitForChar<Char>::type DestUTF;
    
    utf::CodeUnitIterator<decltype(dataStart), UTF, ByteOrder> srcStart(dataStart), srcEnd(dataEnd);
    utf::ConversionResult r = utf::Convert<UTF, DestUTF>(srcStart, srcEnd, begin, end, utf::ConversionFlags::LenientConversion);
    assert(r == utf::ConversionResult::ConversionOK || r == utf::ConversionResult::SourceExhausted);
    if (r == utf::ConversionResult::SourceExhausted || k > 0) {
        r = utf::Traits<DestUTF>::Write(begin, end, utf::UNI_REPLACEMENT_CHAR);
        assert(r == utf::ConversionResult::ConversionOK);
    }
}

template<class StringTraits>
template<class UTF, class ByteOrder>
typename antlr3<StringTraits>::UnicodeCharStream::DataRef
    antlr3<StringTraits>::UnicodeCharStream::decode(void const * data, std::uint32_t size)
{
    typedef typename utf::CodeUnitForChar<Char>::type DestUTF;
    utf::DummyWriteIterator<DestUTF> lenStart(0), lenEnd;
    decode<UTF, ByteOrder>(data, size, lenStart, lenEnd);

    std::size_t len = lenStart.pos();
    Deleter del = [](Char const * d) { delete[] d; };
    Char * ptr = new Char[len];
    DataRef retVal(ptr, len, std::move(del));
    decode<UTF, ByteOrder>(data, size, ptr, ptr + len);
    return std::move(retVal);
}

