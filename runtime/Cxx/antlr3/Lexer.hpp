/** \file 
 * Base interface for any ANTLR3 lexer.
 *
 * An ANLTR3 lexer builds from two sets of components:
 *
 *  - The runtime components that provide common functionality such as
 *    traversing character streams, building tokens for output and so on.
 *  - The generated rules and struutre of the actual lexer, which call upon the
 *    runtime components.
 *
 * A lexer class contains  a character input stream, a base recognizer interface 
 * (which it will normally implement) and a token source interface (which it also
 * implements. The Tokensource interface is called by a token consumer (such as
 * a parser, but in theory it can be anything that wants a set of abstract
 * tokens in place of a raw character stream.
 *
 * So then, we set up a lexer in a sequence akin to:
 *
 *  - Create a character stream (something which implements InputStream)
 *    and initialize it.
 *  - Create a lexer interface and tell it where it its input stream is.
 *    This will cause the creation of a base recognizer class, which it will 
 *    override with its own implementations of some methods. The lexer creator
 *    can also then in turn override anything it likes. 
 *  - The lexer token source interface is then passed to some interface that
 *    knows how to use it, byte calling for a next token. 
 *  - When a next token is called, let ze lexing begin.
 *
 */
#ifndef	_ANTLR3_LEXER
#define	_ANTLR3_LEXER

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

/* Definitions
 */

#include <antlr3/Defs.hpp>
#include <antlr3/CharStream.hpp>
#include <antlr3/CommonToken.hpp>
#include <antlr3/TokenStream.hpp>
#include <antlr3/BaseRecognizer.hpp>

namespace antlr3 {

class Lexer : public BaseRecognizer, public TokenSource
{
    friend class BaseRecognizer;
public:
    Lexer(RecognizerSharedStatePtr state);
    Lexer(CharStreamPtr input, RecognizerSharedStatePtr state);
    virtual ~Lexer();

    virtual void reset() override;
    virtual void reportError() override;

    virtual CommonTokenPtr nextToken() override;
    virtual LocationSourcePtr source() override;

    /// Returns current lexer char stream
    CharStreamPtr charStream();

    /// Sets the charstream source for the lexer and causes it to  be reset.
    void setCharStream(CharStreamPtr input);
    
    /** Pointer to a function that switches the current character input stream to 
     *  a new one, saving the old one, which we will revert to at the end of this 
     *  new one.
     */
    void			pushCharStream(CharStreamPtr input);

    /** Pointer to a function that abandons the current input stream, whether it
     *  is empty or not and reverts to the previous stacked input stream.
     */
    void			popCharStream();

    /** Pointer to a function that emits the supplied token as the next token in
     *  the stream.
     */
    void			emitNew(CommonTokenPtr token);

    /** Pointer to a function that constructs a new token from the lexer stored information 
     */
    CommonTokenPtr	emit();

    /** Pointer to the user provided (either manually or through code generation
     *  function that causes the lexer rules to run the lexing rules and produce 
     *  the next token if there iss one. This is called from nextToken() in the
     *  pANTLR3_TOKEN_SOURCE. Note that the input parameter for this funciton is 
     *  the generated lexer context (stored in ctx in this interface) it is a generated
     *  function and expects the context to be the generated lexer. 
     */
    virtual void mTokens() = 0;

    /// Attempts to match and consume the specified string from the input stream.
    bool matchs(char const * string, size_t len);
    bool matchs(char16_t const * string, size_t len);
    bool matchs(char32_t const * string, size_t len);
    
    template<class CharT, size_t N>
    bool matchs(CharT const (&string)[N]) {
        static_assert(N > 0, "Null-terminated literal cannot be empty");
        assert(!string[N - 1]);
        return matchs(string, N - 1);
    }

    /** Pointer to a function that matches and consumes the specified character from the input stream.
     *  The input stream is required to provide characters via LA() as UTF32 characters. The default lexer
     *  implementation is source encoding agnostic and so input streams do not generally need to 
     *  override the default implmentation.
     */
    bool	matchc(std::uint32_t c);

    /** Pointer to a function that matches any character in the supplied range (I suppose it could be a token range too
     *  but this would only be useful if the tokens were in tsome guaranteed order which is
     *  only going to happen with a hand crafted token set).
     */
    bool	matchRange(std::uint32_t low, std::uint32_t high);

    /** Pointer to a function that matches the next token/char in the input stream
     *  regardless of what it actaully is.
     */
    void		matchAny();

    /** Pointer to a function that recovers from an error found in the input stream.
     *  Generally, this will be a #ExceptionNoviableAlt but it could also
     *  be from a mismatched token that the match() could not recover from.
     */
    void		recover();

    Index charIndex();

    /// Return the text so far for the current token being generated
    String	text();
protected:
    /// Set the complete text of this token; it wipes any previous changes to the text.
    void setText(String s);
    virtual void fillException(Exception* e) override;
    virtual std::uint32_t itemToInt(ItemPtr item) override;
    virtual String getErrorMessage(Exception const * e, ConstString const * tokenNames) override;
    virtual String traceCurrentItem() override;
private:
    CommonTokenPtr nextTokenStr();
    CommonTokenPtr nextTokenNormal();
    CommonTokenPtr nextTokenFiltering();
    
    template<class T>
    bool matchStr(T const * string, size_t len);
};

} // namespace antlr3

#endif
