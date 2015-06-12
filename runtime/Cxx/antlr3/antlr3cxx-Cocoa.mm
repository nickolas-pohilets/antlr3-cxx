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

#include <antlr3/antlr3cxx-Cocoa.hh>

typedef antlr3ex::CocoaStringTraits StringTraits;
typedef StringTraits::String String;
typedef StringTraits::StringLiteral StringLiteral;

template class antlr3<StringTraits>;

namespace {

NSString* escapingString(unichar c) {
    if (c == '\"') return @"\\\"";
    if (c == '\n') return @"\\n";
    if (c == '\r') return @"\\r";
    return [NSString stringWithCharacters:&c length:1];
}

NSString* fitsIntoTaggedPointer(std::uint8_t const * s, size_t n) {
    if (sizeof(s) < 8) return nil;
    enum { kMaxTaggedPointerStringSize = 9 };
    if (n >= kMaxTaggedPointerStringSize) return nil;
    unichar buf[kMaxTaggedPointerStringSize];
    for (size_t i = 0; i < 10; ++i) {
        if (s[i] >= 0x80) return nil;
        buf[i] = s[i];
    }
    return [[NSString alloc] initWithCharacters:buf length:n];
}

} // namespace

namespace antlr3ex {

String StringTraits::string(std::uint8_t const * b, std::uint8_t const * e) {
    return fitsIntoTaggedPointer(b, e - b) ?: [[NSString alloc] initWithBytes:b length:(e - b) encoding:NSISOLatin1StringEncoding];
}

String StringTraits::string(Char const * b, Char const * e) {
    static_assert(sizeof(Char) == sizeof(unichar), "Size of unichar and Char should match");
    return [[NSString alloc] initWithCharacters:reinterpret_cast<unichar const *>(b) length:(e - b)];
}

String StringTraits::toString(int val) { return [@(val) description]; }
String StringTraits::toString(long val) { return [@(val) description]; }
String StringTraits::toString(long long val) { return [@(val) description]; }
String StringTraits::toString(unsigned val) { return [@(val) description]; }
String StringTraits::toString(unsigned long val) { return [@(val) description]; }
String StringTraits::toString(unsigned long long val) { return [@(val) description]; }
String StringTraits::toString(float val) { return [@(val) description]; }
String StringTraits::toString(double val) { return [@(val) description]; }
String StringTraits::toString(long double val) { return [@((double)val) description]; }

std::string StringTraits::toUTF8(String const & s) {
    std::string retVal;
    appendToUTF8(retVal, s);
    return std::move(retVal);
}

String StringTraits::fromUTF8(std::string const & s) {
    return [[NSString alloc] initWithBytes:s.c_str() length:s.size() encoding:NSUTF8StringEncoding];
}

String StringTraits::fromUTF8(char const * s) {
    return [NSString stringWithUTF8String:s];
}

std::string& StringTraits::appendToUTF8(std::string& s8, String s) {
    NSString* str = s.string();
    size_t oldSize = s8.size();
    NSUInteger n = [str lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    s8.resize(oldSize + n);
    NSUInteger k = 0;
    NSRange r = NSMakeRange(0, str.length);
    [str getBytes:&s8[oldSize]
        maxLength:n
       usedLength:&k
         encoding:NSUTF8StringEncoding
          options:NSStringEncodingConversionAllowLossy
            range:r remainingRange:&r];
    NSCAssert(r.length == 0, @"Unexpected error");
    s8.resize(oldSize + k);
    return s8;
}

String& StringTraits::appendUTF8(String& s, std::string const & s8) {
    s += fromUTF8(s8);
    return s;
}

String& StringTraits::appendUTF8(String& s, char const * s8) {
    s += fromUTF8(s8);
    return s;
}

String& StringTraits::appendEscape(String& dest, CocoaString const & src) {
    if (src.empty()) {
        return dest;
    }
    
    NSCharacterSet* charSet = [NSCharacterSet characterSetWithCharactersInString:@"\"\n\r"];
    NSString* srcStr = src.string();
    NSUInteger pos = 0;
    NSUInteger n = srcStr.length;
    NSRange r = [srcStr rangeOfCharacterFromSet:charSet options:0 range:NSMakeRange(pos, n - pos)];
    if (r.length == 0 && src.empty()) {
        return dest = src;
    }
    NSMutableString* dstStr = dest.mutableString();
    while (true) {
        if (r.length == 0) {
            [dstStr appendString:[srcStr substringWithRange:NSMakeRange(pos, n - pos)]];
            break;
        } else {
            if (r.location > pos) {
                [dstStr appendString:[srcStr substringWithRange:NSMakeRange(pos, r.location - pos)]];
            }
            
            for (NSUInteger i = 0; i < r.length; ++i) {
                unichar c = [srcStr characterAtIndex:r.location + i];
                [dstStr appendString:escapingString(c)];
            }
            
            pos = r.location + r.length;
            if (pos == n) break;
            r = [srcStr rangeOfCharacterFromSet:charSet options:0 range:NSMakeRange(pos, n - pos)];
        }
    }
    return dest;
}

String& StringTraits::appendEscape(String& dest, Char src) {
    [dest.mutableString() appendString:escapingString(src)];
    return dest;
}

String& StringTraits::appendEscape(String& dest, std::uint32_t src) {
    if (src == antlr3_defs::CharstreamEof) {
        [dest.mutableString() appendString:@"<EOF>"];
        return dest;
    }
    std::uint32_t maxChar = std::uint32_t(1) << (CHAR_BIT * sizeof(Char));
    if (src < maxChar) {
        return appendEscape(dest, (Char)src);
    }
    [dest.mutableString() appendFormat:@"\\u%04X", (unsigned)src];
    return dest;
}

} // namespace antlr3ex

namespace std {

std::ostream& operator<<(std::ostream& s, antlr3ex::CocoaString const & str) {
    return s << antlr3ex::CocoaStringTraits::toUTF8(str);
}

} // namespace std