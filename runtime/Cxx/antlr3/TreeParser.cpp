/** \file
 *  Implementation of the tree parser and overrides for the base recognizer
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

#include <antlr3/TreeParser.hpp>

template<class StringTraits>
antlr3<StringTraits>::TreeParser::TreeParser(CommonTreeNodeStreamPtr ctnstream, RecognizerSharedStatePtr state)
    : BaseRecognizer(state)
{
    setTreeNodeStream(ctnstream);
}

template<class StringTraits>
void antlr3<StringTraits>::TreeParser::fillException(Exception* ex)
{
    CommonTreeNodeStreamPtr tns = treeNodeStream();

    ItemPtr node = tns->LT(1);	    /* Current input tree node			    */
    ex->item = node;
    ex->location = adaptor_->getLocation(node);
    ex->index = tns->index();
    ex->input = tns;

    // Are you ready for this? Deep breath now...
    //
    {
        CommonTreePtr tnode = pointer_cast<CommonTreePtr>(node);

        if	(tnode->token() == nullptr)
        {
            ex->streamName = ANTLR3_T("-unknown source-");
        }
        else
        {
            if	(tnode->token()->inputStream() == nullptr)
            {
                ex->streamName = ANTLR3_T("");
            }
            else
            {
                ex->streamName = tnode->token()->inputStream()->sourceName();
            }
        }
    }
}

template<class StringTraits>
String antlr3<StringTraits>::TreeParser::getErrorMessage(Exception const * e, StringLiteral const * tokenNames)
{
    if (!e->item)
    {
        assert(false);
        return String();
    }

    std::unique_ptr<Exception> ex = e->clone();
    
    auto token = adaptor_->getToken(e->item);
    if (!token)
    {
        assert(treeNodeStream() == ex->input);
        ex->item.reset(
            new CommonToken(
                adaptor_->getType(e->item),
                adaptor_->getText(e->item)
            )
        );
    }
    else
    {
        ex->item = token;
    }

    return BaseRecognizer::getErrorMessage(ex.get(), tokenNames);
}

TreeParser::~TreeParser()
{
}

/** Set the input stream and reset the parser
 */
template<class StringTraits>
void antlr3<StringTraits>::TreeParser::setTreeNodeStream(CommonTreeNodeStreamPtr input)
{
    input_ = input;
    adaptor_ = input->treeAdaptor();
    reset();
    input->reset();
}

/** Return a pointer to the input stream
 */
template<class StringTraits>
CommonTreeNodeStreamPtr antlr3<StringTraits>::TreeParser::treeNodeStream()
{
    return std::static_pointer_cast<CommonTreeNodeStream>(input_);
}

// Default implementation is for parser and assumes a token stream as supplied by the runtime.
// You MAY need override this function if the standard BASE_TREE is not what you are using.
//
template<class StringTraits>
ItemPtr antlr3<StringTraits>::TreeParser::getMissingSymbol(
    ExceptionPtr e,
    std::uint32_t expectedTokenType,
    Bitset const & follow
)
{
    CommonTreeNodeStreamPtr tns = treeNodeStream();
    
    // Create a new empty node, by stealing the current one, or the previous one if the current one is EOF
    ItemPtr current	= tns->LT(1);
    if (adaptor_->getType(current) == TokenEof)
    {
        current = tns->LT(-1);
    }

    ItemPtr node =  adaptor_->dupNode(current);

    // Find the newly dupicated token
    CommonTokenPtr token = adaptor_->getToken(node);
    
    token->setType(expectedTokenType);

    // Create the token text that shows it has been inserted
    String text = ANTLR3_T("<missing ");
    text += expectedTokenType == TokenEof ? ANTLR3_T("EOF") : state_->tokenNames[expectedTokenType];
    text += ANTLR3_T(">");
    token->setText(std::move(text));
    
    return node;
}

std::uint32_t TreeParser::itemToInt(ItemPtr item) {
    return adaptor_->getType(item);
}

template<class StringTraits>
String antlr3<StringTraits>::TreeParser::traceCurrentItem() {
    ItemPtr t = treeNodeStream()->LT(1);
    return adaptor_->toString(t, state_->tokenNames);
}

