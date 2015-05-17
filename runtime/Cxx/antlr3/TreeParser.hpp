#ifndef	ANTLR3TREEPARSER_H
#define	ANTLR3TREEPARSER_H

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
#include <antlr3/BaseRecognizer.hpp>
#include <antlr3/CommonTreeNodeStream.hpp>

/** Internal structure representing an element in a hash bucket.
 *  Stores the original key so that duplicate keys can be rejected
 *  if necessary, and contains function can be supported If the hash key
 *  could be unique I would have invented the perfect compression algorithm ;-)
 */
template<class StringTraits>
class antlr3<StringTraits>::TreeParser : public BaseRecognizer
{
    friend class BaseRecognizer;
protected:
    virtual void fillException(Exception* e) override;
    virtual String getErrorMessage(Exception const * e, StringLiteral const * tokenNames) override;
public:
    TreeParser(CommonTreeNodeStreamPtr ctnstream, RecognizerSharedStatePtr state);
    ~TreeParser();

    virtual ItemPtr getMissingSymbol(
        ExceptionPtr e,
        std::uint32_t expectedTokenType,
        Bitset const & follow
    ) override;


    /// Return a pointer to the input stream.
    CommonTreeNodeStreamPtr treeNodeStream();

    /// Set the input stream and reset the parser
    void setTreeNodeStream(CommonTreeNodeStreamPtr input);
protected:
    antlr3::TreeAdaptorPtr adaptor_;
    
    ItemPtr LT(std::int32_t index) {
        return treeNodeStream()->LT(index);
    }
    
    virtual std::uint32_t itemToInt(ItemPtr item) override;
    virtual String traceCurrentItem() override;
};

#endif
