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
 * $Id: MainLoop.cpp 93 2007-07-06 03:33:52Z  $
 */

#include "config.h"
#include "MainLoop.h"
#include "FDSelector.h"
#include "PipeManager.h"
#include "ServiceDB.h"
#include "net.h"
#include "ServiceManager.h"
#include "PortReader.h"
#include "Acceptor.h"
#include "Service.h"
#include "SysDataHolder.h"
#include "VersionChecker.h"
#include "PortWriter.h"

#include "tools.h"
#include <errno.h>
extern int errno;
#include <stdio.h>
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

namespace NetPipe {

    MainLoop::MainLoop(){
	selector = new FDSelector();
	if(selector == NULL)
	    throw "FDSelector initialize failed.";
	serviceManagerList.clear();
	activePipeMap.clear();
	upnp = NULL;

#if 0
	dummy_count = 0;
	char buf[1024];
	sprintf(buf, "MainLoop-onPortRecive_IP%s_PID%d_No%d.txt", getGlobalIP4Addr(), getpid(), dummy_count);
	log_fp = fopen(buf, "wb");
	if(log_fp != NULL)
	    setbuf(log_fp, NULL);
#else
	log_fp = NULL;
#endif
    }

    MainLoop::~MainLoop(){
	delete selector;
	selector = NULL;
	if(upnp != NULL)
	    upnp_close(upnp);
	if(log_fp != NULL)
	    fclose(log_fp);
    }

    void MainLoop::addServiceManager(ServiceManager *sm){
	serviceManagerList.push_back(sm);
    }

    void MainLoop::run(int usec){
	if(upnp == NULL)
	    openAcceptPort();
	if(usec <= 0){
	    while(selector->run(0)){
		;
	    }
	}else{
	    selector->run(usec);
	}
    }

    void MainLoop::onAccept(int sock){
	VersionChecker *vc = new VersionChecker(this, sock);
	if(vc == NULL)
	    throw "no more memory.";
	selector->add(vc);
    }

    void MainLoop::onAcceptValidConnection(int sock){
	PortReader *pr = new PortReader(this, sock);
	if(pr == NULL)
	    throw "no more memory.";
	selector->add(pr);
    }

