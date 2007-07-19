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

#include "FDWatcher.h"
#include "tools.h"
#include "net.h"

#include <errno.h>

//extern "C" {
//extern int errno;
//}

namespace NetPipe {

    FDEvent::FDEvent(FDEvent::EventType eventType, int FD, char *buffer, size_t bufSize, void *data){
	type = eventType;
	fd = FD;
	buf = buffer;
	size = bufSize;
	userData = data;
	reciver = NULL;
    }
    FDEvent::~FDEvent(){
	if(buf != NULL)
	    free(buf);
    }
    int FDEvent::getSocket(){
	return fd;
    }
    char *FDEvent::getBuf(){
	return buf;
    }
    size_t FDEvent::getBufSize(){
	return size;
    }
    FDEvent::EventType FDEvent::getType(){
	return type;
    }
    void *FDEvent::getUserData(){
	return userData;
    }
    void FDEvent::setReciver(FDReciver *r){
	reciver = r;
    }
    void FDEvent::invoke(){
	switch(type){
	    case FDEvent::ACCEPT:
		if(reciver != NULL)
		    reciver->onAccept(fd, userData);
		break;
	    case FDEvent::RECV:
		if(reciver != NULL)
		    reciver->onRecive(fd, buf, size, userData);
		break;
	    case FDEvent::CLOSE:
		if(reciver != NULL)
		    reciver->onClose(fd, userData);
		break;
	    default:
		break;
	}
    }

    WatcherRecvBuffer::WatcherRecvBuffer(void *data){
	staticReadSize = false;
	readSize = -1;
	buf = p = NULL;
	bufSize = 0;
	userData = data;
    }
    WatcherRecvBuffer::~WatcherRecvBuffer(){
	if(buf != NULL)
	    free(buf);
	buf = p = NULL;
	bufSize = 0;
    }
    void WatcherRecvBuffer::grow(size_t targetSize){
	if(buf == NULL){
	    p = buf = (char *)malloc(targetSize);
	    if(buf == NULL)
		throw "no more memory.";
	    bufSize = targetSize;
	    return;
	}
	if(bufSize >= targetSize)
	    return;
	char *tmpBuf = (char *)realloc(buf, targetSize);
	if(tmpBuf == NULL)
	    throw "no more memory";
	p = tmpBuf + (p - buf);
	buf = tmpBuf;
	bufSize = targetSize;
    }
    void WatcherRecvBuffer::refresh(){
	p = buf;
	readSize = -1;
    }
    bool WatcherRecvBuffer::socketRecv(int sock){
	int recvRet;
	if(sock < 0)
	    return false;
	if(readSize < 0 && staticReadSize == false){
	    recvRet = recv(sock, (char *)&readSize, sizeof(readSize), 0);
	    if(recvRet != sizeof(readSize) || readSize <= 0)
		return false;
	    grow(readSize + 1);
	    buf[readSize] = '\0'; // ˆê‰ž '\0' terminate ‚µ‚Ä‚¨‚­B‚È‚ñ‚ÄS—D‚µ‚¢‰´I
	}else{
	    recvRet = recv(sock, p, readSize - (p - buf), 0);
	    if(recvRet <= 0)
		return false;
	    if(staticReadSize == false)
		readSize -= recvRet;
	    p += recvRet;
	}
	return true;
    }
    void WatcherRecvBuffer::releaseBuffer(){
	buf = p = NULL;
	if(staticReadSize){
	    grow(bufSize);
	}else{
	    bufSize = 0;
	    readSize = -1;
	}
    }

