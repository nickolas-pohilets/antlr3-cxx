/// \file 
/// Default implementation of CommonTokenStream
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

#include <antlr3/TokenStream.hpp>

template<class StringTraits>
antlr3<StringTraits>::CommonTokenStream::CommonTokenStream(TokenSourcePtr source)
    : TokenStream()
    , tokenSource_(source)
    , tokens_()
    , channelOverrides_()
    , discardSet_()
    , channel_(TokenDefaultChannel)
    , discardOffChannel_(false)
    , p_(NullIndex)
{
}

CommonTokenStream::~CommonTokenStream()
{
}

template<class StringTraits>
String antlr3<StringTraits>::CommonTokenStream::sourceName()
{
    // Slightly convoluted as we must trace back to the lexer's input source
    // via the token source. The streamName that is here is not initialized
    // because this is a token stream, not a file or string stream, which are the
    // only things that have a context for a source name.
    //
    return tokenSource_->source()->sourceName();
}

/** Move the input pointer to the next incoming token.  The stream
 *  must become active with LT(1) available.  consume() simply
 *  moves the input pointer so that LT(1) points at the next
 *  input symbol. Consume at least one token.
 *
 *  Walk past any token not on the channel the parser is listening to.
 */
template<class StringTraits>
void antlr3<StringTraits>::CommonTokenStream::consume()
{
    if(p_ < tokens_.size())
    {
        p_++;
        p_ = skipOffTokenChannels(p_);
    }
}

std::uint32_t CommonTokenStream::LA(std::int32_t i)
{
    CommonTokenPtr tok = LT(i);

    if(tok != NULL)
    {
        return tok->type();
    }
    else
    {
        return TokenInvalid;
    }
}

template<class StringTraits>
MarkerPtr antlr3<StringTraits>::CommonTokenStream::mark()
{
    return std::make_shared<TokenStreamMarker>(index(), shared_from_this());
}

