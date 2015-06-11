#ifndef _ANTLR3_STRING_LITERAL_HPP
#define _ANTLR3_STRING_LITERAL_HPP

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

namespace antlr3ex {

template<class T>
class StringLiteralRef {
public:
    typedef T                                       value_type;
    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef T const *                               pointer;
    typedef T const *                               const_pointer;
    typedef const_pointer                           iterator;
    typedef const_pointer                           const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    
    StringLiteralRef(T const * data, size_t len) : data_(data), len_(len) {}
    
    operator std::basic_string<T>() const {
        return std::basic_string<T>(begin(), end());
    }
    
    bool empty() const { return len_ == 0; }
    size_t size() const { return len_; }
    
    const_iterator begin() const { return data_; }
    const_iterator end() const { return data_ + len_; }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    
    T const * data() const { return data_; }
private:
    T const * data_;
    size_t len_;
};

template<class T>
std::basic_string<T> operator+(StringLiteralRef<T> s1, StringLiteralRef<T> s2) {
    std::basic_string<T> retVal;
    retVal.reserve(s1.size() + s2.size());
    retVal += s1;
    retVal += s2;
    return std::move(retVal);
}

template<class T>
std::basic_string<T>& operator+=(std::basic_string<T>& s1, StringLiteralRef<T> s2) {
    return s1.append(s2.begin(), s2.end());
}

template<class T>
std::basic_string<T> operator+(std::basic_string<T> s1, StringLiteralRef<T> s2) {
    s1 += s2;
    return std::move(s1);
}

template<class T>
std::basic_string<T> operator+(StringLiteralRef<T> s1, std::basic_string<T> const & s2) {
    std::basic_string<T> retVal;
    retVal.reserve(s1.size() + s2.size());
    retVal += s1;
    retVal += s2;
    return std::move(retVal);
}

template<class T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, StringLiteralRef<T> s) {
    return os.write(s.data(), s.size());
}

} // namespace antlr3ex

#endif // _ANTLR3_STRING_LITERAL_HPP