    WatcherSendBuffer::WatcherSendBuffer(char *buffer, size_t bufferSize, void *data){
	userData = data;
	p = buf = buffer;
	if(buffer == NULL){
	    bufSize = 0;
	    size = -1;
	}else{
	    size = (BufSize_t)bufferSize;
	    bufSize = bufferSize;
	}
    }
    WatcherSendBuffer::~WatcherSendBuffer(){
	if(buf != NULL)
	    free(buf);
    }
    WatcherSendBuffer::SendReturn WatcherSendBuffer::socketSend(int fd){
	if(buf == NULL)
	    return WatcherSendBuffer::END_OF_FILE;
	int sendRet;
	if(size >= 0){
	    sendRet = send(fd, (char *)&size, sizeof(size), 0);
	    if(sendRet != sizeof(size))
		return WatcherSendBuffer::END_OF_FILE;
	    size = -1;
	}else{
	    sendRet = send(fd, p, bufSize - (p - buf), 0);
	    if(sendRet == 0)
		return WatcherSendBuffer::END_OF_FILE;
	    if(sendRet < 0)
		return WatcherSendBuffer::SEND_ERROR;
	    p += sendRet;
	    bufSize -= sendRet;
	    if(bufSize == 0)
		return WatcherSendBuffer::END_DATA;
	}
	return WatcherSendBuffer::HAVE_MORE_DATA;
    }

    FDWatcher::FDWatcher(){
	threadStarted = false;
	maxFDNum = -1;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	acceptQueue.clear();
	sendQueue.clear();
	recvQueue.clear();
	eventQueue = new EventQueue();
	if(eventQueue == NULL)
	    throw "no more memory";
	eventQueue->clear();

	thread_mutex_init(&fdsetMutex, NULL);
	thread_mutex_init(&eventQueueMutex, NULL);
	thread_mutex_init(&acceptQueueMutex, NULL);
	thread_mutex_init(&sendQueueMutex, NULL);
	thread_mutex_init(&recvQueueMutex, NULL);
	thread_cond_init(&eventQueueCond);
    }
    FDWatcher *FDWatcher::getInstance(){
	static FDWatcher singleton;
	return &singleton;
    }

    FDWatcher::~FDWatcher(){
	AcceptQueue::iterator aqi;
	FD2SendBuffer::iterator fd2si;
	FD2RecvBuffer::iterator fd2ri;
	if(threadStarted == true){
	    thread_cancel(&selectThreadID);
	}
	for(aqi = acceptQueue.begin(); aqi != acceptQueue.end(); aqi++){
	    if(aqi->second != NULL && aqi->first >= 0)
		closeSocket(aqi->first);
	    if(aqi->second != NULL)
		delete aqi->second;
	}

	for(fd2si == sendQueue.begin(); fd2si != sendQueue.end(); fd2si++){
	    closeSocket(fd2si->first);
	    while(fd2si->second.size() > 0){
		WatcherSendBuffer *wsb = fd2si->second.front();
		if(wsb != NULL)
		    delete wsb;
		fd2si->second.pop_front();
	    }
	}
	for(fd2ri == recvQueue.begin(); fd2ri != recvQueue.end(); fd2ri++){
	    closeSocket(fd2ri->first);
	    if(fd2ri->second != NULL)
		delete fd2ri->second;
	}
	thread_mutex_destroy(&eventQueueMutex);
	thread_mutex_destroy(&acceptQueueMutex);
	thread_mutex_destroy(&sendQueueMutex);
	thread_mutex_destroy(&recvQueueMutex);
	thread_cond_destroy(&eventQueueCond);
    }

