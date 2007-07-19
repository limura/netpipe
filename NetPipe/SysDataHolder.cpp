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
 * $Id: Peer.h 16 2006-08-09 07:40:49Z uirou.j $
 */

#include "SysDataHolder.h"
#include "MainLoop.h"
#include "ServiceManager.h"
#include "net.h"
#include "upnp.h"
#include "FDWatcher.h"
#include "tools.h"

#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

namespace NetPipe {

    class SysDataSender : public Service {
	SysDataHolder *sdh;
    public:
	SysDataSender(SysDataHolder *parent){
	    sdh = parent;
	};

	void onEvent(PipeManager *pm, char *portName, char *arg, Service::EVENT_TYPE type, char *buf, size_t bufSize){
	    switch(type){
		case Service::RECV:
		    if(portName != NULL && strcmp(portName, "stdout")){
			char *data = sdh->getNowStatusXML();
			pm->write("stdout", data, strlen(data));
		    }
		    break;
		default:
		    break;
	    }
	};
    };

    class SysDataSenderCreator : public ServiceCreator {
    public:
	Service *createNewService(void *userData){
	    return new SysDataSender((SysDataHolder *)userData);
	}
    };


    SysDataHolder::SysDataHolder(){
	mainLoop = NULL;
	upnp = NULL;
    }

    SysDataHolder::~SysDataHolder(){
	if(upnp != NULL)
	    upnp_close(upnp);
    }

    SysDataHolder *SysDataHolder::getInstance(){
	static SysDataHolder singleton;
	return &singleton;
    }

    void SysDataHolder::setMainLoop(MainLoop *ml){
	mainLoop = ml;
	if(mainLoop == NULL)
	    return;
	upnp = upnp_listen_stream_with_local(6575);
	if(upnp == NULL)
	    return;
	if(upnp->sock >= 0){
#if 1
	    FDWatcher *watcher = FDWatcher::getInstance();
	    watcher->addAcceptQueue(upnp->sock, this);
#else
	    Acceptor *a = new Acceptor(this, upnp->sock);
	    if(a != NULL){
		ml->selector->add(a);
	    }
#endif
	    if(upnp->wan_ipaddr != NULL && upnp->wan_port != NULL){
		DPRINTF(10, ("SysData listning on %s:%s with UPNP\n", upnp->wan_ipaddr, upnp->wan_port));
	    }else if(upnp->local_ipaddr != NULL){
		DPRINTF(10, ("SysData listning on %s:%d without upnp\n", upnp->local_ipaddr, upnp->local_port));
	    }
	}
	ServiceManager *sm = new ServiceManager("SystemData");
	sm->addWritePort("stdout");
	sm->addServiceCreator(new SysDataSenderCreator(), this);
	mainLoop->addServiceManager(sm);
    }

    void SysDataHolder::onAccept(int fd, void *userData){
	char *data = getNowStatusXML();

	send(fd, data, (int)strlen(data), 0);
	closeSocket(fd);
    }
    void SysDataHolder::onRecive(int fd, char *buf, size_t size, void *userData){
	// nothing to do!
    }
    void SysDataHolder::onClose(int fd, void *userData){
	// nothing to do!
    }

