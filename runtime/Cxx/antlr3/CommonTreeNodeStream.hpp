/// \file
/// Definition of the ANTLR3 common tree node stream.
///

#ifndef _ANTLR3_COMMON_TREE_NODE_STREAM__HPP
#define _ANTLR3_COMMON_TREE_NODE_STREAM__HPP

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
#include <antlr3/CommonTreeAdaptor.hpp>
#include <antlr3/CommonTree.hpp>
#include <antlr3/IntStream.hpp>
#include <antlr3/String.hpp>
#include <stack>

namespace antlr3 {

/// Token buffer initial size settings ( will auto increase)
///
#define DEFAULT_INITIAL_BUFFER_SIZE  100
#define INITIAL_CALL_STACK_SIZE   10

class TreeNodeStream : public IntStream
{
public:
    virtual ItemPtr LI(std::int32_t i) override { return LT(i); }

    /// Get tree node at current input pointer + i ahead where i=1 is next node.
    /// i<0 indicates nodes in the past. So LT(-1) is previous node, but
    /// implementations are not required to provide results for k < -1.
    /// LT(0) is undefined. For i>=n, return null.
    /// Return NULL for LT(0) and any index that results in an absolute address
    /// that is negative (beyond the start of the list).
    ///
    /// This is analogous to the LT() method of the TokenStream, but this
    /// returns a tree node instead of a token. Makes code gen identical
    /// for both parser and tree grammars. :)
    ///
    virtual ItemPtr LT(std::int32_t k) = 0;

    /// Where is this stream pulling nodes from? This is not the name, but
    /// the object that provides node objects.
    ///
    virtual ItemPtr treeSource() = 0;

    /// What adaptor can tell me how to interpret/navigate nodes and
    /// trees. E.g., get text of a node.
    ///
    virtual TreeAdaptorPtr treeAdaptor() = 0;

    /// As we flatten the tree, we use UP, DOWN nodes to represent
    /// the tree structure. When debugging we need unique nodes
    /// so we have to instantiate new ones. When doing normal tree
    /// parsing, it's slow and a waste of memory to create unique
    /// navigation nodes. Default should be false;
    ///
    virtual bool hasUniqueNavigationNodes() = 0;
    virtual void setUniqueNavigationNodes(bool uniqueNavigationNodes) = 0;

    virtual String toString() = 0;

    /// Return the text of all nodes from start to stop, inclusive.
    /// If the stream does not buffer all the nodes then it can still
    /// walk recursively from start until stop. You can always return
    /// null or ANTLR3_T("") too, but users should not access $ruleLabel.text in
    /// an action of course in that case.
    ///
    virtual String toString(ItemPtr start, ItemPtr stop) = 0;

    /// Return the text of all nodes from start to stop, inclusive, into the
    /// supplied buffer.
    /// If the stream does not buffer all the nodes then it can still
    /// walk recursively from start until stop. You can always return
    /// null or ANTLR3_T("") too, but users should not access $ruleLabel.text in
    /// an action of course in that case.
    ///
    virtual void toStringWork(ItemPtr start, ItemPtr stop, String& buf) = 0;

    /// Get a tree node at an absolute index i; 0..n-1.
    /// If you don't want to buffer up nodes, then this method makes no
    /// sense for you.
    ///
    virtual ItemPtr get(std::int32_t i) = 0;

    // REWRITING TREES (used by tree parser)

    /// Replace from start to stop child index of parent with t, which might
    /// be a list. Number of children may be different
    /// after this call. The stream is notified because it is walking the
    /// tree and might need to know you are monkeying with the underlying
    /// tree. Also, it might be able to modify the node stream to avoid
    /// restreaming for future phases.
    ///
    /// If parent is null, don't do anything; must be at root of overall tree.
    /// Can't replace whatever points to the parent externally. Do nothing.
    ///
    virtual void replaceChildren(ItemPtr parent, std::int32_t startChildIndex, std::int32_t stopChildIndex, ItemPtr t) = 0;

};

class CommonTreeNodeStream : public TreeNodeStream, public std::enable_shared_from_this<CommonTreeNodeStream>
{
    class TreeNodeStreamMarker : public Marker
    {
        Index p_;
        CommonTreeNodeStreamPtr stream_;
    public:
        TreeNodeStreamMarker(Index p, CommonTreeNodeStreamPtr stream)
            : Marker()
            , p_(p)
            , stream_(stream)
        {}
        
