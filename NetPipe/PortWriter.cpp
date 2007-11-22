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
#include "tools.h"
#include "PortWriter.h"
#include "StreamBuffer.h"
#include "Service.h"
#include "PipeManager.h"
#include "net.h"
#include <stdlib.h>
#include <stdio.h>

namespace NetPipe {
    PortWriter::PortWriter(int sock, StreamBuffer *origBuf, PipeManager *pm, Service *service, char *portName){
	pipeManager = pm;
	targetService = service;
	if(this->portName != NULL){
	    this->portName = strdup(portName);
	}else{
	    this->portName = NULL;
	}
	buf = NULL;
	static char name[] = "PortWriter";
	myName = name;
	fd = sock;
	if(origBuf != NULL){
	    buf = origBuf->dup();
	    if(buf == NULL)
		throw "no more memory";
	}
//printf("PortWriter::PortWriter() fd: %d buffer: %p (%d bytes)\n", fd,
//       buf, buf != NULL ? buf->getSize() : 0);
	headerBuf = NULL;
	linkDown = false;
    }

    PortWriter::~PortWriter(){
	if(buf != NULL)
	    delete buf;
	if(headerBuf != NULL)
	    delete headerBuf;
	if(portName != NULL)
	    free(portName);
	if(linkDown && fd >= 0){
	    closeSocket(fd);
	}
    }

    StreamBuffer *PortWriter::getHeaderBuf(){
	if(headerBuf == NULL)
	    headerBuf = new StreamBuffer();
	return headerBuf;
    }

    bool PortWriter::onWrite(){
DPRINTF(100, ("PortWriter::onWrite() fd: %d\n", fd));
DPRINTF(100, ("  header: %p (%d bytes), buffer: %p (%d bytes)\n",
        headerBuf, headerBuf != NULL ? headerBuf->getSize() : 0,
        buf, buf != NULL ? buf->getSize() : 0));
        try {
	    if(headerBuf != NULL){
		if(headerBuf->socketWrite(fd) == false){
		    delete headerBuf;
		    headerBuf = NULL;
		    if(buf == NULL)
			return false;
		}
		return true;
	    }
	    if(buf != NULL && buf->socketWrite(fd) == false){
		delete buf;
		buf = NULL;
	    }
	}catch(StreamBufferSendError){
	    if(targetService != NULL){
		targetService->onEvent(pipeManager, portName, NULL, Service::SEND_DOWN, NULL, 0);
	    }
	    if(headerBuf != NULL){
		delete headerBuf;
		headerBuf = NULL;
	    }
	    if(buf != NULL){
		delete buf;
		buf = NULL;
	    }
	    throw "SendError";
	    //return false;
	}
	if(headerBuf != NULL || buf != NULL)
	    return true;
	return false; // ����I�������E���Ă��炤
    }
    void PortWriter::setLinkDown(){
	linkDown = true;
    }

    void PortWriter::deleteService(Service *service){
	if(targetService == service)
	    targetService = NULL;
    }
};