    bool MainLoop::onPortRecive(char *buf, size_t size){
	if(log_fp != NULL){
//	    SysDataHolder *sdr = SysDataHolder::getInstance();
//	    char *p = sdr->getNowStatusXML();
//	    fprintf(log_fp, "\n\n---COUNT: %d---\n", dummy_count++);
//	    fprintf(log_fp, "%s", p);
	}
printf("onPortRecive() got %d bytes data\n", size);
 
	uint32_t action;
	if(size < sizeof(action))
	    return false;
	action = ntohl(*((uint32_t *)buf));
	size -= sizeof(action);
	buf += sizeof(action);

if(action != 0)
    printf("onPortRecive: action: %d\n", action);

	uint32_t strLen;
	if(size < sizeof(strLen))
	    return false;
	strLen = ntohl(*((uint32_t *)buf));
	size -= sizeof(strLen);
	buf += sizeof(strLen);
//printf("onPortRecive: headerLen: %d\n", size);

	if(size < strLen)
	    return false;
	char *stringBuf = buf;
	stringBuf[strLen - 1] = '\0'; // 最後は '\n' のはずなのでそいつを上書きして '\0' Terminate させてもらう
	buf += strLen;
	size -= strLen;
//printf("onPortRecive: header: %s", stringBuf);

	char *p;
	char *inputPort;
	inputPort = stringBuf;
	p = strchr(inputPort, '\n');
	if(p == NULL)
	    return false;
	*p = '\0';
	char *pipePath = p + 1;
	p = strchr(inputPort, ';');
	if(p == NULL)
	    return false;
	*p = '\0';
	p++;
	char *serviceName = p;
printf("onPortRecive: serviceName: %s\n", serviceName);

	char *inputArg;
	inputArg = strchr(serviceName, ' ');
	if(inputArg != NULL){
	    inputArg++;
	}

//	ActivePipe *ap = activePipeMap[pipePath];
	ActivePipe *ap = activePipeMap[serviceName];
	if(ap == NULL){ // サービスの新しいインスタンスを作らないといかん
//printf("check action: %d <-> %d\n", action, PipeManager::PORT_ACTION_CLOSE);
	    if(action == PipeManager::PORT_ACTION_CLOSE)
		return false;
//printf("check passed.\n");
	    for(ServiceManagerList::iterator i = serviceManagerList.begin(); i != serviceManagerList.end(); i++){
		char *name = (*i)->getServiceName();
		if(inputArg != NULL)
		    inputArg[-1] = '\0';
		if(name != NULL && *name == *serviceName && strcmp(name, serviceName) == 0){
		    ap = new ActivePipe();
		    if(ap == NULL)
			throw "no more memory";
		    ap->service = (*i)->createNewService();
		    if(ap->service == NULL)
			throw "can not create new service instance.";
		    ap->pipeManager = new PipeManager(selector, pipePath, serviceName, ap->service, this);
		    if(ap->pipeManager == NULL)
			throw "no more memory";
		    (*i)->registReadFDInput(ap->pipeManager);
		    if(inputArg != NULL)
			inputArg[-1] = ' ';
//		    activePipeMap[pipePath] = ap;
		    activePipeMap[serviceName] = ap;
		    connectToNextPort(ap->pipeManager, serviceName, pipePath);
		    break;
		}
	    }
	    if(ap == NULL){
//printf("ap == NULL\n");
		return false; // ap が作られていなかった == 次の奴へ connect できなかった、ならあきらめる。
	    }
	}
	if(ap != NULL && action == PipeManager::PORT_ACTION_CLOSE){
//printf("onPortRecive: port decliment. service: %s\n", serviceName);
	    ap->pipeManager->declimentInputPort(inputPort);
	    return false;
	}
//printf("onPortRecive: port incliment.\n");
	ap->pipeManager->inclimentInputPort();
	if(inputArg != NULL)
	    inputArg[-1] = '\0';
	if(ap != NULL && ap->service != NULL)
	    ap->service->onEvent(ap->pipeManager, inputPort, inputArg, Service::RECV, buf, size);
	return true;
    }