    char *SysDataHolder::getNowStatusXML(){
	static char buf[40960];
	char *p = buf;
	p += sprintf(p, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n");
	p += sprintf(p, "MainLoop: %p\n\n", mainLoop);
	if(mainLoop == NULL)
	    return buf;
#ifdef HAVE_TIMEGETTIME
	DWORD dw = timeGetTime();
	p += sprintf(p, "timeGetTime: %f\n\n", dw / 100.0);
#else
	struct timeval tv;
	if(gettimeofday(&tv, NULL) == 0){
	    p += sprintf(p, "gettimeofday: %d.%06d\n\n", tv.tv_sec, tv.tv_usec);
	}
#endif
	p += sprintf(p, "upnp->sock: %d\n", mainLoop->upnp->sock);
	p += sprintf(p, "upnp->wan_ipaddr: %s\n", mainLoop->upnp->wan_ipaddr == NULL ? "(null)" : mainLoop->upnp->wan_ipaddr);
	p += sprintf(p, "upnp->wan_port: %s\n", mainLoop->upnp->wan_port == NULL ? "(null)" : mainLoop->upnp->wan_port);
	p += sprintf(p, "upnp->local_ipaddr: %s\n", mainLoop->upnp->local_ipaddr == NULL ? "(null)" : mainLoop->upnp->local_ipaddr);
	p += sprintf(p, "upnp->local_port: %d\n", mainLoop->upnp->local_port);
	p += sprintf(p, "upnp->IGD_control_url: %s\n", mainLoop->upnp->IGD_control_url == NULL ? "(null)" : mainLoop->upnp->IGD_control_url);
	p += sprintf(p, "upnp->IGD_service_type: %s\n\n", mainLoop->upnp->IGD_service_type == NULL ? "(null)" : mainLoop->upnp->IGD_service_type);
	p += sprintf(p, "serviceManagerList.size(): %d\n", mainLoop->serviceManagerList.size());
	int num = 1;
	for(MainLoop::ServiceManagerList::iterator i = mainLoop->serviceManagerList.begin(); i != mainLoop->serviceManagerList.end(); i++, num++){
	    p += sprintf(p, "%2d: serviceName: %s\n", num, (*i)->serviceName);
	    p += sprintf(p, "    serviceCreator: %p\n", (*i)->creator);
	    p += sprintf(p, "    creatorUserData: %p\n", (*i)->creatorUserData);
	    for(ServiceManager::ReadPortMap::iterator j = (*i)->readPortMap.begin(); j != (*i)->readPortMap.end(); j++){
		if(j->second == NULL)
		    continue;
		p += sprintf(p, "    ReadPort: %s  Description: %s\n", j->second->portName == NULL ? "(null)" : j->second->portName,
			     j->second->description == NULL ? "(null)" : j->second->description);
	    }
	    for(ServiceManager::WritePortMap::iterator j = (*i)->writePortMap.begin(); j != (*i)->writePortMap.end(); j++){
		if(j->second == NULL)
		    continue;
		p += sprintf(p, "    WritePort: %s  Description: %s\n", j->second->portName == NULL ? "(null)" : j->second->portName,
			     j->second->description == NULL ? "(null)" : j->second->description);
	    }
	    for(ServiceManager::FDInputMap::iterator j = (*i)->fdInputMap.begin(); j != (*i)->fdInputMap.end(); j++){
		if(j->second == NULL)
		    continue;
		p += sprintf(p, "    FDInput: %d  Description: %s\n", j->second->fd,
			     j->second->description == NULL ? "(null)" : j->second->description);
	    }
	}
	p += sprintf(p, "\nactivePipeMap.size() %d\n", mainLoop->activePipeMap.size());
	num = 1;
	for(MainLoop::string2ActivePipeMap::iterator i = mainLoop->activePipeMap.begin(); i != mainLoop->activePipeMap.end(); i++, num++){
	    p += sprintf(p, "%2d: key: %s\n", num, i->first.c_str());
	    p += sprintf(p, "    ActivePipe: %p\n", i->second);
	    if(i->second == NULL)
		continue;
	    p += sprintf(p, "    service: %p\n", i->second->service);
	    p += sprintf(p, "    pipeManager: %p\n", i->second->pipeManager);
	    p += sprintf(p, "    pipeManager->serviceName %s\n", i->second->pipeManager->serviceName == NULL ? "(null)" : i->second->pipeManager->serviceName);
	}

	FDWatcher *watcher = FDWatcher::getInstance();
	p += sprintf(p, "\nFDWatcher %p\n", watcher);
	if(watcher != NULL){
	    p += sprintf(p, "  acceptQueue.size(): %d\n", watcher->acceptQueue.size());
	    num = 1;
	    for(FDWatcher::AcceptQueue::iterator i = watcher->acceptQueue.begin();
		i != watcher->acceptQueue.end(); i++, num++){
		    p += sprintf(p, "     Reader No: %d (fd: %d) reciver: %p , userData: %p\n",
			num, i->first, i->second->reciver, i->second->userData);
	    }

	    p += sprintf(p, "  recvQueue.size(): %d\n", watcher->recvQueue.size());
	    num = 1;
	    for(FDWatcher::FD2RecvBuffer::iterator i = watcher->recvQueue.begin();
		i != watcher->recvQueue.end(); i++, num++){
		    p += sprintf(p, "     Reader No: %d (fd: %d) %sreciver: %p , userData: %p\n",
			num, i->first, i->second->staticReadSize ? "StaticRead " : "",
			i->second->reciver, i->second->userData);
	    }

	    p += sprintf(p, "  sendQueue.size(): %d\n", watcher->sendQueue.size());
	    num = 1;
	    for(FDWatcher::FD2SendBuffer::iterator i = watcher->sendQueue.begin();
		i != watcher->sendQueue.end(); i++, num++){
		    p += sprintf(p, "     Sender No: %d (fd: %d)\n", num, i->first);
		    for(FDWatcher::WatcherSendBufferList::iterator j = i->second.begin();
			j != i->second.end(); j++){
			    p += sprintf(p, "       size: %d\n", (*j)->size);
		    }
	    }

	    p += sprintf(p, "  eventQueue.size(): %d\n", watcher->eventQueue->size());
	    num = 1;
	    for(FDWatcher::EventQueue::iterator i = watcher->eventQueue->begin();
		i != watcher->eventQueue->end(); i++, num++){
		    p += sprintf(p, "     Event No: %d type: %d bufSize: %d\n",
			num, (*i)->getType(), (*i)->getBufSize());
	    }
	}
	return buf;
    }
};

