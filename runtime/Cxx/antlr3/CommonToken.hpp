/** \file
 * \brief Defines the interface for a common token.
 *
 * All token streams should provide their tokens using an instance
 * of this common token. A custom pointer is provided, wher you may attach
 * a further structure to enhance the common token if you feel the need
 * to do so. The C runtime will assume that a token provides implementations
 * of the interface functions, but all of them may be rplaced by your own
 * implementation if you require it.
 */
#ifndef _ANTLR3_COMMON_TOKEN_HPP
#define _ANTLR3_COMMON_TOKEN_HPP

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

namespace antlr3 {

/* Base token types, which all lexer/parser tokens come after in sequence.
 */

/// Indicator of an invalid token
std::uint32_t const	TokenInvalid = 0;
std::uint32_t const	EorTokenType = 1;

/// Imaginary token type to cause a traversal of child nodes in a tree parser
std::uint32_t const	TokenDown = 2;

/// Imaginary token type to signal the end of a stream of child nodes.
std::uint32_t const	TokenUp = 3;

/// First token that can be used by users/generated code
std::uint32_t const MinTokenType = TokenUp + 1;

/** End of file token
 */
std::uint32_t const TokenEof = CharstreamEof;

/** Default channel for a token
 */
std::uint32_t const	TokenDefaultChannel = 0;

/** Reserved channel number for a HIDDEN token - a token that
 *  is hidden from the parser.
 */
std::uint32_t const TokenHiddenChannel = 99;

/** The definition of an ANTLR3 common token structure, which all implementations
 * of a token stream should provide, installing any further structures in the
 * custom pointer element of this structure.
 *
 * \remark
 * Token streams are in essence provided by lexers or other programs that serve
 * as lexers.
 */
class CommonToken
{
    /// The actual type of this token
    std::uint32_t type_;

    /// The virtual channel that this token exists in.
    std::uint32_t channel_;

    /// What the index of this token is, 0, 1, .., n-2, n-1 tokens
    Index index_;

    /// Pointer to the input stream that this token originated in.
    LocationSourcePtr input_;

    /// The character offset in the input stream where the text for this token starts.
    Index start_;

    /// The character offset in the input stream where the text for this token stops.
    Index stop_;

    /// True if tokText_ contains text.
    /// False if text should be taken from input stream
	bool hasText_;
	String tokText_;
public:
    CommonToken();
    CommonToken(std::uint32_t ttype);
    CommonToken(std::uint32_t ttype, String text);
    virtual ~CommonToken() {}

    // Default copy ctor and assignment op are ok.

    /// Returns the text of a token.
    /// Use toString() if you want a printable representation of the token for debug purposes.
    String text() const;

    /// Overrides text associated with a token.
    void setText(String text);

    /// Token type of this token
    std::uint32_t type() const;
    void setType(std::uint32_t ttype);

    /// The channel that this token was placed in (parsers can 'tune' to these channels).
    std::uint32_t channel() const;
    void setChannel(std::uint32_t channel);

    /// Pointer to the input stream that this token originated in.
    LocationSourcePtr inputStream() const;
    void setInputStream(LocationSourcePtr stream);

    /// Zero-based index of the token in the token input stream.
    Index tokenIndex() const;
    void setTokenIndex(Index);

    /// Index of the first character of this token in character stream.
    Index startIndex() const;
    void setStartIndex(Index index);
    
    Location startLocation() const;
    
    /// Index of the first character of next token in character stream.
    Index stopIndex() const;
    void setStopIndex(Index index);
    
    Location stopLocation() const;

    /// Returns this token as a text representation that can be
    /// printed with embedded control codes such as \n replaced with the printable sequence ANTLR3_T("\\n")
    /// This also yields a string structure that can be used more easily than the pointer to
    /// the input stream in certain situations.
    String  toString(ConstString const * tokenNames = nullptr) const;
};

inline ConstString getTokenName(std::uint32_t tokenType, ConstString const * tokenNames)
{
    return tokenType == TokenEof ? ANTLR3_T("EOF") : tokenNames[tokenType];
}

} // namespace antlr3

#endif
