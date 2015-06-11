/// \file
/// Provides the debugging functions invoked by a recognizer
/// built using the debug generator mode of the antlr tool.
/// See antlr3debugeventlistener.h for documentation.
///

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

// Not everyone wishes to include the debugger stuff in their final deployment because
// it will then rely on being linked with the socket libraries. Hence if the programmer turns
// off the debugging, we do some dummy stuff that satifies compilers etc but means there is
// no debugger and no reliance on the socket librarires. If you set this flag, then using the -debug
// option to generate your code will produce code that just crashes, but then I presme you are smart
// enough to realize that building the libraries without debugger support means you can't call the
// debugger ;-)
//
#ifndef ANTLR3_NODEBUGGER

#include <antlr3/DebugEventSocketProxy.hpp>
#include <antlr3/BaseTreeAdaptor.hpp>
#include <antlr3/Exception.hpp>
#include <antlr3/Socket.hpp>
#include <antlr3/CharStream.hpp>

/// The version of the debugging protocol supported by the DebugEventSocketProxy.
/// ANTLR 3.1 is at protocol version 2
static int const PROTOCOL_VERSION = 2;

template<class T, class... Arg> void initOpaque(void*& store, Arg... arg)
{
    if (sizeof(T) > sizeof(store))
    {
        store = new T(std::move(arg)...);
    }
    else
    {
        new(&store) T(std::move(arg)...);
    }
}

template<class T> T& getOpaque(void*& store)
{
    if (sizeof(T) > sizeof(store))
    {
        return *static_cast<T*>(store);
    }
    else
    {
        return *reinterpret_cast<T*>(&store);
    }
}

template<class T, class... Arg> void freeOpaque(void*& store)
{
    if (sizeof(T) > sizeof(store))
    {
        delete static_cast<T*>(store);
    }
    else
    {
        reinterpret_cast<T*>(&store)->~T();
    }
}

template<class StringTraits>
antlr3<StringTraits>::DebugEventSocketProxy::DebugEventSocketProxy(TreeAdaptorPtr adaptor)
    : DebugEventSocketProxy(DEFAULT_DEBUGGER_PORT, adaptor)
{
}

template<class StringTraits>
antlr3<StringTraits>::DebugEventSocketProxy::DebugEventSocketProxy(std::uint32_t port, TreeAdaptorPtr adaptor)
    : port_(port)
    , socket_()
    , grammarFileName_()
    , initialized_(false)
    , tokenString_()
    , adaptor_(adaptor)
{
    initOpaque<SOCKET>(socket_, INVALID_SOCKET);
}

template<class StringTraits>
antlr3<StringTraits>::DebugEventSocketProxy::~DebugEventSocketProxy()
{
    SOCKET& s = getOpaque<SOCKET>(socket_);
    if(s != INVALID_SOCKET)
    {
        shutdown(s, SHUT_RDWR);
        ANTLR3_CLOSESOCKET(s);
        s = INVALID_SOCKET;
    }
    freeOpaque<SOCKET>(socket_);
}

