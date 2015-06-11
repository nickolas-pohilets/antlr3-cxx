/** \file
 *
 * Base implementation of an antlr 3 lexer.
 *
 * An ANTLR3 lexer implements a base recongizer, a token source and
 * a lexer interface. It constructs a base recognizer with default
 * functions, then overrides any of these that are parser specific (usual
 * default implementation of base recognizer.
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

#include <antlr3/Lexer.hpp>

template<class StringTraits>
antlr3<StringTraits>::Lexer::Lexer(RecognizerSharedStatePtr state)
    : BaseRecognizer(state)
{
}

template<class StringTraits>
antlr3<StringTraits>::Lexer::Lexer(CharStreamPtr input, RecognizerSharedStatePtr state)
: Lexer(state)
{
    setCharStream(std::move(input));
}

template<class StringTraits>
antlr3<StringTraits>::Lexer::~Lexer()
{
}


template<class StringTraits>
void antlr3<StringTraits>::Lexer::reset()
{
    this->state_->token			    = NULL;
    this->state_->type			    = TokenInvalid;
    this->state_->channel			    = TokenDefaultChannel;
    this->state_->tokenStartCharIndex	= -1;
    this->state_->text	            = ANTLR3_T("");
}

///
/// \brief
/// Returns the next available token from the current input stream.
/// 
/// \param toksource
/// Points to the implementation of a token source. The lexer is 
/// addressed by the super structure pointer.
/// 
/// \returns
/// The next token in the current input stream or the EOF token
/// if there are no more tokens.
/// 
/// \remarks
/// Write remarks for nextToken here.
/// 
/// \see nextToken
///
template<class StringTraits>
typename antlr3<StringTraits>::CommonTokenPtr
    antlr3<StringTraits>::Lexer::nextTokenStr() {
    return this->filteringMode_ ? nextTokenFiltering() : nextTokenNormal();
}

template<class StringTraits>
typename antlr3<StringTraits>::CommonTokenPtr
    antlr3<StringTraits>::Lexer::nextTokenNormal()
{
    /// Loop until we get a non skipped token or EOF
    ///
    for	(;;)
    {
        // Get rid of any previous token (token factory takes care of
        // any de-allocation when this token is finally used up.
        //
        this->state_->token		    = NULL;
        this->state_->error		    = false;	    // Start out without an exception
        this->state_->failed		    = false;

        // Now call the matching rules and see if we can generate a new token
        //
        for	(;;)
        {
            // Record the start of the token in our input stream.
            //
            this->state_->channel = TokenDefaultChannel;
            this->state_->tokenStartCharIndex	= charStream()->index();
            this->state_->text = ANTLR3_T("");

            if  (this->input_->LA(1) == CharstreamEof)
            {
                // Reached the end of the current stream, nothing more to do if this is
                // the last in the stack.
                //
                CommonTokenPtr teof = std::make_shared<CommonToken>(TokenEof);
                teof->setInputStream(charStream());
                teof->setStartIndex(charIndex());
                teof->setStopIndex(charIndex());
                return teof;
            }

            this->state_->token		= NULL;
            this->state_->error		= false;	    // Start out without an exception
            this->state_->failed		= false;

            // Call the generated lexer, see if it can get a new token together.
            //
            mTokens();

            if  (this->state_->error  == true)
            {
                // Recognition exception, report it and try to recover.
                //
                this->state_->failed	    = true;
                reportError();
                recover(); 
            }
            else
            {
                if (this->state_->token == NULL)
                {
                    // Emit the real token, which adds it in to the token stream basically
                    //
                    emit();
                }
                else if	(this->state_->token->type() == TokenInvalid)
                {
                    // A real token could have been generated, but "Computer say's naaaaah" and it
                    // it is just something we need to skip altogether.
                    //
                    continue;
                }

                // Good token, not skipped, not EOF token
                //
                return this->state_->token;
            }
        }
    }
}

/** An override of the lexer's nextToken() method that backtracks over mTokens() looking
 *  for matches in lexer filterMode.  No error can be generated upon error; just rewind, consume
 *  a token and then try again. state_->backtracking needs to be set as well.
 *  Make rule memoization happen only at levels above 1 as we start mTokens
 *  at state_->backtracking==1.
 */
