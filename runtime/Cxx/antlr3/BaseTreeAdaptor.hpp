/** \file
 * Definition of the ANTLR3 base tree adaptor.
 */

#ifndef _ANTLR3_BASE_TREE_ADAPTOR_HPP
#define _ANTLR3_BASE_TREE_ADAPTOR_HPP

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

#include <antlr3/TreeAdaptor.hpp>
#include <antlr3/BaseTree.hpp>
#include <antlr3/CommonToken.hpp>

namespace antlr3 {

template<class ChildT>
class BaseTreeAdaptor : public TreeAdaptor
{
public:
    BaseTreeAdaptor() {}
    virtual ~BaseTreeAdaptor() {}
    
    virtual ItemPtr create(CommonTokenPtr payload) = 0;
    virtual ItemPtr dupNode(ItemPtr treeNode) = 0;
    virtual ItemPtr dupTree(ItemPtr tree) override {
        if (!tree) {
			return nullptr;
		}
		ItemPtr newTree = dupNode(tree);
        auto nc = getChildCount(tree);
		for (std::int32_t i = 0; i < nc; i++) {
			ItemPtr child = getChild(tree, i);
			ItemPtr newSubTree = dupTree(child);
			addChild(newTree, newSubTree);
		}
		return newTree;
    }
    
    virtual ItemPtr nil() override {
        return create(nullptr);
    }
    
    virtual ItemPtr errorNode(
        TokenStreamPtr input,
        CommonTokenPtr start,
        CommonTokenPtr stop,
        ExceptionPtr e
    ) = 0;
    
    virtual void addChild(ItemPtr t, ItemPtr child) override {
        if (t && child) {
            std::static_pointer_cast<ChildT>(t)->addChild(
                std::static_pointer_cast<ChildT>(child)
            );
        }
    }
    
    virtual ItemPtr becomeRoot(ItemPtr newRoot, ItemPtr oldRoot) override {
		if (!oldRoot) {
			return newRoot;
		}
		// handle ^(nil real-node)
        if (isNil(newRoot)) {
            int nc = getChildCount(newRoot);
            if ( nc==1 ) {
                newRoot = getChild(newRoot, 0);
            } else if ( nc >1 ) {
				assert(false && "more than one node as root");
			}
        }
        assert(!isNil(newRoot));
        
		// add oldRoot to newRoot; addChild takes care of case where oldRoot
		// is a flat list (i.e., nil-rooted tree).  All children of oldRoot
		// are added to newRoot.
        addChild(newRoot, oldRoot);
		return newRoot;
    }
    
    virtual ItemPtr rulePostProcessing(ItemPtr root) override {
		if (root && isNil(root)) {
            auto nc = getChildCount(root);
            if (nc == 0) {
                return nullptr;
            }
			if (nc == 1) {
                root = getChild(root, 0);
                setParent(root, nullptr);
                setChildIndex(root, -1);
			}
		}
		return root;
    }
    
    virtual std::uint32_t getUniqueID(ItemPtr item) override {
        return (std::uint32_t)reinterpret_cast<size_t>(item.get());
    }
    
    // R e w r i t e  R u l e s

    virtual ItemPtr becomeRoot(CommonTokenPtr newRoot, ItemPtr oldRoot) override {
        return becomeRoot(create(newRoot), std::move(oldRoot));
    }
    
    virtual ItemPtr create(std::uint32_t tokenType, CommonTokenPtr fromToken) = 0;
    virtual ItemPtr create(std::uint32_t tokenType, CommonTokenPtr fromToken, String text) = 0;
    virtual ItemPtr create(std::uint32_t tokenType, String text) = 0;
    
    // C o n t e n t
    
    virtual std::uint32_t getType(ItemPtr t) = 0;
//    virtual void setType(ItemPtr t, std::uint32_t type) = 0;
    virtual String getText(ItemPtr t) = 0;
//    virtual void setText(ItemPtr t, String text) = 0;
    virtual CommonTokenPtr getToken(ItemPtr) = 0;
    virtual void setTokenBoundaries(ItemPtr t, CommonTokenPtr startToken, CommonTokenPtr stopToken) = 0;
    virtual Index getTokenStartIndex(ItemPtr t) = 0;
    virtual Index getTokenStopIndex(ItemPtr t) = 0;

    // N a v i g a t i o n  /  T r e e  P a r s i n g
    
    virtual ItemPtr getChild(ItemPtr t, std::uint32_t i) override {
        return std::static_pointer_cast<BaseTree<ChildT>>(t)->getChild(i);
    }
    
    virtual void setChild(ItemPtr t, std::uint32_t i, ItemPtr child) override {
        std::static_pointer_cast<BaseTree<ChildT>>(t)->setChild(
            i,
            std::static_pointer_cast<ChildT>(child)
        );
    }
    
    virtual void deleteChild(ItemPtr t, std::uint32_t i) override {
        std::static_pointer_cast<BaseTree<ChildT>>(t)->deleteChild(i);
    }
    
    virtual std::uint32_t getChildCount(ItemPtr t) override {
        return std::static_pointer_cast<BaseTree<ChildT>>(t)->childCount();
    }
    
    virtual ItemPtr getParent(ItemPtr child) = 0;
    virtual void setParent(ItemPtr child, ItemPtr parent) = 0;
    virtual void setChildIndex(ItemPtr, std::int32_t i) = 0;
    virtual std::int32_t getChildIndex(ItemPtr) = 0;
    
    virtual void replaceChildren(
        ItemPtr parent,
        std::int32_t startChildIndex,
        std::int32_t stopChildIndex,
        ItemPtr t
    ) override {
        std::static_pointer_cast<BaseTree<ChildT>>(parent)->replaceChildren(
            startChildIndex,
            stopChildIndex,
            std::static_pointer_cast<ChildT>(t)
        );
    }
};

} // namespace antlr3

#endif
