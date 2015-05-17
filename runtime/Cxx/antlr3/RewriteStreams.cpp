/// \file
/// Implementation of token/tree streams that are used by the
/// tree re-write rules to manipulate the tokens and trees produced
/// by rules that are subject to rewrite directives.
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

#include <antlr3/RewriteStreams.hpp>

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleElementStream::RewriteRuleElementStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description)
    : cursor_(0)
    , singleElement_()
    , elements_()
    , elementDescription_(description)
    , adaptor_(std::move(adaptor))
    , dirty_(false)
{
}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleElementStream::RewriteRuleElementStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, ItemPtr oneElement)
    : RewriteRuleElementStream(std::move(adaptor), rec, description)
{
    add(std::move(oneElement));
}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleElementStream::RewriteRuleElementStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<ItemPtr> vector)
    : RewriteRuleElementStream(std::move(adaptor), rec, description)
{
	elements_ = std::move(vector);
}

RewriteRuleElementStream::~RewriteRuleElementStream()
{
}

template<class StringTraits>
void antlr3<StringTraits>::RewriteRuleElementStream::reset()
{
	dirty_	= true;
	cursor_	= 0;
}

template<class StringTraits>
void antlr3<StringTraits>::RewriteRuleElementStream::add(ItemPtr el)
{
	if (!el) {
		return;
	}
    
	// As we may be reusing a stream, we may already have allocated
	// a rewrite stream vector. If we have then is will be empty if
	// we have either zero or just one element in the rewrite stream
	if (!elements_.empty())
	{
		// We already have >1 entries in the stream. So we can just add this new element to the existing
		// collection. 
		//
		elements_.push_back(el);
		return;
	}
	if (singleElement_ == NULL)
	{
		singleElement_ = el;
		return;
	}

	// If we got here then we had only the one element so far
	// and we must now create a vector to hold a collection of them
	elements_.push_back(singleElement_);
	elements_.push_back(el);
	singleElement_.reset();
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleElementStream::next()
{
	std::uint32_t s = size();
	if (cursor_ >= s && s == 1)
	{
		return dup(_next());
	}
	return _next();
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleElementStream::nextTree() 
{
	std::uint32_t n = size();

	if (dirty_ || (cursor_ >= n && n==1) )
	{
		// if out of elements and size is 1, dup
		//
		ItemPtr el = _next();
		return dup(el);
	}

	// test size above then fetch
	//
	return _next();
}

template<class StringTraits>
bool antlr3<StringTraits>::RewriteRuleElementStream::hasNext()
{
	return (singleElement_ && cursor_ < 1) || (!elements_.empty() && cursor_ < elements_.size());
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleElementStream::nextNode()
{
	ItemPtr el = _next();

	std::uint32_t n = size();
	if (dirty_ || (cursor_ > n && n == 1))
	{
		// We are out of elements and the size is 1, which means we just 
		// dup the node that we have
		//
		return adaptor_->dupNode(el);
	}

	// We were not out of nodes, so the one we received is the one to return
	//
	return el;
}

std::uint32_t RewriteRuleElementStream::size()
{
	if (singleElement_)
	{
		return 1;
	}
    
    return (std::uint32_t)elements_.size();
}

template<class StringTraits>
StringLiteral antlr3<StringTraits>::RewriteRuleElementStream::description()
{
    return elementDescription_ ? elementDescription_ : ANTLR3_T("<unknown source>");
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleElementStream::toTree(ItemPtr element)
{
    /// We don't explicitly convert to a tree unless the call goes to
    /// nextTree, which means rewrites are heterogeneous 
    ///
	return std::move(element);
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleElementStream::_next()
{
	std::uint32_t n = size();
	if (n == 0)
	{
		// This means that the stream is empty
        assert(false);
		return nullptr;
	}

	// Traversed all the available elements already?
	//
	if (cursor_ >= n)
	{
		if (n == 1)
		{
			// Special case when size is single element, it will just dup a lot
			//
			return toTree(singleElement_);
		}

		// Out of elements and the size is not 1, so we cannot assume
		// that we just duplicate the entry n times (such as ID ent+ -> ^(ID ent)+)
		// This means we ran out of elements earlier than was expected.
		assert(false);
		return nullptr;
	}

	// Elements available either for duping or just available
	if (singleElement_ != nullptr)
	{
		cursor_++; // Cursor advances even for single element as this tells us to dup()
		return toTree(singleElement_);
	}

	// More than just a single element so we extract it from the vector.
	ItemPtr t = toTree(elements_.at(cursor_));
    cursor_++;
	return t;
}

#pragma mark RewriteRuleTokenStream

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleTokenStream::RewriteRuleTokenStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description)
    : RewriteRuleElementStream(std::move(adaptor), rec, description)
{}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleTokenStream::RewriteRuleTokenStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, CommonTokenPtr oneElement)
    : RewriteRuleElementStream(std::move(adaptor), rec, description, std::move(oneElement))
{}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleTokenStream::RewriteRuleTokenStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<CommonTokenPtr> const & vector)
    : RewriteRuleElementStream(std::move(adaptor), rec, description, std::vector<ItemPtr>(vector.begin(), vector.end()))
{}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleTokenStream::dup(ItemPtr)
{
    assert(false && ANTLR3_T("dup() cannot be called on a token rewrite stream!!"));
	return nullptr;
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleTokenStream::nextNode()
{
    CommonTokenPtr token = std::static_pointer_cast<CommonToken>(_next());
    return adaptor_->create(token);
}

template<class StringTraits>
CommonTokenPtr antlr3<StringTraits>::RewriteRuleTokenStream::nextToken()
{
	return pointer_cast<CommonTokenPtr>(_next());
}

#pragma mark RewriteRuleSubtreeStream

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleSubtreeStream::RewriteRuleSubtreeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description)
    : RewriteRuleElementStream(std::move(adaptor), rec, description)
{}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleSubtreeStream::RewriteRuleSubtreeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, ItemPtr oneElement)
    : RewriteRuleElementStream(std::move(adaptor), rec, description, std::move(oneElement))
{}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleSubtreeStream::RewriteRuleSubtreeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<ItemPtr> vector)
    : RewriteRuleElementStream(std::move(adaptor), rec, description, std::move(vector))
{}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleSubtreeStream::dup(ItemPtr element)
{
	return adaptor_->dupNode(std::move(element));
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleSubtreeStream::nextNode()
{
    return RewriteRuleElementStream::nextNode();
}

#pragma mark RewriteRuleNodeStream

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleNodeStream::RewriteRuleNodeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description)
    : RewriteRuleElementStream(std::move(adaptor), rec, description)
{}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleNodeStream::RewriteRuleNodeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, ItemPtr oneElement)
    : RewriteRuleElementStream(std::move(adaptor), rec, description, std::move(oneElement))
{}

template<class StringTraits>
antlr3<StringTraits>::RewriteRuleNodeStream::RewriteRuleNodeStream(TreeAdaptorPtr adaptor, BaseRecognizer * rec, StringLiteral description, std::vector<ItemPtr> vector)
    : RewriteRuleElementStream(std::move(adaptor), rec, description, std::move(vector))
{}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleNodeStream::dup(ItemPtr element)
{
	assert(false && ANTLR3_T("dup() cannot be called on a node rewrite stream!!!"));
	return nullptr;
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleNodeStream::toTree(ItemPtr element)
{
    return adaptor_->dupNode(std::move(element));
}

template<class StringTraits>
ItemPtr antlr3<StringTraits>::RewriteRuleNodeStream::nextNode()
{
    return _next();
}

