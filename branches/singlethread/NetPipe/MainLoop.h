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
 * $Id: MainLoop.h 58 2007-07-04 06:03:18Z  $
 */

#ifndef NETPIPE_MAINLOOP_H
#define NETPIPE_MAINLOOP_H

#include "Service.h"
#include "FDSelector.h"
#include "upnp.h"
#include "Acceptor.h"

#include <map>
#include <list>
#include <string>

namespace NetPipe {
    class PortReader;
    class SysDataHolder;
    class PipeManager;
    class VersionChecker;
    class MainLoop : public AcceptEventHandler {
	friend class Acceptor;
	friend class PortReader;
	friend class SysDataHolder;
	friend class PipeManager;
	friend class VersionChecker;
    private:
	FDSelector *selector;
	cookai_upnp *upnp;
	Acceptor *acceptor;

	int dummy_count;
	FILE *log_fp;

	typedef struct {
	    Service *service;
	    PipeManager *pipeManager;
	} ActivePipe;
	typedef std::map<std::string, ActivePipe *> string2ActivePipeMap;
	string2ActivePipeMap activePipeMap;
	typedef std::list<ServiceManager *> ServiceManagerList;
	ServiceManagerList serviceManagerList;

	void connectToNextPort(PipeManager *pm, char *serviceName, char *pipePath);

	void onAccept(int sock);
	void onAcceptValidConnection(int sock);
	void openAcceptPort();
	bool onPortRecive(char *buf, size_t size);
	void deleteActivePipe(PipeManager *pm);
    public:
	MainLoop();
	virtual ~MainLoop();

	void addServiceManager(ServiceManager *sm);
	void run(int usec = 0);
    };
};

#endif /* NETPIPE_MAINLOOP_H */