template<class StringTraits>
typename antlr3<StringTraits>::CommonTokenPtr
    antlr3<StringTraits>::Lexer::nextTokenFiltering()
{
    /* Get rid of any previous token (token factory takes care of
     * any deallocation when this token is finally used up.
     */
    this->state_->token = nullptr;
    this->state_->error = false;	 // Start out without an exception
    this->state_->failed = false;

    // Record the start of the token in our input stream.
    this->state_->tokenStartCharIndex = this->input_->index();
    this->state_->text = antlr3::String();

    // Now call the matching rules and see if we can generate a new token
    for	(;;)
    {
		if (this->input_->LA(1) == antlr3::CharstreamEof)
		{
			/* Reached the end of the stream, nothing more to do.
			 */
			antlr3::CommonTokenPtr teof = std::make_shared<CommonToken>(antlr3::TokenEof);
            teof->setInputStream(charStream());
            teof->setStartIndex(charIndex());
            teof->setStopIndex(charIndex());
            return teof;
		}

		this->state_->token = nullptr;
		this->state_->error = false; // Start out without an exception

        antlr3::MarkerPtr m = this->input_->mark();
        this->state_->backtracking = 1; // No exceptions
        this->state_->failed = false;

        // Call the generated lexer, see if it can get a new token together.
        mTokens();
        this->state_->backtracking = 0;

        // mTokens backtracks with synpred at state_->backtracking==2
        // and we set the synpredgate to allow actions at level 1.

        if (this->state_->failed)
        {
            // Advance one char and try again
            m->rewind();
            this->input_->consume();
        }
        else
        {
            // Assemble the token and emit it to the stream
            emit();
            return this->state_->token;
        }
    }
}

/**
 * \brief
 * Default implementation of the nextToken() call for a lexer.
 * 
 * \param toksource
 * Points to the implementation of a token source. The lexer is 
 * addressed by the super structure pointer.
 * 
 * \returns
 * The next token in the current input stream or the EOF token
 * if there are no more tokens in any input stream in the stack.
 * 
 * Write detailed description for nextToken here.
 * 
 * \remarks
 * Write remarks for nextToken here.
 * 
 * \see nextTokenStr
 */
template<class StringTraits>
typename antlr3<StringTraits>::CommonTokenPtr
    antlr3<StringTraits>::Lexer::nextToken()
{
    // Find the next token in the current stream
    //
    CommonTokenPtr tok = nextTokenStr();

    // If we got to the EOF token then switch to the previous
    // input stream if there were any and just return the
    // EOF if there are none. We must check the next token
    // in any outstanding input stream we pop into the active
    // role to see if it was sitting at EOF after PUSHing the
    // stream we just consumed, otherwise we will return EOF
    // on the reinstalled input stream, when in actual fact
    // there might be more input streams to POP before the
    // real EOF of the whole logical inptu stream. Hence we
    // use a while loop here until we find somethign in the stream
    // that isn't EOF or we reach the actual end of the last input
    // stream on the stack.
    //
    while(tok->type() == TokenEof)
    {
        if  (!this->state_->streams.empty())
        {
            // We have another input stream in the stack so we
            // need to revert to it, then resume the loop to check
            // it wasn't sitting at EOF itself.
            //
            popCharStream();
            tok = nextTokenStr();
        }
        else
        {
            // There were no more streams on the input stack
            // so this EOF is the 'real' logical EOF for
            // the input stream. So we just exit the loop and 
            // return the EOF we have found.
            //
            break;
        }
        
    }

    // return whatever token we have, which may be EOF
    //
    return  tok;
}

template<class StringTraits>
typename antlr3<StringTraits>::LocationSourcePtr antlr3<StringTraits>::Lexer::source()
{
    return charStream();
}

template<class StringTraits>
void antlr3<StringTraits>::Lexer::reportError()
{
    // Indicate this recognizer had an error while processing.
    //
    this->state_->errorCount++;
    this->displayRecognitionError(this->state_->exception.get(), this->state_->tokenNames);
}

template<class StringTraits>
void antlr3<StringTraits>::Lexer::fillException(Exception* ex)
{
    ex->input		= this->input_;
    ex->item		= this->input_->LI(1); // Current input character
    ex->index		= this->input_->index();
    ex->location    = charStream()->location(ex->index);
    ex->streamName	= this->input_->sourceName();
}

