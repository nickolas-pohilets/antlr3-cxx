#ifndef _ANTLR3_VISITOR_HPP
#define _ANTLR3_VISITOR_HPP

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

namespace antlr3 {

template<class... T> class TypeList;

template<> class TypeList<> {};

template<class T, class... Y> class TypeList<T, Y...>
{
public:
    typedef T head;
    typedef TypeList<Y...> tail;
};

template<template<class> class Transform, class List> class TransformTypeList;
template<template<class> class Transform, class... T> class TransformTypeList<Transform, TypeList<T...> >
{
public:
    typedef TypeList< typename Transform<T>::type... > type;
};

template<class List> class VisitorBuilder;

template<> class VisitorBuilder< TypeList<> >
{
public:
    class Visitor
    {
    public:
        void visit() {}
    };

    template<class V>
    class DefVisitorImpl : public V
    {
    public:
        virtual void visitDefault() = 0;
    };

    typedef DefVisitorImpl<Visitor> DefVisitor;
};

template<class T, class... Y> class VisitorBuilder< TypeList<T, Y...> >
{
public:
    typedef VisitorBuilder< TypeList<Y...> > Base;
    typedef typename Base::Visitor BaseVisitor;

    class Visitor : public BaseVisitor
    {
    public:
        using BaseVisitor::visit;

        virtual void visit(T) = 0;
    };

    template<class V>
    class DefVisitorImpl : public Base::template DefVisitorImpl<V>
    {
    public:
        using Base::template DefVisitor<V>::visit;

        virtual void visit(T) { this->visitDefault(); }
    };

    typedef DefVisitorImpl<Visitor> DefVisitor;
};

} // namespace antlr3

#endif // _ANTLR3_VISITOR_HPP
