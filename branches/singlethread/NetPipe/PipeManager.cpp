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
 * $Id: PipeManager.cpp 86 2007-07-05 10:55:02Z  $
 */

#include "PipeManager.h"
#include "FDSelector.h"
#include "PortWriter.h"
#include "Service.h"
#include "FDReader.h"
#include "MainLoop.h"

#include "config.h"

#include <string.h>
#include <stdio.h>

//#ifdef HAVE_SYSLOG_H
//#include <syslog.h>
//#endif

namespace NetPipe {

    PipeManager::PipeManager(FDSelector *myselector, char *thisPipePath, char *thisServiceName, Service *serv, MainLoop *ml){
	if(myselector == NULL || thisPipePath == NULL || thisServiceName == NULL || serv == NULL)
	    throw "fatal error";
	selector = myselector;
	serviceName = strdup(thisServiceName);
	if(serviceName == NULL)
	    throw "no more memory";
	int thisPipePathLen = (int)strlen(thisPipePath);
	if(thisPipePath[thisPipePathLen - 1] == '\n'){
	    pipePath = strdup(thisPipePath);
	    if(pipePath == NULL)
		throw "no more memory.";
	}else{
	    pipePath = (char *)malloc(thisPipePathLen + 2);
	    if(pipePath == NULL)
		throw "no more memory";
	    memcpy(pipePath, thisPipePath, thisPipePathLen);
	    pipePath[thisPipePathLen] = '\n';
	    pipePath[thisPipePathLen + 1] = '\0';
	}
	service = serv;
	writePortMap.clear();
	inputSockNum = 0;
	parent = ml;
    }

    PipeManager::~PipeManager(){
printf("PipeManager DELETING ALL NEXT CONNECTIONS FOR %s\n", serviceName);
	for(string2WritePortMap::iterator i = writePortMap.begin(); i != writePortMap.end(); i++){
	    if(i->second == NULL)
		continue;
	    for(portServiceList::iterator j = i->second->nextPortService.begin();
		j != i->second->nextPortService.end(); j++){
		    if((*j)->sock >= 0 && (*j)->PortService != NULL || pipePath != NULL){
			StreamBuffer *buf = new StreamBuffer(256);
			int nextPortServiceLen = (int)strlen((*j)->PortService);
			int pipePathLen = (int)strlen(pipePath);
printf("PipeManager send CLOSE packet to fd:%d. size: %u, portService: %s\n",
(*j)->sock,
(uint32_t)(nextPortServiceLen + sizeof(char) + pipePathLen + sizeof(uint32_t) * 2),
(*j)->PortService
);

			buf->WriteUint32((uint32_t)(nextPortServiceLen + sizeof(char) + pipePathLen + sizeof(uint32_t) * 2));
			buf->WriteUint32(PORT_ACTION_CLOSE);
			buf->WriteUint32(nextPortServiceLen + sizeof(char) + pipePathLen);
			buf->WriteBinary((*j)->PortService, nextPortServiceLen);
			buf->WriteInt8('\n');
			buf->WriteBinary(pipePath, pipePathLen);
			PortWriter *pw = new PortWriter((*j)->sock, buf);
			pw->setLinkDwon();
			selector->add(pw);
			delete buf;
			
			//PortCloser *pc = new PortCloser((*j)->sock); // ���肪���a�I�ɗ��Ƃ��Ă���̂�҂B
			//selector->add(pc);
		    }
		    if((*j)->PortService != NULL)
			free((*j)->PortService);
	    }
	    if(i->second->buf != NULL){
printf(" DELETING StreamBuffer: %p\n", i->second->buf);
		delete i->second->buf;
	    }
	    i->second->buf = NULL;
	}

	if(pipePath != NULL)
	    free(pipePath);
	pipePath = NULL;
printf("DELETING Service: %p\n", service);
	if(service != NULL)
	    delete service;
	service = NULL;
    }

