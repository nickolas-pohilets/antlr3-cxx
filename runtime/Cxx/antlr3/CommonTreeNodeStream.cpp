/// \file
/// Defines the implementation of the common node stream the default
/// tree node stream used by ANTLR.
///

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

#include <antlr3/CommonTreeNodeStream.hpp>

template<class StringTraits>
antlr3<StringTraits>::CommonTreeNodeStream::CommonTreeNodeStream(ItemPtr tree)
    : CommonTreeNodeStream(TreeAdaptorPtr(new CommonTreeAdaptor()), std::move(tree))
{}

template<class StringTraits>
antlr3<StringTraits>::CommonTreeNodeStream::CommonTreeNodeStream(TreeAdaptorPtr adaptor, ItemPtr tree)
    : TreeNodeStream()
    , downNode_(std::make_shared<CommonTree>(
        std::make_shared<CommonToken>(TokenDown, ANTLR3_T("DOWN"))
      ))
    , upNode_(std::make_shared<CommonTree>(
        std::make_shared<CommonToken>(TokenUp, ANTLR3_T("UP"))
      ))
    , eofNode_(std::make_shared<CommonTree>(
        std::make_shared<CommonToken>(TokenEof, ANTLR3_T("EOF"))
      ))
    , invalidNode_(std::make_shared<CommonTree>(
        std::make_shared<CommonToken>(TokenInvalid, ANTLR3_T("INVALID"))
      ))
    , nodes_()
    , uniqueNavigationNodes_(false)
    , root_(std::move(tree))
    , adaptor_(std::move(adaptor))
    , nodeStack_(new std::stack<Index>())
    , p_(NullIndex)
    , isRewriter_(false)
{
}

template<class StringTraits>
antlr3<StringTraits>::CommonTreeNodeStream::CommonTreeNodeStream(CommonTreeNodeStream const * inStream)
    : TreeNodeStream()
    , downNode_(std::make_shared<CommonTree>(inStream->downNode_->token()))
    , upNode_(std::make_shared<CommonTree>(inStream->upNode_->token()))
    , eofNode_(std::make_shared<CommonTree>(inStream->eofNode_->token()))
    , invalidNode_(std::make_shared<CommonTree>(inStream->invalidNode_->token()))
    , nodes_()
    , uniqueNavigationNodes_(false)
    , root_(inStream->root_)
    , adaptor_(inStream->adaptor_)
    , nodeStack_(inStream->nodeStack_)
    , p_(NullIndex)
    , isRewriter_(true)
{
}

CommonTreeNodeStream::~CommonTreeNodeStream()
{
}

// ------------------------------------------------------------------------------
// Local helpers
//

/// Walk and fill the tree node buffer from the root tree
///
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::fillBufferRoot()
{
    // Call the generic buffer routine with the root as the
    // argument
    //
    fillBuffer(root_);
    p_ = 0;					// Indicate we are at buffer start
}

/// Walk tree with depth-first-search and fill nodes buffer.
/// Don't add in DOWN, UP nodes if the supplied tree is a list (t is isNilNode)
// such as the root tree is.
///
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::fillBuffer(ItemPtr t)
{
    bool nilNode = adaptor_->isNil(t);

    // If the supplied node is not a nil (list) node then we
    // add in the node itself to the vector
    //
    if(!nilNode)
    {
        nodes_.push_back(t);
    }

    // Only add a DOWN node if the tree is not a nil tree and
    // the tree does have children.
    //
    std::uint32_t nCount = adaptor_->getChildCount(t);
    if	(!nilNode && nCount>0)
    {
        addNavigationNode(TokenDown);
    }

    // We always add any children the tree contains, which is
    // a recursive call to this function, which will cause similar
    // recursion and implement a depth first addition
    //
    for	(std::uint32_t c = 0; c < nCount; c++)
    {
        fillBuffer(adaptor_->getChild(t, c));
    }

    // If the tree had children and was not a nil (list) node, then we
    // we need to add an UP node here to match the DOWN node
    //
    if	(!nilNode && nCount > 0)
    {
        addNavigationNode(TokenUp);
    }
}

