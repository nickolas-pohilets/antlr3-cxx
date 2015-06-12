/** \file
 * Contains the base functions that all tree adaptors start with.
 * this implementation can then be overridden by any higher implementation.
 * 
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

#include <antlr3/TreeAdaptor.hpp>

template<class StringTraits>
class dot_utils : public antlr3<StringTraits> {

typedef typename antlr3<StringTraits>::String String;
typedef typename antlr3<StringTraits>::StringLiteral StringLiteral;
typedef typename antlr3<StringTraits>::StringUtils StringUtils;
typedef typename antlr3<StringTraits>::TreeAdaptorPtr TreeAdaptorPtr;
typedef typename antlr3<StringTraits>::ItemPtr ItemPtr;

public:

static void defineDotNodes(TreeAdaptorPtr adaptor, ItemPtr t, String & dotSpec)
{
    // How many nodes are we talking about?
	//
    int nCount = adaptor->getChildCount(t);

	if	(nCount == 0)
	{
		// This will already have been included as a child of another node
		// so there is nothing to add.
		//
		return;
	}

	// For each child of the current tree, define a node using the
	// memory address of the node to name it
	//
	for	(int i = 0; i<nCount; i++)
	{

		// Pick up a pointer for the child
		//
		ItemPtr child = adaptor->getChild(t, i);

		// Name the node
		//
        char buff[64];
		sprintf(buff, "\tn%p[label=\"", child.get());
		StringTraits::appendUTF8(dotSpec, buff);
        StringUtils::appendEscape(dotSpec, adaptor->getText(child));
		dotSpec += ANTLR3_T("\"]\n");

		// And now define the children of this child (if any)
		//
		defineDotNodes(adaptor, child, dotSpec);
	}
}

static void defineDotEdges(TreeAdaptorPtr adaptor, ItemPtr t, String & dotSpec)
{
	if	(t == NULL)
	{
		// No tree, so do nothing
		//
		return;
	}

	// Count the nodes
	//
	int nCount = adaptor->getChildCount(t);

	if	(nCount == 0)
	{
		// This will already have been included as a child of another node
		// so there is nothing to add.
		//
		return;
	}

	// For each child, define an edge from this parent, then process
	// and children of this child in the same way
	//
	for	(int i=0; i<nCount; i++)
	{
		// Next child
		//
		ItemPtr child = adaptor->getChild(t, i);

		// Create the edge relation
		//
        char buff[128];
		sprintf(buff, "\t\tn%p -> n%p\t\t// ", t.get(), child.get());
        StringTraits::appendUTF8(dotSpec, buff);

		// Document the relationship
		//
        StringUtils::appendEscape(dotSpec, adaptor->getText(t));
        dotSpec += ANTLR3_T(" -> ");
        StringUtils::appendEscape(dotSpec, adaptor->getText(child));
		dotSpec += ANTLR3_T("\n");
        
		// Define edges for this child
		//
		defineDotEdges(adaptor, child, dotSpec);
	}
}

};

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::makeDot(TreeAdaptorPtr adaptor, ItemPtr theTree)
{
    // Default look and feel
    //
    String dotSpec = ANTLR3_T(
        "digraph {\n\n"
        "\tordering=out;\n"
        "\tranksep=.4;\n"
        "\tbgcolor=\")lightgrey\";  node [shape=box, fixedsize=false, fontsize=12, fontname=\"Helvetica-bold\", fontcolor=\"blue\"\n"
        "\twidth=.25, height=.25, color=\"black\", fillcolor=\"white\", style=\"filled, solid, bold\"];\n\n"
        "\tedge [arrowsize=.5, color=\"black\", style=\"bold\"]\n\n"
    );

    if	(theTree == NULL)
	{
		// No tree, so create a blank spec
		//
		dotSpec += ANTLR3_T("n0[label=\"EMPTY TREE\"]\n");
		return dotSpec;
	}

    char buff[64];
    sprintf(buff, "\tn%p[label=\"", theTree.get());
	StringTraits::appendUTF8(dotSpec, buff);
    StringUtils::appendEscape(dotSpec, adaptor->getText(theTree));
    dotSpec += ANTLR3_T("\"]\n");

	// First produce the node defintions
	//
	dot_utils<StringTraits>::defineDotNodes(adaptor, theTree, dotSpec);
	dotSpec += ANTLR3_T("\n");
	dot_utils<StringTraits>::defineDotEdges(adaptor, theTree, dotSpec);
	
	// Terminate the spec
	//
	dotSpec += ANTLR3_T("\n}");

	// Result
	//
	return dotSpec;
}
