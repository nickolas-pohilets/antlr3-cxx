#ifndef _ANTLR3_COCOA_HH
#define _ANTLR3_COCOA_HH

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

#include <antlr3/antlr3.cpp>
#import <Foundation/Foundation.h>

namespace antlr3ex {

class CocoaString {
public:
    CocoaString() : immutable_(nil), mutable_(nil) {}
    CocoaString(NSString* s) : immutable_(s), mutable_(nil) {}
    CocoaString(NSMutableString* s) : immutable_(nil), mutable_(s) {}
    CocoaString(CocoaString const & other) = default;
    CocoaString(CocoaString && other)
        : immutable_(other.immutable_)
        , mutable_(other.mutable_)
    {
        other.immutable_ = nil;
        other.mutable_ = nil;
    }
    
    
    CocoaString& operator=(CocoaString other);
    CocoaString& operator+=(unichar c);
    CocoaString& operator+=(CocoaString const & c);
    
    bool empty() const;
    size_t size() const;
    
    unichar const* begin() const;
    unichar const* end() const;
    
    CocoaString& append(char const * b, char const * e);
private:
    NSString* immutable_;
    NSMutableString* mutable_;
};

CocoaString operator+(CocoaString lhs, CocoaString const & rhs);

class CocoaStringStream {
private:
    NSMutableString* buffer_;
    bool borrowed_;
};

class CocoaStringLiteralBuffer {
public:
    NSString* data;
    
    CocoaStringLiteralBuffer(NSString* x) : data(x) {}
    
    CocoaString literal() const { return CocoaString(data); }
};

class CocoaStringTraits {
public:
    typedef CocoaString String;
    typedef char16_t Char;
    typedef CocoaString StringLiteral;
    typedef CocoaStringStream StringStream;
    
    template<size_t N8, size_t N16> using LiteralBuffer = CocoaStringLiteralBuffer;
    static NSString * selectLiteral(NSString * s) { return s; }
    static antlr3_defs::TryNextStringLiteral selectLiteral(char const *);
    static antlr3_defs::TryNextStringLiteral selectLiteral(char16_t const *);
    template<class T, size_t N>
    static antlr3_defs::TryNextStringLiteral selectLiteral(std::array<T, N> const &);
//    static antlr3_defs::TryNextStringLiteral selectLiteral(...);
    
    static String string(std::uint8_t const * b, std::uint8_t const * e);
    static String string(Char const * b, Char const * e);
    
    static String toString(int val);
    static String toString(long val);
    static String toString(long long val);
    static String toString(unsigned val);
    static String toString(unsigned long val);
    static String toString(unsigned long long val);
    static String toString(float val);
    static String toString(double val);
    static String toString(long double val);
    
    static std::string toUTF8(String s);
    /* static std::string toUTF8(StringLiteral s); */
    static String fromUTF8(std::string s);
    static String fromUTF8(char const * s);

    static std::string& appendToUTF8(std::string& s8, String s);
    /* static std::string& appendToUTF8(std::string& s8, StringLiteral s); */
    static String& appendUTF8(String& s, std::string const & s8);
    static String& appendUTF8(String& s, char const * s8);
    
    static void foo() {
        NSString* s = selectLiteral(@"ABC");
        NSLog(@"%@", s);
    }
};

} // namespace antlr3ex

namespace std {
std::ostream& operator<<(std::ostream& s, antlr3ex::CocoaString const & str);
} // namespace std

extern template class antlr3<antlr3ex::CocoaStringTraits>;

#endif //_ANTLR3_COCOA_HH
