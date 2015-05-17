/** \file
 * This is the standard tree adaptor used by the C runtime unless the grammar
 * source file says to use anything different. It embeds a BASE_TREE to which
 * it adds its own implementation of anything that the base tree is not 
 * good for, plus a number of methods that any other adaptor type
 * needs to implement too.
 * \ingroup CommonTreePtrAdaptor
 */

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

#include <antlr3/CommonTreeAdaptor.hpp>
#include <antlr3/CommonTree.hpp>

template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeAdaptor::create(CommonTokenPtr payload) {
    return std::make_shared<CommonTree>(std::move(payload));
}
template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeAdaptor::dupNode(ItemPtr treeNode) {
    return std::static_pointer_cast<CommonTree>(treeNode)->dupNode();
}
template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeAdaptor::errorNode(
    TokenStreamPtr input,
    CommonTokenPtr start,
    CommonTokenPtr stop,
    ExceptionPtr e
) {
    return std::make_shared<CommonErrorNode>(input, start, stop, e);
}

template<class StringTraits>
bool antlr3<StringTraits>::CommonTreeAdaptor::isNil(ItemPtr t) {
    return std::static_pointer_cast<CommonTree>(t)->isNil();
}

// R e w r i t e  R u l e s

template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeAdaptor::create(std::uint32_t tokenType, CommonTokenPtr fromToken) {
    auto tok = fromToken ? std::make_shared<CommonToken>(*fromToken) : std::make_shared<CommonToken>();
    tok->setType(tokenType);
    return create(tok);
}
template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeAdaptor::create(std::uint32_t tokenType, CommonTokenPtr fromToken, String text) {
    auto tok = fromToken ? std::make_shared<CommonToken>(*fromToken) : std::make_shared<CommonToken>();
    tok->setType(tokenType);
    tok->setText(std::move(text));
    return create(tok);
}
template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeAdaptor::create(std::uint32_t tokenType, String text) {
    auto tok = std::make_shared<CommonToken>(tokenType, std::move(text));
    return create(tok);
}

// C o n t e n t

std::uint32_t CommonTreeAdaptor::getType(ItemPtr t) {
    if (!t) return TokenInvalid;
    return std::static_pointer_cast<CommonTree>(t)->type();
}
//    void setType(ItemPtr t, std::uint32_t type);
template<class StringTraits>
String antlr3<StringTraits>::CommonTreeAdaptor::getText(ItemPtr t) {
    if (!t) return String();
    return std::static_pointer_cast<CommonTree>(t)->text();
}
//    void setText(ItemPtr t, String text);
template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::CommonTreeAdaptor::getToken(ItemPtr t) {
    if (!t) return nullptr;
    return std::static_pointer_cast<CommonTree>(t)->token();
}
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeAdaptor::setTokenBoundaries(ItemPtr t, CommonTokenPtr startToken, CommonTokenPtr stopToken) {
    if (!t) return;
    auto tx = std::static_pointer_cast<CommonTree>(t);
    if (tx->hasTokenBoundaries()) return;
    Index startIndex = startToken ? startToken->tokenIndex() : NullIndex;
    Index stopIndex = stopToken ? stopToken->tokenIndex() : NullIndex;
    tx->setTokenStartIndex(startIndex);
    tx->setTokenStopIndex(stopIndex);
    if (stopIndex < startIndex) {
        auto n = tx->childCount();
        for (std::uint32_t i = 0; i < n; ++i) {
            auto c = tx->getChild(i);
            if (c && c->tokenStartIndex() == NullIndex && c->tokenStopIndex() == NullIndex) {
                setTokenBoundaries(c, startToken, stopToken);
            }
        }
    }
}

template<class StringTraits>
Index antlr3<StringTraits>::CommonTreeAdaptor::getTokenStartIndex(ItemPtr t) {
    if (!t) return NullIndex;
    return std::static_pointer_cast<CommonTree>(t)->tokenStartIndex();
}

template<class StringTraits>
Index antlr3<StringTraits>::CommonTreeAdaptor::getTokenStopIndex(ItemPtr t) {
    if (!t) return NullIndex;
    return std::static_pointer_cast<CommonTree>(t)->tokenStopIndex();
}

// N a v i g a t i o n  /  T r e e  P a r s i n g

template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeAdaptor::getParent(ItemPtr child) {
    if (!child) return nullptr;
    return std::static_pointer_cast<CommonTree>(child)->parent();
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTreeAdaptor::setParent(ItemPtr child, ItemPtr parent) {
    if (!child) return;
    std::static_pointer_cast<CommonTree>(child)->setParent(
        std::static_pointer_cast<CommonTree>(parent)
    );
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTreeAdaptor::setChildIndex(ItemPtr child, std::int32_t i) {
    if (!child) return;
    std::static_pointer_cast<CommonTree>(child)->setChildIndex(i);
}

std::int32_t CommonTreeAdaptor::getChildIndex(ItemPtr child) {
    if (!child) return 0;
    return std::static_pointer_cast<CommonTree>(child)->childIndex();
}

template<class StringTraits>
Location antlr3<StringTraits>::CommonTreeAdaptor::getLocation(ItemPtr t) {
    if (!t) return Location();
    return std::static_pointer_cast<CommonTree>(t)->location();
}

template<class StringTraits>
String antlr3<StringTraits>::CommonTreeAdaptor::toString(ItemPtr t, StringLiteral const * tokenNames) {
    if (!t) return String();
    return std::static_pointer_cast<CommonTree>(t)->toString(tokenNames);
}