template<class StringTraits>
typename antlr3<StringTraits>::String const &
    antlr3<StringTraits>::DebugEventSocketProxy::grammarFileName() const
{
    return grammarFileName_;
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::setGrammarFileName(String grammarFileName)
{
    grammarFileName_ = std::move(grammarFileName);
}

//--------------------------------------------------------------------------------
// Support functions for sending stuff over the socket interface
//
template<class StringTraits>
bool antlr3<StringTraits>::DebugEventSocketProxy::sockSend(const char * ptr, int len)
{
    SOCKET sock = getOpaque<SOCKET>(socket_);
    
    int sent	= 0;
        
    while(sent < len)
    {
        // Send as many bytes as we can
        //
        ssize_t thisSend = send(sock, ptr, len - sent, 0);

        // Check for errors and tell the user if we got one
        //
        if(thisSend == -1)
        {
            return false;
        }

        // Increment our offset by how many we were able to send
        //
        ptr	 += thisSend;
        sent += thisSend;
    }
    return true;
}

template<class StringTraits>
bool antlr3<StringTraits>::DebugEventSocketProxy::handshake()
{
    if(!initialized_)
    {
        // Windows requires us to initialize WinSock.
        //
#ifdef ANTLR3_WINDOWS
        {
            WORD		wVersionRequested;
            WSADATA		wsaData;
            int			err;			// Return code from WSAStartup

            // We must initialise the Windows socket system when the DLL is loaded.
            // We are asking for Winsock 1.1 or better as we don't need anything
            // too complicated for this.
            //
            wVersionRequested = MAKEWORD(1, 1);

            err = WSAStartup( wVersionRequested, &wsaData );

            if( err != 0 ) 
            {
                // Tell the user that we could not find a usable
                // WinSock DLL
                //
                return false;
            }
        }
#endif

        // Create the server socket, we are the server because we just wait until
        // a debugger connects to the port we are listening on.
        //
        SOCKET serverSocket	= socket(AF_INET, SOCK_STREAM, 0);
        if(serverSocket == INVALID_SOCKET)
        {
            return false;
        }

        // Set the listening port
        //
        ANTLR3_SOCKADDRT server;
        server.sin_port			= htons((unsigned short)port_);
        server.sin_family		= AF_INET;
        server.sin_addr.s_addr	= htonl(INADDR_ANY);

        // We could allow a rebind on the same addr/port pair I suppose, but
        // I imagine that most people will just want to start debugging one parser at once.
        // Maybe change this at some point, but rejecting the bind at this point will ensure
        // that people realize they have left something running in the background.
        //
        if(bind(serverSocket, (pANTLR3_SOCKADDRC)&server, sizeof(server)) == -1)
        {
            return false;
        }

        // We have bound the socket to the port and address so we now ask the TCP subsystem
        // to start listening on that address/port
        //
        if(listen(serverSocket, 1) == -1)
        {
            // Some error, just fail
            //
            return false;
        }

        // Now we can try to accept a connection on the port
        //
        ANTLR3_SOCKADDRT client;
        ANTLR3_SALENT sockaddr_len	= sizeof(client);
        SOCKET sock = accept(serverSocket, (pANTLR3_SOCKADDRC)&client, &sockaddr_len);

        // Having accepted a connection, we can stop listening and close down the socket
        //
        shutdown(serverSocket, 0x02);
        ANTLR3_CLOSESOCKET(serverSocket);

        if(sock == INVALID_SOCKET)
        {
            return false;
        }

        // Disable Nagle as this is essentially a chat exchange
        //
        const int optVal	= 1;
        setsockopt(sock, SOL_SOCKET, TCP_NODELAY, (char const*)&optVal, sizeof(optVal));
        getOpaque<SOCKET>(socket_) = sock;
    }

    // We now have a good socket connection with the debugging client, so we
    // send it the protocol version we are using and what the name of the grammar
    // is that we represent.
    //
    char message[256];
    sprintf(message, "ANTLR %d\n", PROTOCOL_VERSION);
    sockSend(message,(int)strlen(message));
    sprintf(message, "grammar \"%s\n", StringTraits::toUTF8(grammarFileName_).c_str());
    sockSend(message, (int)strlen(message));
    ack();

    initialized_ = true;
    return	true;
}

// Send the supplied text and wait for an ack from the client
template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::transmit(const char * ptr)
{
    sockSend(ptr, (int)strlen(ptr));
    ack();
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::ack()
{
    // Local buffer to read the next character in to
    //
    char	buffer;
    ssize_t rCount;

    // Ack terminates in a line feed, so we just wait for
    // one of those. Speed is not of the essence so we don't need
    // to buffer the input or anything.
    //
    SOCKET sock = getOpaque<SOCKET>(socket_);
    do
    {
        rCount = recv(sock, &buffer, 1, 0);
    }
    while(rCount == 1 && buffer != '\n');

    // If the socket ws closed on us, then we will get an error or
    // (with a graceful close), 0. We can assume the the debugger stopped for some reason
    // (such as Java crashing again). Therefore we just exit the program
    // completely if we don't get the terminating '\n' for the ack.
    //
    if(rCount != 1)
    {
        assert(false && !"Exiting debugger as remote client closed the socket\n");
        //ANTLR3_PRINTF("Received char count was %lld, and last char received was %02X\n", (long long)rCount, buffer);
        exit(0);
    }
}

// Given a buffer string and a source string, serialize the
// text, escaping any newlines and linefeeds. We have no need
// for speed here, this is the debugger.
//
template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::serializeText(std::string& buffer, String text)
{
    std::string text8 = StringTraits::toUTF8(text);

    // strings lead in with a "
    //
    buffer += "\t\"";

    // Now we replace linefeeds, newlines and the escape
    // leadin character '%' with their hex equivalents
    // prefixed by '%'
    //
    for(char c : text8)
    {
        switch(c)
        {
            case '\n':

                buffer += "%0A";
                break;

            case '\r':
            
                buffer += "%0D";
                break;

            case '%':
            
                buffer += "%25";
                break;

                // Other characters: The Song Remains the Same.
                //
            default:
                    
                buffer += c;
                break;
        }
    }
}

// Given a token, create a stringified version of it, in the supplied
// buffer. We create a string for this in the debug 'object', if there 
// is not one there already, and then reuse it here if asked to do this
// again.
//
template<class StringTraits>
std::string antlr3<StringTraits>::DebugEventSocketProxy::serializeToken(CommonTokenPtr t)
{
    // Empty string
    //
    tokenString_.clear();

    // Now we serialize the elements of the token.Note that the debugger only
    // uses 32 bits.
    //
    tokenString_ += std::to_string((std::int32_t)t->tokenIndex());
    tokenString_ += '\t';
    tokenString_ += std::to_string((std::int32_t)t->type());
    tokenString_ += '\t';
    tokenString_ += std::to_string((std::int32_t)t->channel());
    tokenString_ += '\t';
    Location loc = t->inputStream()->location(t->startIndex());
    tokenString_ += std::to_string((std::int32_t)loc.line());
    tokenString_ += '\t';
    tokenString_ += std::to_string((std::int32_t)loc.charPositionInLine());

    // Now send the text that the token represents.
    //
    serializeText(tokenString_, t->text());

    // Finally, as the debugger is a Java program it will expect to get UTF-8
    // encoded strings. We don't use UTF-8 internally to the C runtime, so we 
    // must force encode it. We have a method to do this in the string class, but
    // it returns malloc space that we must free afterwards.
    //
    return tokenString_;
}

// Given a tree node, create a stringified version of it in the supplied
// buffer.
//
template<class StringTraits>
std::string antlr3<StringTraits>::DebugEventSocketProxy::serializeNode(ItemPtr node)
{
    // Empty string
    //
    tokenString_.clear();

    // Protect against bugs/errors etc
    //
    if(node == NULL)
    {
        return tokenString_;
    }

    // Now we serialize the elements of the node.Note that the debugger only
    // uses 32 bits.
    //
    tokenString_ += '\t';

    // Adaptor ID
    //
    tokenString_ += std::to_string(adaptor_->getUniqueID(node));
    tokenString_ += '\t';

    // Type of the current token (which may be imaginary)
    //
    tokenString_ += std::to_string(adaptor_->getType(node));

    // See if we have an actual token or just an imaginary
    //
    CommonTokenPtr token = adaptor_->getToken(node);

    tokenString_ += '\t';
    if(token != NULL)
    {
        // Real token
        //
        Location loc = token->startLocation();
        tokenString_ += std::to_string((std::int32_t)loc.line());
        tokenString_ += ' ';
        tokenString_ += std::to_string((std::int32_t)loc.charPositionInLine());
    }
    else
    {
        // Imaginary tokens have no location
        //
        tokenString_ += std::to_string(-1);
        tokenString_ += '\t';
        tokenString_ += std::to_string(-1);
    }

    // Start Index of the node
    //
    tokenString_ += '\t';
    tokenString_ += std::to_string((std::uint32_t)(adaptor_->getTokenStartIndex(node)));

    // Now send the text that the node represents.
    //
    serializeText(tokenString_, adaptor_->getText(node));

    // Finally, as the debugger is a Java program it will expect to get UTF-8
    // encoded strings. We don't use UTF-8 internally to the C runtime, so we 
    // must force encode it. We have a method to do this in the string class, but
    // there is no utf8 string implementation as of yet
    //
    return tokenString_;
}

//------------------------------------------------------------------------------------------------------------------
// EVENTS
//
template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::enterRule(const char * grammarFileName, const char * ruleName)
{
    // Create the message (speed is not of the essence)
    //
    char buffer[512];
    sprintf(buffer, "enterRule\t%s\t%s\n", grammarFileName, ruleName);
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::enterAlt(int alt)
{
    char	buffer[512];

    // Create the message (speed is not of the essence)
    //
    sprintf(buffer, "enterAlt\t%d\n", alt);
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::exitRule(const char * grammarFileName, const char * ruleName)
{
    char	buffer[512];

    // Create the message (speed is not of the essence)
    //
    sprintf(buffer, "exitRule\t%s\t%s\n", grammarFileName, ruleName);
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::enterSubRule(int decisionNumber)
{
    char	buffer[512];

    // Create the message (speed is not of the essence)
    //
    sprintf(buffer, "enterSubRule\t%d\n", decisionNumber);
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::exitSubRule(int decisionNumber)
{
    char	buffer[512];

    // Create the message (speed is not of the essence)
    //
    sprintf(buffer, "exitSubRule\t%d\n", decisionNumber);
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::enterDecision(int decisionNumber)
{
    char	buffer[512];

    // Create the message (speed is not of the essence)
    //
    sprintf(buffer, "enterDecision\t%d\n", decisionNumber);
    transmit(buffer);

}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::exitDecision(int decisionNumber)
{
    char	buffer[512];

    // Create the message (speed is not of the essence)
    //
    sprintf(buffer, "exitDecision\t%d\n", decisionNumber);
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::consumeToken(CommonTokenPtr t)
{
    std::string msg;

    // Create the serialized token
    //
    msg = serializeToken(t);

    // Insert the debug event indicator
    //
    msg.insert(0, "consumeToken\t");

    msg += '\n';

    // Transmit the message and wait for ack
    //
    transmit(msg.c_str());
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::consumeHiddenToken(CommonTokenPtr t)
{
    std::string msg;

    // Create the serialized token
    //
    msg = serializeToken(t);

    // Insert the debug event indicator
    //
    msg.insert(0, "consumeHiddenToken\t");

    msg += '\n';

    // Transmit the message and wait for ack
    //
    transmit(msg.c_str());
}

// Looking at the next token event.
//
template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::LT(int i, CommonTokenPtr t)
{
    std::string msg;

    if(t != NULL)
    {
        // Create the serialized token
        //
        msg = serializeToken(t);

        // Insert the index parameter
        //
        msg.insert(0, "\t");
        msg.insert(0, std::to_string(i));

        // Insert the debug event indicator
        //
        msg.insert(0, "LT\t");

        msg += '\n';

        // Transmit the message and wait for ack
        //
        transmit(msg.c_str());
    }
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::mark(int marker)
{
    char buffer[128];

    sprintf(buffer, "mark\t%d\n", (marker & 0xFFFFFFFF));

    // Transmit the message and wait for ack
    //
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::rewind(int marker)
{
    char buffer[128];

    sprintf(buffer, "rewind\t%d\n", (marker & 0xFFFFFFFF));

    // Transmit the message and wait for ack
    //
    transmit(buffer);

}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::beginBacktrack(int level)
{
    char buffer[128];

    sprintf(buffer, "beginBacktrack\t%d\n", (level & 0xFFFFFFFF));

    // Transmit the message and wait for ack
    //
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::endBacktrack(int level, bool successful)
{
    char buffer[128];

    sprintf(buffer, "endBacktrack\t%d\t%d\n", level, successful);

    // Transmit the message and wait for ack
    //
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::location(int line, int pos)
{
    char buffer[128];

    sprintf(buffer, "location\t%d\t%d\n", line, pos);

    // Transmit the message and wait for ack
    //
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::recognitionException(ExceptionPtr e)
{
    char buffer[256];

    sprintf(buffer, "exception\t%s\t%d\t%d\t%d\n",
        e->name(), (int)(e->index), (int)e->location.line(), (int)e->location.charPositionInLine()
    );

    // Transmit the message and wait for ack
    //
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::beginResync()
{
    transmit("beginResync\n");
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::endResync()
{
    transmit("endResync\n");
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::semanticPredicate(bool result, const char * predicate)
{
    if(predicate == NULL)
    {
        return;
    }

    std::vector<char> bufferImpl;
    bufferImpl.resize(64 + 2 * strlen(predicate));
    char* const buffer = &bufferImpl[0];
    char* out = buffer + sprintf(buffer, "semanticPredicate\t%s\t", result ? "true" : "false");
    while(*predicate != '\0')
    {
        switch(*predicate)
        {
            case '\n':
                *out++	= '%';
                *out++	= '0';
                *out++	= 'A';
                break;
            case '\r':
                *out++	= '%';
                *out++	= '0';
                *out++	= 'D';
                break;
            case '%':
                *out++	= '%';
                *out++	= '0';
                *out++	= 'D';
                break;
            default:
                *out++	= *predicate;
                break;
        }

        predicate++;
    }
    *out++	= '\n';
    *out++	= '\0';

    // Send it and wait for the ack
    //
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::commence()
{
    // Nothing to see here
    //
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::terminate()
{
    // Terminate sequence
    //
    sockSend("terminate\n", 10);		// Send out the command
}

//----------------------------------------------------------------
// Tree parsing events
//
template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::consumeNode(ItemPtr t)
{
    std::string	buffer = serializeNode(t);

    // Now prepend the command
    //
    buffer.insert(0, "consumeNode\t");
    buffer += '\n';

    // Send to the debugger and wait for the ack
    //
    transmit(tokenString_.c_str());
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::LTT(int i, ItemPtr t)
{
    std::string	buffer = serializeNode(t);

    // Now prepend the command
    //
    buffer.insert(0, "\t");
    buffer.insert(0, std::to_string(i).c_str());
    buffer.insert(0, "LN\t");
    buffer += '\n';

    // Send to the debugger and wait for the ack
    //
    transmit(tokenString_.c_str());
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::nilNode(ItemPtr t)
{
    char buffer[128];
    sprintf(buffer, "nilNode\t%d\n", adaptor_->getUniqueID(t));
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::createNode(ItemPtr t)
{
    // Empty string
    //
    tokenString_ = "createNodeFromTokenElements ";

    // Now we serialize the elements of the node.Note that the debugger only
    // uses 32 bits.
    //
    // Adaptor ID
    //
    tokenString_ += std::to_string(adaptor_->getUniqueID(t));
    tokenString_ += '\t';

    // Type of the current token (which may be imaginary)
    //
    tokenString_ += std::to_string(adaptor_->getType(t));

    // The text that this node represents
    //
    serializeText(tokenString_, adaptor_->getText(t));
    tokenString_ += '\n';

    // Finally, as the debugger is a Java program it will expect to get UTF-8
    // encoded strings. We don't use UTF-8 internally to the C runtime, so we 
    // must force encode it. We have a method to do this in the string class, but
    // there is no utf8 string implementation as of yet
    //
    transmit(tokenString_.c_str());
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::errorNode(ItemPtr t)
{
    // Empty string
    //
    tokenString_ = "errorNode\t";

    // Now we serialize the elements of the node.Note that the debugger only
    // uses 32 bits.
    //
    // Adaptor ID
    //
    tokenString_ += std::to_string(adaptor_->getUniqueID(t));
    tokenString_ += '\t';

    // Type of the current token (which is an error)
    //
    tokenString_ += std::to_string(TokenInvalid);

    // The text that this node represents
    //
    serializeText(tokenString_, adaptor_->getText(t));
    tokenString_ += '\n';

    // Finally, as the debugger is a Java program it will expect to get UTF-8
    // encoded strings. We don't use UTF-8 internally to the C runtime, so we 
    // must force encode it. We have a method to do this in the string class, but
    // there is no utf8 string implementation as of yet
    //
    transmit(tokenString_.c_str());
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::createNodeTok(ItemPtr node, CommonTokenPtr token)
{
    char	buffer[128];

    sprintf(buffer, "createNode\t%d\t%d\n",	adaptor_->getUniqueID(node), (std::uint32_t)token->tokenIndex());

    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::becomeRoot(ItemPtr newRoot, ItemPtr oldRoot)
{
    char	buffer[128];

    sprintf(buffer, "becomeRoot\t%d\t%d\n",	adaptor_->getUniqueID(newRoot),
                                            adaptor_->getUniqueID(oldRoot)
                                            );
    transmit(buffer);
}


template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::addChild(ItemPtr root, ItemPtr child)
{
    char	buffer[128];

    sprintf(buffer, "addChild\t%d\t%d\n",	adaptor_->getUniqueID(root),
                                            adaptor_->getUniqueID(child)
                                            );
    transmit(buffer);
}

template<class StringTraits>
void antlr3<StringTraits>::DebugEventSocketProxy::setTokenBoundaries(ItemPtr t, Index tokenStartIndex, Index tokenStopIndex)
{
    char	buffer[128];

    sprintf(buffer, "becomeRoot\t%d\t%d\t%d\n",	adaptor_->getUniqueID(t),
                                                (std::uint32_t)tokenStartIndex,
                                                (std::uint32_t)tokenStopIndex
                                            );
    transmit(buffer);
}

#endif // ANTLR3_NODEBUGGER

