#ifndef _ANTLR3_BASERECOGNIZER_HPP
#define _ANTLR3_BASERECOGNIZER_HPP

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
#include <antlr3/Exception.hpp>
#include <antlr3/CharStream.hpp>
#include <antlr3/TokenStream.hpp>
#include <antlr3/CommonToken.hpp>
#include <antlr3/CommonTreeNodeStream.hpp>
#include <antlr3/DebugEventListener.hpp>
#include <antlr3/RecognizerSharedState.hpp>

/// A generic recognizer that can handle recognizers generated from
/// lexer, parser, and tree grammars.  This is all the parsing
/// support code essentially; most of it is error recovery stuff and
/// backtracking.
class BaseRecognizer
{
    friend class CyclicDfa;
public:
    BaseRecognizer(RecognizerSharedStatePtr state);
    virtual ~BaseRecognizer();
    
    /** Match current input symbol against ttype.  Attempt
     *  single token insertion or deletion error recovery.  If
     *  that fails, throw MismatchedTokenException.
     *
     *  To turn off single token insertion or deletion error
     *  recovery, override recoverFromMismatchedToken() and have it
     *  throw an exception. See TreeParser.recoverFromMismatchedToken().
     *  This way any error in a rule will cause an exception and
     *  immediate exit from rule.  Rule would recover by resynchronizing
     *  to the set of symbols that can follow rule ref.
     */
    

    /// Function that matches the current input symbol
    /// against the supplied type. the function causes an error if a
    /// match is not found and the default implementation will also
    /// attempt to perform one token insertion or deletion if that is
    /// possible with the input stream. You can override the default
    /// implementation by installing a pointer to your own function
    /// in this interface after the recognizer has initialized. This can
    /// perform different recovery options or not recover at all and so on.
    /// To ignore recovery altogether, see the comments in the default
    /// implementation of this function in antlr3baserecognizer.c
    ///
    /// Note that errors are signalled by setting the error flag below
    /// and creating a new exception structure and installing it in the
    /// exception pointer below (you can chain these if you like and handle them
    /// in some customized way).
    ///
    ItemPtr match(std::uint32_t ttype, Bitset const & follow);

    /// Function that matches the next token/char in the input stream
    /// regardless of what it actually is.
    ///
    void matchAny();
    
    /// Pointer to a function that decides if the token ahead of the current one is the 
    /// one we were loking for, in which case the curernt one is very likely extraneous
    /// and can be reported that way.
    ///
    bool mismatchIsUnwantedToken(std::uint32_t ttype);

    /// Pointer to a function that decides if the current token is one that can logically
    /// follow the one we were looking for, in which case the one we were looking for is 
    /// probably missing from the input.
    ///
    bool mismatchIsMissingToken(Bitset const & follow);

    /// Pointer to a function to call to report a recognition problem. You may override
    /// this function with your own function, but refer to the standard implementation
    /// in antlr3baserecognizer.c for guidance. The function should recognize whether
    /// error recovery is in force, so that it does not print out more than one error messages
    /// for the same error. From the java comments in BaseRecognizer.java:
    ///
    /// This method sets errorRecovery to indicate the parser is recovering
    /// not parsing.  Once in recovery mode, no errors are generated.
    /// To get out of recovery mode, the parser must successfully match
    /// a token (after a resync).  So it will go:
    ///
    ///   1. error occurs
    ///   2. enter recovery mode, report error
    ///   3. consume until token found in resynch set
    ///   4. try to resume parsing
    ///   5. next match() will reset errorRecovery mode
    ///
    virtual void reportError();

    /// Get number of recognition errors (lexer, parser, tree parser).  Each
    /// recognizer tracks its own number. So parser and lexer each have
    /// separate count. Does not count the spurious errors found between
    /// an error and next valid token match
    ///
    /// \see reportError()
    ///
    std::uint32_t numberOfSyntaxErrors();

    /// Pointer to a function that recovers from an error found in the input stream.
    /// Generally, this will be a #ExceptionNoviableAlt but it could also
    /// be from a mismatched token that the match() could not recover from.
    ///
    void recover();

    /// Pointer to a function that is a hook to listen to token consumption during error recovery.
    /// This is mainly used by the debug parser to send events to the listener.
    ///
    void beginResync();

    /// Pointer to a function that is a hook to listen to token consumption during error recovery.
    /// This is mainly used by the debug parser to send events to the listener.
    ///
    void endResync();

    /// Pointer to a function that is a hook to listen to token consumption during error recovery.
    /// This is mainly used by the debug parser to send events to the listener.
    ///
    void beginBacktrack(std::uint32_t level);

    /// Pointer to a function that is a hook to listen to token consumption during error recovery.
    /// This is mainly used by the debug parser to send events to the listener.
    ///
    void endBacktrack(std::uint32_t level, bool successful);

    /// Pointer to a function to computer the error recovery set for the current rule.
    /// \see antlr3ComputeErrorRecoverySet() for details.
    ///
    Bitset computeErrorRecoverySet();

    /// Pointer to a function that computes the context-sensitive FOLLOW set for the 
    /// current rule.
    ///\see antlr3ComputeCSRuleFollow() for details.
    ///
    Bitset computeCSRuleFollow();

    /// Pointer to a function to combine follow bitsets.
    ///\see antlr3CombineFollows() for details.
    ///
    Bitset combineFollows(bool exact);
 
