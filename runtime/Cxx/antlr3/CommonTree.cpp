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

namespace antlr3 {

CommonTree::CommonTree(CommonTokenPtr aToken)
    : BaseTree()
    , startIndex_(NullIndex)
    , stopIndex_(NullIndex)
    , token_(std::move(aToken))
    , parent_()
    , childIndex_(-1)
{
}

CommonTokenPtr CommonTree::token()
{
    return token_;
}

bool CommonTree::hasTokenBoundaries() const {
    return startIndex_ != NullIndex || stopIndex_ != NullIndex;
}

Index CommonTree::tokenStartIndex()
{
    if (startIndex_ == NullIndex)
    {
        Index startIndex = NullIndex;
        for (auto c : children_) {
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

void CommonTree::setTokenStartIndex(Index index)
{
    startIndex_ = index;
}

Index CommonTree::tokenStopIndex()
{
    if (stopIndex_ == NullIndex)
    {
        Index stopIndex = NullIndex;
        for (auto c : children_) {
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

void CommonTree::setTokenStopIndex(Index index)
{
    stopIndex_ = index;
}

CommonTreePtr CommonTree::dupNode()
{
    return std::make_shared<CommonTree>(*this);
}

bool CommonTree::isNil()
{
    return !token_;
}

std::uint32_t CommonTree::type()
{
    return token_ ? token_->type() : 0;
}

String CommonTree::text()
{
    if (!token_) {
        return String();
    }
    return token_->text();
}

Location CommonTree::location()
{
    if (token_ != NULL)
    {
        auto loc = token_->startLocation();
        if (loc.isValid()) {
            return loc;
        }
    }
    
    if  (childCount() > 0)
    {
        return getChild(0)->location();
    }
    
    return Location();
}

String CommonTree::toString(ConstString const * tokenNames)
{
    if (isNil())
    {
        return ANTLR3_T("nil");
    }

    return token_->toString(tokenNames);
}

CommonTreePtr CommonTree::parent()
{
    return parent_.lock();
}

void CommonTree::setParent(CommonTreePtr aParent)
{
    parent_ = aParent;
}

void CommonTree::setChildIndex(std::int32_t i)
{
    childIndex_ = i;
}

std::int32_t CommonTree::childIndex()
{
    return childIndex_;
}

} // namespace antlr3
