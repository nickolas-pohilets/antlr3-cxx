/** \file
 * Defines the interface for an ANTLR3 common token stream. Custom token streams should create
 * one of these and then override any functions by installing their own pointers
 * to implement the various functions.
 */
#ifndef _ANTLR3_TOKENSTREAM_HPP
#define _ANTLR3_TOKENSTREAM_HPP

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
#include <antlr3/CommonToken.hpp>
#include <antlr3/Bitset.hpp>
#include <antlr3/DebugEventListener.hpp>
#include <unordered_map>
#include <unordered_set>

namespace antlr3 {

/** Definition of a token source, which has a pointer to a function that 
 *  returns the next token (using a token factory if it is going to be
 *  efficient) and a pointer to an InputStream. This is slightly
 *  different to the Java interface because we have no way to implement
 *  multiple interfaces without defining them in the interface structure
 *  or casting (void *), which is too convoluted.
 */
class TokenSource
{
public:
    virtual ~TokenSource() {}

    /** Pointer to a function that returns the next token in the stream. 
     */
    virtual CommonTokenPtr nextToken() = 0;

    /** When the token source is constructed, it is populated with the file
     *  name from whence the tokens were produced by the lexer. This pointer is a
     *  copy of the one supplied by the CharStream (and may be NULL) so should
     *  not be manipulated other than to copy or print it.
     */
    virtual LocationSourcePtr source() = 0;
};

class TokenStream : public IntStream
{
public:
    /** Get Token at current input pointer + i ahead where i=1 is next Token.
     *  i<0 indicates tokens in the past.  So -1 is previous token and -2 is
     *  two tokens ago. LT(0) is undefined.  For i>=n, return Token.EOFToken.
     *  Return null for LT(0) and any index that results in an absolute address
     *  that is negative.
     */
    virtual CommonTokenPtr LT(std::int32_t k) = 0;

    /** Get a token at an absolute index i; 0..n-1.  This is really only
     *  needed for profiling and debugging and token stream rewriting.
     *  If you don't want to buffer up tokens, then this method makes no
     *  sense for you.  Naturally you can't use the rewrite stream feature.
     *  I believe DebugTokenStream can easily be altered to not use
     *  this method, removing the dependency.
     */
    virtual CommonTokenPtr get(Index i) = 0;

    /** Where is this stream pulling tokens from?  This is not the name, but
     *  a pointer into an interface that contains a TokenSource interface.
     *  The Token Source interface contains a pointer to the input stream and a pointer
     *  to a function that returns the next token.
     */
    virtual TokenSourcePtr tokenSource() = 0;

    /** Return the text of all the tokens in the stream, as the old tramp in 
     *  Leeds market used to say; ANTLR3_T("Get the lot!")
     */
    virtual String toString() = 0;

    /** Return the text of all tokens from start to stop, inclusive.
     *  If the stream does not buffer all the tokens then it can just
     *  return an empty String or NULL;  Grammars should not access $ruleLabel.text in
     *  an action in that case.
     */
    virtual String toString(std::uint32_t start, std::uint32_t stop) = 0;

    /** Because the user is not required to use a token with an index stored
     *  in it, we must provide a means for two token objects themselves to
     *  indicate the start/end location.  Most often this will just delegate
     *  to the other toString(int,int).  This is also parallel with
     *  the pTREENODE_STREAM->toString(Object,Object).
     */
    virtual String toString(CommonTokenPtr start, CommonTokenPtr stop) = 0;
};

/** Common token stream is an implementation of ANTLR_TOKEN_STREAM for the default
 *  parsers and recognizers. You may of course build your own implementation if
 *  you are so inclined.
 */
class CommonTokenStream : public TokenStream, public std::enable_shared_from_this<CommonTokenStream>
{
    class TokenStreamMarker : public Marker
    {
        Index p_;
        CommonTokenStreamPtr stream_;
    public:
        TokenStreamMarker(Index p, CommonTokenStreamPtr stream)
            : Marker()
            , p_(p)
            , stream_(stream)
        {}
        
        virtual void rewind() {
            stream_->p_ = p_;
        }
    };
    
    /** Pointer to the token source for this stream
     */
    TokenSourcePtr tokenSource_;

    /** Records every single token pulled from the source indexed by the token index.
     *  There might be more efficient ways to do this, such as referencing directly in to
     *  the token factory pools, but for now this is convenient and the List is not
     *  a huge overhead as it only stores pointers anyway, but allows for iterations and 
     *  so on.
     */
    std::vector<CommonTokenPtr> tokens_;

    /** Override map of tokens. If a token type has an entry in here, then
     *  the pointer in the table points to an int, being the override channel number
     *  that should always be used for this token type.
     */
    std::unordered_map<std::uint32_t, std::uint32_t> channelOverrides_;

    /** Discared set. If a token has an entry in this table, then it is thrown
     *  away (data pointer is always NULL).
     */
    std::unordered_set<std::uint32_t> discardSet_;