template<class StringTraits>
Index antlr3<StringTraits>::CommonTokenStream::index()
{
    fillBufferIfNeeded();
    return p_;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTokenStream::seek(Index index)
{
    p_ = index;
}


template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::CommonTokenStream::LT(std::int32_t k)
{
    if(k <= 0)
    {
        return LB(-k);
    }

    fillBufferIfNeeded();

    // Here we used to check for k == 0 and return 0, but this seems
    // a superfluous check to me. LT(k=0) is therefore just undefined
    // and we won't waste the clock cycles on the check
    //

    if((p_ + k - 1) >= tokens_.size())
    {
        return eofToken();
    }

    Index i = p_;
    std::int32_t n = 1;

    /* Need to find k good tokens, skipping ones that are off channel
    */
    while(n < k)
    {
        /* Skip off-channel tokens */
        i = skipOffTokenChannels(i+1); /* leave p on valid token    */
        n++;
    }
    if(i >= (std::int32_t)tokens_.size())
    {
        return eofToken();
    }

    // Here the token must be in the input vector. Rather then incur
    // function call penalty, we just return the pointer directly
    // from the vector
    //
    return tokens_.at(i);
}

template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::CommonTokenStream::get(Index i)
{
    return tokens_.at(i);
}

template<class StringTraits>
TokenSourcePtr antlr3<StringTraits>::CommonTokenStream::tokenSource()
{
    return tokenSource_;
}

template<class StringTraits>
String antlr3<StringTraits>::CommonTokenStream::toString()
{
    fillBufferIfNeeded();
    return  toString(0, (std::uint32_t)tokens_.size());
}

template<class StringTraits>
String antlr3<StringTraits>::CommonTokenStream::toString(std::uint32_t start, std::uint32_t stop)
{
    fillBufferIfNeeded();

    assert(!tokens_.empty() && tokens_.back()->type() == TokenEof);
    const std::uint32_t maxIndex = (std::uint32_t)tokens_.size() - 1;
    start = std::min(start, maxIndex);
    stop = std::min(stop, maxIndex);

    String string;

    for(std::uint32_t i = start; i < stop; i++)
    {
        CommonTokenPtr tok = get(i);
        if(tok != NULL)
        {
            string += tok->text();
        }
    }

    return string;
}

template<class StringTraits>
String antlr3<StringTraits>::CommonTokenStream::toString(CommonTokenPtr start, CommonTokenPtr stop)
{
    if(start != NULL && stop != NULL)
    {
        return toString((std::uint32_t)start->tokenIndex(), (std::uint32_t)stop->tokenIndex());
    }
    else
    {
        return ANTLR3_T("");
    }
}

/** A simple filter mechanism whereby you can tell this token stream
 *  to force all tokens of type ttype to be on channel.  For example,
 *  when interpreting, we cannot execute actions so we need to tell
 *  the stream to force all WS and NEWLINE to be a different, ignored,
 *  channel.
 */
template<class StringTraits>
void antlr3<StringTraits>::CommonTokenStream::setTokenTypeChannel(std::uint32_t ttype, std::uint32_t channel)
{
    /* We add one to the channel so we can distinguish NULL as being no entry in the
     * table for a particular token type.
     */
    channelOverrides_[ttype] = channel + 1;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTokenStream::discardTokenType(std::uint32_t ttype)
{
    /* We add one to the channel so we can distinguish NULL as being no entry in the
     * table for a particular token type. We could use bitsets for this I suppose too.
     */
    discardSet_.insert(ttype);
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTokenStream::discardOffChannelToks(bool discard)
{
    discardOffChannel_ = discard;
}

std::vector<CommonTokenPtr> CommonTokenStream::tokens()
{
    fillBufferIfNeeded();
    return tokens_;
}

std::vector<CommonTokenPtr> CommonTokenStream::getTokenRange(std::uint32_t start, std::uint32_t stop)
{
    return getTokensSet(start, stop, Bitset());
}

/** Given a start and stop index, return a List of all tokens in
 *  the token type BitSet.  Return null if no tokens were found.  This
 *  method looks at both on and off channel tokens.
 */
std::vector<CommonTokenPtr> CommonTokenStream::getTokensSet(std::uint32_t start, std::uint32_t stop, Bitset const & types)
{
    fillBufferIfNeeded();

    stop = std::min(stop, (std::uint32_t)tokens_.size() - 1);

    /* We have the range set, now we need to iterate through the
     * installed tokens and create a new list with just the ones we want
     * in it. We are just moving pointers about really.
     */
    std::vector<CommonTokenPtr> filteredList;

    for(std::uint32_t i = start; i<= stop; i++)
    {
        CommonTokenPtr tok = get(i);

        if(types.isMember(tok->type()))
        {
            filteredList.push_back(tok);
        }
    }
    
    return  filteredList;
}

std::vector<CommonTokenPtr> CommonTokenStream::getTokensList(std::uint32_t start, std::uint32_t stop, std::vector<std::uint32_t> const & list)
{
    return getTokensSet(start, stop, Bitset::fromBits(list));
}

std::vector<CommonTokenPtr> CommonTokenStream::getTokensType(std::uint32_t start, std::uint32_t stop, std::uint32_t type)
{
    return getTokensSet(start, stop, Bitset::fromBits(type, -1));
}

// Reset a token stream so it can be used again and can reuse it's
// resources.
//
template<class StringTraits>
void antlr3<StringTraits>::CommonTokenStream::reset()
{
    // Free any resources that ar most like specifc to the
    // run we just did.
    //
    discardSet_.clear();
    channelOverrides_.clear();

    // Now, if there were any existing tokens in the stream,
    // then we just reset the vector count so that it starts
    // again. We must traverse the entries unfortunately as
    // there may be free pointers for custom token types and
    // so on. However that is just a quick NULL check on the
    // vector entries.
    //
    tokens_.clear();

    // Reset to defaults
    //
    discardOffChannel_  = false;
    channel_            = TokenDefaultChannel;
    p_	            = -1;
}
    
template<class StringTraits>
bool antlr3<StringTraits>::CommonTokenStream::shouldDiscard(CommonTokenPtr token) const
{
    if(discardSet_.count(token->type()) > 0)
    {
        return true;
    }
    
    if(discardOffChannel_ && token->channel() != channel_)
    {
        return true;
    }
    
    return false;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTokenStream::fillBufferIfNeeded()
{
    if (p_ != NullIndex) {
        return;
    }
    
    // Start at index 0 of course
    std::uint32_t index = 0;

    while(true)
    {
        CommonTokenPtr tok = tokenSource_->nextToken();
        if (!shouldDiscard(tok)) {
            // See if this type is in the override map
            auto channelI = channelOverrides_.find(tok->type() + 1);
            if(channelI != channelOverrides_.end())
            {
                // Override found
                tok->setChannel(channelI->second - 1);
            }

            // If not discarding it, add it to the list at the current index
            tok->setTokenIndex(index);
            p_++;
            tokens_.push_back(tok);
            index++;
        }

        if (tok->type() == TokenEof) {
            break;
        }
    }

    // Set the consume pointer to the first token that is on our channel
    p_ = 0;
    p_ = skipOffTokenChannels(p_);
}

template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::CommonTokenStream::LB(std::uint32_t k)
{
    fillBufferIfNeeded();
    
    if(k == 0)
    {
        return NULL;
    }
    
    if(p_ < k)
    {
        return NULL;
    }

    Index i = p_;
    std::int32_t n = 1;

    /* Need to find k good tokens, going backwards, skipping ones that are off channel
     */
    while(n <= (std::int32_t) k)
    {
        /* Skip off-channel tokens
         */

        i = skipOffTokenChannelsReverse(i - 1); /* leave p on valid token    */
        n++;
    }
    if(i == NullIndex)
    {
        return NULL;
    }
    // Here the token must be in the input vector. Rather then incut
    // function call penalty, we jsut return the pointer directly
    // from the vector
    //
    return tokens_.at(i);
}

/// Given a starting index, return the index of the first on-channel
///  token.
///
template<class StringTraits>
Index antlr3<StringTraits>::CommonTokenStream::skipOffTokenChannels(Index i)
{
    Index n = tokens_.size();

    while(i < n)
    {
        CommonTokenPtr tok = tokens_.at(i);

        if(tok->channel() != channel_)
        {
            i++;
        }
        else
        {
            return i;
        }
    }
    return i;
}

template<class StringTraits>
Index antlr3<StringTraits>::CommonTokenStream::skipOffTokenChannelsReverse(Index x)
{
    while(x != NullIndex)
    {
        CommonTokenPtr tok = tokens_.at(x);
        
        if(tok->channel() != channel_)
        {
            x--;
        }
        else
        {
            return x;
        }
    }
    return x;
}
    
template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::CommonTokenStream::eofToken() {
    fillBufferIfNeeded();
    assert(!tokens_.empty() && tokens_.back()->type() == TokenEof);
    return tokens_.back();
}

#pragma mark -

template<class StringTraits>
antlr3<StringTraits>::DebugTokenStream::DebugTokenStream(TokenStreamPtr input, DebugEventListenerPtr dbg)
    : TokenStream()
    , input_(input)
    , debugger_(std::move(dbg))
    , initialStreamState_(true)
{
    assert(debugger_);
}

DebugTokenStream::~DebugTokenStream()
{

}

template<class StringTraits>
String antlr3<StringTraits>::DebugTokenStream::sourceName()
{
    return input_->sourceName();
}

template<class StringTraits>
void antlr3<StringTraits>::DebugTokenStream::consume()
{
    consumeInitialHiddenTokens();
    
    Index a = input_->index();		// Where are we right now?
    CommonTokenPtr t = input_->LT(1);		// Current token from stream
    input_->consume();
    Index b = input_->index();

    debugger_->consumeToken(t);	// Tell the debugger that we consumed the first token

    // If the standard consume caused the index to advance by more than 1,
    // which can only happen if it skipped some off-channel tokens.
    // We need to tell the debugger about those tokens.
    //
    for(Index i = a+1; i<b; i++)
    {
        debugger_->consumeHiddenToken(input_->get(i));
    }
}

std::uint32_t DebugTokenStream::LA(std::int32_t i)
{
    consumeInitialHiddenTokens();
    debugger_->LT(i, input_->LT(i));
    return input_->LA(i);
}

template<class StringTraits>
MarkerPtr antlr3<StringTraits>::DebugTokenStream::mark()
{
    Index index = input_->index();
    debugger_->mark((int)index);
    return std::make_shared<DebugTokenStreamMarker>(input_->mark(), (int)index, debugger_);
}

template<class StringTraits>
Index antlr3<StringTraits>::DebugTokenStream::index()
{
    return input_->index();
}

template<class StringTraits>
void antlr3<StringTraits>::DebugTokenStream::seek(Index index)
{
    // TODO: Implement seek in debugger when Ter adds it to Java
    //
    input_->seek(index);
}

template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::DebugTokenStream::LT(std::int32_t k)
{
    consumeInitialHiddenTokens();
    return input_->LT(k);
}

template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::DebugTokenStream::get(Index i)
{
    return input_->get(i);
}

template<class StringTraits>
TokenSourcePtr antlr3<StringTraits>::DebugTokenStream::tokenSource()
{
    return input_->tokenSource();
}

template<class StringTraits>
String antlr3<StringTraits>::DebugTokenStream::toString()
{
    return input_->toString();
}

template<class StringTraits>
String antlr3<StringTraits>::DebugTokenStream::toString(std::uint32_t start, std::uint32_t stop)
{
    return input_->toString(start, stop);
}

template<class StringTraits>
String antlr3<StringTraits>::DebugTokenStream::toString(CommonTokenPtr start, CommonTokenPtr stop)
{
    return input_->toString(start, stop);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugTokenStream::consumeInitialHiddenTokens()
{
    if(initialStreamState_)
    {
        Index first = input_->index();
        for (Index i = 0; i < first; i++)
        {
            debugger_->consumeHiddenToken(input_->get(i));
        }

        initialStreamState_ = false;
    }
}

