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

#define ANTLR3_DECL_PTR(ClassName) \
class ClassName; \
typedef std::shared_ptr<class ClassName> ClassName##Ptr; \
typedef std::weak_ptr<class ClassName> ClassName##WeakPtr

/// Pseudo-namespace that contains definitions that don't depend on string traits.
class antlr3_defs {
public:

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

template<class T> class StringLiteralRef;
class StdUTF16StringTraits;
class StdUTF8StringTraits;

template<class ChildT> class BaseTree;
/// Indicates end of character stream and is an invalid Unicode code point.
static std::uint32_t const CharstreamEof = 0xFFFFFFFF;

static Index const NullIndex = Index(std::ptrdiff_t(-1));

/// Indicates memoizing on a rule failed.
static Index const MEMO_RULE_FAILED = NullIndex - 1;
/// Indicates that rule haven't been parsed yet.
static Index const MEMO_RULE_UNKNOWN = NullIndex;

/// Indicator of an invalid token
static std::uint32_t const	TokenInvalid = 0;
static std::uint32_t const	EorTokenType = 1;

/// Imaginary token type to cause a traversal of child nodes in a tree parser
static std::uint32_t const	TokenDown = 2;

/// Imaginary token type to signal the end of a stream of child nodes.
static std::uint32_t const	TokenUp = 3;

/// First token that can be used by users/generated code
static std::uint32_t const MinTokenType = TokenUp + 1;

/// End of file token
static std::uint32_t const TokenEof = CharstreamEof;

/// Default channel for a token
static std::uint32_t const	TokenDefaultChannel = 0;

/// Reserved channel number for a HIDDEN token - a token that is hidden from the parser.
static std::uint32_t const TokenHiddenChannel = 99;

typedef std::shared_ptr<void> ItemPtr;
typedef std::weak_ptr<void> ItemWeakPtr;

ANTLR3_DECL_PTR(Marker);
ANTLR3_DECL_PTR(Location);
ANTLR3_DECL_PTR(Bitset);

template<class T, class Y> static inline T pointer_cast(std::shared_ptr<Y> p)
{
    return std::static_pointer_cast<typename T::element_type>(std::move(p));
}

template<class T> static std::shared_ptr<T> pointer_cast(std::shared_ptr<T> p)
{
    return std::move(p);
}

static inline bool isBetween(std::uint32_t min, std::uint32_t val, std::uint32_t max)
{
    return min <= val && val <= max;
}

};

/// Pseudo-namespace that contains all ANTLR defintinitions.
template<class StringTraits>
class antlr3 : public antlr3_defs {
public:
typedef typename StringTraits::String String;
typedef typename StringTraits::Char Char;
typedef typename StringTraits::StringLiteral StringLiteral;

class StringUtils;

template<class CodeUnit> class BasicCharStream;
template<class ChildT> class BaseTreeAdaptor;

ANTLR3_DECL_PTR(CharItem);
ANTLR3_DECL_PTR(CommonToken);
ANTLR3_DECL_PTR(CommonTree);
ANTLR3_DECL_PTR(CommonErrorNode);
ANTLR3_DECL_PTR(LocationSource);
ANTLR3_DECL_PTR(IntStream);
ANTLR3_DECL_PTR(CharStream);
ANTLR3_DECL_PTR(ByteCharStream);
ANTLR3_DECL_PTR(UnicodeCharStream);
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
ANTLR3_DECL_PTR(MismatchedTokenException);
ANTLR3_DECL_PTR(UnwantedTokenException);
ANTLR3_DECL_PTR(MissingTokenException);
ANTLR3_DECL_PTR(NoViableAltException);
ANTLR3_DECL_PTR(MismatchedSetException);
ANTLR3_DECL_PTR(MismatchedRangeException);
ANTLR3_DECL_PTR(EarlyExitException);
ANTLR3_DECL_PTR(FailedPredicateException);
ANTLR3_DECL_PTR(RewriteCardinalityException);
ANTLR3_DECL_PTR(RewriteEarlyExitException);
ANTLR3_DECL_PTR(TokenSource);
ANTLR3_DECL_PTR(TreeAdaptor);
ANTLR3_DECL_PTR(CommonTreeAdaptor);
ANTLR3_DECL_PTR(RewriteRuleTokenStream);
ANTLR3_DECL_PTR(RewriteRuleSubtreeStream);
ANTLR3_DECL_PTR(RewriteRuleNodeStream);
ANTLR3_DECL_PTR(DebugEventListener);
ANTLR3_DECL_PTR(Bitset);
ANTLR3_DECL_PTR(CyclicDfa);

static StringLiteral getTokenName(std::uint32_t tokenType, StringLiteral const * tokenNames);

/// Produce a DOT specification for graphviz freeware suite from a base tree
static String makeDot(TreeAdaptorPtr adaptor, ItemPtr theTree);

};

#undef ANTLR3_DECL_PTR

#endif	/* _ANTLR3DEFS_H	*/
