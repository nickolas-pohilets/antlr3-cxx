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

namespace antlr3 {

Lexer::Lexer(RecognizerSharedStatePtr state)
    : BaseRecognizer(state)
{
}

Lexer::Lexer(CharStreamPtr input, RecognizerSharedStatePtr state)
: Lexer(state)
{
    setCharStream(std::move(input));
}

Lexer::~Lexer()
{
}


void Lexer::reset()
{
    state_->token			    = NULL;
    state_->type			    = TokenInvalid;
    state_->channel			    = TokenDefaultChannel;
    state_->tokenStartCharIndex	= -1;
    state_->text	            = ANTLR3_T("");
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
CommonTokenPtr Lexer::nextTokenStr()
{
    /// Loop until we get a non skipped token or EOF
    ///
    for	(;;)
    {
        // Get rid of any previous token (token factory takes care of
        // any de-allocation when this token is finally used up.
        //
        state_->token		    = NULL;
        state_->error		    = false;	    // Start out without an exception
        state_->failed		    = false;

        // Now call the matching rules and see if we can generate a new token
        //
        for	(;;)
        {
            // Record the start of the token in our input stream.
            //
            state_->channel = TokenDefaultChannel;
            state_->tokenStartCharIndex	= charStream()->index();
            state_->text = ANTLR3_T("");

            if  (input_->LA(1) == CharstreamEof)
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

            state_->token		= NULL;
            state_->error		= false;	    // Start out without an exception
            state_->failed		= false;

            // Call the generated lexer, see if it can get a new token together.
            //
            mTokens();

            if  (state_->error  == true)
            {
                // Recognition exception, report it and try to recover.
                //
                state_->failed	    = true;
                reportError();
                recover(); 
            }
            else
            {
                if (state_->token == NULL)
                {
                    // Emit the real token, which adds it in to the token stream basically
                    //
                    emit();
                }
                else if	(state_->token->type() == TokenInvalid)
                {
                    // A real token could have been generated, but "Computer say's naaaaah" and it
                    // it is just something we need to skip altogether.
                    //
                    continue;
                }

                // Good token, not skipped, not EOF token
                //
                return  state_->token;
            }
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
CommonTokenPtr Lexer::nextToken()
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
        if  (!state_->streams.empty())
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

LocationSourcePtr Lexer::source()
{
    return charStream();
}

void Lexer::reportError()
{
    // Indicate this recognizer had an error while processing.
    //
    state_->errorCount++;
    displayRecognitionError(state_->exception.get(), state_->tokenNames);
}

void Lexer::fillException(Exception* ex)
{
    ex->input		= input_;
    ex->item		= input_->LI(1); // Current input character
    ex->index		= input_->index();
    ex->location    = charStream()->location(ex->index);
    ex->streamName	= input_->sourceName();
}

std::uint32_t Lexer::itemToInt(ItemPtr item) {
    return CharStream::charFromItem(item);
}

static String getCharErrorDisplay(Char c)
{
    return ANTLR3_T("\'") + escape(c) + ANTLR3_T("\'");
}

static String getCharSetErrorDisplay(Bitset const & set)
{
    return set.toString(escape<Char>);
}

String Lexer::getErrorMessage(Exception const * e, ConstString const * tokenNames)
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
            Char c = (Char)reinterpret_cast<size_t>(e->item.get());
            retVal = ANTLR3_T("no viable alternative at character ")+ getCharErrorDisplay(c);
        }
        virtual void visit(MismatchedSetException const * e) override
        {
            Char c = (Char)reinterpret_cast<size_t>(e->item.get());
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

CharStreamPtr Lexer::charStream()
{
    return std::static_pointer_cast<CharStream>(input_);
}

void Lexer::setCharStream(CharStreamPtr input)
{
    /* Install the input interface
     */
    input_	= std::move(input);

    /* Set the current token to nothing
     */
    state_->token = NULL;
    state_->text = ANTLR3_T("");
    state_->tokenStartCharIndex	= -1;
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
void Lexer::pushCharStream(CharStreamPtr input)
{
    // We have a stack, so we can save the current input stream
    // into it.
    //
    RecognizerSharedState::StreamState save = { input_->mark(), charStream() };
    state_->streams.push(save);

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
void Lexer::popCharStream()
{
    // If we do not have a stream stack or we are already at the
    // stack bottom, then do nothing.
    //
    if	(!state_->streams.empty())
    {
        // We just leave the current stream to its fate, we do not close
        // it or anything as we do not know what the programmer intended
        // for it. This method can always be overridden of course.
        // So just find out what was currently saved on the stack and use
        // that now, then pop it from the stack.
        //
        auto save = state_->streams.top();
        state_->streams.pop();

        // Now install the stream as the current one.
        //
        setCharStream(save.stream);
        save.marker->rewind();
    }
}

void Lexer::emitNew(CommonTokenPtr token)
{
    state_->token    = token;
}

CommonTokenPtr Lexer::emit()
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
    token->setType(state_->type);
    token->setChannel(state_->channel);
    token->setStartIndex(state_->tokenStartCharIndex);
    token->setStopIndex(charIndex());
    token->setInputStream(charStream());

    if(!state_->text.empty())
    {
        token->setText(state_->text);
    }

    state_->token	    = token;

    return  token;
}

/** Implementation of matchs for the lexer, overrides any
 *  base implementation in the base recognizer. 
 *
 *  \remark
 *  Note that the generated code lays down arrays of ints for constant
 *  strings so that they are int UTF32 form!
 */
bool Lexer::matchs(Char * string)
{
    while(*string != StringTerminator)
    {
        if (!matchc(*string))
        {
            return false;
        }

        ++string;
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
bool Lexer::matchc(Char c)
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
bool Lexer::matchRange(Char low, Char high)
{
    // What is in the stream at the moment?
    Char c	= input_->LA(1);
    if( c >= low && c <= high)
    {
        // Matched correctly, consume it
        input_->consume();

        // Reset any failed indicator
        state_->failed = false;

        return	true;
    }
    
    // Failed to match, execption and recovery time.
    if	(state_->backtracking > 0)
    {
        state_->failed  = true;
        return	false;
    }

    this->recordException(new MismatchedRangeException(low, high));

    // TODO: Implement exception creation more fully
    recover();

    return  false;
}

void Lexer::matchAny()
{
    input_->consume();
}

void Lexer::recover()
{
    input_->consume();
}

Index Lexer::charIndex()
{
    return input_->index();
}

String Lexer::text()
{
    if (!state_->text.empty())
    {
        return state_->text;
    }

    return charStream()->substr(state_->tokenStartCharIndex, charIndex());
}
    
void Lexer::setText(String s)
{
    state_->text = std::move(s);
}

String Lexer::traceCurrentItem() {
    Char c = charStream()->LA(1);
    Location loc = charStream()->location(charStream()->index());
    return getCharErrorDisplay(c) + ANTLR3_T(" at ") + toString(loc.line()) + ANTLR3_T(":") + toString(loc.charPositionInLine());
}

} // namespace antlr3
