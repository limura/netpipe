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
 * $Id: PipeManager.h 50 2007-07-03 00:25:15Z  $
 */

#ifndef NETPIPE_PIPEMANAGER_H
#define NETPIPE_PIPEMANAGER_H

//#include "FDSelector.h"
#include "StreamBuffer.h"
#include "FDWatcher.h"

#include <map>
#include <list>
#include <string>

namespace NetPipe {
    class MainLoop;
    class Service;
    class SysDataHolder;
    class PipeManager : public FDReciver {
	friend class MainLoop;
	friend class SysDataHolder;
    private:
	char *pipePath;
	char *serviceName;
	//FDSelector *selector;
	Service *service;
	int inputSockNum;
	MainLoop *parent;

	typedef struct {
	    int sock;
	    char *PortService;
	} PortService;
	typedef std::list<PortService *> portServiceList;
	typedef struct {
	    portServiceList nextPortService;
	    StreamBuffer *buf;
	} WritePort;

	typedef std::map<std::string, WritePort *> string2WritePortMap;
	string2WritePortMap writePortMap;

	void addWritePort(char *portName, int fd, char *nextPortService);
	void inclimentInputPort();
	void declimentInputPort(char *portName);
	void onAccept(int fd, void *userData);
	void onRecive(int fd, char *buf, size_t size, void *userData);
	void onClose(int fd, void *userData);

    public:
	enum {
	    PORT_ACTION_NORMAL = 0,
	    PORT_ACTION_CLOSE = 1,
	};
	PipeManager(char *thisPipePath, char *serviceName, Service *service, MainLoop *ml);
	virtual ~PipeManager();

	void addReadFD(int fd, size_t bufsize = 4096);
	bool write(char *portName, char *buf, size_t size);
	StreamBuffer *getWriteBuffer(char *portName);
	bool commit(char *portName);
	void exit();
    };
}; /* namespace NetPipe */

#endif /* NETPIPE_PIPEMANAGER_H */