    void MainLoop::connectToNextPort(PipeManager *pm, char *serviceName, char *pipePath){
	if(pm == NULL || serviceName == NULL || pipePath == NULL)
	    return;
	int serviceNameLen = (int)strlen(serviceName);
	ServiceDB *db = ServiceDB::getInstance();
	char *p, *pp = pipePath;
	char *savePtr;
//printf("connect to next port.\nMyServiceName: %s\ntargetPath:\n%s\n", serviceName, pipePath);
	while((p = strtok_r(pp, "\n", &savePtr)) != NULL){
	    pp = NULL;
	    char *outPort, *nextPortService;
	    if(strncmp(p, serviceName, serviceNameLen) == 0){
		char outPortBuf[1024];
		p = strchr(p, ';');
		if(p == NULL){
printf("PipePath check error (3). in MainLoop::ConnecToNextPort()\n");
		    continue;
		}
		p++;
		outPort = p;
		p = strchr(p, ';');
		if(p == NULL){
printf("PipePath check error (1). in MainLoop::ConnecToNextPort()\n");
		    continue;
		}
		if((int)(p - outPort) > (int)(sizeof(outPortBuf) - 1)){
printf("PipePath check error (2). in MainLoop::ConnecToNextPort()\n");
		    continue;
		}
		memcpy(outPortBuf, outPort, p - outPort);
		outPortBuf[p - outPort] = '\0';
		outPort = outPortBuf;
		p++;
		nextPortService = p;
		p = strchr(p, ';');
		if(p == NULL){
printf("PipePath check error (3). in MainLoop::ConnecToNextPort()\n");
		    continue;
		}
		p++;
		char *nextService = p;
		char *nextIPaddr = db->QueryIPHostName(nextService);
		char *nextPortName = db->QueryTCPPortName(nextService);
		if(nextIPaddr == NULL || nextPortName == NULL){
printf("can not query nextService \"%s\". ip/port: %s:%s\n", nextService, nextIPaddr, nextPortName);
		    continue;
		}

		// UPNP をしている本人がUPNPで開いたWAN_PORTにつなぎに行くと
		// 失敗するルータがあるので、local に開いたportに繋ぐ
		char portBuf[64];
		if(upnp != NULL && upnp->wan_port != NULL && upnp->wan_ipaddr != NULL &&
		    strcmp(nextIPaddr, upnp->wan_ipaddr) == 0 &&
		    strcmp(nextPortName, upnp->wan_port) == 0){
		    nextIPaddr = upnp->local_ipaddr;
		    sprintf(portBuf, "%d", upnp->local_port);
		    nextPortName = portBuf;
		}
		errno = 0;
		int fd = connect_stream(nextIPaddr, nextPortName);
		if(fd < 0){
		    printf(" can not connect to nextService: %s:%s (%s)\n", nextIPaddr, nextPortName, strerror(errno));
		    continue;
		}
		StreamBuffer *buf = new StreamBuffer(32);
		if(buf == NULL){
		    closeSocket(fd);
printf("no more memory. in MainLoop::ConnecToNextPort() by create StreamBuffer\n");
		    continue;
		}
		buf->WriteBinary(NETPIPE_HELLO_STRING, strlen(NETPIPE_HELLO_STRING));
		PortWriter *pw = new PortWriter(fd, buf);
		if(pw == NULL){
printf("no more memory. in MainLoop::ConnecToNextPort() by create PortWriter\n");
		    closeSocket(fd);
		    delete buf;
		    continue;
		}
		selector->add(pw);
printf("nextPortService %s (%s:%s) connected.\npm->addWritePort outPort: %s, fd: %d\n",
       nextPortService, nextIPaddr, nextPortName, outPort, fd);
		pm->addWritePort(outPort, fd, nextPortService);
	    }
	}
    }

    void MainLoop::openAcceptPort(){
	int port = 8471;
	int fd = -1;
	char portBuf[6];
	char *portStr = NULL;
	char *IPaddr = NULL;

	portStr = NULL;
	upnp = upnp_listen_stream_with_local(port);
	if(upnp == NULL)
	    throw "can not listen accept port.";
	if(upnp->sock < 0){
	    upnp_close(upnp);
	    throw "can not listen accept port.";
	}

	fd = upnp->sock;
	if(upnp->wan_port != NULL){
	    portStr = upnp->wan_port;
	}else{
	    sprintf(portBuf, "%d", upnp->local_port);
	    portStr = portBuf;
	}
	if(upnp->wan_ipaddr != NULL){
	    IPaddr = upnp->wan_ipaddr;
	}else{
	    IPaddr = upnp->local_ipaddr;
	}
	if(portStr == NULL || IPaddr == NULL){
	    upnp_close(upnp);
	    throw "can not get my IPaddr and portNumber";
	}
printf("lisning on %s %s\n", IPaddr, portStr);

	Acceptor *acceptor = new Acceptor(this, fd);
	if(acceptor == NULL){
	    upnp_close(upnp);
	    throw "no more memory.";
	}
	selector->add(acceptor);

	for(ServiceManagerList::iterator i = serviceManagerList.begin(); i != serviceManagerList.end(); i++){
	    (*i)->registerServiceDB(IPaddr, portStr);
	}

	SysDataHolder *sdh = SysDataHolder::getInstance();
	if(sdh != NULL)
	    sdh->setMainLoop(this);
    }

    void MainLoop::deleteActivePipe(PipeManager *pm){
	for(string2ActivePipeMap::iterator i = activePipeMap.begin(); i != activePipeMap.end(); i++){
	    if(i->second != NULL && i->second->pipeManager == pm){
		delete i->second->pipeManager;
		delete i->second;
		activePipeMap.erase(i);
		return;
	    }
	}
    }

}; /* namespace NetPipe */
