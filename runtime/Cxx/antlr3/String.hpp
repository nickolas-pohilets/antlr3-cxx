/** \file
 * Simple string interface allows indiscriminate allocation of strings
 * such that they can be allocated all over the place and released in 
 * one chunk via a string factory - saves lots of hassle in remembering what
 * strings were allocated where.
 */
#ifndef _ANTLR3_STRING_HPP
#define _ANTLR3_STRING_HPP

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
#include <string>
#include <sstream>

namespace antlr3 {

#if ANTLR3_UTF16

typedef std::u16string String;
typedef char16_t const * ConstString;
typedef std::basic_stringstream<char16_t> StringStream;

#define ANTLR3_T(X) u##X

std::string toUTF8(String const & s);
std::string toUTF8(ConstString s);
String fromUTF8(std::string const & s);
String fromUTF8(char const * s);

std::string& appendToUTF8(std::string& s8, String const & s);
std::string& appendToUTF8(std::string& s8, ConstString s);
String& appendUTF8(String& s, std::string const & s8);
String& appendUTF8(String& s, char const * s8);

std::ostream& operator<<(std::ostream& s, const String& str);
std::ostream& operator<<(std::ostream& s, ConstString str);

#else

typedef std::string String;
typedef char const * ConstString;
typedef std::stringstream StringStream;

#define ANTLR3_T(X) X

inline std::string toUTF8(String s) { return std::move(s); }
inline std::string toUTF8(ConstString s) { return s; }
inline String fromUTF8(std::string s) { return std::move(s); }
inline String fromUTF8(char const * s) { return s; }

inline std::string& appendToUTF8(std::string& s8, String const & s) { return s8 += s; }
inline std::string& appendToUTF8(std::string& s8, ConstString s) { return s8 += s; }
inline String& appendUTF8(String& s, std::string const & s8) { return s += s8; }
inline String& appendUTF8(String& s, char const * s8) { return s += s8; }

#endif

String toString(int val);
String toString(long val);
String toString(long long val);
String toString(unsigned val);
String toString(unsigned long val);
String toString(unsigned long long val);
String toString(float val);
String toString(double val);
String toString(long double val);

String& appendEscape(String& dest, String const & src);
String& appendEscape(String& dest, String::value_type src);
String& appendEscape(String& dest, Char src);

template<class T>
String escape(T const & x) {
    String retVal;
    appendEscape(retVal, x);
    return std::move(retVal);
}

} // namespace antlr3

#endif