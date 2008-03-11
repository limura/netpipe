/*
 * Copyright (c) 2007-2008 IIMURA Takuji. All rights reserved.
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

#include <string>

#include "config.h"
#include "ServiceManager.h"
#include "FDSelector.h"
#include "net.h"
#include "Acceptor.h"
#include "PortWriter.h"
#include "PipeManager.h"
#include "TimerHandler.h"

#define NETPIPE_HEADER_BUFFER_LENGTH (1024)

namespace NetPipe {
    
    ServiceManager::ServiceManager(char *name, char *tag){
	if(name == NULL)
	    throw "undefined service name. ";
	serviceName = strdup(name);
	if(serviceName == NULL)
	    throw "no more memory. ";
	if(tag == NULL)
	    tag = "";
	serviceTag = strdup(tag);
	if(serviceTag == NULL){
	    free(serviceName);
	    throw "no more memory.";
	}
	creator = NULL;
	creatorUserData = NULL;
    }
    ServiceManager::~ServiceManager(){
	if(serviceName != NULL){
	    free(serviceName);
	    serviceName = NULL;
	}
	if(serviceTag != NULL){
	    free(serviceTag);
	    serviceTag = NULL;
	}
    }
    char *ServiceManager::getServiceName(){
	return serviceName;
    }

    void ServiceManager::addReadPort(char *portName, char *description){
	ReadPort *rp = new ReadPort();
	if(rp == NULL)
	    throw "no more memory";
	rp->portName = strdup(portName);
	if(rp->portName == NULL){
	    delete rp;
	    throw "no more memory";
	}
	rp->description = strdup(description);
	if(rp->description == NULL){
	    free(rp->portName);
	    delete rp;
	    throw "no more memory";
	}
	readPortMap[portName] = rp;
    }
    void ServiceManager::addWritePort(char *portName, char *description){
	WritePort *wp = new WritePort();
	if(wp == NULL)
	    throw "no more memory";
	wp->portName = strdup(portName);
	if(wp->portName == NULL){
	    delete wp;
	    throw "no more memory";
	}
	wp->description = strdup(description);
	if(wp->description == NULL){
	    free(wp->portName);
	    delete wp;
	    throw "no more memory";
	}
	writePortMap[portName] = wp;
    }
    void ServiceManager::addReadFD(int fd, char *description, size_t bufsize){
	FDInput *fdi = fdInputMap[fd];
	if(fdi == NULL){
	    fdi = new FDInput();
	    fdInputMap[fd] = fdi;
	}
	fdi->fd = fd;
	fdi->description = strdup(description);
	if(fdi->description == NULL){
	    delete fdi;
	    throw "no more memory";
	}
	fdi->bufSize = bufsize;
    }
    void ServiceManager::addTimer(int usec, void *userData, char *description){

    }
    void ServiceManager::addServiceCreator(ServiceCreator *serviceCreator, void *userdata){
	creator = serviceCreator;
	creatorUserData = userdata;
    }
    Service *ServiceManager::createNewService(){
	if(creator == NULL)
	    return NULL;
	return creator->createNewService(creatorUserData);
    }
    void ServiceManager::registReadFDInput(PipeManager *pm){
	for(FDInputMap::iterator i = fdInputMap.begin(); i != fdInputMap.end(); i++){
	    pm->addReadFD(i->second->fd, i->second->bufSize);
	}
    }

    void ServiceManager::registerServiceDB(char *IPaddr, char *TCPPort){
	std::string str("");
	/*
	 * i\tServiceName\tInputPortName\tInputPortName\tInputPortName...
	 * o\tServiceName\tOutputPortName\tOutputPortName\tOutputPortName...
	 * p\tInputPortName:ServiceName\tIPAddr\tTCPPortString\tTag...
	 */
	if(IPaddr == NULL || TCPPort == NULL)
	    throw "IPAddr == NULL || TCPPort == NULL";

	if(readPortMap.size() > 0){
	    //strncat(ServiceString, "i\t", sizeof(ServiceString));
	    //strncat(ServiceString, serviceName, sizeof(ServiceString));
	    str.append("i\t");
	    str.append(serviceName);
	    for(ReadPortMap::iterator i = readPortMap.begin(); i != readPortMap.end(); i++){
		//strncat(ServiceString, "\t", sizeof(ServiceString));
		//strncat(ServiceString, i->second->portName, sizeof(ServiceString));
		str.append("\t");
		str.append(i->second->portName);
	    }
	    //strncat(ServiceString, "\n", sizeof(ServiceString));
	    str.append("\n");
	}

	if(writePortMap.size() > 0){
	    //strncat(ServiceString, "o\t", sizeof(ServiceString));
	    //strncat(ServiceString, serviceName, sizeof(ServiceString));
	    str.append("o\t");
	    str.append(serviceName);
	    for(WritePortMap::iterator i = writePortMap.begin(); i != writePortMap.end(); i++){
		//strncat(ServiceString, "\t", sizeof(ServiceString));
		//strncat(ServiceString, i->second->portName, sizeof(ServiceString));
		str.append("\t");
		str.append(i->second->portName);
	    }
	    //strncat(ServiceString, "\n", sizeof(ServiceString));
	    str.append("\n");
	}

	//strncat(ServiceString, "p\t", sizeof(ServiceString));
	//strncat(ServiceString, serviceName, sizeof(ServiceString));
	//strncat(ServiceString, "\t", sizeof(ServiceString));
	//strncat(ServiceString, IPaddr, sizeof(ServiceString));
	//strncat(ServiceString, "\t", sizeof(ServiceString));
	//strncat(ServiceString, TCPPort, sizeof(ServiceString));
	//strncat(ServiceString, "\t", sizeof(ServiceString));
	//strncat(ServiceString, serviceTag, sizeof(ServiceString));
	//strncat(ServiceString, "\n", sizeof(ServiceString));
	str.append("p\t"); str.append(serviceName);
	str.append("\t"); str.append(IPaddr);
	str.append("\t"); str.append(TCPPort);
	str.append("\t"); str.append(serviceTag);
	str.append("\n");

	//if(ServiceString[0] != '\0')
	    //ServiceDB::getInstance()->Regist(ServiceString);
	if(str.length() > 0)
	    ServiceDB::getInstance()->Regist((char *)str.c_str());
    }

    void ServiceManager::registFDSelector(NetPipe::FDSelector *selector){
	if(selector == NULL)
	    return;
	selector;
    }
}; /* namespace NetPipe */