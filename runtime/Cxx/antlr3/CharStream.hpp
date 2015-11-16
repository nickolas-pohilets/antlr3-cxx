/** \file
 * Defines the basic structures used to manipulate character
 * streams from any input source. Any character size and encoding
 * can in theory be used, so long as a set of functinos is provided that
 * can return a 32 bit Integer representation of their characters amd efficiently mark and revert
 * to specific offsets into their input streams.
 */
#ifndef _ANTLR3_INPUT_HPP
#define _ANTLR3_INPUT_HPP

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

#include <antlr3/Defs.hpp>
#include <antlr3/String.hpp>
//#include <antlr3/CommonToken.hpp>
#include <antlr3/IntStream.hpp>
#include <antlr3/Location.hpp>
#include <type_traits>
#include <cstring>

namespace antlr3 {

class CharStream : public IntStream, public LocationSource
{
public:
    static ItemPtr itemFromChar(Char c);
    static Char charFromItem(ItemPtr const & item);
    
    virtual ItemPtr LI(std::int32_t i) override;

    /// Returns the line number of the current position in the input stream.
    /// Interpretation of line number is determined by the stream itself.
    virtual Location currentLocation() { return location(index()); }
};

template<class CodeUnit>
class BasicCharStream : public CharStream, public std::enable_shared_from_this<BasicCharStream<CodeUnit>>
{
public:
    typedef std::function<void(CodeUnit const *)> Deleter;
    class DataRef
    {
    public:
        DataRef() : ptr_(), end_() {}

        DataRef(CodeUnit const * data, std::size_t size)
        {
            Deleter del = [](CodeUnit const * d) { delete[] d; };
            ptr_ = decltype(ptr_)(new CodeUnit[size], std::move(del));
            memcpy(const_cast<CodeUnit*>(ptr_.get()), data, size);
        }

        DataRef(CodeUnit const * data, CodeUnit const * dataEnd)
            : DataRef(data, dataEnd - data)
        {}
        DataRef(CodeUnit const * data, std::size_t size, Deleter deleter)
            : ptr_(data, std::move(deleter)), end_(data + size)
        {}
        DataRef(CodeUnit const * data, CodeUnit const * dataEnd, Deleter deleter) : ptr_(data, deleter), end_(dataEnd) {}
        DataRef(DataRef &&) = default;

        CodeUnit const * begin() const { return ptr_.get(); }
        CodeUnit const * end() const { return end_; }
        size_t size() const { return end_ - ptr_.get(); }
    private:
        std::unique_ptr<CodeUnit const [], Deleter> ptr_;
        CodeUnit const * end_;
    };

    BasicCharStream(DataRef data, String name);
    ~BasicCharStream() override;

    // IntStream

    virtual String sourceName() override;
    virtual void consume() override;
    virtual std::uint32_t LA(std::int32_t i) override;
    virtual MarkerPtr mark() override;
    virtual Index index() override;
    virtual void seek(Index index) override;

    // CharStream

    virtual Location location(Index index) override;
    virtual String substr(Index start, Index stop) override;

    /// Resets the input stream to start reading from the begining.
    void reset();

    std::uint32_t size();

    /// Returns character that triggers line number increment.
    /// By default it is '\n'.
    /// If for some reason you do not want the counters and pointers to be restee, you can set the 
    /// chracter to some impossible character such as '\0' or whatever.
    /// This is a single character only, so choose the last character in a sequence of two or more.
    std::uint8_t newLineChar() const;
    void setNewLineChar(std::uint8_t newlineChar);
private:
    class CharStreamMarker : public Marker
    {
    public:
        CharStreamMarker(CodeUnit const * pos, std::shared_ptr<BasicCharStream<CodeUnit>> stream)
            : Marker()
            , pos_(pos)
            , stream_(stream)
        {}
        
        ~CharStreamMarker() {}
        
        CodeUnit const * pos_;
        std::shared_ptr<BasicCharStream<CodeUnit>> stream_;
        
        virtual void rewind() {
            assert(pos_ <= stream_->lastPos_);
            stream_->currentPos_ = pos_;
        }
    };

    /// Stream name used for error reporting.
    String streamName_;

    /// Smart pointer to the input buffer slice.
    DataRef data_;

    /** List of start of line offsets
     */
    std::vector<CodeUnit const *> lines_;
    
    /// Current position
    CodeUnit const * lastPos_;
    
    /// Last reached position
    CodeUnit const * currentPos_;
    
    /// Character that automatically causes an internal line count
    /// increment.
    ///
    std::uint8_t newlineChar_;

    static std::uint32_t read(CodeUnit const * ptr)
    {
        return std::uint32_t(typename std::make_unsigned<CodeUnit>::type(*ptr));
    }
};

class ByteCharStream : public BasicCharStream<std::uint8_t>
{
public:
    ByteCharStream(DataRef data, String name);
    ByteCharStream(void const * data, std::uint32_t size, String name);
    ByteCharStream(void const * data, std::uint32_t size, Deleter deleter, String name);
    ~ByteCharStream();
};

class UnicodeCharStream : public BasicCharStream<String::value_type>
{
    typedef String::value_type CharType;
    
    static DataRef decodeData(void const * data, std::uint32_t size, TextEncoding encoding);
    template<class UTF, class ByteOrder>
    static DataRef decode(void const * data, std::uint32_t size);
    template<class UTF, class ByteOrder, class OutIterator>
    static void decode(void const * data, std::uint32_t size, OutIterator& begin, OutIterator end);
public:
    UnicodeCharStream(void const * data, std::uint32_t size, String name, TextEncoding encoding);
    ~UnicodeCharStream();
};

} // namespace antlr3

#endif // _ANTLR3_INPUT_HPP
