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
 * $Id: VersionChecker.cpp 95 2007-07-06 07:08:18Z  $
 */

#include "VersionChecker.h"
#include "MainLoop.h"
#include "tools.h"

namespace NetPipe {
    VersionChecker::VersionChecker(MainLoop *ml, int FD){
	parent = ml;
	fd = FD;
	static char name[] = "VersionChecker";
	myName = name;
    }
    VersionChecker::~VersionChecker(){
    }

    bool VersionChecker::onRecive(){
	char buf[1024];
	int len = recv(fd, buf, strlen(NETPIPE_HELLO_STRING), 0);
	if(len <= 0){
	    closeSocket(fd);
	    return false;
	}
	buf[len] = '\0';
	if(len == (int)strlen(NETPIPE_HELLO_STRING) &&
	    strncmp(buf, NETPIPE_HELLO_STRING, len) == 0){
		parent->onAcceptValidConnection(fd);
	    return false;
	}
	DPRINTF(4, ("version check error. close socket.\n"));
	closeSocket(fd);
	return false;
    }
};