    /// Pointer to a function that recovers from a mismatched token in the input stream.
    ///\see antlr3RecoverMismatch() for details.
    ///
    ItemPtr recoverFromMismatchedToken(std::uint32_t ttype, Bitset const & follow);

    /// Pointer to a function that recovers from a mismatched set in the token stream, in a similar manner
    /// to recoverFromMismatchedToken
    ///
    ItemPtr recoverFromMismatchedSet(Bitset const & follow);

    /// Pointer to common routine to handle single token insertion for recovery functions.
    ///
    bool recoverFromMismatchedElement(Bitset const & follow);
    
    /// Pointer to function that consumes input until the next token matches
    /// the given token.
    ///
    void consumeUntil(std::uint32_t tokenType);

    /// Pointer to function that consumes input until the next token matches
    /// one in the given set.
    ///
    void consumeUntilSet(Bitset const & set);

    /// Pointer to a function to return whether the rule has parsed input starting at the supplied 
    /// start index before. If the rule has not parsed input starting from the supplied start index,
    /// then it will return MemoRuleUnknown. If it has parsed from the suppled start point
    /// then it will return the point where it last stopped parsing after that start point.
    ///
    Index getRuleMemoization(Index ruleIndex, Index ruleParseStart);

    /// Pointer to function that determines whether the rule has parsed input at the current index
    /// in the input stream
    ///
    bool alreadyParsedRule(Index ruleIndex);

    /// Pointer to function that records whether the rule has parsed the input at a 
    /// current position successfully or not.
    ///
    void memoize(Index ruleIndex, Index ruleParseStart);

    /// Returns the current input symbol.
    /// The is placed into any label for the associated token ref; e.g., x=ID. Token
    /// and tree parsers need to return different objects. Rather than test
    /// for input stream type or change the IntStream interface, I use
    /// a simple method to ask the recognizer to tell me what the current
    /// input symbol is.
    ///
    /// This is ignored for lexers and the lexer implementation of this
    /// function should return NULL.
    ///
    ItemPtr currentInputSymbol();

    /// Conjure up a missing token during error recovery.
    ///
    /// The recognizer attempts to recover from single missing
    /// symbols. But, actions might refer to that missing symbol.
    /// For example, x=ID {f($x);}. The action clearly assumes
    /// that there has been an identifier matched previously and that
    /// $x points at that token. If that token is missing, but
    /// the next token in the stream is what we want we assume that
    /// this token is missing and we keep going. Because we
    /// have to return some token to replace the missing token,
    /// we have to conjure one up. This method gives the user control
    /// over the tokens returned for missing tokens. Mostly,
    /// you will want to create something special for identifier
    /// tokens. For literals such as '{' and ',', the default
    /// action in the parser or tree parser works. It simply creates
    /// a CommonToken of the appropriate type. The text will be the token.
    /// If you change what tokens must be created by the lexer,
    /// override this method to create the appropriate tokens.
    ///
    virtual ItemPtr getMissingSymbol(
        ExceptionPtr e,
        std::uint32_t expectedTokenType,
        Bitset const & follow
    );

    /// Pointer to a function that returns whether the supplied grammar function
    /// will parse the current input stream or not. This is the way that syntactic
    /// predicates are evaluated. Unlike java, C is perfectly happy to invoke code
    /// via a pointer to a function (hence that's what all the ANTLR3 C interfaces
    /// do.
    ///
    bool synpred(void * ctx, void (*predicate)(void * ctx));

    /// Reset the recognizer
    ///
    virtual void reset();
protected:
    /// If set to something other than NULL, then this structure is
    /// points to an instance of the debugger interface. In general, the
    /// debugger is only referenced internally in recovery/error operations
    /// so that it does not cause overhead by having to check this pointer
    /// in every function/method
    ///
    DebugEventListenerPtr debugger_;

    /// A pointer to the shared recognizer state, such that multiple
    /// recognizers can use the same inputs streams and so on (in
    /// the case of grammar inheritance for instance.
    ///
    RecognizerSharedStatePtr const state_;

    IntStreamPtr input_;
    
    bool filteringMode_;
    
    std::uint32_t LA(std::int32_t i) {
        std::uint32_t t = input_->LA(i);
        return t;
    }

    /// Pointer to a function that is called to display a recognition error message. You may
    /// override this function independently of reportError() above as that function calls
    /// this one to do the actual exception printing.
    ///
    virtual void displayRecognitionError(Exception const *e, StringLiteral const * tokenNames);
    virtual String getErrorHeader(Exception const * e, StringLiteral const * tokenNames);
    virtual String getErrorMessage(Exception const * e, StringLiteral const * tokenNames);
    virtual void emitErrorMessage(String msg);

    virtual void fillException(Exception* ex) = 0;
    void recordException(std::unique_ptr<Exception> ex);
    void recordException(Exception* e);
    
    virtual std::uint32_t itemToInt(ItemPtr item) = 0;

    void followPush(Bitset const * data);
    void followPop();

    bool evalPredicate(bool result, const char * predicate);
    
    virtual std::ostream& traceStream();
    virtual String traceCurrentItem() = 0;
    void traceIn(StringLiteral ruleName, int ruleNo);
    void traceOut(StringLiteral ruleName, int ruleNo);
};

#endif // _ANTLR3_BASERECOGNIZER_HPP
