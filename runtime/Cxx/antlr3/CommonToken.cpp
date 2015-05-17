/**
 * Contains the default implementation of the common token used within
 * java. Custom tokens should create this structure and then append to it using the
 * custom pointer to install their own structure and API.
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

#include <antlr3/CommonToken.hpp>
#include <antlr3/CharStream.hpp>

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::CommonToken::text() const
{
    if(hasText_)
    {
        return tokText_;
    }

    // EOF is a special case
    //
    if(type_ == TokenEof)
    {
        return ANTLR3_T("<EOF>");
    }

    // We had nothing installed in the token, create a new string
    // from the input stream
    //
    if (input_ != NULL)
    {
        return input_->substr(startIndex(), stopIndex());
    }

    // Nothing to return, there is no input stream
    //
    return String();
}

/** \brief Install the supplied text string as teh text for the token.
 * The method assumes that the existing text (if any) was created by a factory
 * and so does not attempt to release any memory it is using.Text not created
 * by a string fctory (not advised) should be released prior to this call.
 */
template<class StringTraits>
void antlr3<StringTraits>::CommonToken::setText(String text)
{
    hasText_ = true;
    tokText_ = std::move(text);
}

template<class StringTraits>
antlr3_defs::Location antlr3<StringTraits>::CommonToken::startLocation() const
{
    return input_ ? input_->location(start_) : Location();
}

template<class StringTraits>
antlr3_defs::Location antlr3<StringTraits>::CommonToken::stopLocation() const
{
    return input_ ? input_->location(stop_) : Location();
}

template<class StringTraits>
typename antlr3<StringTraits>::String
    antlr3<StringTraits>::CommonToken::toString(StringLiteral const * tokenNames) const
{
    String outtext;

    /* Now we use our handy dandy string utility to assemble the
     * the reporting string
     * return ANTLR3_T("[@")+tokenIndex()+ANTLR3_T(",")+start+ANTLR3_T(":")+stop+ANTLR3_T("='")+txt+ANTLR3_T("',<")+type+ANTLR3_T(">")+channelStr+ANTLR3_T(",")+line+ANTLR3_T(":")+charPositionInLine()+ANTLR3_T("]");
     */
    outtext += ANTLR3_T("[@");
    outtext += StringTraits::toString((std::int32_t)tokenIndex());
    outtext += ANTLR3_T(",");
    outtext += StringTraits::toString((std::int32_t)startIndex());
    outtext += ANTLR3_T(":");
    outtext += StringTraits::toString((std::int32_t)stopIndex());
    outtext += ANTLR3_T("='");
    outtext += escape(text());
    outtext += ANTLR3_T("',<");
    if (tokenNames) {
        outtext += getTokenName(type(), tokenNames);
    } else {
        outtext += StringTraits::toString(type());
    }
    outtext += ANTLR3_T(">");

    if (channel() > TokenDefaultChannel)
    {
        outtext += ANTLR3_T(",channel=");
        outtext += toString((std::int32_t)channel());
    }

    Location loc = startLocation();
    outtext += ANTLR3_T(",");
    outtext += toString(loc.line());
    outtext += ANTLR3_T(":");
    outtext += toString(loc.charPositionInLine());
    outtext += "]";

    return outtext;
}
