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

/* Definitions
 */
#ifndef _ANTLR3_LOCATION
#define _ANTLR3_LOCATION

#include <antlr3/Defs.hpp>
#include <antlr3/String.hpp>

namespace antlr3 {
    
class Location
{
public:
    Location()
        : line_(0)
        , charPositionInLine_(0)
    {}
    
    Location(std::uint32_t line, std::uint32_t charPositionInLine)
        : line_(line)
        , charPositionInLine_(charPositionInLine)
    {}
    
    bool isValid() const {
        return line_ > 0 || charPositionInLine_ > 0;
    }
    
    /// Returns 1-based line number of the current position in the input stream.
    std::uint32_t line() const { return line_; }
    
    /// Return 1-based offset of the current position from the begining of the line in the input stream.
    std::uint32_t charPositionInLine() const { return charPositionInLine_; }
    
    bool operator==(Location other) const {
        return line_ == other.line_ && charPositionInLine_ == other.charPositionInLine_;
    }
    bool operator!=(Location other) const {
        return !(*this == other);
    }
    bool operator<(Location other) const {
        return line_ < other.line_ || (line_ == other.line_ && charPositionInLine_ < other.charPositionInLine_);
    }
    bool operator>(Location other) const {
        return other < *this;
    }
    bool operator<=(Location other) const {
        return !(other < *this);
    }
    bool operator>=(Location other) const {
        return !(*this < other);
    }
private:
    std::uint32_t line_;
    std::uint32_t charPositionInLine_;
};
    
inline String& operator+=(String& s, Location location) {
    s += toString(location.line());
    s += ':';
    s += toString(location.charPositionInLine());
    return s;
}
    
inline String operator+(String s, Location location) {
    s += location;
    return std::move(s);
}

inline String operator+(Location location, String const & s) {
    String s0;
    s0 += location;
    s0 += s;
    return std::move(s0);
}

inline std::ostream& operator<<(std::ostream& s, Location loc) {
    return s << loc.line() << ':' << loc.charPositionInLine();
}

class LocationSource
{
public:
    virtual ~LocationSource() {}
    
    virtual String sourceName() = 0;
    virtual Location location(Index) = 0;
    
    /// Returns subtring from start to stop in UTF-8 encoding.
    /// @todo: Clarify interpretation of the stop: is it over-the-end pointer or pointer to first code unit of the last character
    virtual String substr(Index start, Index stop) = 0;
};

} // namespace antlr3


#endif // _ANTLR3_LOCATION