        virtual void rewind() {
            stream_->p_ = p_;
        }
    };
    
    /// Dummy tree node that indicates a descent into a child
    /// tree. Initialized by a call to create a new interface.
    ///
    CommonTreePtr downNode_;

    /// Dummy tree node that indicates a descent up to a parent
    /// tree. Initialized by a call to create a new interface.
    ///
    CommonTreePtr upNode_;

    /// Dummy tree node that indicates the termination point of the
    /// tree. Initialized by a call to create a new interface.
    ///
    CommonTreePtr eofNode_;

    /// Dummy node that is returned if we need to indicate an invalid node
    /// for any reason.
    ///
    CommonTreePtr invalidNode_;

    /// The complete mapping from stream index to tree node.
    /// This buffer includes pointers to DOWN, UP, and EOF nodes.
    /// It is built upon ctor invocation. The elements are type
    /// Object as we don't what the trees look like.
    ///
    /// Load upon first need of the buffer so we can set token types
    /// of interest for reverseIndexing. Slows us down a wee bit to
    /// do all of the if p==-1 testing everywhere though, though in C
    /// you won't really be able to measure this.
    ///
    /// Must be freed when the tree node stream is torn down.
    ///
    std::vector<ItemPtr> nodes_;

    /// If set to True then the navigation nodes UP, DOWN are
    /// duplicated rather than reused within the tree.
    ///
    bool uniqueNavigationNodes_;

    /// Which tree are we navigating ?
    ///
    ItemPtr root_;

    /// Pointer to tree adaptor interface that manipulates/builds
    /// the tree.
    ///
    std::shared_ptr<TreeAdaptor> adaptor_;

    /// As we walk down the nodes, we must track parent nodes so we know
    /// where to go after walking the last child of a node. When visiting
    /// a child, push current node and current index (current index
    /// is first stored in the tree node structure to avoid two stacks.
    ///
    std::shared_ptr<std::stack<Index>> nodeStack_;

    /// The current index into the nodes vector of the current tree
    /// we are parsing and possibly rewriting.
    ///
    Index p_;

    /// Indicates whether this node stream was derived from a prior
    /// node stream to be used by a rewriting tree parser for instance.
    /// If this flag is set to True, then when this stream is
    /// closed it will not free the root tree as this tree always
    /// belongs to the origniating node stream.
    ///
    bool isRewriter_;

    void fillBuffer(ItemPtr t);
    void fillBufferRoot();

    ItemPtr LB(std::uint32_t k);

public:
    CommonTreeNodeStream(ItemPtr tree);
    CommonTreeNodeStream(TreeAdaptorPtr, ItemPtr tree);
    CommonTreeNodeStream(CommonTreeNodeStream const * inStream);
    ~CommonTreeNodeStream();

    virtual String sourceName() override;
    virtual void consume() override;
    virtual std::uint32_t LA(std::int32_t i) override;
    virtual MarkerPtr mark() override;
    virtual Index index() override;
    virtual void seek(Index index) override;

    virtual ItemPtr LT(std::int32_t k) override;
    virtual ItemPtr treeSource() override;
    virtual TreeAdaptorPtr treeAdaptor() override;
    virtual bool hasUniqueNavigationNodes() override;
    virtual void setUniqueNavigationNodes(bool uniqueNavigationNodes) override;
    virtual String toString() override;
    virtual String toString(ItemPtr start, ItemPtr stop) override;
    virtual void toStringWork(ItemPtr start, ItemPtr stop, String& buf) override;
    virtual ItemPtr get(std::int32_t i) override;
    virtual void replaceChildren(ItemPtr parent, std::int32_t startChildIndex, std::int32_t stopChildIndex, ItemPtr t) override;
    
    void setTreeAdaptor(TreeAdaptorPtr adaptor);

    // INTERFACE
    //
    void fill( std::int32_t k);

    void addLookahead(ItemPtr node);

    bool hasNext();

    ItemPtr next();

    ItemPtr handleRootnode();

    ItemPtr visitChild(std::uint32_t child);

    void addNavigationNode(std::uint32_t ttype);

    ItemPtr newDownNode();

    ItemPtr newUpNode();

    void walkBackToMostRecentNodeWithUnvisitedChildren();

    void push(Index index);
    Index pop();

    void reset();
};

} // namespace antlr3

#endif
