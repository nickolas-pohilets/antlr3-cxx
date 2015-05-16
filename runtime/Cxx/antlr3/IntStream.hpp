/** \file
 * Defines the the class interface for an antlr3 INTSTREAM.
 * 
 * Certain functionality (such as DFAs for instance) abstract the stream of tokens
 * or characters in to a steam of integers. Hence this structure should be included
 * in any stream that is able to provide the output as a stream of integers (which is anything
 * basically.
 *
 * There are no specific implementations of the methods in this interface in general. Though
 * for purposes of casting and so on, it may be necesssary to implement a function with
 * the signature in this interface which abstracts the base immplementation. In essence though
 * the base stream provides a pointer to this interface, within which it installs its
 * normal match() functions and so on. Interaces such as DFA are then passed the pANTLR3_INT_STREAM
 * and can treat any input as an int stream. 
 *
 * For instance, a lexer implements a pANTLR3_BASE_RECOGNIZER, within which there is a pANTLR3_INT_STREAM.
 * However, a pANTLR3_INPUT_STREAM also provides a pANTLR3_INT_STREAM, which it has constructed from
 * it's normal interface when it was created. This is then pointed at by the pANTLR_BASE_RECOGNIZER
 * when it is intialized with a pANTLR3_INPUT_STREAM.
 *
 * Similarly if a pANTLR3_BASE_RECOGNIZER is initialized with a pANTLR3_TOKEN_STREAM, then the 
 * pANTLR3_INT_STREAM is taken from the pANTLR3_TOKEN_STREAM. 
 *
 * If a pANTLR3_BASE_RECOGNIZER is initialized with a pANTLR3_TREENODE_STREAM, then guess where
 * the pANTLR3_INT_STREAM comes from?
 *
 * Note that because the context pointer points to the actual interface structure that is providing
 * the IntStream it is defined as a (void *) in this interface. There is no direct implementation
 * of an IntStream (unless someone did not understand what I was doing here =;?P
 */
#ifndef _ANTLR3_INTSTREAM_HPP
#define _ANTLR3_INTSTREAM_HPP

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

/// Opaque marker object.
/// @sa IntStream::mark().
class Marker
{
public:
    virtual ~Marker() {}
    
    /// Resets parent stream to the position that was current when marker
    /// was created.
    virtual void rewind() = 0;
};

class IntStream
{
public:
    virtual ~IntStream() {}

	/// Return a string that identifies the input source
	virtual String sourceName() = 0;

    /// Consume the next int in the stream
    virtual void consume() = 0;

    /// Get int at current input pointer + i ahead, where
    /// * i=1 is next std::uint32_t
    /// * i=2 is the one after next...
    /// * i=-1 is the previous one
    /// * i=0 is illegal
    virtual std::uint32_t LA(std::int32_t i) = 0;

    /// Get Item at current input pointer + i ahead.
    /// @sa LA().
    virtual ItemPtr LI(std::int32_t i) = 0;

    /// Tell the stream to start buffering if it hasn't already.
    /// Returns opaque position marker.
    /// Use Marker::rewind() to return to current position later.
    /// Marker can be used for rewinding zero or more times.
    /// Same position can be marked multiple times - implementation
    /// is free to return same or different marker objects.
    /// Associated buffers are cleared when marker object is destructed,
    /// so implementation should not store any strong pointers to marker
    /// object.
    /// Calling mark()->rewind() should not affect the input cursor.
    virtual MarkerPtr mark() = 0;
    
    /// Return the current input symbol index 0..n where n indicates the
    /// last symbol has been read.
    virtual Index index() = 0;

    /** Set the input cursor to the position indicated by index. This is
     *  normally used to seek ahead in the input stream. No buffering is
     *  required to do this unless you know your stream will use seek to
     *  move backwards such as when backtracking.
     *
     *  This is different from rewind in its multi-directional
     *  requirement and in that its argument is strictly an input cursor (index).
     *
     *  For char streams, seeking forward must update the stream state such
     *  as line number. For seeking backwards, you will be presumably
     *  backtracking using the mark/rewind mechanism that restores state and
     *  so this method does not need to update state when seeking backwards.
     *
     *  Currently, this method is only used for efficient backtracking, but
     *  in the future it may be used for incremental parsing.
     */
    virtual void seek(Index index) = 0;
};

} // namespace antlr3

#endif