    void FDWatcher::threadFunc(void *userData){
	FDWatcher *parent = (FDWatcher *)userData;
	fd_set rfds, wfds;
	AcceptQueue::iterator aqi;
	FD2SendBuffer::iterator fd2si;
	FD2RecvBuffer::iterator fd2ri;
	while(1){
	    thread_mutex_lock(&parent->fdsetMutex);
	    memcpy(&rfds, &parent->read_fds, sizeof(rfds));
	    memcpy(&wfds, &parent->write_fds, sizeof(wfds));
	    thread_mutex_unlock(&parent->fdsetMutex);
	    int selectRet = select(parent->maxFDNum + 1, &rfds, &wfds, NULL, NULL);
	    if(selectRet <= 0){
#ifdef EINTR
		if(errno == EINTR)
		    continue;
#endif
		return;
	    }
	    thread_mutex_lock(&parent->acceptQueueMutex);
	    for(aqi = parent->acceptQueue.begin(); aqi != parent->acceptQueue.end() && selectRet > 0; aqi++){
		if(FD_ISSET(aqi->first, &rfds)){
		    selectRet--;
		    int fd = accept(aqi->first, NULL, NULL);
		    if(fd < 0)
			break;
		    FDEvent *acceptEvent = new FDEvent(FDEvent::ACCEPT, fd, NULL, 0,
			aqi->second != NULL ? aqi->second->userData : NULL);
		    if(acceptEvent == NULL)
			break;
		    if(aqi->second != NULL)
			acceptEvent->setReciver(aqi->second->reciver);
		    parent->addNewEvent(acceptEvent);
		}
	    }
	    thread_mutex_unlock(&parent->acceptQueueMutex);

	    thread_mutex_lock(&parent->recvQueueMutex);
	    for(fd2ri = parent->recvQueue.begin(); fd2ri != parent->recvQueue.end() && selectRet > 0; fd2ri++){
		if(FD_ISSET(fd2ri->first, &rfds)){
		    selectRet--;
		    if(fd2ri->second == NULL)
			continue;
		    if(fd2ri->second->socketRecv(fd2ri->first) == false){
			parent->deleteRecvQueue(fd2ri);
			break;
		    }
		    if(fd2ri->second->staticReadSize == true || fd2ri->second->readSize == 0){
			FDEvent *recvEvent = new FDEvent(FDEvent::RECV, fd2ri->first,
			    fd2ri->second->buf, fd2ri->second->p - fd2ri->second->buf, fd2ri->second->userData);
			if(recvEvent == NULL)
			    break;
			recvEvent->setReciver(fd2ri->second->reciver);
			fd2ri->second->releaseBuffer();
			parent->addNewEvent(recvEvent);
		    }
		}
	    }
	    thread_mutex_unlock(&parent->recvQueueMutex);

	    thread_mutex_lock(&parent->sendQueueMutex);
	    for(fd2si = parent->sendQueue.begin(); fd2si != parent->sendQueue.end() && selectRet > 0; fd2si++){
		if(FD_ISSET(fd2si->first, &wfds)){
		    selectRet--;
		    if(fd2si->second.size() <= 0){
			thread_mutex_lock(&parent->fdsetMutex);
			FD_CLR(fd2si->first, &parent->write_fds);
			thread_mutex_unlock(&parent->fdsetMutex);
			continue;
		    }
		    WatcherSendBuffer *wsb = fd2si->second.front();
		    if(wsb == NULL){
			thread_mutex_unlock(&parent->sendQueueMutex);
			parent->closeSendSocket(fd2si->first);
			continue;
		    }
		    switch(wsb->socketSend(fd2si->first)){
			case WatcherSendBuffer::END_DATA:
			    delete wsb;
			    fd2si->second.pop_front();
			    if(fd2si->second.size() <= 0){
				thread_mutex_lock(&parent->fdsetMutex);
				FD_CLR(fd2si->first, &parent->write_fds);
				thread_mutex_unlock(&parent->fdsetMutex);
			    }
			    break;
			case WatcherSendBuffer::END_OF_FILE:
			case WatcherSendBuffer::SEND_ERROR:
			    thread_mutex_unlock(&parent->sendQueueMutex);
			    parent->closeSendSocket(fd2si->first);
			    return;
			    break;
			case WatcherSendBuffer::HAVE_MORE_DATA:
			    break;
			default:
			    break;
		    }
		}
	    }
	    thread_mutex_unlock(&parent->sendQueueMutex);
	}
    }

    void FDWatcher::addNewEvent(FDEvent *ev){
	thread_mutex_lock(&eventQueueMutex);
	eventQueue->push_back(ev);
	thread_mutex_unlock(&eventQueueMutex);
	thread_cond_signal(&eventQueueCond);
    }

    FDEvent *FDWatcher::nextEvent(int usec){
	FDEvent *ev = NULL;
	thread_mutex_lock(&eventQueueMutex);
	if(thread_cond_timedwait(&eventQueueCond, &eventQueueMutex, usec) != TRUE){
	    thread_mutex_unlock(&eventQueueMutex);
	    return NULL;
	}
	if(!eventQueue->empty()){
	    ev = eventQueue->front();
	    eventQueue->pop_front();
	}
	thread_mutex_unlock(&eventQueueMutex);
	return ev;
    }

