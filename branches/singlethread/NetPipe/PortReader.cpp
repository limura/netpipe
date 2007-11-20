/*
 * Copyright (c) 2007 IIMURA Takuji. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * $Id$
 */

#include "config.h"
#include "MainLoop.h"
#include "PortReader.h"
#include "net.h"

#include <stdio.h>

namespace NetPipe {
    PortReader::PortReader(MainLoop *Parent, int sock){
	if(Parent == NULL || sock < 0)
	    throw "invalid data";
	fd = sock;
	static char name[] = "PortReader";
	myName = name;
	parent = Parent;
	buf = (char *)malloc(1024);
	if(buf == NULL)
	    throw "no more memory.";
	p = buf;
	bufSize = 1024;
	readingBytes = -1;
#if 0 // non blocking I/O: ¡Œã‚Ì‚½‚ß‚É
#ifdef HAVE_FCNTL
	fcntl(fd, F_SETFL, O_NONBLOCK);
#endif
#ifdef HAVE_IOCTLSOCKET
	{
	    unsigned long argp = 1;
	    ioctlsocket(fd, FIONBIO, &argp);
	}
#endif
#endif
    }

    PortReader::~PortReader(){
	if(fd >= 0)
	    closeSocket(fd);
	fd = -1;
	parent = NULL;
    }

    bool PortReader::onRecive(){
	if(fd < 0)
	    return false;
	if(readingBytes < 0){ // initial state. reading data bytes
	    uint32_t ui32;
	    if(recv(fd, (char *)&ui32, sizeof(ui32), 0) != sizeof(ui32)){
		return false;
	    }
	    readingBytes = ntohl(ui32);
printf("PortReader readingBytes: %d\n", readingBytes);
	    return true;
	}
	if((bufSize - (p - buf)) < readingBytes){
	    if(readingBytes > PORTREADER_MAX_READ_SIZE){
printf("portReader size too big. disconnect.\n");
		return false;
	    }
	    size_t newSize = bufSize - (p - buf);
	    newSize = (readingBytes / 4096 + 1) * 4096;
printf("PortReader reallocing: %d to %d\n", bufSize, newSize);
	    char *tmp_buf = (char *)realloc(buf, newSize);
	    if(tmp_buf == NULL)
		throw "no more memory.";
	    bufSize = newSize;
	    p = tmp_buf + (p - buf);
	    buf = tmp_buf;
	}
	int len = recv(fd, p, readingBytes, 0);
	if(len <= 0){
	    throw "RecvError";
	    return false;
	}
	p += len;
	readingBytes -= len;
printf("PortReader readingBytes: %d (-= %d)\n", readingBytes, len);
	if(readingBytes <= 0){
	    if(parent->onPortRecive(buf, p - buf) == false)
		return false;
	    readingBytes = -1;
printf("PortReader readingBytes: %d (= -1)\n", readingBytes);
	    p = buf;
	}
	return true;
    }
}; /* namespace NetPipe */
