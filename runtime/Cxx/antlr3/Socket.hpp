/**
 * \file
 * Encapsulates platform-dependent socket API
 */

#ifndef _ANTLR3_SOCKET_HPP
#define _ANTLR3_SOCKET_HPP

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

#include <antlr3/Defs.hpp>

#if defined(_WIN64) || defined(_WIN32)

//--------------[ WINDOWS ]-------------------------

#define ANTLR3_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define	WIN32_LEAN_AND_MEAN
#endif

/* Allow VC 8 (vs2005) and above to use 'secure' versions of various functions such as sprintf
 */
#ifndef	_CRT_SECURE_NO_DEPRECATE
#define	_CRT_SECURE_NO_DEPRECATE
#endif

#include <windows.h>
#include <stdlib.h>
#include <winsock.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

typedef	int	ANTLR3_SALENT;								// Type used for size of accept structure
typedef struct sockaddr_in	ANTLR3_SOCKADDRT, * pANTLR3_SOCKADDRT;	// Type used for socket address declaration
typedef struct sockaddr		ANTLR3_SOCKADDRC, * pANTLR3_SOCKADDRC;	// Type used for cast on accept()

#define	ANTLR3_CLOSESOCKET	closesocket

//--------------------------------------------------

#else

//----------------[ UNIX ]--------------------------

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>

#define _stat   stat

// SOCKET not defined on Unix
typedef	int	SOCKET;

#define INVALID_SOCKET      ((SOCKET)-1)
#define	ANTLR3_CLOSESOCKET  close

#ifdef __hpux

// HPUX is always different usually for no good reason. Tru64 should have kicked it
// into touch and everyone knows it ;-)
//
typedef struct sockaddr_in ANTLR3_SOCKADDRT, * pANTLR3_SOCKADDRT;	// Type used for socket address declaration
typedef void * pANTLR3_SOCKADDRC;				// Type used for cast on accept()
typedef int ANTLR3_SALENT;

#else

# if defined(_AIX) || __GNUC__ > 3

typedef  socklen_t   ANTLR3_SALENT;

# else

typedef size_t	ANTLR3_SALENT;

# endif

typedef struct sockaddr_in   ANTLR3_SOCKADDRT, * pANTLR3_SOCKADDRT;	// Type used for socket address declaration
typedef struct sockaddr	* pANTLR3_SOCKADDRC;                    // Type used for cast on accept()

#endif

//--------------------------------------------------

#endif

// Cleanup global namespace polluted by dirty plain C headers

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef index
#undef index
#endif

#endif // _ANTLR3_SOCKET_HPP