    void FDWatcher::invokeEvents(int usec){
	thread_mutex_lock(&eventQueueMutex);
	if(thread_cond_timedwait(&eventQueueCond, &eventQueueMutex, usec) != TRUE){
	    thread_mutex_unlock(&eventQueueMutex);
	    return;
	}
	EventQueue *newQueue = NULL;
	if(!eventQueue->empty()){
	    newQueue = eventQueue;
	    eventQueue = new EventQueue();
	    if(eventQueue == NULL)
		throw "no more memory";
	}
	thread_mutex_unlock(&eventQueueMutex);
	if(newQueue != NULL){
	    while(newQueue->size() > 0){
		FDEvent *ev = newQueue->front();
		newQueue->pop_front();
		ev->invoke();
		delete ev;
	    }
	    delete newQueue;
	}
    }

    void FDWatcher::deleteAcceptQueue(AcceptQueue::iterator i){
	int fd = i->first;
	if(i->second != NULL)
	    delete i->second;
	acceptQueue.erase(i);
	if(acceptQueue.count(fd) <= 0){
	    thread_mutex_lock(&fdsetMutex);
	    FD_CLR(fd, &read_fds);
	    thread_mutex_unlock(&fdsetMutex);
	}
    }
    void FDWatcher::deleteRecvQueue(FD2RecvBuffer::iterator i){
	int fd = i->first;
	if(i->second != NULL)
	    delete i->second;
	recvQueue.erase(i);
	if(recvQueue.count(fd) <= 0){
	    thread_mutex_lock(&fdsetMutex);
	    FD_CLR(fd, &read_fds);
	    thread_mutex_unlock(&fdsetMutex);
	}
    }

    /*
    void FDWatcher::deleteSendQueue(FD2SendBuffer::iterator i){
	int fd = i->first;
	if(i->second != NULL)
	    delete i->second;
	sendQueue.erase(i);
	if(sendQueue.count(fd) <= 0){
	    thread_mutex_lock(&fdsetMutex);
	    FD_CLR(fd, &write_fds);
	    thread_mutex_unlock(&fdsetMutex);
	}
    }
    */

    void FDWatcher::setReadBytes(int fd, size_t size){
	thread_mutex_lock(&recvQueueMutex);
	WatcherRecvBuffer *wrb = recvQueue[fd];
	if(wrb != NULL)
	    wrb->readSize = size;
	thread_mutex_unlock(&recvQueueMutex);
    }

    bool FDWatcher::startWatcher(){
	if(threadStarted)
	    return true;
	if(thread_create(&selectThreadID, (thread_func)threadFunc, this) == FALSE)
	    return false;

	threadStarted = true;
	return true;
    }

    void FDWatcher::addAcceptQueue(int fd, FDReciver *reciver, void *userData){
	WatcherAcceptBuffer *wab = new WatcherAcceptBuffer();
	if(wab == NULL)
	    throw "no more memory";
	wab->reciver = reciver;
	wab->userData = userData;
	thread_mutex_lock(&acceptQueueMutex);
	if(acceptQueue[fd] != NULL)
	    delete acceptQueue[fd];
	acceptQueue[fd] = wab;
	thread_mutex_unlock(&acceptQueueMutex);
	thread_mutex_lock(&fdsetMutex);
	FD_SET(fd, &read_fds);
	thread_mutex_unlock(&fdsetMutex);
    }
    void FDWatcher::addReciveQueue(int fd, FDReciver *reciver, void *userData){
	WatcherRecvBuffer *wrb = new WatcherRecvBuffer();
	if(wrb == NULL)
	    throw "no more memory";
	wrb->reciver = reciver;
	wrb->userData = userData;
	thread_mutex_lock(&recvQueueMutex);
	if(recvQueue[fd] != NULL)
	    delete recvQueue[fd];
	recvQueue[fd] = wrb; 
	thread_mutex_unlock(&recvQueueMutex);
	thread_mutex_lock(&fdsetMutex);
	FD_SET(fd, &read_fds);
	thread_mutex_unlock(&fdsetMutex);
    }
    void FDWatcher::addNoSizedReciveQueue(int fd, size_t bufSize, FDReciver *reciver, void *userData){
	WatcherRecvBuffer *wrb = new WatcherRecvBuffer();
	if(wrb == NULL)
	    throw "no more memory";
	wrb->reciver = reciver;
	wrb->userData = userData;
	wrb->staticReadSize = true;
	wrb->grow(bufSize);
	wrb->readSize = bufSize;
	thread_mutex_lock(&recvQueueMutex);
	if(recvQueue[fd] != NULL)
	    delete recvQueue[fd];
	recvQueue[fd] = wrb; 
	thread_mutex_unlock(&recvQueueMutex);
	thread_mutex_lock(&fdsetMutex);
	FD_SET(fd, &read_fds);
	thread_mutex_unlock(&fdsetMutex);
    }

