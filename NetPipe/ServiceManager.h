/*
 * Copyright (c) 20076 IIMURA Takuji. All rights reserved.
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

#ifndef NETPIPE_SERVICEMANAGER_H
#define NETPIPE_SERVICEMANAGER_H

#include "config.h"
#include "StreamBuffer.h"
#include "FDSelector.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "PortReader.h"
#include "ServiceDB.h"
#include "ServiceCreator.h"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_SYS_TYME_H
#include <sys/time.h>
#endif

#include <map>
#include <list>

namespace NetPipe {
    class Acceptor;
    class SysDataHolder;
    class ServiceManager {
	friend class MainLoop;
	friend class SysDataHolder;
    private:
	typedef std::list<StreamBuffer *> BufferList;
	typedef struct {
	    char *portName;
	    char *description;
	} ReadPort;
	typedef struct {
	    char *portName;
	    char *description;
	} WritePort;
	typedef struct {
	    int fd;
	    size_t bufSize;
	    char *description;
	} FDInput;
	typedef struct {
	    int usec;
	    struct timeval prev;
	    char *description;
	} Timer;

	char *serviceName;
	ServiceCreator *creator;
	void *creatorUserData;

	typedef std::map<std::string, ReadPort *> ReadPortMap;
	ReadPortMap readPortMap;
	typedef std::map<std::string, WritePort *> WritePortMap;
	WritePortMap writePortMap;
	typedef std::map<int, FDInput *> FDInputMap;
	FDInputMap fdInputMap;
	typedef std::list<Timer *> TimerMap;
	TimerMap timerMap;

	Service *createNewService();
	void registerServiceDB(char *IPaddr, char *TCPPort);
	void registFDSelector(FDSelector *selector);

    public:
	ServiceManager(char *serviceName);
	~ServiceManager();
	char *getServiceName();

	void addReadPort(char *portName, char *description = "UNDEFINED");
	void addWritePort(char *portName, char *description = "UNDEFINED");
	void addReadFD(int fd, char *description = "UNDEFINED", size_t bufsize = 4096);
	void addTimer(int usec, void *userData, char *description = "UNDEFINED");
	void addServiceCreator(ServiceCreator *serviceCreator, void *userdata = NULL);
	void registReadFDInput(PipeManager *pm);
    };
}; /* using namespace NetPipe */

#endif /* NETPIPE_SERVICEMANAGER_H */
