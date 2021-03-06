/** \file
 * Base implementation of an ANTLR3 parser.
 *
 *
 */
#ifndef _ANTLR3_PARSER_HPP
#define _ANTLR3_PARSER_HPP

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
#include <antlr3/BaseRecognizer.hpp>

namespace antlr3 {

/** This is the main interface for an ANTLR3 parser.
 */
class Parser : public BaseRecognizer
{
    friend class BaseRecognizer;
public:
    Parser(RecognizerSharedStatePtr state);
    Parser(TokenStreamPtr tstream, RecognizerSharedStatePtr state);
    Parser(TokenStreamPtr tstream, DebugEventListenerPtr dbg, RecognizerSharedStatePtr state);
    ~Parser();

    virtual ItemPtr getMissingSymbol(
        ExceptionPtr e,
        std::uint32_t expectedTokenType,
        Bitset const & follow
    ) override;

	/** A pointer to a function that installs a debugger object (it also
	 *  installs the debugging versions of the parser methods. This means that
	 *  a non debug parser incurs no overhead because of the debugging stuff.
	 */
	void setDebugListener(DebugEventListenerPtr dbg);

    /// Returns token stream used by the parser.
    TokenStreamPtr tokenStream();

    /// Sets token stream used by the parser.
    void setTokenStream(TokenStreamPtr);
protected:
    CommonTokenPtr LT(std::int32_t index) {
        return tokenStream()->LT(index);
    }
    
    virtual void fillException(Exception* ex) override;
    virtual std::uint32_t itemToInt(ItemPtr item) override;
    virtual String traceCurrentItem() override;
};

} // namespace

#endif
