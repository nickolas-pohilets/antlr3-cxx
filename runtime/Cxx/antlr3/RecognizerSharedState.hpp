/** \file
 * While the C runtime does not need to model the state of
 * multiple lexers and parsers in the same way as the Java runtime does
 * it is no overhead to reflect that model. In fact the
 * C runtime has always been able to share recognizer state.
 *
 * This 'class' therefore defines all the elements of a recognizer
 * (either lexer, parser or tree parser) that are need to
 * track the current recognition state. Multiple recognizers
 * may then share this state, for instance when one grammar
 * imports another.
 */

#ifndef _ANTLR3_RECOGNIZER_SHARED_STATE_HPP
#define _ANTLR3_RECOGNIZER_SHARED_STATE_HPP

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
#include <stack>
#include <map>

namespace antlr3 {

/** All the data elements required to track the current state
 *  of any recognizer (lexer, parser, tree parser).
 * May be share between multiple recognizers such that 
 * grammar inheritance is easily supported.
 */
struct RecognizerSharedState
{
    RecognizerSharedState();
    ~RecognizerSharedState();

    /** If set to True then the recognizer has an exception
     * condition (this is tested by the generated code for the rules of
     * the grammar).
     */
    bool	    error;

    /** Points to the first in a possible chain of exceptions that the
     *  recognizer has discovered.
     */
    ExceptionPtr exception;

    /** Track the set of token types that can follow any rule invocation.
     *  Stack structure, to support: List<BitSet>.
     */
    std::vector<Bitset const *> following;

    /** This is true when we see an error and before having successfully
     *  matched a token.  Prevents generation of more than one error message
     *  per error.
     */
    bool	errorRecovery;
    
    /** The index into the input stream where the last error occurred.
     * 	This is used to prevent infinite loops where an error is found
     *  but no token is consumed during recovery...another error is found,
     *  ad nauseam.  This is a failsafe mechanism to guarantee that at least
     *  one token/tree node is consumed for two errors.
     */
    Index lastErrorIndex;

    /** In lieu of a return value, this indicates that a rule or token
     *  has failed to match.  Reset to false upon valid token match.
     */
    bool failed;

    /** When the recognizer terminates, the error handling functions
     *  will have incremented this value if any error occurred (that was displayed). It can then be
     *  used by the grammar programmer without having to use static globals.
     */
    std::uint32_t	errorCount;

    /** If 0, no backtracking is going on.  Safe to exec actions etc...
     *  If >0 then it's the level of backtracking.
     */
    std::int32_t	backtracking;

    /** Vector of List for rule memoizing.
     *  Tracks  the stop token index for each rule.  ruleMemo[ruleIndex] is
     *  the memoization table for ruleIndex.  For key ruleStartIndex, you
     *  get back the stop token for associated rule or MEMO_RULE_FAILED.
     *
     *  This is only used if rule memoization is on.
     */
    std::map<Index, std::map<Index, Index>>	ruleMemo;

    /** Pointer to an array of token names
     *  that are generally useful in error reporting. The generated parsers install
     *  this pointer. The table it points to is statically allocated as 8 bit ascii
     *  at parser compile time - grammar token names are thus restricted in character
     *  sets, which does not seem to terrible.
     */
    ConstString const * tokenNames;

    /** The goal of all lexer rules/methods is to create a token object.
     *  This is an instance variable as multiple rules may collaborate to
     *  create a single token.  For example, NUM : INT | FLOAT ;
     *  In this case, you want the INT or FLOAT rule to set token and not
     *  have it reset to a NUM token in rule NUM.
     */
    std::deque<CommonTokenPtr> tokenBuffer;

    /** The channel number for the current token
     */
    std::uint32_t		channel;

    /** The token type for the current token
     */
    std::uint32_t		type;
    
    /** What character index in the stream did the current token start at?
     *  Needed, for example, to get the text for current token.  Set at
     *  the start of nextToken.
     */
    Index tokenStartCharIndex;

    /** Text for the current token. This can be overridden by setting this 
     *  variable directly or by using the SETTEXT() macro (preferred) in your
     *  lexer rules.
     */
    String		text;

    /** Input stream stack, which allows the C programmer to switch input streams 
     *  easily and allow the standard nextToken() implementation to deal with it
     *  as this is a common requirement.
     */
    struct StreamState { MarkerPtr marker; CharStreamPtr stream; };
    std::stack<StreamState> streams;
};

} // namespace antlr3

#endif
