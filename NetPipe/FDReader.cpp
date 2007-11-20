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
#include "FDReader.h"
#include "Service.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

namespace NetPipe {
    FDReader::FDReader(int inFD, size_t size, Service *serv, PipeManager *parent){
#ifdef _WIN32_WCE
	throw "_WIN32_WCE have no read(2) and close(2) system call?";
#else
	static char name[] = "FDReader";
	myName = name;
	buf = (char *)malloc(size);
	if(buf == NULL)
	    throw "no more memory";
	bufsize = size;
	fd = inFD;
	targetService = serv;
	pipeManager = parent;
#endif
    }

    FDReader::~FDReader(){
#ifdef _WIN32_WCE
	throw "_WIN32_WCE have no read(2) and close(2) system call?";
#else
	if(fd >= 0)
	    close(fd);
#endif
    }

    bool FDReader::onRecive(){
#ifdef _WIN32_WCE
	throw "_WIN32_WCE have no read(2) and close(2) system call?";
#else
	int ret = read(fd, buf, (size_t)bufsize);
	if(ret <= 0){
	    if(targetService != NULL)
		targetService->onEvent(pipeManager, NULL, NULL, Service::FD_DOWN, NULL, 0);
	    return false;
	}
	if(targetService != NULL)
	    targetService->onEvent(pipeManager, NULL, NULL, Service::FD_INPUT, buf, ret);
	return true;
#endif
    }

}; /* namespace NetPipe */
