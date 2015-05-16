/**
 * \file
 * Defines the basic structures of an ANTLR3 bitset. this is a C version of the 
 * cut down Bitset class provided with the java version of antlr 3.
 * 
 * 
 */
#ifndef _ANTLR3_BITSET_HPP
#define _ANTLR3_BITSET_HPP

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
#include <antlr3/String.hpp>

namespace antlr3 {

class Bitset
{
public:
    typedef std::vector<Bitword> Data;

    static Bitset fromData(Data data);
    static Bitset fromBits(std::vector<std::uint32_t> const & inBits);
    static Bitset fromBits(std::int32_t bit, ...);

    Bitset();
    Bitset(std::uint32_t numBits);
    Bitset(Bitset const &) = default;
    Bitset(Bitset &&) = default;
    ~Bitset();

    Bitset bor(Bitset const & other) const;
    void borInPlace(Bitset const & other);
    std::uint32_t size() const;
    void add(std::uint32_t bit);
    bool equals(Bitset const & other) const;
    bool isMember(std::uint32_t bit) const;
    std::uint32_t capacity() const;
    void remove(std::uint32_t bit);
    bool isNilNode() const;
    std::vector<std::uint32_t> toIntList() const;

    String toString() const { return toString(nullptr); }
    String toString(std::function<String(std::uint32_t)> tokenNamer) const;
private:
    /// The actual bits themselves
	///
	Data bits_;

    Bitset(Data data);
    void growToSize(std::size_t size);
};

} // namespace antlr3

#endif

