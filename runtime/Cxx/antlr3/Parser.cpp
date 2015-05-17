/** \file
 * Implementation of the base functionality for an ANTLR3 parser.
 */

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

#include <antlr3/Parser.hpp>

template<class StringTraits>
antlr3<StringTraits>::Parser::Parser(RecognizerSharedStatePtr state)
    : BaseRecognizer(state)
{
}

template<class StringTraits>
antlr3<StringTraits>::Parser::Parser(TokenStreamPtr tstream, RecognizerSharedStatePtr state)
    : Parser(state)
{
    setTokenStream(tstream);
}

template<class StringTraits>
antlr3<StringTraits>::Parser::Parser(TokenStreamPtr tstream, DebugEventListenerPtr dbg, RecognizerSharedStatePtr state)
    : Parser(tstream, state)
{
    setDebugListener(std::move(dbg));
}

Parser::~Parser()
{
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::Parser::getMissingSymbol(
     ExceptionPtr e,
     std::uint32_t expectedTokenType,
     Bitset const & follow
)
{
    // Dereference the standard pointers
    //
    TokenStreamPtr ts = tokenStream();

    // Work out what to use as the current symbol to make a line and offset etc
    // If we are at EOF, we use the token before EOF
    //
    CommonTokenPtr current = ts->LT(1);
    if	(current->type() == TokenEof)
    {
        current = ts->LT(-1);
    }

    // Create a new empty token
    //
    CommonTokenPtr token = std::make_shared<CommonToken>();
    token->setInputStream(current->inputStream());

    // Set some of the token properties based on the current token
    //
    token->setStartIndex(current->startIndex());
    token->setStopIndex(current->startIndex());
    token->setChannel(TokenDefaultChannel);
    token->setType(expectedTokenType);

    // Create the token text that shows it has been inserted
    //
    String text = ANTLR3_T("<missing ");
    text += expectedTokenType == TokenEof ? "EOF" : state_->tokenNames[expectedTokenType];
    text += ANTLR3_T(">");
    token->setText(text);

    // Finally return the pointer to our new token
    //
    return token;
}

template<class StringTraits>
void antlr3<StringTraits>::Parser::fillException(Exception* ex)
{
    TokenStreamPtr cts = tokenStream();
    CommonTokenPtr token = cts->LT(1);

    ex->input = input_;
    ex->item  = token;
    ex->location = token->startLocation();
    ex->index = cts->index();

    if (token->inputStream() == NULL)
    {
        ex->streamName = ANTLR3_T("");
    }
    else
    {
        ex->streamName = token->inputStream()->sourceName();
    }
}

std::uint32_t Parser::itemToInt(ItemPtr item) {
    return std::static_pointer_cast<CommonToken>(item)->type();
}

template<class StringTraits>
void antlr3<StringTraits>::Parser::setDebugListener(DebugEventListenerPtr dbg)
{
    // Set the debug listener. There are no methods to override
    // because currently the only ones that notify the debugger
    // are error reporting and recovery. Hence we can afford to
    // check and see if the debugger interface is null or not
    // there. If there is ever an occasion for a performance
    // sensitive function to use the debugger interface, then
    // a replacement function for debug mode should be supplied
    // and installed here.
    //
    debugger_ = std::move(dbg);

    // If there was a tokenstream installed already
    // then we need to tell it about the debug interface
    //
    if	(input_ != NULL)
    {
        setTokenStream(std::make_shared<DebugTokenStream>(tokenStream(), debugger_));
    }
}

template<class StringTraits>
TokenStreamPtr antlr3<StringTraits>::Parser::tokenStream()
{
    return std::static_pointer_cast<TokenStream>(input_);
}

template<class StringTraits>
void antlr3<StringTraits>::Parser::setTokenStream(TokenStreamPtr tstream)
{
    input_ = tstream;
    reset();
}
    
template<class StringTraits>
String antlr3<StringTraits>::Parser::traceCurrentItem() {
    CommonTokenPtr t = tokenStream()->LT(1);
    return t->toString(state_->tokenNames);
}

