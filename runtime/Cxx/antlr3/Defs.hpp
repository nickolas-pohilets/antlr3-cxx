/** \file
 * Basic type and constant definitions for ANTLR3 Runtime.
 */
#ifndef	_ANTLR3DEFS_H
#define	_ANTLR3DEFS_H

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

/* Common definitions come first
 */
#include <cassert>
#include <new>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdint>

namespace antlr3 {

/// Definitions that indicate the encoding scheme character streams and strings etc
enum class TextEncoding
{
    UTF8,
    UTF16BE,
    UTF16LE,
    UTF32BE,
    UTF32LE
};

typedef std::uint64_t Bitword;
typedef std::size_t	  Index;

/// Indicates end of character stream and is an invalid Unicode code point.
std::uint32_t const CharstreamEof = 0xFFFFFFFF;

Index const NullIndex = Index(std::ptrdiff_t(-1));

/// Indicates memoizing on a rule failed.
Index const MEMO_RULE_FAILED = NullIndex - 1;
Index const MEMO_RULE_UNKNOWN = NullIndex;

#define ANTLR3_DECL_PTR(ClassName) \
    typedef std::shared_ptr<class ClassName> ClassName##Ptr; \
    typedef std::weak_ptr<class ClassName> ClassName##WeakPtr

typedef std::shared_ptr<void> ItemPtr;
typedef std::weak_ptr<void> ItemWeakPtr;

ANTLR3_DECL_PTR(Marker);
ANTLR3_DECL_PTR(CharItem);
ANTLR3_DECL_PTR(CommonToken);
ANTLR3_DECL_PTR(CommonTree);
ANTLR3_DECL_PTR(Location);
ANTLR3_DECL_PTR(LocationSource);
ANTLR3_DECL_PTR(IntStream);
ANTLR3_DECL_PTR(CharStream);
ANTLR3_DECL_PTR(TokenStream);
ANTLR3_DECL_PTR(CommonTokenStream);
ANTLR3_DECL_PTR(TreeNodeStream);
ANTLR3_DECL_PTR(CommonTreeNodeStream);
ANTLR3_DECL_PTR(RecognizerSharedState);
ANTLR3_DECL_PTR(BaseRecognizer);
ANTLR3_DECL_PTR(Lexer);
ANTLR3_DECL_PTR(Parser);
ANTLR3_DECL_PTR(TreeParser);
ANTLR3_DECL_PTR(Exception);
ANTLR3_DECL_PTR(TokenSource);
ANTLR3_DECL_PTR(TreeAdaptor);
ANTLR3_DECL_PTR(CommonTreeAdaptor);
ANTLR3_DECL_PTR(RewriteRuleTokenStream);
ANTLR3_DECL_PTR(RewriteRuleSubtreeStream);
ANTLR3_DECL_PTR(RewriteRuleNodeStream);
ANTLR3_DECL_PTR(DebugEventListener);
ANTLR3_DECL_PTR(Bitset);
ANTLR3_DECL_PTR(CyclicDfa);
    
#undef ANTLR3_DECL_PTR

template<class T, class Y> inline T pointer_cast(std::shared_ptr<Y> p)
{
    return std::static_pointer_cast<typename T::element_type>(std::move(p));
}

template<class T> std::shared_ptr<T> pointer_cast(std::shared_ptr<T> p)
{
    return std::move(p);
}

inline bool isBetween(std::uint32_t min, std::uint32_t val, std::uint32_t max)
{
    return min <= val && val <= max;
}

} // namespace antlr3 {

#endif	/* _ANTLR3DEFS_H	*/
