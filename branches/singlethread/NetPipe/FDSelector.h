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

#ifndef NETPIPE_FDSELECTOR_H
#define NETPIPE_FDSELECTOR_H

#include "config.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "TimerHandler.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#include <stdlib.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif

#include <map>
#include <list>

using namespace std;

namespace NetPipe {
    class SysDataHolder;
    class Service;
    class FDSelector {
	friend class SysDataHolder;
    private:
	fd_set rfds, wfds;
	int maxfd;
	typedef list<StreamReader *> streamReaderList;
	typedef map<int, streamReaderList> streamReaderListMap;
	streamReaderListMap rlmap;
	typedef list<StreamWriter *> streamWriterList;
	typedef map<int, streamWriterList> streamWriterListMap;
	streamWriterListMap wlmap;
	bool del(streamReaderList::iterator i);
	bool del(streamWriterList::iterator i);

	typedef list<TimerHandler *> timerHandlerList;
	timerHandlerList thlist;
	int64_t timerCheck();

    public:
	FDSelector();
	~FDSelector();

	bool add(StreamReader *sr);
	bool add(StreamWriter *sw);
	bool del(StreamReader *sr);
	bool del(StreamWriter *sw);
	bool add(TimerHandler *th);
	bool del(TimerHandler *th);
	bool delFD(int fd);

	bool run(int usec);

	void deleteService(Service *targetService);
    };

}; /* namespace NetPipe */

#endif /* NETPIPE_FDSELECTOR_H */
