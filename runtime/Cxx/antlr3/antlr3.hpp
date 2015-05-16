#ifndef	_ANTLR3_H
#define	_ANTLR3_H

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

#include <antlr3/Exception.hpp>
#include <antlr3/String.hpp>
#include <antlr3/CharStream.hpp>
#include <antlr3/CyclicDFA.hpp>
#include <antlr3/IntStream.hpp>
#include <antlr3/RecognizerSharedState.hpp>
#include <antlr3/BaseRecognizer.hpp>
#include <antlr3/CommonToken.hpp>
#include <antlr3/TokenStream.hpp>
#include <antlr3/Bitset.hpp>
#include <antlr3/Lexer.hpp>
#include <antlr3/Parser.hpp>
#include <antlr3/TreeParser.hpp>
#include <antlr3/BaseTreeAdaptor.hpp>
#include <antlr3/CommonTreeAdaptor.hpp>
#include <antlr3/RewriteStreams.hpp>
#include <antlr3/DebugEventListener.hpp>
#include <antlr3/DebugEventSocketProxy.hpp>

#endif
