// \file
//
// Implementation of ANTLR3 CommonTree, which you can use as a
// starting point for your own tree. Though it is often easier just to tag things on
// to the user pointer in the tree unless you are building a different type
// of structure.
//

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

#include <antlr3/CommonTree.hpp>

template<class StringTraits>
antlr3<StringTraits>::CommonTree::CommonTree(CommonTokenPtr aToken)
    : BaseTree<CommonTree>()
    , startIndex_(NullIndex)
    , stopIndex_(NullIndex)
    , token_(std::move(aToken))
    , parent_()
    , childIndex_(-1)
{
}

template<class StringTraits>
typename antlr3<StringTraits>::CommonTokenPtr
    antlr3<StringTraits>::CommonTree::token()
{
    return token_;
}

template<class StringTraits>
bool antlr3<StringTraits>::CommonTree::hasTokenBoundaries() const {
    return startIndex_ != NullIndex || stopIndex_ != NullIndex;
}

template<class StringTraits>
typename antlr3<StringTraits>::Index
    antlr3<StringTraits>::CommonTree::tokenStartIndex()
{
    if (startIndex_ == NullIndex)
    {
        Index startIndex = NullIndex;
        for (auto c : this->children_) {
            Index i = c->tokenStartIndex();
            if (i != NullIndex && (startIndex == NullIndex || i < startIndex)) {
                startIndex = i;
            }
        }
        if (startIndex != NullIndex) {
            return startIndex;
        }
        if (token_ != nullptr) {
            return token_->tokenIndex();
        }
    }
    return startIndex_;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTree::setTokenStartIndex(Index index)
{
    startIndex_ = index;
}

template<class StringTraits>
typename antlr3<StringTraits>::Index
    antlr3<StringTraits>::CommonTree::tokenStopIndex()
{
    if (stopIndex_ == NullIndex)
    {
        Index stopIndex = NullIndex;
        for (auto c : this->children_) {
            Index i = c->tokenStopIndex();
            if (i != NullIndex && (stopIndex == NullIndex || i > stopIndex)) {
                stopIndex = i;
            }
        }
        if (stopIndex != NullIndex) {
            return stopIndex;
        }
        if (token_ != nullptr) {
            return token_->tokenIndex();
        }
    }
    return stopIndex_;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTree::setTokenStopIndex(Index index)
{
    stopIndex_ = index;
}

template<class StringTraits>
typename antlr3<StringTraits>::CommonTreePtr
    antlr3<StringTraits>::CommonTree::dupNode()
{
    return std::make_shared<CommonTree>(*this);
}

template<class StringTraits>
bool antlr3<StringTraits>::CommonTree::isNil()
{
    return !token_;
}

template<class StringTraits>
std::uint32_t antlr3<StringTraits>::CommonTree::type()
{
    return token_ ? token_->type() : 0;
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::CommonTree::text()
{
    if (!token_) {
        return String();
    }
    return token_->text();
}

template<class StringTraits>
typename antlr3<StringTraits>::Location
    antlr3<StringTraits>::CommonTree::location()
{
    if (token_ != NULL)
    {
        auto loc = token_->startLocation();
        if (loc.isValid()) {
            return loc;
        }
    }
    
    if (this->childCount() > 0)
    {
        return this->getChild(0)->location();
    }
    
    return Location();
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::CommonTree::toString(StringLiteral const * tokenNames)
{
    if (isNil())
    {
        return ANTLR3_T("nil");
    }

    return token_->toString(tokenNames);
}

template<class StringTraits>
typename antlr3<StringTraits>::CommonTreePtr
    antlr3<StringTraits>::CommonTree::parent()
{
    return parent_.lock();
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTree::setParent(CommonTreePtr aParent)
{
    parent_ = aParent;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTree::setChildIndex(std::int32_t i)
{
    childIndex_ = i;
}

template<class StringTraits>
std::int32_t antlr3<StringTraits>::CommonTree::childIndex()
{
    return childIndex_;
}