template<class StringTraits>
std::uint32_t  antlr3<StringTraits>::Lexer::itemToInt(ItemPtr item) {
    return CharStream::charFromItem(item);
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::getCharErrorDisplay(std::uint32_t c)
{
    return ANTLR3_T("\'") + StringUtils::escape(c) + ANTLR3_T("\'");
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::getCharSetErrorDisplay(Bitset const & set)
{
    return set.toString(StringUtils::template escape<std::uint32_t>);
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::Lexer::getErrorMessage(Exception const * e, StringLiteral const * tokenNames)
{
    class Visitor : public Exception::Visitor
    {
    public:
        String retVal;

//        virtual void visit(RecognitionException const * e) override
//        {
//            retVal = ANTLR3_T("mismatched character ") + getCharErrorDisplay(e->item->toInt());
//        }
        virtual void visit(MismatchedTokenException const * e) override
        {
            retVal = ANTLR3_T("mismatched character ") + getCharErrorDisplay(CharStream::charFromItem(e->item)) +
                     ANTLR3_T(", expecting ") + getCharErrorDisplay(e->expecting);
        }
        virtual void visit(NoViableAltException const * e) override
        {
            // for development, can add "decision=<<"+nvae.grammarDecisionDescription+">>"
            // and "(decision="+nvae.decisionNumber+") and
            // "state "+nvae.stateNumber
            std::uint32_t c = CharStream::charFromItem(e->item);
            retVal = ANTLR3_T("no viable alternative at character ")+ getCharErrorDisplay(c);
        }
        virtual void visit(MismatchedSetException const * e) override
        {
            std::uint32_t c = CharStream::charFromItem(e->item);
            retVal = ANTLR3_T("mismatched character ") + getCharErrorDisplay(c) +
                     ANTLR3_T(", expecting set ") + getCharSetErrorDisplay(e->expectingSet);
        }
        virtual void visit(MismatchedRangeException const * e) override
        {
            retVal = ANTLR3_T("mismatched character ") + getCharErrorDisplay(CharStream::charFromItem(e->item)) +
                     ANTLR3_T(", expecting range ") + getCharErrorDisplay(e->low) +
                     ANTLR3_T("..") + getCharErrorDisplay(e->high);
        }
        virtual void visit(EarlyExitException const * e) override
        {
            // for development, can add "(decision="+eee.decisionNumber+")"
            retVal = ANTLR3_T("required (...)+ loop did not match anything at character ") +
                     getCharErrorDisplay(CharStream::charFromItem(e->item));
        }
        virtual void visit(FailedPredicateException const * e) override
        {
            retVal = retVal + ANTLR3_T("rule ") + e->ruleName + ANTLR3_T(" failed predicate: {") + e->predicateText + ANTLR3_T("}?");
        }
        virtual void visit(RewriteEarlyExitException const * e) override
        {
            retVal = ANTLR3_T("RewriteEarlyExitException");
        }
        virtual void visit(UnwantedTokenException const * e) override
        {
            retVal = ANTLR3_T("extraneous character ") + getCharErrorDisplay(CharStream::charFromItem(e->item));
        }
        virtual void visit(MissingTokenException const * e) override
        {
            retVal = ANTLR3_T("missing character ") + getCharErrorDisplay(e->expecting);
        }
    } v;
    e->accept(v);
    return v.retVal;
}

template<class StringTraits>
typename antlr3<StringTraits>::CharStreamPtr
    antlr3<StringTraits>::Lexer::charStream()
{
    return std::static_pointer_cast<CharStream>(this->input_);
}

template<class StringTraits>
void antlr3<StringTraits>::Lexer::setCharStream(CharStreamPtr input)
{
    /* Install the input interface
     */
    this->input_ = std::move(input);

    /* Set the current token to nothing
     */
    this->state_->token = NULL;
    this->state_->text = ANTLR3_T("");
    this->state_->tokenStartCharIndex	= -1;
}

/*!
 * \brief
 * Change to a new input stream, remembering the old one.
 * 
 * \param lexer
 * Pointer to the lexer instance to switch input streams for.
 * 
 * \param input
 * New input stream to install as the current one.
 * 
 * Switches the current character input stream to 
 * a new one, saving the old one, which we will revert to at the end of this 
 * new one.
 */
template<class StringTraits>
void antlr3<StringTraits>::Lexer::pushCharStream(CharStreamPtr input)
{
    // We have a stack, so we can save the current input stream
    // into it.
    //
    typename RecognizerSharedState::StreamState save = { this->input_->mark(), charStream() };
    this->state_->streams.push(save);

    // And now we can install this new one
    //
    setCharStream(input);
}

/*!
 * \brief
 * Stops using the current input stream and reverts to any prior
 * input stream on the stack.
 * 
 * \param lexer
 * Description of parameter lexer.
 * 
 * Pointer to a function that abandons the current input stream, whether it
 * is empty or not and reverts to the previous stacked input stream.
 *
 * \remark
 * The function fails silently if there are no prior input streams.
 */
template<class StringTraits>
void antlr3<StringTraits>::Lexer::popCharStream()
{
    // If we do not have a stream stack or we are already at the
    // stack bottom, then do nothing.
    //
    if	(!this->state_->streams.empty())
    {
        // We just leave the current stream to its fate, we do not close
        // it or anything as we do not know what the programmer intended
        // for it. This method can always be overridden of course.
        // So just find out what was currently saved on the stack and use
        // that now, then pop it from the stack.
        //
        auto save = this->state_->streams.top();
        this->state_->streams.pop();

        // Now install the stream as the current one.
        //
        setCharStream(save.stream);
        save.marker->rewind();
    }
}

template<class StringTraits>
void antlr3<StringTraits>::Lexer::emitNew(CommonTokenPtr token)
{
    this->state_->token    = token;
}

template<class StringTraits>
typename antlr3<StringTraits>::CommonTokenPtr
    antlr3<StringTraits>::Lexer::emit()
{
    /* We could check pointers to token factories and so on, but
    * we are in code that we want to run as fast as possible
    * so we are not checking any errors. So make sure you have installed an input stream before
    * trying to emit a new token.
    */
    CommonTokenPtr token = std::make_shared<CommonToken>();

    /* Install the supplied information, and some other bits we already know
    * get added automatically, such as the input stream it is associated with
    * (though it can all be overridden of course)
    */
    token->setType(this->state_->type);
    token->setChannel(this->state_->channel);
    token->setStartIndex(this->state_->tokenStartCharIndex);
    token->setStopIndex(charIndex());
    token->setInputStream(charStream());

    if(!this->state_->text.empty())
    {
        token->setText(this->state_->text);
    }

    this->state_->token = token;

    return  token;
}

template<class StringTraits>
bool antlr3<StringTraits>::Lexer::matchs(char const * string, size_t len)
{
    // Default char may be both signed and unsigned.
    // Cast it to unsigned to be sure.
    return matchStr(reinterpret_cast<unsigned char const *>(string), len);
}
    
template<class StringTraits>
bool antlr3<StringTraits>::Lexer::matchs(char16_t const * string, size_t len)
{
    return matchStr(string, len);
}

template<class StringTraits>
bool antlr3<StringTraits>::Lexer::matchs(char32_t const * string, size_t len)
{
    return matchStr(string, len);
}

template<class StringTraits>
template<class T>
bool antlr3<StringTraits>::Lexer::matchStr(T const * string, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (!matchc(string[i])) {
            return false;
        }
    }
    return true;
}

/** Implementation of matchc for the lexer, overrides any
 *  base implementation in the base recognizer. 
 *
 *  \remark
 *  Note that the generated code lays down arrays of ints for constant
 *  strings so that they are int UTF32 form!
 */
template<class StringTraits>
bool antlr3<StringTraits>::Lexer::matchc(std::uint32_t c)
{
    return matchRange(c, c);
}

/** Implementation of match range for the lexer, overrides any
 *  base implementation in the base recognizer. 
 *
 *  \remark
 *  Note that the generated code lays down arrays of ints for constant
 *  strings so that they are int UTF32 form!
 */
template<class StringTraits>
bool antlr3<StringTraits>::Lexer::matchRange(std::uint32_t low, std::uint32_t high)
{
    // What is in the stream at the moment?
    std::uint32_t c	= this->input_->LA(1);
    if( c >= low && c <= high)
    {
        // Matched correctly, consume it
        this->input_->consume();

        // Reset any failed indicator
        this->state_->failed = false;

        return	true;
    }
    
    // Failed to match, execption and recovery time.
    if	(this->state_->backtracking > 0)
    {
        this->state_->failed  = true;
        return false;
    }

    this->recordException(new MismatchedRangeException(low, high));

    // TODO: Implement exception creation more fully
    recover();

    return  false;
}

template<class StringTraits>
void antlr3<StringTraits>::Lexer::matchAny()
{
    this->input_->consume();
}

template<class StringTraits>
void antlr3<StringTraits>::Lexer::recover()
{
    this->input_->consume();
}

template<class StringTraits>
typename antlr3<StringTraits>::Index
    antlr3<StringTraits>::Lexer::charIndex()
{
    return this->input_->index();
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::Lexer::text()
{
    if (!this->state_->text.empty())
    {
        return this->state_->text;
    }

    return charStream()->substr(this->state_->tokenStartCharIndex, charIndex());
}
    
template<class StringTraits>
void antlr3<StringTraits>::Lexer::setText(String s)
{
    this->state_->text = std::move(s);
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::Lexer::traceCurrentItem() {
    std::uint32_t c = charStream()->LA(1);
    Location loc = charStream()->location(charStream()->index());
    return getCharErrorDisplay(c) + ANTLR3_T(" at ") + StringTraits::toString(loc.line()) + ANTLR3_T(":") + StringTraits::toString(loc.charPositionInLine());
}

