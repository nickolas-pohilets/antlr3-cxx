/** \file
 * Definition of the ANTLR3 common tree adaptor.
 */

#ifndef _ANTLR3_COMMON_TREE_ADAPTOR_HPP
#define _ANTLR3_COMMON_TREE_ADAPTOR_HPP

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
#include <antlr3/String.hpp>
#include <antlr3/BaseTreeAdaptor.hpp>
#include <antlr3/CommonTree.hpp>
#include <antlr3/DebugEventListener.hpp>

namespace antlr3 {

class CommonTreeAdaptor : public BaseTreeAdaptor<CommonTree>
{
public:
    CommonTreeAdaptor();
    virtual ~CommonTreeAdaptor();
    
    virtual ItemPtr create(CommonTokenPtr payload) override;
    virtual ItemPtr dupNode(ItemPtr treeNode) override;
    virtual ItemPtr errorNode(TokenStreamPtr input, CommonTokenPtr start, CommonTokenPtr stop, ExceptionPtr e) override;
    
    virtual bool isNil(ItemPtr t) override;
    
    // R e w r i t e  R u l e s

    virtual ItemPtr create(std::uint32_t tokenType, CommonTokenPtr fromToken) override;
    virtual ItemPtr create(std::uint32_t tokenType, CommonTokenPtr fromToken, String text) override;
    virtual ItemPtr create(std::uint32_t tokenType, String text) override;
    
    // C o n t e n t
    
    virtual std::uint32_t getType(ItemPtr t) override;
//    virtual void setType(ItemPtr t, std::uint32_t type) override;
    virtual String getText(ItemPtr t) override;
//    virtual void setText(ItemPtr t, String text) override;
    virtual CommonTokenPtr getToken(ItemPtr t) override;
    virtual void setTokenBoundaries(ItemPtr t, CommonTokenPtr startToken, CommonTokenPtr stopToken) override;
    
    virtual Index getTokenStartIndex(ItemPtr t) override;
    virtual Index getTokenStopIndex(ItemPtr t) override;

    // N a v i g a t i o n  /  T r e e  P a r s i n g
    
    virtual ItemPtr getParent(ItemPtr child) override;
    virtual void setParent(ItemPtr child, ItemPtr parent) override;
    
    virtual void setChildIndex(ItemPtr child, std::int32_t i) override;
    virtual std::int32_t getChildIndex(ItemPtr child) override;
    
    virtual Location getLocation(ItemPtr t) override;
    
    virtual String toString(ItemPtr t, ConstString const * tokenNames) override;
};

} // namespace antlr3

#endif