    /* The channel number that this token stream is tuned to. For instance, whitespace
     * is usually tuned to channel 99, which no token stream would normally tune to and
     * so it is thrown away.
     */
    std::uint32_t channel_;

    /** If this flag is set to True, then tokens that the stream sees that are not
     *  in the channel that this stream is tuned to, are not tracked in the
     *  tokens table. When set to false, ALL tokens are added to the tracking.
     */
    bool discardOffChannel_;

    /** The index into the tokens list of the current token (the next one that will be
     *  consumed. p = -1 indicates that the token list is empty.
     */
    Index p_;

    bool shouldDiscard(CommonTokenPtr token) const;
    void fillBufferIfNeeded();
    CommonTokenPtr LB(std::uint32_t i);
    Index skipOffTokenChannels(Index i);
    Index skipOffTokenChannelsReverse(Index i);
    CommonTokenPtr eofToken();
public:
    CommonTokenStream(TokenSourcePtr source);
    ~CommonTokenStream();

    /// IntStream

    virtual String sourceName() override;
    virtual void consume() override;
    virtual std::uint32_t LA(std::int32_t i) override;
    virtual ItemPtr LI(std::int32_t i) override { return LT(i); }
    virtual MarkerPtr mark() override;
    virtual Index index() override;
    virtual void seek(Index index) override;

    /// TokenStream

    virtual CommonTokenPtr LT(std::int32_t k) override;
    virtual CommonTokenPtr get(Index i) override;
    virtual TokenSourcePtr tokenSource() override;
    virtual String toString() override;
    virtual String toString(std::uint32_t start, std::uint32_t stop) override;
    virtual String toString(CommonTokenPtr start, CommonTokenPtr stop) override;

    void setTokenTypeChannel(std::uint32_t ttype, std::uint32_t channel);
    void discardTokenType(std::uint32_t ttype);
    void discardOffChannelToks(bool discard);

    std::vector<CommonTokenPtr> tokens();

    /** Function that returns all the tokens between a start and a stop index.
     */
    std::vector<CommonTokenPtr> getTokenRange(std::uint32_t start, std::uint32_t stop);

    /** Function that returns all the tokens indicated by the specified bitset, within a range of tokens
     */
    std::vector<CommonTokenPtr> getTokensSet(std::uint32_t start, std::uint32_t stop, Bitset const & types);
    
    /** Function that returns all the tokens indicated by being a member of the supplied List
     */
    std::vector<CommonTokenPtr> getTokensList(std::uint32_t start, std::uint32_t stop, std::vector<std::uint32_t> const & list);

    /** Function that returns all tokens of a certain type within a range.
     */
    std::vector<CommonTokenPtr> getTokensType(std::uint32_t start, std::uint32_t stop, std::uint32_t type);

    /** Function that resets the token stream so that it can be reused, but
     *  but that does not free up any resources, such as the token factory
     *  the factory pool and so on. This prevents the need to keep freeing
     *  and reallocating the token pools if the thing you are building is
     *  a multi-shot dameon or somethign like that. It is much faster to
     *  just reuse all the vectors.
     */
    void reset();
};

class DebugTokenStream : public TokenStream
{
    class DebugTokenStreamMarker : public Marker
    {
        MarkerPtr innerMarker_;
        int index_;
        DebugEventListenerPtr debugger_;
    public:
        DebugTokenStreamMarker(MarkerPtr innerMarker, int index, DebugEventListenerPtr debugger)
            : Marker()
            , innerMarker_(std::move(innerMarker))
            , index_(index)
            , debugger_(debugger)
        {}
        
        virtual void rewind() {
            debugger_->rewind(index_);
            innerMarker_->rewind();
        }
    };
    
    /// Wrapped token stream.
    TokenStreamPtr input_;

    /// Debugger interface, is this is a debugging token stream
    ///
    DebugEventListenerPtr debugger_;

    /// Indicates the initial stream state for dbgConsume()
    ///
    bool initialStreamState_;

    void consumeInitialHiddenTokens();
public:
    DebugTokenStream(TokenStreamPtr input, DebugEventListenerPtr dbg);
    ~DebugTokenStream();

    /// IntStream

    virtual String sourceName() override;
    virtual void consume() override;
    virtual std::uint32_t LA(std::int32_t i) override;
    virtual ItemPtr LI(std::int32_t i) override { return LT(i); }
    
    virtual MarkerPtr mark() override;
    virtual Index index() override;
    virtual void seek(Index index) override;

    /// TokenStream

    virtual CommonTokenPtr LT(std::int32_t k) override;
    virtual CommonTokenPtr get(Index i) override;
    virtual TokenSourcePtr tokenSource() override;
    virtual String toString() override;
    virtual String toString(std::uint32_t start, std::uint32_t stop) override;
    virtual String toString(CommonTokenPtr start, CommonTokenPtr stop) override;
};

} // namespace antlr3

#endif