// ------------------------------------------------------------------------------
// Interface functions
//

/// Reset the input stream to the start of the input nodes.
///
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::reset()
{
    if	(p_ != NullIndex)
    {
        p_ = 0;
    }

    // Free and reset the node stack only if this is not
    // a rewriter, which is going to reuse the originating
    // node streams node stack
    //
    if(!isRewriter_)
    {
        if(nodeStack_ != NULL)
        {
            nodeStack_.reset(new std::stack<Index>());
        }
    }
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeNodeStream::LB(std::uint32_t k)
{
    if (k==0)
    {
        return invalidNode_;
    }

    if (p_ < k)
    {
        return invalidNode_;
    }

    return nodes_.at(p_ - k);
}

/// Get tree node at current input pointer + i ahead where i=1 is next node.
/// i<0 indicates nodes in the past.  So -1 is previous node and -2 is
/// two nodes ago. LT(0) is undefined.  For i>=n, return null.
/// Return null for LT(0) and any index that results in an absolute address
/// that is negative.
///
/// This is analogous to the LT() method of the TokenStream, but this
/// returns a tree node instead of a token.  Makes code gen identical
/// for both parser and tree grammars. :)
///
template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeNodeStream::LT(std::int32_t k)
{
    if(p_ == NullIndex)
    {
        fillBufferRoot();
    }

    if(k < 0)
    {
        return LB(-k);
    }
    else if	(k == 0)
    {
        return invalidNode_;
    }

    // k was a legitimate request, 
    //
    if((p_ + k - 1) >= (std::int32_t)nodes_.size())
    {
        return eofNode_;
    }

    return nodes_.at(p_ + k - 1);
}

/// Where is this stream pulling nodes from?  This is not the name, but
/// the object that provides node objects.
///
template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeNodeStream::treeSource()
{
    return root_;
}

template<class StringTraits>
String antlr3<StringTraits>::CommonTreeNodeStream::sourceName()
{
    assert(false);
    return String();
}

/// Consume the next node from the input stream
///
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::consume()
{
    if(p_ == NullIndex)
    {
        fillBufferRoot();
    }

    p_++;
}

std::uint32_t CommonTreeNodeStream::LA(std::int32_t i)
{
    // Ask LT for the 'token' at that position
    //
    ItemPtr t = LT(i);
    if	(t == NULL)
    {
        return	TokenInvalid;
    }

    // Token node was there so return the type of it
    return adaptor_->getType(t);
}

/// Mark the state of the input stream so that we can come back to it
/// after a syntactic predicate and so on.
///
template<class StringTraits>
MarkerPtr antlr3<StringTraits>::CommonTreeNodeStream::mark()
{
    return std::make_shared<TreeNodeStreamMarker>(index(), shared_from_this());
}

/// consume() ahead until we hit index.  Can't just jump ahead--must
/// spit out the navigation nodes.
///
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::seek(Index index)
{
    p_ = index;
}

template<class StringTraits>
Index antlr3<StringTraits>::CommonTreeNodeStream::index()
{
    if	(p_ == NullIndex)
    {
        fillBufferRoot();
    }
    return p_;
}

/// Expensive to compute the size of the whole tree while parsing.
/// This method only returns how much input has been seen so far.  So
/// after parsing it returns true size.
///
//std::uint32_t CommonTreeNodeStream::size()
//{
//    if(p_ == NullIndex)
//    {
//        fillBufferRoot();
//    }
//
//    return (std::uint32_t)nodes_.size();
//}

/// As we flatten the tree, we use UP, DOWN nodes to represent
/// the tree structure.  When debugging we need unique nodes
/// so instantiate new ones when uniqueNavigationNodes is true.
///
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::addNavigationNode(std::uint32_t ttype)
{
    ItemPtr	    node;

    node = NULL;

    if(ttype == TokenDown)
    {
        if(hasUniqueNavigationNodes())
        {
            node = newDownNode();
        }
        else
        {
            node = downNode_;
        }
    }
    else
    {
        if(hasUniqueNavigationNodes())
        {
            node = newUpNode();
        }
        else
        {
            node = upNode_;
        }
    }

    // Now add the node we decided upon.
    //
    nodes_.push_back(node);
}


template<class StringTraits>
TreeAdaptorPtr antlr3<StringTraits>::CommonTreeNodeStream::treeAdaptor()
{
    return adaptor_;
}

template<class StringTraits>
bool antlr3<StringTraits>::CommonTreeNodeStream::hasUniqueNavigationNodes()
{
    return uniqueNavigationNodes_;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::setUniqueNavigationNodes(bool uniqueNavigationNodes)
{
    uniqueNavigationNodes_ = uniqueNavigationNodes;
}


/// Print out the entire tree including DOWN/UP nodes.  Uses
/// a recursive walk.  Mostly useful for testing as it yields
/// the token types not text.
///
template<class StringTraits>
String antlr3<StringTraits>::CommonTreeNodeStream::toString()
{
    return toString(root_, NULL);
}

template<class StringTraits>
String antlr3<StringTraits>::CommonTreeNodeStream::toString(ItemPtr start, ItemPtr stop)
{
    String  buf;
    toStringWork(start, stop, buf);
    return  buf;
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::toStringWork(ItemPtr p, ItemPtr stop, String& buf)
{
    if(adaptor_->isNil(p))
    {
        String text = adaptor_->toString(p);
        if  (text.empty())
        {
            text += ' ';
            text += antlr3::toString(adaptor_->getType(p));
        }

        buf += text;;
    }

    if	(p == stop)
    {
        return;		/* Finished */
    }

    std::uint32_t n = adaptor_->getChildCount(p);
    if	(n > 0 && !adaptor_->isNil(p))
    {
        buf += ' ';
        buf += antlr3::toString(TokenDown);
    }

    for	(std::uint32_t c = 0; c<n ; c++)
    {
        ItemPtr child = adaptor_->getChild(p, c);
        toStringWork(child, stop, buf);
    }

    if	(n > 0 && !adaptor_->isNil(p))
    {
        buf += ' ';
        buf += antlr3::toString(TokenUp);
    }
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeNodeStream::newDownNode()
{
    CommonTokenPtr token = std::make_shared<CommonToken>(TokenDown, ANTLR3_T("DOWN"));
    return std::make_shared<CommonTree>(token);
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeNodeStream::newUpNode()
{
    CommonTokenPtr token = std::make_shared<CommonToken>(TokenUp, ANTLR3_T("UP"));
    return std::make_shared<CommonTree>(token);
}

/// Replace from start to stop child index of parent with t, which might
/// be a list.  Number of children may be different
/// after this call.  The stream is notified because it is walking the
/// tree and might need to know you are monkey-ing with the underlying
/// tree.  Also, it might be able to modify the node stream to avoid
/// re-streaming for future phases.
///
/// If parent is null, don't do anything; must be at root of overall tree.
/// Can't replace whatever points to the parent externally.  Do nothing.
///
template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::replaceChildren(ItemPtr parent, std::int32_t startChildIndex, std::int32_t stopChildIndex, ItemPtr t)
{
    if	(parent != NULL)
    {
        auto adaptor = treeAdaptor();
        adaptor->replaceChildren(parent, startChildIndex, stopChildIndex, t);
    }
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::CommonTreeNodeStream::get(std::int32_t k)
{
    if(p_ == NullIndex)
    {
        fillBufferRoot();
    }

    return nodes_.at(k);
}

template<class StringTraits>
void antlr3<StringTraits>::CommonTreeNodeStream::push(Index index)
{
    nodeStack_->push(p_);	// Save current index
    seek(index);
}

template<class StringTraits>
Index antlr3<StringTraits>::CommonTreeNodeStream::pop()
{
    Index retVal = nodeStack_->top();
    nodeStack_->pop();
    seek(retVal);
    return retVal;
}

