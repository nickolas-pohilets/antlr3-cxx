#ifndef _ANTLR3_BASE_TREE_HPP
#define _ANTLR3_BASE_TREE_HPP

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

#include <antlr3/IntStream.hpp>
#include <antlr3/String.hpp>
#include <antlr3/Location.hpp>

/// A generic tree implementation with no payload.  You must subclass to
/// actually have any user data.  ANTLR v3 uses a list of children approach
/// instead of the child-sibling approach in v2.  A flat tree (a list) is
/// an empty node whose children represent the list.  An empty (as in it does not
/// have payload itself), but non-null node is called ANTLR3_T("nil").
///
template<class ChildT>
class antlr3_defs::BaseTree : public std::enable_shared_from_this<ChildT>
{
protected:
    typedef std::shared_ptr<ChildT> ChildPtr;
    std::vector<ChildPtr> children_;
public:
    BaseTree() {}
    
    /// Copy contructor does nothing for BaseTree
    /// as there are no fields other than the children list, which cannot
    /// be copied as the children are not considered part of this node.
    ///
    BaseTree(BaseTree const & other) {}
    
    /// Use cases for move constructor are not clear for now,
    /// so it is disabled until it will be needed.
    BaseTree(BaseTree && other) = delete;
    
    virtual ~BaseTree() {}
    
    
    ChildPtr getChild(std::uint32_t i) {
        if (i >= children_.size()) {
            return nullptr;
        }
        return children_[i];
    }
    
    // getFirstChildWithType - TBD
    
    std::uint32_t childCount() {
        return (std::uint32_t)children_.size();
    }
    
    void addChild(ChildPtr child) {
        if (child == nullptr) {
            return;
        }
        
        if (child.get() == this) {
            assert(false && "Attempt to add child list to itself");
        }
        
        auto sharedThis = this->shared_from_this();
        if (child->isNil()) {
            for (auto const & grandChild : child->children_) {
                children_.push_back(grandChild);
                grandChild->setParent(sharedThis);
                grandChild->setChildIndex((std::uint32_t)children_.size() - 1);
            }
            child->children_.clear();
        } else {
            children_.push_back(child);
            child->setParent(sharedThis);
            child->setChildIndex((std::uint32_t)children_.size() - 1);
        }
    }
    
    void addChildren(std::vector<ChildPtr> const & kids) {
        for (auto const & child : kids) {
            addChild(child);
        }
    }
    
    void setChild(std::uint32_t i, ChildPtr child) {
        if (!child) {
            return;
        }
        
        if (child->isNil()) {
            assert(false && "Can't set single child to a list");
        }
        
        /// @todo What if index is out of bounds?
        children_[i] = child;
        child->setParent(this->shared_from_this());
        child->setChildIndex((std::uint32_t)children_.size() - 1);
    }
    
    void insertChild(std::int32_t i, ChildPtr child) {
        children_.insert(children_.begin() + i, child);
        child->setParent(this->shared_from_this());
        freshenParentAndChildIndexes(i);
    }
    
    ChildPtr deleteChild(std::int32_t i) {
        ChildPtr retVal = children_[i];
        children_.erase(children_.begin() + i);
        freshenParentAndChildIndexes(i);
        return std::move(retVal);
    }
    
    void replaceChildren(std::uint32_t startChildIndex, std::uint32_t stopChildIndex, ChildPtr t) {
        assert(startChildIndex <= stopChildIndex + 1 && "Child indices are invalid");
        assert(stopChildIndex < children_.size() && "Child indices are invalid");
        std::vector<ChildPtr> newChildren = t->isNil() ? std::move(t->children_) : std::vector<ChildPtr>{ t };
        t->children_.clear();
        size_t replacingHowMany = stopChildIndex - startChildIndex + 1;
        size_t replacingWithHowMany = newChildren.size();
        auto sharedThis = this->shared_from_this();
        if (replacingHowMany > replacingWithHowMany) {
            auto ib = children_.begin() + startChildIndex + replacingWithHowMany;
            auto ie = children_.begin() + startChildIndex + replacingHowMany;
            children_.erase(ib, ie);
            replacingHowMany = replacingWithHowMany;
        } else if(replacingHowMany < replacingWithHowMany) {
            auto pos = children_.begin() + replacingHowMany;
            auto ib = newChildren.begin() + replacingHowMany;
            auto ie = newChildren.end();
            children_.insert(pos, ib, ie);
        }
        
        std::copy(
            newChildren.begin(),
            newChildren.begin() + replacingHowMany,
            children_.begin() + startChildIndex
        );
        for (auto const & child : newChildren) {
            child->setParent(sharedThis);
        }
        freshenParentAndChildIndexes(startChildIndex);
    }
protected:
    void freshenParentAndChildIndexes(std::uint32_t index) {
        size_t n = children_.size();
        for (; index < n; ++index) {
            children_[index]->setChildIndex(index);
        }
    }
//    /// A pointer to a function that returns the common token pointer
//    /// for the payload in the supplied tree.
//    ///
//    virtual CommonTokenPtr token() = 0;
//
//    virtual BaseTreePtr dupNode() = 0;
//
//    BaseTreePtr dupTree();
//
//    virtual Location location();
//
//
//    virtual void setChildIndex(std::int32_t) = 0;
//
//    virtual std::int32_t childIndex() = 0;
//
//    virtual BaseTree * parent() = 0;
//
//    virtual void setParent(BaseTree * parent) = 0;
//
//    virtual std::uint32_t type() = 0;
//
//    BaseTreePtr getFirstChildWithType(std::uint32_t type);
//
//    virtual String text() = 0;
//
//    virtual bool isNilNode() = 0;
//
//
//    String toStringTree();
//
//    virtual String toString(StringLiteral const * tokenNames) = 0;
//
//    void freshenPACIndexesAll();
//
//    void freshenPACIndexes(std::uint32_t offset);
//
//    virtual std::uint32_t toInt() const override
//    {
//        return const_cast<BaseTree*>(this)->type();
//    }
};

#endif
