///
/// \file
/// Contains the C implementation of ANTLR3 bitsets as adapted from Terence Parr's
/// Java implementation.
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

#include <antlr3/Bitset.hpp>
#include <stdarg.h>

namespace antlr3 {
namespace {

/** How many bits in the elements
 */
std::uint32_t const BitsetBits = 64;

/** log2 of BitsetBits 2^BitsetLogBits = BitsetBits
 */
std::uint32_t const BitsetLogBits = 6;

/** We will often need to do a mod operator (i mod nbits).
 *  For powers of two, this mod operation is the
 *  same as:
 *   - (i & (nbits-1)).  
 *
 * Since mod is relatively slow, we use an easily
 * precomputed mod mask to do the mod instead.
 */
std::uint32_t const BitsetModMask = BitsetBits - 1;

std::uint64_t bitMask(std::uint32_t bitNumber)
{
    return  ((std::uint64_t)1) << (bitNumber & (BitsetModMask));
}

std::uint32_t wordNumber(std::uint32_t bit)
{
    return  bit >> BitsetLogBits;
}

}

Bitset::Bitset(std::vector<Bitword> data)
: bits_(std::move(data))
{
}

Bitset::Bitset()
{
}

Bitset::Bitset(std::uint32_t numBits)
{
    // Avoid memory thrashing at the up front expense of a few bytes
    //
    numBits = std::max(numBits, 8 * BitsetBits);

    // No we need to allocate the memory for the number of bits asked for
    // in multiples of std::uint64_t. 
    //
    std::uint32_t numElements = ((numBits - 1) >> BitsetLogBits) + 1;
    bits_.resize(numElements);
}

Bitset::~Bitset()
{
}

Bitset Bitset::fromData(std::vector<Bitword> data)
{
    return Bitset(std::move(data));
}

Bitset Bitset::fromBits(std::vector<std::uint32_t> const & inBits)
{
    Bitset retVal;

    // We have no idea what exactly is in the list
    // so create a default bitset and then just add stuff
    // as we enumerate.
    //
    for (std::uint32_t bit : inBits)
    {
        retVal.add(bit);
    }
    return std::move(retVal);
}


///
/// \brief
/// Creates a new bitset with at least one element, but as
/// many elements are required.
/// 
/// \param[in] bit
/// A variable number of bits to add to the set, ending in -1 (impossible bit).
/// 
/// \returns
/// A new bit set with all of the specified elements added into it.
/// 
/// Call as:
///  - Bitset = Bitset::fromBits(n, n1, n2, -1);
///  - Bitset = Bitset::fromBits(-1);  Create empty bitset 
///
/// \remarks
/// Stdargs function - must supply -1 as last paremeter, which is NOT
/// added to the set.
/// 
///
Bitset Bitset::fromBits(std::int32_t bit, ...)
{
    Bitset retVal;
    va_list ap;
    va_start(ap, bit);
    while(bit != -1)
    {
		retVal.add(bit);
		bit = va_arg(ap, std::int32_t);
    }
    va_end(ap);
    return std::move(retVal);
}

Bitset Bitset::bor(Bitset const & other) const
{
    Bitset retVal = *this;
    retVal.borInPlace(other);
    return std::move(retVal);
}

void Bitset::add(std::uint32_t bit)
{
    std::uint32_t word = wordNumber(bit);
    growToSize(word + 1);
    bits_[word] |= bitMask(bit);
}

void Bitset::growToSize(std::size_t word)
{
    while (word > bits_.size()) {
        bits_.push_back(0);
    }
}

void Bitset::borInPlace(Bitset const & other)
{
    // First make sure that the target bitset is big enough
    // for the new bits to be ored in.
    //
    growToSize(other.bits_.size());

    // Or the miniimum number of bits after any resizing went on
    //
    for(std::size_t i = 0; i < other.bits_.size(); ++i)
    {
		bits_[i] |= other.bits_[i];
    }
}

std::uint32_t Bitset::size() const
{
    // TODO: Come back to this, it may be faster to & with 0x01
    // then shift right a copy of the 4 bits, than shift left a constant of 1.
    // But then again, the optimizer might just work this out
    // anyway.
    //
    std::uint32_t degree  = 0;
    for (Bitword word : bits_)
    {
        while (word > 0) {
            if (word & 1) {
                ++degree;
            }
            word >>= 1;
        }
    }
    return degree;
}

bool Bitset::equals(Bitset const & other) const
{
    // Work out the minimum comparison set
    //
    std::size_t minimum = std::min(bits_.size(), other.bits_.size());

    // Make sure explict in common bits are equal
    //
    for	(std::size_t i = 0; i < minimum; i++)
    {
		if  (bits_[i] != other.bits_[i])
		{
			return false;
		}
    }

    // Now make sure the bits of the larger set are all turned
    // off.
    //
    if (bits_.size() > minimum)
    {
		for (std::size_t i = minimum; i < bits_.size(); i++)
		{
			if	(bits_[i] != 0)
			{
				return false;
			}
		}
    }
    else if (other.bits_.size() > minimum)
    {
		for (std::size_t i = minimum; i < other.bits_.size(); i++)
		{
			if	(other.bits_[i] != 0)
			{
				return false;
			}
		}
    }

    return true;
}

bool Bitset::isMember(std::uint32_t bit) const
{
    std::uint32_t wordNo  = wordNumber(bit);

    if (wordNo >= bits_.size())
    {
		return false;
    }

    return ((bits_[wordNo] & bitMask(bit)) != 0);
}

void Bitset::remove(std::uint32_t bit)
{
    std::uint32_t wordNo = wordNumber(bit);

    if	(wordNo < bits_.size())
    {
		bits_[wordNo] &= ~(bitMask(bit));
    }
}

bool Bitset::isNilNode() const
{
    for(Bitword word : bits_)
    {
        if(word != 0)
        {
            return false;
        }
    }

    return true;
}

std::uint32_t Bitset::capacity() const
{
    return (std::uint32_t)(bits_.size() << BitsetLogBits);
}

/** Produce an integer list of all the bits that are turned on
 *  in this bitset. Used for error processing in the main as the bitset
 *  reresents a number of integer tokens which we use for follow sets
 *  and so on.
 *
 *  The first entry is the number of elements following in the list.
 */
std::vector<std::uint32_t> Bitset::toIntList() const
{
    std::vector<std::uint32_t> intList;
    intList.reserve(size());

    // Enumerate the bits that are turned on
    //
    std::uint32_t cap = capacity();
    for(std::uint32_t i = 0; i < cap; i++)
    {
		if(isMember(i))
		{
			intList[i + 1]    = i;
		}
    }

    // Result set
    //
    return  intList;
}

String Bitset::toString(std::function<String(std::uint32_t)> tokenNamer) const
{
    String buf = ANTLR3_T("{ ");
    bool havePrintedAnElement = false;
    std::uint32_t cap = capacity();
    for (std::uint32_t i = 0; i < cap; i++)
    {
        if (isMember(i))
        {
            if (havePrintedAnElement)
            {
                buf.append(ANTLR3_T(", "));
            }

            if ( tokenNamer)
            {
                buf += tokenNamer(i);
            }
            else
            {
                buf += antlr3::toString(i);
            }

            havePrintedAnElement = true;
        }
    }
    buf.append(ANTLR3_T(" }"));
    return std::move(buf);
}

} // namespace antlr3