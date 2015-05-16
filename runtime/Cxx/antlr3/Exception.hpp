/** \file
 *  Contains the definition of a basic ANTLR3 exception structure created
 *  by a recognizer when errors are found/predicted.
 */
#ifndef _ANTLR3_EXCEPTION_HPP
#define _ANTLR3_EXCEPTION_HPP

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
#include <antlr3/Visitor.hpp>
#include <antlr3/Bitset.hpp>
#include <antlr3/Location.hpp>

namespace antlr3 {

/** Base structure for an ANTLR3 exception tracker
 */
class Exception
{
    template<class T> struct AddConstPtr { typedef T const * type; };
public:
    typedef ::antlr3::TypeList<
        class MismatchedTokenException,
        class UnwantedTokenException,
        class MissingTokenException,
        class MismatchedSetException,
        class MismatchedRangeException,
        class NoViableAltException,
        class EarlyExitException,
        class FailedPredicateException,
        class RewriteEarlyExitException
    > TypeList;

    typedef TransformTypeList<AddConstPtr, TypeList>::type PtrTypeList;
    typedef ::antlr3::VisitorBuilder<PtrTypeList>::Visitor Visitor;
    typedef ::antlr3::VisitorBuilder<PtrTypeList>::DefVisitor DefVisitor;

    /** Name of the file/input source for reporting. Note that this may be NULL!!
     */
    String streamName;

    /** Indicates the index of the 'token' we were looking at when the
     *  exception occurred.
     */
    Index index;

    /** Indicates what the current character/token/tree was when the error occurred.
     */
    ItemPtr item;

    /** Track the location at which the error occurred in case this is
     *  generated from a lexer.  We need to track this since the
     *  unexpected char doesn't carry the line info.
     */
    Location location;

    /** Pointer to the input stream that this exception occurred in.
     */
    IntStreamPtr input;

    Exception();
    virtual ~Exception();

    virtual char const * name() const = 0;
    virtual std::unique_ptr<Exception> clone() const = 0;
    virtual void accept(Visitor& v) const = 0;
protected:
    template<class T>
    static std::unique_ptr<T> makeClone(T const * src)
    {
        return std::unique_ptr<T>(new T(*src));
    }
};

#define CommonExceptionStuff \
public: \
    virtual std::unique_ptr<Exception> clone() const override \
    { return makeClone(this); } \
    virtual void accept(Visitor& v) const override \
    { v.visit(this); }

/// Indicates that the recognizer was expecting one token and found a
/// a different one.
class MismatchedTokenException : public Exception
{
    CommonExceptionStuff
public:
     /// Indicates the token we were expecting to see next when the error occurred
    std::uint32_t const expecting;

    MismatchedTokenException(std::uint32_t expectingType)
        : Exception()
        , expecting(expectingType)
    {}

    virtual char const * name() const override
    {
        return "org.antlr.runtime.MismatchedTokenException";
    }
};

/// Custom case of the MismatchedTokenException
/// Recognnizer recovers by removing this token.
class UnwantedTokenException : public MismatchedTokenException
{
    CommonExceptionStuff
public:
    UnwantedTokenException(std::uint32_t expectingType)
        : MismatchedTokenException(expectingType)
    {}
    
    virtual char const * name() const override
    {
        return "org.antlr.runtime.UnwantedTokenException";
    }
};

/// Custom case of the MismatchedTokenException
/// Recognnizer recovers by inserting imaginary expected token.
class MissingTokenException : public MismatchedTokenException
{
    CommonExceptionStuff
public:
    MissingTokenException(std::uint32_t expectingType)
        : MismatchedTokenException(expectingType)
    {}
    
    virtual char const * name() const override
    {
        return "org.antlr.runtime.MissingTokenException";
    }
};

/// Recognizer could not find a valid alternative from the input
class NoViableAltException : public Exception
{
    CommonExceptionStuff
public:
    NoViableAltException(ConstString descr, std::uint32_t decision, std::uint32_t s)
        : Exception()
        , decisionDescription(descr)
        , decisionNum(decision)
        , state(s)
    {}
    
    ConstString const decisionDescription;

    /// Decision number.
    std::uint32_t const decisionNum;

    /// State
    std::uint32_t const state;
    
    virtual char const * name() const override
    {
        return "org.antlr.runtime.NoViableAltException";
    }
};

/// Character in a set was not found
class MismatchedSetException : public Exception
{
    CommonExceptionStuff
public:
    /// For compatibility with MismatchedSetException(nullptr)
    MismatchedSetException()
        : Exception()
        , expectingSet()
    {}

    MismatchedSetException(Bitset expecting)
        : Exception()
        , expectingSet(std::move(expecting))
    {}
    
    /// Indicates a set of tokens that we were expecting to see one of when the
    /// error occurred. It is a following bitset list, so you can use load it and use ->toIntList() on it
    /// to generate an array of integer tokens that it represents.
    Bitset expectingSet;

    virtual char const * name() const override
    {
        return "org.antlr.runtime.MismatchedSetException";
    }
};

class MismatchedRangeException : public Exception
{
    CommonExceptionStuff
public:
    MismatchedRangeException(std::uint32_t lo, std::uint32_t hi)
        : Exception()
        , low(lo), high(hi)
    {}
    
    std::uint32_t const low;
    std::uint32_t const high;

    virtual char const * name() const override
    {
        return "org.antlr.runtime.MismatchedRangeException";
    }
};

/// A rule predicting at least n elements found less than that,
/// such as: WS: " "+;
class EarlyExitException : public Exception
{
    CommonExceptionStuff
public:
    virtual char const * name() const override
    {
        return "org.antlr.runtime.EarlyExitException";
    }
};

/// Input was recognizer successfully, but validating predicate failed.
class FailedPredicateException : public Exception
{
    CommonExceptionStuff
public:
    FailedPredicateException(ConstString rule, ConstString predicate)
        : Exception()
        , ruleName(rule)
        , predicateText(predicate)
    {}

    ConstString const ruleName;
    ConstString const predicateText;

    virtual char const * name() const override
    {
        return "org.antlr.runtime.FailedPredicateException";
    }
};

class RewriteCardinalityException : public Exception
{
public:
    RewriteCardinalityException(ConstString elementDescr = nullptr)
        : Exception()
        , elementDescription(elementDescr)
    {}
    
    ConstString const elementDescription;
};

class RewriteEarlyExitException : public RewriteCardinalityException
{
    CommonExceptionStuff
public:
    using RewriteCardinalityException::RewriteCardinalityException;
    
    virtual char const * name() const override
    {
        return "org.antlr.runtime.tree.RewriteEarlyExitException";
    }
};

// Not implemented:
// "org.antlr.runtime.MismatchedTreeNodeException";

#undef CommonExceptionStuff

} // namespace antlr3

#endif
