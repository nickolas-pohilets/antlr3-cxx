#ifndef ANTLR3REWRITESTREAM_H
#define ANTLR3REWRITESTREAM_H

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
#include <antlr3/CommonTreeAdaptor.hpp>
#include <antlr3/BaseRecognizer.hpp>

namespace antlr3 {

/// A generic list of elements tracked in an alternative to be used in
/// a -> rewrite rule. 
///
/// In the C implementation, all tree oriented streams return a pointer to 
/// the same type: BaseTreePtr. Anything that has subclassed from this
/// still passes this type, within which there is a super pointer, which points
/// to it's own data and methods. Hence we do not need to implement this as
/// the equivalent of an abstract class, but just fill in the appropriate interface
/// as usual with this model.
///
/// Once you start next()ing, do not try to add more elements. It will
/// break the cursor tracking I believe.
///
/// 
/// \see #pANTLR3_REWRITE_RULE_NODE_STREAM
/// \see #pANTLR3_REWRITE_RULE_ELEMENT_STREAM
/// \see #pANTLR3_REWRITE_RULE_SUBTREE_STREAM
///
/// TODO: add mechanism to detect/puke on modification after reading from stream
///
class RewriteRuleElementStream
{
public:
    RewriteRuleElementStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description);
    RewriteRuleElementStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, ItemPtr oneElement);
    RewriteRuleElementStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<ItemPtr> vector);
    virtual ~RewriteRuleElementStream();

    // Methods 

    /// Reset the condition of this stream so that it appears we have
    /// not consumed any of its elements. Elements themselves are untouched.
    ///
    void reset();

    /// Add a new BaseTreePtr to this stream
    ///
    void add(ItemPtr);

    /// Return the next element in the stream. If out of elements, throw
    /// an exception unless size()==1. If size is 1, then return elements[0].
    ///
    ItemPtr next();
    
    /// Return the next element in the stream.  If out of elements, throw
    /// an exception unless size()==1.  If size is 1, then return elements[0].
    /// Return a duplicate node/subtree if stream is out of elements and
    /// size==1.  If we've already used the element, dup (dirty bit set).
    ///
    ItemPtr nextTree();

    /// Returns true if there is a next element available
    ///
    bool hasNext();

    /// Treat next element as a single node even if it's a subtree.
    /// This is used instead of next() when the result has to be a
    /// tree root node. Also prevents us from duplicating recently-added
    /// children; e.g., ^(type ID)+ adds ID to type and then 2nd iteration
    /// must dup the type node, but ID has been added.
    ///
    /// Referencing to a rule result twice is ok; dup entire tree as
    /// we can't be adding trees; e.g., expr expr. 
    ///
    virtual ItemPtr nextNode();

    /// Number of elements available in the stream
    ///
    std::uint32_t size();

    /// Returns the description string if there is one available (check for NULL).
    ///
    StringLiteral description();
    
protected:
    /// Pointer to the tree adaptor in use for this stream
    TreeAdaptorPtr adaptor_;
    
    /// When constructing trees, sometimes we need to dup a token or AST
    /// subtree. Dup'ing a token means just creating another AST node
    /// around it. For trees, you must call the adaptor.dupTree().
    ///
    virtual ItemPtr dup(ItemPtr el) = 0;

    /// Ensure stream emits trees; tokens must be converted to AST nodes.
    /// AST nodes can be passed through unmolested.
    ///
    virtual ItemPtr toTree(ItemPtr el);
    
    /// Do the work of getting the next element, making sure that it's
    /// a tree node or subtree.  Deal with the optimization of single-
    /// element list versus list of size > 1.  Throw an exception (or something similar)
    /// if the stream is empty or we're out of elements and size>1.
    /// You can override in a 'subclass' if necessary.
    ///
    ItemPtr _next();
private:
    /// Cursor 0..n-1. If singleElement!=NULL, cursor is 0 until you next(),
    /// which bumps it to 1 meaning no more elements.
    ///
    std::uint32_t cursor_;

    /// Track single elements w/o creating a list. Upon 2nd add, alloc list 
    ///
    ItemPtr singleElement_;

    /// The list of tokens or subtrees we are tracking 
    ///
    std::vector<ItemPtr> elements_;

    /// The element or stream description; usually has name of the token or
    /// rule reference that this list tracks. Can include rulename too, but
    /// the exception would track that info.
    ///
    StringLiteral elementDescription_;

    /// Once a node / subtree has been used in a stream, it must be dup'ed
    /// from then on. Streams are reset after sub rules so that the streams
    /// can be reused in future sub rules. So, reset must set a dirty bit.
    /// If dirty, then next() always returns a dup.
    ///
    bool dirty_;

    // Pointer to the recognizer shared state to which this stream belongs
    //
    BaseRecognizer * rec_;
};

/// This is an implementation of a token stream, which is basically an element
/// stream that deals with tokens only.
///
class RewriteRuleTokenStream : public RewriteRuleElementStream
{
public:
    RewriteRuleTokenStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description);
    RewriteRuleTokenStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, CommonTokenPtr oneElement);
    RewriteRuleTokenStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<CommonTokenPtr> const & vector);
    
    ItemPtr dup(ItemPtr el) override;
    ItemPtr nextNode() override;
    
    /// Return the next element for a caller that wants just the token
    ///
    CommonTokenPtr nextToken();
};

/// This is an implementation of a subtree stream which is a set of trees
/// modelled as an element stream.
///
class RewriteRuleSubtreeStream : public RewriteRuleElementStream
{
public:
    RewriteRuleSubtreeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description);
    RewriteRuleSubtreeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, ItemPtr oneElement);
    RewriteRuleSubtreeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<ItemPtr> vector);

    ItemPtr dup(ItemPtr el) override;
    ItemPtr nextNode() override;
};

/// This is an implementation of a node stream, which is basically an element
/// stream that deals with tree nodes only.
///
class RewriteRuleNodeStream : public RewriteRuleElementStream
{
public:
    RewriteRuleNodeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description);
    RewriteRuleNodeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, ItemPtr oneElement);
    RewriteRuleNodeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<ItemPtr> vector);

    ItemPtr dup(ItemPtr el) override;
    ItemPtr nextNode() override;
    ItemPtr toTree(ItemPtr el) override;
};

} // namespace antlr3

#endif
