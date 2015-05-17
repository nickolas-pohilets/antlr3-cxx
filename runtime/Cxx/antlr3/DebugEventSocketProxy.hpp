#ifndef	DebugEventSocketProxy_H
#define	DebugEventSocketProxy_H

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

#include <antlr3/DebugEventListener.hpp>

/// A proxy debug event listener that forwards events over a socket to a debugger (or any other listener) using a simple text-based protocol; one event per line.
class DebugEventSocketProxy : public DebugEventListener
{
    /// The port number which the debug listener should listen on for a connection
    ///
    std::uint32_t port_;

    /// Opaque platform-dependent socket handler
    ///
    void * socket_;

    /// The name of the grammar file that we are debugging
    ///
    String grammarFileName_;

    /// Indicates whether we have already connected or not
    ///
    bool initialized_;

    /// Used to serialize the values of any particular token we need to
    /// send back to the debugger.
    ///
    std::string tokenString_;

    /// Allows the debug event system to access the adapter in use
    /// by the recognizer, if this is a tree parser of some sort.
    ///
    TreeAdaptorPtr adaptor_;

    bool sockSend(char const * ptr, int len);
    void transmit(char const * ptr);
    std::string serializeToken(CommonTokenPtr t);
    std::string serializeNode(ItemPtr node);
public:
    /// Default debugging port
    static std::uint32_t const DEFAULT_DEBUGGER_PORT = 0xBFCC;

    /// Create and initialize a new debug event listener that can be connected to
    /// by ANTLRWorks and any other debugger via a socket.
    ///
    DebugEventSocketProxy(TreeAdaptorPtr adaptor);
    DebugEventSocketProxy(std::uint32_t port, TreeAdaptorPtr adaptor);
    ~DebugEventSocketProxy();

    String const & grammarFileName() const;
    void setGrammarFileName(String grammarFileName_);


    /// Wait for a connection from the debugger and initiate the
    /// debugging session.
    ///
    bool handshake();
    void ack();

    virtual void enterRule(const char * grammarFileName, const char * ruleName) override;
    virtual void enterAlt(int alt) override;
    virtual void exitRule(const char * grammarFileName, const char * ruleName) override;
    virtual void enterSubRule(int decisionNumber) override;
    virtual void exitSubRule(int decisionNumber) override;
    virtual void enterDecision(int decisionNumber) override;
    virtual void exitDecision(int decisionNumber) override;
    virtual void consumeToken(CommonTokenPtr t) override;
    virtual void consumeHiddenToken(CommonTokenPtr t) override;
    virtual void LT(int i, CommonTokenPtr t) override;
    virtual void mark(int marker) override;
    virtual void rewind(int marker) override;
    virtual void beginBacktrack(int level) override;
    virtual void endBacktrack(int level, bool successful) override;
    virtual void location(int line, int pos) override;
    virtual void recognitionException(ExceptionPtr e) override;
    virtual void beginResync() override;
    virtual void endResync() override;
    virtual void semanticPredicate(bool result, const char * predicate) override;
    virtual void commence() override;
    virtual void terminate() override;

    // T r e e  P a r s i n g

    virtual void consumeNode(ItemPtr t) override;
    virtual void LTT(int i, ItemPtr t) override;

    // A S T  E v e n t s

    virtual void nilNode(ItemPtr t) override;
    virtual void errorNode(ItemPtr t) override;
    virtual void createNode(ItemPtr t) override;
    virtual void createNodeTok(ItemPtr node, CommonTokenPtr token) override;
    virtual void becomeRoot(ItemPtr newRoot, ItemPtr oldRoot) override;
    virtual void addChild(ItemPtr root, ItemPtr child) override;
    virtual void setTokenBoundaries(ItemPtr t, Index tokenStartIndex, Index tokenStopIndex) override;
};

#endif // DebugEventSocketProxy_H