    void PipeManager::addWritePort(char *portName, int sock, char *nextPortService){
	if(portName == NULL || sock < 0)
	    return;
	WritePort *wp = writePortMap[portName];
	if(wp == NULL){
	    wp = new WritePort();
	    if(wp == NULL)
		throw "no more memory.";
	    wp->nextPortService.clear();
	    wp->buf = new StreamBuffer();
	    if(wp->buf == NULL)
		throw "no more memory.";
	    writePortMap[portName] = wp;
	}

	PortService *ps = new PortService();
	if(ps == NULL)
	    throw "no more memory";
	ps->sock = sock;
	ps->PortService = strdup(nextPortService);
	if(ps->PortService == NULL)
	    throw "no more memory";
	printf("  add nextPortService \"%s\" on myPort \"%s\" (fd: %d)\n", nextPortService, portName, sock);
	wp->nextPortService.push_back(ps);
    }

    StreamBuffer *PipeManager::getWriteBuffer(char *portName){
	WritePort *wp = writePortMap[portName];
	if(wp == NULL)
	    return NULL;
	return wp->buf;
    }

    bool PipeManager::write(char *portName, char *buf, size_t size){
	WritePort *wp = writePortMap[portName];
	if(wp == NULL || wp->buf == NULL)
	    return false;
	wp->buf->WriteBinary(buf, size);
	return true;
    }
    bool PipeManager::commit(char *portName){
	WritePort *wp = writePortMap[portName];
	if(wp == NULL || wp->buf == NULL){
printf("  can not SEND on PortName \"%s\" not found WritePortMap.\n", portName);
//#ifdef HAVE_SYSLOG_H
//syslog(LOG_LOCAL3, "can not SEND to \"%s\"  %s", portName, getGlobalIP4Addr());
//#endif
	    return false;
	}
	int pipePathLen = (int)strlen(pipePath);
printf(" send to next portService. number of %d portService I have.\n", wp->nextPortService.size());
	for(portServiceList::iterator i = wp->nextPortService.begin(); i != wp->nextPortService.end(); i++){
	    PortWriter *pw = new PortWriter((*i)->sock, wp->buf);
	    if(pw == NULL)
		throw "no more memory.";

	    StreamBuffer *headerBuf = pw->getHeaderBuf();
	    int nextPortServiceLen = (int)strlen((*i)->PortService);
	    int headerLength = nextPortServiceLen + 1 + pipePathLen;
printf("SEND TO NEXT PORT_SERVICE: %s (%d bytes data)\n", (*i)->PortService, wp->buf->getSize());
	    headerBuf->WriteUint32((uint32_t)(wp->buf->getSize() + headerLength + sizeof(uint32_t) * 2));
	    headerBuf->WriteUint32(PORT_ACTION_NORMAL);
	    headerBuf->WriteUint32(headerLength);
	    headerBuf->WriteBinary((*i)->PortService, nextPortServiceLen); // �󂯎��敶���񂻂̂���
	    headerBuf->WriteInt8('\n'); // ���s����؂蕶��
	    headerBuf->WriteBinary(pipePath, pipePathLen); // PipePath
	    selector->add(pw);
	}
	wp->buf->clearBuffer();
	return true;
    }

    void PipeManager::addReadFD(int fd, size_t bufsize){
	FDReader *fdr = new FDReader(fd, bufsize, service, this);
	if(fdr == NULL)
	    throw "no more memory";
	selector->add(fdr);
    }

    void PipeManager::inclimentInputPort(){
	inputSockNum++;
    }
    void PipeManager::declimentInputPort(char *portName){
	inputSockNum--;
	if(inputSockNum <= 0 && service != NULL){
	    inputSockNum = 0;
	    char *p = strchr(portName, ' ');
	    if(p != NULL){
		*p = '\0';
		p++;
	    }
	    service->onEvent(this, portName, p, Service::RECV_DOWN, NULL, 0);
	    if(p != NULL)
		p[-1] = ' ';
	}
    }

    void PipeManager::exit(){
	if(parent != NULL)
	    parent->deleteActivePipe(this);
    }
}; /* namespace NetPipe */