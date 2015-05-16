/** Interface for an ANTLR3 common tree which is what gets
 *  passed around by the AST producing parser.
 */

#ifndef _ANTLR3_COMMON_TREE_HPP
#define _ANTLR3_COMMON_TREE_HPP

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

#include <antlr3/BaseTree.hpp>
#include <antlr3/CommonToken.hpp>

namespace antlr3 {

class CommonTree : public BaseTree<CommonTree>
{
private:
    /// Start token index that encases this tree
    ///
    Index startIndex_;

    /// End token that encases this tree
    ///
    Index stopIndex_;

    /// A single token, this is the payload for the tree
    ///
    CommonTokenPtr token_;

    /// Points to the node that has this node as a child.
    /// If this is NULL, then this is the root node.
    ///
    CommonTreeWeakPtr parent_;

    /// What index is this particular node in the child list it
    /// belongs to?
    ///
    std::int32_t childIndex_;
public:
    /// An encapsulated BASE TREE structure (NOT a pointer)
    /// that performs a lot of the dirty work of node management
    /// To this we add just a few functions that are specific to the 
    /// payload. You can further abstract common tree so long
    /// as you always have a baseTree pointer in the top structure
    /// and copy it from the next one down. 
    /// So, lets say we have a structure JIMS_TREE. 
    /// It needs an BaseTree that will support all the
    /// general tree duplication stuff.
    /// It needs a CommonTree structure embedded or completely
    /// provides the equivalent interface.
    /// It provides it's own methods and data.
    /// To create a new one of these, the function provided to
    /// the tree adaptor (see comments there) should allocate the
    /// memory for a new JIMS_TREE structure, then call
    /// antlr3InitCommonTree(<addressofembeddedCOMMON_TREE>)
    /// antlr3BaseTreeNew(<addressofBASETREE>)
    /// The interfaces for BASE_TREE and COMMON_TREE will then
    /// be initialized. You then call and you can override them or just init
    /// JIMS_TREE (note that the base tree in common tree will be ignored)
    /// just the top level base tree is used). Codegen will take care of the rest.
    ///

    CommonTree(CommonTokenPtr token);

    CommonTokenPtr token();
    
    bool hasTokenBoundaries() const;

	Index tokenStartIndex();
    void setTokenStartIndex(Index index);
    Index tokenStopIndex();
    void setTokenStopIndex(Index index);

    virtual std::uint32_t type();
    Location location();
    virtual String text();

    CommonTreePtr parent();
    void setParent(CommonTreePtr parent);

    std::int32_t childIndex();
    void setChildIndex(std::int32_t);

    CommonTreePtr dupNode();
    virtual bool isNil();
    
    String toString() { return toString(nullptr); }
    virtual String toString(ConstString const * tokenNames);
};

/// @todo This class inherits some unused fields from CommonTree.
class CommonErrorNode : public CommonTree
{
public:
    CommonErrorNode(TokenStreamPtr input, CommonTokenPtr start, CommonTokenPtr stop, ExceptionPtr e)
        : CommonTree(nullptr)
        , input_(input)
        , start_(start)
        , stop_(stop)
        , trappedException_(e)
    {
        if (!stop || (stop->tokenIndex() < start->tokenIndex() && stop->type() == TokenEof)) {
            // sometimes resync does not consume a token (when LT(1) is
			// in follow set.  So, stop will be 1 to left to start. adjust.
			// Also handle case where start is the first token and no token
			// is consumed during recovery; LT(-1) will return null.
            stop_ = start;
        }
    }
    
    virtual bool isNil() override {
        return false;
    }
    
    virtual std::uint32_t type() override {
        return TokenInvalid;
    }
    
    virtual String text() override {
        /// @todo
        return String();
    }
    
    virtual String toString(ConstString const * tokenNames) override {
        /// @todo
        return String();
    }
private:
    TokenStreamPtr input_;
    CommonTokenPtr start_;
    CommonTokenPtr stop_;
    ExceptionPtr trappedException_;
    
};

} // namespace antlr3

#endif


