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

#include <antlr3/BaseRecognizer.hpp>
#include <antlr3/RecognizerSharedState.hpp>
#include <iostream>

template<class StringTraits>
antlr3<StringTraits>::RecognizerSharedState::RecognizerSharedState()
    : error(false)
    , exception(nullptr)
    , following()
    , errorRecovery(false)
    , lastErrorIndex(NullIndex)
    , failed(false)
    , errorCount(0)
    , backtracking(0)
    , ruleMemo()
    , tokenNames(nullptr)
    , token(nullptr)
    , channel(0)
    , type(TokenInvalid)
    , tokenStartCharIndex(0)
    , text()
    , streams()
{
}

template<class StringTraits>
antlr3<StringTraits>::RecognizerSharedState::~RecognizerSharedState()
{
}

template<class StringTraits>
antlr3<StringTraits>::BaseRecognizer::BaseRecognizer(RecognizerSharedStatePtr state)
    : state_(state ? state : std::make_shared<RecognizerSharedState>())
    , debugger_()
    , input_()
    , filteringMode_(false)
{
    assert(state_ != nullptr);
}

template<class StringTraits>
antlr3<StringTraits>::BaseRecognizer::~BaseRecognizer()
{
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::recordException(std::unique_ptr<Exception> ex)
{
    fillException(ex.get());
    state_->exception = std::move(ex);
    state_->error = true;    // Exception is outstanding
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::recordException(Exception* ex)
{
    recordException(std::unique_ptr<Exception>(ex));
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::followPush(Bitset const * data)
{
    state_->following.push_back(data);
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::followPop()
{
    state_->following.pop_back();
}

template<class StringTraits>
bool antlr3<StringTraits>::BaseRecognizer::evalPredicate(bool result, const char * predicate)
{
    if (debugger_ != nullptr)
    {
        debugger_->semanticPredicate(result, predicate);
    }
    return result;
}

/// Match current input symbol against ttype.  Upon error, do one token
/// insertion or deletion if possible.  
/// To turn off single token insertion or deletion error
/// recovery, override mismatchRecover() and have it call
/// plain mismatch(), which does not recover.  Then any error
/// in a rule will cause an exception and immediate exit from
/// rule.  Rule would recover by resynchronizing to the set of
/// symbols that can follow rule ref.
///
template<class StringTraits>
typename antlr3<StringTraits>::ItemPtr
    antlr3<StringTraits>::BaseRecognizer::match(std::uint32_t ttype, Bitset const & follow)
{
    // Pick up the current input token/node for assignment to labels
    //
    ItemPtr matchedSymbol = currentInputSymbol();

    if(input_->LA(1) == ttype)
    {
        // The token was the one we were told to expect
        //
        input_->consume();						// Consume that token from the stream
        state_->errorRecovery	= false;	// Not in error recovery now (if we were)
        state_->failed			= false;	// The match was a success
        return matchedSymbol;				// We are done
    }

    // We did not find the expected token type, if we are backtracking then
    // we just set the failed flag and return.
    //
    if	(state_->backtracking > 0)
    {
        // Backtracking is going on
        //
        state_->failed  = true;
        return matchedSymbol;
    }

    // We did not find the expected token and there is no backtracking
    // going on, so we mismatch, which creates an exception in the recognizer exception
    // stack.
    //
    matchedSymbol = recoverFromMismatchedToken(ttype, follow);
    return matchedSymbol;
}

/// Consumes the next token, whatever it is, and resets the recognizer state
/// so that it is not in error.
///
/// \param recognizer
/// Recognizer context pointer
///
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::matchAny()
{
    state_->errorRecovery = false;
    state_->failed		  = false;
    input_->consume();
    return;
}

template<class StringTraits>
bool antlr3<StringTraits>::BaseRecognizer::mismatchIsUnwantedToken(std::uint32_t ttype)
{
    return input_->LA(2) == ttype;
}

template<class StringTraits>
bool antlr3<StringTraits>::BaseRecognizer::mismatchIsMissingToken(Bitset const & follow)
{
    // The C bitset maps are laid down at compile time by the
    // C code generation. Hence we cannot remove things from them
    // and so on. So, in order to remove EOR (if we need to) then
    // we clone the static bitset.
    //
    Bitset followClone = follow;
    
    // Compute what can follow this grammar reference
    //
    if	(followClone.isMember(EorTokenType))
    {
        // EOR can follow, but if we are not the start symbol, we
        // need to remove it.
        //
        if	(state_->following.size() > 0)
        {
            followClone.remove(EorTokenType);
        }

        // Now compute the visiable tokens that can follow this rule, according to context
        // and make them part of the follow set.
        //
        Bitset viableTokensFollowingThisRule = computeCSRuleFollow();
        followClone.borInPlace(viableTokensFollowingThisRule);
    }

    /// if current token is consistent with what could come after set
    /// then we know we're missing a token; error recovery is free to
    /// "insert" the missing token
    ///
    /// BitSet cannot handle negative numbers like -1 (EOF) so I leave EOR
    /// in follow set to indicate that the fall of the start symbol is
    /// in the set (EOF can follow).
    ///
    return followClone.isMember(input_->LA(1)) || followClone.isMember(EorTokenType);
}

/// Report a recognition problem.
///
/// This method sets errorRecovery to indicate the parser is recovering
/// not parsing.  Once in recovery mode, no errors are generated.
/// To get out of recovery mode, the parser must successfully match
/// a token (after a resync).  So it will go:
///
///		1. error occurs
///		2. enter recovery mode, report error
///		3. consume until token found in resynch set
///		4. try to resume parsing
///		5. next match() will reset errorRecovery mode
///
/// If you override, make sure to update errorCount if you care about that.
///
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::reportError()
{
    // Invoke the debugger event if there is a debugger listening to us
    //
    if(debugger_ != NULL)
    {
        debugger_->recognitionException(state_->exception);
    }

    if	(state_->errorRecovery == true)
    {
        // Already in error recovery so don't display another error while doing so
        //
        return;
    }

    // Signal we are in error recovery now
    //
    state_->errorRecovery = true;
    
    // Indicate this recognizer had an error while processing.
    //
    state_->errorCount++;

    // Call the error display routine
    //
    displayRecognitionError(state_->exception.get(), state_->tokenNames);
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::beginBacktrack(std::uint32_t level)
{
    if	(debugger_ != NULL)
    {
        debugger_->beginBacktrack(level);
    }
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::endBacktrack(std::uint32_t level, bool successful)
{
    if	(debugger_ != NULL)
    {
        debugger_->endBacktrack(level, successful);
    }
}
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::beginResync()
{
    if	(debugger_ != NULL)
    {
        debugger_->beginResync();
    }
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::endResync()
{
    if	(debugger_ != NULL)
    {
        debugger_->endResync();
    }
}

/// Compute the error recovery set for the current rule.
/// Documentation below is from the Java implementation.
///
/// During rule invocation, the parser pushes the set of tokens that can
/// follow that rule reference on the stack; this amounts to
/// computing FIRST of what follows the rule reference in the
/// enclosing rule. This local follow set only includes tokens
/// from within the rule; i.e., the FIRST computation done by
/// ANTLR stops at the end of a rule.
//
/// EXAMPLE
//
/// When you find a "no viable alt exception", the input is not
/// consistent with any of the alternatives for rule r.  The best
/// thing to do is to consume tokens until you see something that
/// can legally follow a call to r *or* any rule that called r.
/// You don't want the exact set of viable next tokens because the
/// input might just be missing a token--you might consume the
/// rest of the input looking for one of the missing tokens.
///
/// Consider grammar:
///
/// a : '[' b ']'
///   | '(' b ')'
///   ;
/// b : c '^' INT ;
/// c : ID
///   | INT
///   ;
///
/// At each rule invocation, the set of tokens that could follow
/// that rule is pushed on a stack.  Here are the various "local"
/// follow sets:
///
/// FOLLOW(b1_in_a) = FIRST(']') = ']'
/// FOLLOW(b2_in_a) = FIRST(')') = ')'
/// FOLLOW(c_in_b) = FIRST('^') = '^'
///
/// Upon erroneous input "[]", the call chain is
///
/// a -> b -> c
///
/// and, hence, the follow context stack is:
///
/// depth  local follow set     after call to rule
///   0         <EOF>                    a (from main())
///   1          ']'                     b
///   3          '^'                     c
///
/// Notice that ')' is not included, because b would have to have
/// been called from a different context in rule a for ')' to be
/// included.
///
/// For error recovery, we cannot consider FOLLOW(c)
/// (context-sensitive or otherwise).  We need the combined set of
/// all context-sensitive FOLLOW sets--the set of all tokens that
/// could follow any reference in the call chain.  We need to
/// resync to one of those tokens.  Note that FOLLOW(c)='^' and if
/// we resync'd to that token, we'd consume until EOF.  We need to
/// sync to context-sensitive FOLLOWs for a, b, and c: {']','^'}.
/// In this case, for input "[]", LA(1) is in this set so we would
/// not consume anything and after printing an error rule c would
/// return normally.  It would not find the required '^' though.
/// At this point, it gets a mismatched token error and throws an
/// exception (since LA(1) is not in the viable following token
/// set).  The rule exception handler tries to recover, but finds
/// the same recovery set and doesn't consume anything.  Rule b
/// exits normally returning to rule a.  Now it finds the ']' (and
/// with the successful match exits errorRecovery mode).
///
/// So, you can see that the parser walks up call chain looking
/// for the token that was a member of the recovery set.
///
/// Errors are not generated in errorRecovery mode.
///
/// ANTLR's error recovery mechanism is based upon original ideas:
///
/// "Algorithms + Data Structures = Programs" by Niklaus Wirth
///
/// and
///
/// "A note on error recovery in recursive descent parsers":
/// http://portal.acm.org/citation.cfm?id=947902.947905
///
/// Later, Josef Grosch had some good ideas:
///
/// "Efficient and Comfortable Error Recovery in Recursive Descent
/// Parsers":
/// ftp://www.cocolab.com/products/cocktail/doca4.ps/ell.ps.zip
///
/// Like Grosch I implemented local FOLLOW sets that are combined
/// at run-time upon error to avoid overhead during parsing.
///
template<class StringTraits>
typename antlr3<StringTraits>::Bitset antlr3<StringTraits>::BaseRecognizer::computeErrorRecoverySet()
{
    return combineFollows(false);
}

/// Compute the context-sensitive FOLLOW set for current rule.
/// Documentation below is from the Java runtime.
///
/// This is the set of token types that can follow a specific rule
/// reference given a specific call chain.  You get the set of
/// viable tokens that can possibly come next (look ahead depth 1)
/// given the current call chain.  Contrast this with the
/// definition of plain FOLLOW for rule r:
///
///  FOLLOW(r)={x | S=>*alpha r beta in G and x in FIRST(beta)}
///
/// where x in T* and alpha, beta in V*; T is set of terminals and
/// V is the set of terminals and non terminals.  In other words,
/// FOLLOW(r) is the set of all tokens that can possibly follow
/// references to r in///any* sentential form (context).  At
/// runtime, however, we know precisely which context applies as
/// we have the call chain.  We may compute the exact (rather
/// than covering superset) set of following tokens.
///
/// For example, consider grammar:
///
/// stat : ID '=' expr ';'      // FOLLOW(stat)=={EOF}
///      | "return" expr '.'
///      ;
/// expr : atom ('+' atom)* ;   // FOLLOW(expr)=={';','.',')'}
/// atom : INT                  // FOLLOW(atom)=={'+',')',';','.'}
///      | '(' expr ')'
///      ;
///
/// The FOLLOW sets are all inclusive whereas context-sensitive
/// FOLLOW sets are precisely what could follow a rule reference.
/// For input input "i=(3);", here is the derivation:
///
/// stat => ID '=' expr ';'
///      => ID '=' atom ('+' atom)* ';'
///      => ID '=' '(' expr ')' ('+' atom)* ';'
///      => ID '=' '(' atom ')' ('+' atom)* ';'
///      => ID '=' '(' INT ')' ('+' atom)* ';'
///      => ID '=' '(' INT ')' ';'
///
/// At the "3" token, you'd have a call chain of
///
///   stat -> expr -> atom -> expr -> atom
///
/// What can follow that specific nested ref to atom?  Exactly ')'
/// as you can see by looking at the derivation of this specific
/// input.  Contrast this with the FOLLOW(atom)={'+',')',';','.'}.
///
/// You want the exact viable token set when recovering from a
/// token mismatch.  Upon token mismatch, if LA(1) is member of
/// the viable next token set, then you know there is most likely
/// a missing token in the input stream.  "Insert" one by just not
/// throwing an exception.
///
template<class StringTraits>
typename antlr3<StringTraits>::Bitset antlr3<StringTraits>::BaseRecognizer::computeCSRuleFollow()
{
    return combineFollows(false);
}

/// Compute the current followset for the input stream.
///
template<class StringTraits>
typename antlr3<StringTraits>::Bitset
    antlr3<StringTraits>::BaseRecognizer::combineFollows(bool exact)
{
    std::size_t top = state_->following.size();
    
    Bitset followSet;
    for (std::size_t i = top; i>0; i--)
    {
        Bitset const & localFollowSet = *state_->following.at(i-1);
        followSet.borInPlace(localFollowSet);

        if	(exact)
        {
            if (!localFollowSet.isMember(EorTokenType))
            {
                // Only leave EOR in the set if at top (start rule); this lets us know
                // if we have to include the follow(start rule); I.E., EOF
                //
                if (i>1)
                {
                    followSet.remove(EorTokenType);
                }
            }
            else
            {
                break;	// Cannot see End Of Rule from here, just drop out
            }
        }
    }

    return std::move(followSet);
}

/// Standard/Example error display method.
/// No generic error message display funciton coudl possibly do everything correctly
/// for all possible parsers. Hence you are provided with this example routine, which
/// you should override in your parser/tree parser to do as you will.
///
/// Here we depart somewhat from the Java runtime as that has now split up a lot
/// of the error display routines into spearate units. However, ther is little advantage
/// to this in the C version as you will probably implement all such routines as a 
/// separate translation unit, rather than install them all as pointers to functions
/// in the base recognizer.
///
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::displayRecognitionError(Exception const * e, StringLiteral const * tokenNames)
{
    emitErrorMessage(getErrorHeader(e, tokenNames) + ANTLR3_T(" ") + getErrorMessage(e, tokenNames));
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::emitErrorMessage(String msg)
{
    std::cerr << msg << std::endl;
}

/// Return how many syntax errors were detected by this recognizer
///
template<class StringTraits>
std::uint32_t antlr3<StringTraits>::BaseRecognizer::numberOfSyntaxErrors()
{
    return state_->errorCount;
}

/// Recover from an error found on the input stream.  Mostly this is
/// NoViableAlt exceptions, but could be a mismatched token that
/// the match() routine could not recover from.
///
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::recover()
{
    // Are we about to repeat the same error?
    //
    if(state_->lastErrorIndex == input_->index())
    {
        // The last error was at the same token index point. This must be a case
        // where LT(1) is in the recovery token set so nothing is
        // consumed. Consume a single token so at least to prevent
        // an infinite loop; this is a failsafe.
        //
        input_->consume();
    }

    // Record error index position
    //
    state_->lastErrorIndex = input_->index();
    
    // Work out the follows set for error recovery
    //
    Bitset followSet = computeErrorRecoverySet();

    // Call resync hook (for debuggers and so on)
    //
    beginResync();

    // Consume tokens until we have resynced to something in the follows set
    //
    consumeUntilSet(followSet);

    // End resync hook 
    //
    endResync();

    // Reset the inError flag so we don't re-report the exception
    //
    state_->error	= false;
    state_->failed	= false;
}


/// Attempt to recover from a single missing or extra token.
///
/// EXTRA TOKEN
///
/// LA(1) is not what we are looking for.  If LA(2) has the right token,
/// however, then assume LA(1) is some extra spurious token.  Delete it
/// and LA(2) as if we were doing a normal match(), which advances the
/// input.
///
/// MISSING TOKEN
///
/// If current token is consistent with what could come after
/// ttype then it is ok to "insert" the missing token, else throw
/// exception For example, Input "i=(3;" is clearly missing the
/// ')'.  When the parser returns from the nested call to expr, it
/// will have call chain:
///
///    stat -> expr -> atom
///
/// and it will be trying to match the ')' at this point in the
/// derivation:
///
///       => ID '=' '(' INT ')' ('+' atom)* ';'
///                          ^
/// match() will see that ';' doesn't match ')' and report a
/// mismatched token error.  To recover, it sees that LA(1)==';'
/// is in the set of tokens that can follow the ')' token
/// reference in rule atom.  It can assume that you forgot the ')'.
///
/// The exception that was passed in, in the java implementation is
/// sorted in the recognizer exception stack in the C version. To 'throw' it we set the
/// error flag and rules cascade back when this is set.
///
template<class StringTraits>
typename antlr3<StringTraits>::ItemPtr
    antlr3<StringTraits>::BaseRecognizer::recoverFromMismatchedToken(std::uint32_t ttype, Bitset const & follow)
{
    // If the next token after the one we are looking at in the input stream
    // is what we are looking for then we remove the one we have discovered
    // from the stream by consuming it, then consume this next one along too as
    // if nothing had happened.
    //
    if (mismatchIsUnwantedToken(ttype))
    {
        recordException(new UnwantedTokenException(ttype));

        // "delete" the extra token
        //
        beginResync();
        input_->consume();
        endResync();

        // Print out the error after we consume so that ANTLRWorks sees the
        // token in the exception.
        //
        reportError();

        // Return the token we are actually matching
        //
        ItemPtr matchedSymbol = currentInputSymbol();

        // Consume the token that the rule actually expected to get as if everything
        // was hunky dory.
        //
        input_->consume();

        state_->error  = false;	// Exception is not outstanding any more

        return matchedSymbol;
    }

    // Single token deletion (Unwanted above) did not work
    // so we see if we can insert a token instead by calculating which
    // token would be missing
    //
    if	(mismatchIsMissingToken(follow))
    {
        // We can fake the missing token and proceed
        //
        ItemPtr matchedSymbol = getMissingSymbol(nullptr, ttype, follow);

        recordException(new MissingTokenException(itemToInt(matchedSymbol)));

        // Print out the error after we insert so that ANTLRWorks sees the
        // token in the exception.
        //
        reportError();

        state_->error  = false;	// Exception is not outstanding any more

        return	matchedSymbol;
    }


    // Neither deleting nor inserting tokens allows recovery
    // must just report the exception.
    //
    recordException(new MismatchedTokenException(ttype));
    state_->error	    = true;
    return nullptr;
}

template<class StringTraits>
typename antlr3<StringTraits>::ItemPtr
    antlr3<StringTraits>::BaseRecognizer::recoverFromMismatchedSet(Bitset const & follow)
{
    if	(mismatchIsMissingToken(follow) == true)
    {
        // We can fake the missing token and proceed
        //
        ItemPtr matchedSymbol = getMissingSymbol(nullptr, TokenInvalid, follow);
        
        recordException(new MissingTokenException(itemToInt(matchedSymbol)));

        // Print out the error after we insert so that ANTLRWorks sees the
        // token in the exception.
        //
        reportError();

        state_->error  = false;	// Exception is not outstanding any more

        return	matchedSymbol;
    }

    // TODO - Single token deletion like in recoverFromMismatchedToken()
    //

    // Neither deleting nor inserting tokens allows recovery
    // must just report the exception.
    //
    recordException(new MismatchedSetException(Bitset()));
    state_->error	= true;
    state_->failed	= true;
    return NULL;
}

/// This code is factored out from mismatched token and mismatched set
///  recovery.  It handles "single token insertion" error recovery for
/// both.  No tokens are consumed to recover from insertions.  Return
/// true if recovery was possible else return false.
///
template<class StringTraits>
bool antlr3<StringTraits>::BaseRecognizer::recoverFromMismatchedElement(Bitset const & followBits)
{
    Bitset follow(followBits);

    /* We have a bitmap for the follow set, hence we can compute 
     * what can follow this grammar element reference.
     */
    if	(follow.isMember(EorTokenType))
    {
        /* First we need to know which of the available tokens are viable
         * to follow this reference.
         */
        Bitset viableToksFollowingRule = computeCSRuleFollow();

        /* Remove the EOR token, which we do not wish to compute with
         */
        follow.remove(EorTokenType);

        /* We now have the computed set of what can follow the current token
         */
    }

    /* We can now see if the current token works with the set of tokens
     * that could follow the current grammar reference. If it looks like it
     * is consistent, then we can "insert" that token by not throwing
     * an exception and assuming that we saw it. 
     */
    if(follow.isMember(input_->LA(1)))
    {
        /* report the error, but don't cause any rules to abort and stuff
         */
        reportError();
        state_->error			= false;
        state_->failed			= false;
        return true;	/* Success in recovery	*/
    }

    /* We could not find anything viable to do, so this is going to 
     * cause an exception.
     */
    return  false;
}

/// Eat tokens from the input stream until we get one of JUST the right type
///
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::consumeUntil(std::uint32_t tokenType)
{
    // What do have at the moment?
    //
    std::uint32_t ttype	= input_->LA(1);

    // Start eating tokens until we get to the one we want.
    //
    while(ttype != TokenEof && ttype != tokenType)
    {
        input_->consume();
        ttype = input_->LA(1);
    }
}

/// Eat tokens from the input stream until we find one that
/// belongs to the supplied set.
///
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::consumeUntilSet(Bitset const & set)
{
    // What do have at the moment?
    //
    std::uint32_t ttype	= input_->LA(1);

    // Start eating tokens until we get to one we want.
    //
    while(ttype != TokenEof && set.isMember(ttype) == false)
    {
        input_->consume();
        ttype	= input_->LA(1);
    }
}

/** Pointer to a function to return whether the rule has parsed input starting at the supplied 
 *  start index before. If the rule has not parsed input starting from the supplied start index,
 *  then it will return MemoRuleUnknown. If it has parsed from the suppled start point
 *  then it will return the point where it last stopped parsing after that start point.
 *
 * \remark
 * The rule memos are an List of Lists, however if this becomes any kind of performance
 * issue (it probably won't, the hash tables are pretty quick) then we could make a special int only
 * version of the table.
 */
template<class StringTraits>
typename antlr3<StringTraits>::Index
    antlr3<StringTraits>::BaseRecognizer::getRuleMemoization(Index ruleIndex, Index ruleParseStart)
{
    /* The rule memos are an List of List.
     */
    auto ruleList = state_->ruleMemo.find(ruleIndex);
    if (ruleList == state_->ruleMemo.end())
    {
        return MEMO_RULE_UNKNOWN;
    }

    auto stopIndex = ruleList->second.find(ruleParseStart);
    if (stopIndex == ruleList->second.end())
    {
        return MEMO_RULE_UNKNOWN;
    }

    return stopIndex->second;
}

/** Has this rule already parsed input at the current index in the
 *  input stream?  Return true if we have and false
 *  if we have not.
 *
 *  This method has a side-effect: if we have seen this input for
 *  this rule and successfully parsed before, then seek ahead to
 *  1 past the stop token matched for this rule last time.
 */
template<class StringTraits>
bool antlr3<StringTraits>::BaseRecognizer::alreadyParsedRule(Index ruleIndex)
{
    if (!filteringMode_ || state_->backtracking > 1) {
        /* See if we have a memo marker for this.
         */
        Index stopIndex = getRuleMemoization(ruleIndex, input_->index());

        if (stopIndex == MEMO_RULE_UNKNOWN)
        {
            return false;
        }

        if (stopIndex == MEMO_RULE_FAILED)
        {
            state_->failed = true;
        }
        else
        {
            input_->seek(stopIndex + 1);
        }

        /* If here then the rule was executed for this input already
         */
        return  true;
    } else {
        return false;
    }
}

/** Record whether or not this rule parsed the input at this position
 *  successfully.
 */
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::memoize(Index ruleIndex, Index ruleParseStart)
{
    if (!filteringMode_ || state_->backtracking > 1) {
        Index stopIndex = state_->failed == true ? MEMO_RULE_FAILED : input_->index();
        auto & ruleList = state_->ruleMemo[ruleIndex];
        ruleList[ruleParseStart] = stopIndex;
    }
}

/** A syntactic predicate.  Returns true/false depending on whether
 *  the specified grammar fragment matches the current input stream.
 *  This resets the failed instance var afterwards.
 */
template<class StringTraits>
bool antlr3<StringTraits>::BaseRecognizer::synpred(void * ctx, void (*predicate)(void * ctx))
{
    /* Begin backtracking so we can get back to where we started after trying out
     * the syntactic predicate.
     */
    MarkerPtr start = input_->mark();
    state_->backtracking++;

    /* Try the syntactical predicate
     */
    predicate(ctx);

    /* Reset
     */
    start->rewind();
    state_->backtracking--;

    if	(state_->failed == true)
    {
        /* Predicate failed
         */
        state_->failed = false;
        return	false;
    }
    else
    {
        /* Predicate was successful
         */
        state_->failed	= false;
        return	true;
    }
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::reset()
{

    // Reset the state flags
    //
    state_->errorRecovery	= false;
    state_->lastErrorIndex	= -1;
    state_->failed			= false;
    state_->errorCount		= 0;
    state_->backtracking		= 0;
    state_->following.clear();
    state_->ruleMemo.clear();
}

template<class StringTraits>
typename antlr3<StringTraits>::ItemPtr
    antlr3<StringTraits>::BaseRecognizer::currentInputSymbol()
{
    return input_->LI(1);
}

// Default implementation is for parser and assumes a token stream as supplied by the runtime.
// You MAY need override this function if the standard COMMON_TOKEN_STREAM is not what you are using.
//
template<class StringTraits>
typename antlr3<StringTraits>::ItemPtr
    antlr3<StringTraits>::BaseRecognizer::getMissingSymbol(
    ExceptionPtr e,
    std::uint32_t expectedTokenType,
    Bitset const & follow
)
{
    return ItemPtr();
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::BaseRecognizer::getErrorHeader(Exception const * e, StringLiteral const *)
{
    String retVal;

    // See if there is a 'filename' we can use
    if(e->streamName.empty())
    {
        retVal += ANTLR3_T("<unknown source>");
    }
    else
    {
        retVal += e->streamName;
    }

    // Next comes the line number
    retVal += ANTLR3_T(":");
    retVal += StringTraits::toString(e->location.line());
    retVal += ANTLR3_T(":");
    retVal += StringTraits::toString(e->location.charPositionInLine());
    retVal += ANTLR3_T(": error: ");
    return retVal;
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::getTokenErrorDisplay(std::uint32_t type, StringLiteral const * tokenNames)
{
    return ANTLR3_T("<") + getTokenName(type, tokenNames) + ANTLR3_T(">");
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::getTokenErrorDisplay(ItemPtr item, StringLiteral const * tokenNames)
{
    CommonTokenPtr t = std::static_pointer_cast<CommonToken>(item);
    if (t == nullptr)
    {
        assert(false);
        return String();
    }

    String s = t->text();
    if (!s.empty())
    {
        /// TODO: Ensure string is printable
        return s;
    }

    return getTokenErrorDisplay(t->type(), tokenNames);
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::getTokenSetErrorDisplay(Bitset const & set, StringLiteral const * tokenNames)
{
    return set.toString(
        [=](std::uint32_t ttype) -> String {
            return String(ANTLR3_T("<")) + getTokenName(ttype, tokenNames) + ANTLR3_T(">");
        }
    );
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::BaseRecognizer::getErrorMessage(Exception const * e, StringLiteral const * tokenNames)
{
    // Default implementation is provided for parser.

    static const String s;

    class Visitor : public Exception::Visitor
    {
    public:
        String retVal;
        StringLiteral const * const tokenNames;

        Visitor(StringLiteral const * names) : tokenNames(names) {}

//        virtual void visit(RecognitionException const * e) override
//        {
//            retVal = ANTLR3_T("mismatched character ") + getCharErrorDisplay(e->item->toInt());
//        }
        virtual void visit(MismatchedTokenException const * e) override
        {
            retVal = s + ANTLR3_T("mismatched input ") + getTokenErrorDisplay(e->item, tokenNames) +
            ANTLR3_T(", expecting ") + getTokenName(e->expecting, tokenNames);
        }
        virtual void visit(NoViableAltException const * e) override
        {
            // for development, can add "decision=<<"+nvae.grammarDecisionDescription+">>"
            // and "(decision="+nvae.decisionNumber+")" and
            // "state "+nvae.stateNumber
            retVal = s + ANTLR3_T("no viable alternative at input ") + getTokenErrorDisplay(e->item, tokenNames);
        }
        virtual void visit(MismatchedSetException const * e) override
        {
            retVal = s + ANTLR3_T("mismatched input ") + getTokenErrorDisplay(e->item, tokenNames) +
                ANTLR3_T(", expecting set ") + getTokenSetErrorDisplay(e->expectingSet, tokenNames);
        }
        virtual void visit(MismatchedRangeException const * e) override
        {
            retVal = ANTLR3_T("mismatched input ") + getTokenErrorDisplay(e->item, tokenNames) +
                     ANTLR3_T(", expecting range ") + getTokenErrorDisplay(e->low, tokenNames) +
                     ANTLR3_T("..") + getTokenErrorDisplay(e->high, tokenNames);
        }
        virtual void visit(EarlyExitException const * e) override
        {
            // for development, can add "(decision="+eee.decisionNumber+")"
            retVal = s + ANTLR3_T("required (...)+ loop did not match anything at input ") +
                getTokenErrorDisplay(e->item, tokenNames);
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
            retVal = s + ANTLR3_T("extraneous input ") + getTokenErrorDisplay(e->item, tokenNames);
        }
        virtual void visit(MissingTokenException const * e) override
        {
            retVal = s + ANTLR3_T("missing token ") + getTokenName(e->expecting, tokenNames);
        }
    } v(tokenNames);
    e->accept(v);
    return v.retVal;
}

template<class StringTraits>
std::ostream& antlr3<StringTraits>::BaseRecognizer::traceStream() {
    return std::cout;
}
    
template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::traceIn(StringLiteral ruleName, int ruleNo) {
    auto itemStr = traceCurrentItem();
    auto & s = traceStream();
    s << "enter " << ruleName << " " << itemStr;
    if (state_->backtracking > 0) {
        s << " backtracking=" << state_->backtracking;
    }
    s << std::endl;
}

template<class StringTraits>
void antlr3<StringTraits>::BaseRecognizer::traceOut(StringLiteral ruleName, int ruleNo) {
    auto itemStr = traceCurrentItem();
    auto & s = traceStream();
    s << "enter " << ruleName << " " << itemStr;
    if (state_->backtracking > 0) {
        s << " backtracking=" << state_->backtracking << (state_->failed ? " failed" : " succeeded");
    }
    s << std::endl;
}

