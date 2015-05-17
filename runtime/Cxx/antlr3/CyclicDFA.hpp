/// Definition of a cyclic dfa structure such that it can be
/// initialized at compile time and have only a single
/// runtime function that can deal with all cyclic dfa
/// structures and show Java how it is done ;-)
///
#ifndef	Cyclicdfa_H
#define	Cyclicdfa_H

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

#include <antlr3/BaseRecognizer.hpp>
#include <antlr3/IntStream.hpp>

// If this header file is included as part of a generated recognizer that
// is being compiled as if it were C++, and this is Windows, then the const elements
// of the structure cause the C++ compiler to (rightly) point out that
// there can be no instantiation of the structure because it needs a constructor
// that can initialize the data, however these structures are not
// useful for C++ as they are pre-generated and static in the recognizer.
// So, we turn off those warnings, which are only at /W4 anyway.
//
#ifdef Windows
#pragma warning	(push)
#pragma warning (disable : 4510)
#pragma warning (disable : 4512)
#pragma warning (disable : 4610)
#endif

class CyclicDfa
{
    // Instance variables are intentionally public to allow struct-style initialization.
public:
    typedef std::int32_t (*SPECIAL_FUNC)(
        void * ctx,
        BaseRecognizer * rec,
        IntStream * is,
        std::int32_t s,
        MarkerPtr marker
    );

    /// Decision number that a particular static structure
    ///  represents.
    ///
    std::int32_t const decisionNumber;

    /// What this decision represents
    ///
    StringLiteral const description;

    SPECIAL_FUNC const specialStateTransitionFunc;

    std::int32_t const * const eot;
    std::int32_t const * const eof;
    std::int32_t const * const min;
    std::int32_t const * const max;
    std::int32_t const * const accept;
    std::int32_t const * const special;
    std::int32_t const * const * const transition;
public:
    std::int32_t predict(void * ctx, BaseRecognizer * recognizer, IntStream * is) const;
private:
    std::int32_t specialStateTransition(void * ctx, BaseRecognizer * recognizer, IntStream * is, std::int32_t s, MarkerPtr marker) const;
    void noViableAlt(BaseRecognizer * rec, std::uint32_t s) const;
};

#ifdef Windows
#pragma warning	(pop)
#endif

#endif