    void FDWatcher::addSendQueue(int fd, StreamBuffer *buf){
	addSendQueue(fd, buf->getBuffer(), buf->getSize());
	buf->releaseBuffer();
	delete buf;
    }

    void FDWatcher::addSendQueue(int fd, char *buf, size_t size){
	WatcherSendBuffer *wsb = new WatcherSendBuffer(buf, size);
	if(wsb == NULL)
	    throw "no more memory";
	thread_mutex_lock(&sendQueueMutex);
	sendQueue[fd].push_back(wsb);
	thread_mutex_unlock(&sendQueueMutex);
	thread_mutex_lock(&fdsetMutex);
	FD_SET(fd, &write_fds);
	thread_mutex_unlock(&fdsetMutex);
    }
    void FDWatcher::addNoSizedSendQueue(int fd, char *buf, size_t size){
	WatcherSendBuffer *wsb = new WatcherSendBuffer(buf, size);
	if(wsb == NULL)
	    throw "no more memory";
	wsb->size = -1;
	thread_mutex_lock(&sendQueueMutex);
	sendQueue[fd].push_back(wsb);
	thread_mutex_unlock(&sendQueueMutex);
	thread_mutex_lock(&fdsetMutex);
	FD_SET(fd, &write_fds);
	thread_mutex_unlock(&fdsetMutex);
    }

    void FDWatcher::closeAcceptSocket(int fd){
	FDReciver *reciver = NULL;
	void *userData = NULL;
	thread_mutex_lock(&acceptQueueMutex);

	for(AcceptQueue::iterator i = acceptQueue.begin(); i != acceptQueue.end(); i++){
	    if(i->first == fd){
		deleteAcceptQueue(i);
		reciver = i->second->reciver;
		userData = i->second->userData;
	    }
	}
	thread_mutex_unlock(&acceptQueueMutex);
	closeSocket(fd);

	FDEvent *eofEvent = new FDEvent(FDEvent::CLOSE, fd, NULL, 0, userData);
	if(eofEvent == NULL)
	    return;
	eofEvent->setReciver(reciver);
	addNewEvent(eofEvent);
  }
    void FDWatcher::closeReciveSocket(int fd){
	FDReciver *reciver = NULL;
	void *userData = NULL;
	thread_mutex_lock(&recvQueueMutex);
	for(FD2RecvBuffer::iterator i = recvQueue.begin(); i != recvQueue.end(); i++){
	    if(i->first == fd){
		deleteRecvQueue(i);
		reciver = i->second->reciver;
		userData = i->second->userData;
	    }
	}
	thread_mutex_unlock(&recvQueueMutex);
	closeSocket(fd);

	FDEvent *eofEvent = new FDEvent(FDEvent::CLOSE, fd, NULL, 0, userData);
	if(eofEvent == NULL)
	    return;
	eofEvent->setReciver(reciver);
	addNewEvent(eofEvent);
    }
    void FDWatcher::closeSendSocket(int fd){
	thread_mutex_lock(&sendQueueMutex);
	for(FD2SendBuffer::iterator i = sendQueue.begin(); i != sendQueue.end(); i++){
	    if(i->first == fd){
		while(i->second.size() > 0){
		    WatcherSendBuffer *wsb = i->second.front();
		    if(wsb != NULL)
			delete wsb;
		    i->second.pop_front();
		}
	    }
	}
	thread_mutex_unlock(&sendQueueMutex);
	closeSocket(fd);
  
    	FDEvent *eofEvent = new FDEvent(FDEvent::CLOSE, fd, NULL, 0, NULL);
	if(eofEvent == NULL)
	    return;
	addNewEvent(eofEvent);
    }
};