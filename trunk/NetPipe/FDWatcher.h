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

#ifndef NETPIPE_FDWATCHER_H
#define NETPIPE_FDWATCHER_H

#include "thread.h"
#include "StreamBuffer.h"
#include "config.h"

#include <stdlib.h>

#include <list>
#include <map>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
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
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

namespace NetPipe {
    class FDReciver {
    public:
	virtual void onAccept(int fd, void *userData) = 0;
	virtual void onRecive(int fd, char *buf, size_t size, void *userData) = 0;
	virtual void onClose(int fd, void *userData) = 0;
    };

    class FDEvent {
    public:
	typedef enum {
	    RECV,
	    CLOSE,
	    ACCEPT,
	} EventType;
    private:
	EventType type;
	char *buf;
	int fd;
	size_t size;
	void *userData;
	FDReciver *reciver;

    public:
	FDEvent(EventType type, int FD, char *buffer, size_t bufSize, void *userData);
	~FDEvent();

	EventType getType();
	char *getBuf();
	size_t getBufSize();
	int getSocket();
	void *getUserData();
	void setReciver(FDReciver *reciver);

	void invoke();
    };

    class WatcherAcceptBuffer {
    public:
	FDReciver *reciver;
	void *userData;
    };

    typedef int32_t BufSize_t;
    class WatcherRecvBuffer {
    public:
	bool staticReadSize;
	BufSize_t readSize;
	char *buf, *p;
	size_t bufSize;
	void *userData;
	FDReciver *reciver;

	WatcherRecvBuffer(void *userData = NULL);
	~WatcherRecvBuffer();
	void grow(size_t targetSize);
	void refresh();
	bool socketRecv(int sock);
	void releaseBuffer();
    };

    class WatcherSendBuffer {
    public:
	typedef enum {
	    HAVE_MORE_DATA,
	    END_DATA,
	    END_OF_FILE,
	    SEND_ERROR,
	} SendReturn;
	BufSize_t size;
	char *buf, *p;
	size_t bufSize;
	void *userData;

	WatcherSendBuffer(char *buf, size_t bufSize, void *userData = NULL);
	~WatcherSendBuffer();
	SendReturn socketSend(int sock);
    };

    class SysDataHolder;
    class FDWatcher {
	friend class SysDataHolder;
    private:
	threadID selectThreadID;
	bool threadStarted;
	fd_set read_fds, write_fds;
	int maxFDNum;

	thread_mutex fdsetMutex, acceptQueueMutex, sendQueueMutex, recvQueueMutex, eventQueueMutex;
	thread_cond eventQueueCond;

	typedef std::list<FDEvent *> EventQueue;
	EventQueue *eventQueue;

	typedef std::map<int, WatcherAcceptBuffer *> AcceptQueue;
	typedef std::list<WatcherSendBuffer *> WatcherSendBufferList;
	typedef std::map<int, WatcherSendBufferList> FD2SendBuffer;
	typedef std::map<int, WatcherRecvBuffer *> FD2RecvBuffer;
	AcceptQueue acceptQueue;
	FD2SendBuffer sendQueue;
	FD2RecvBuffer recvQueue;
	static void threadFunc(void *userData);
	void deleteAcceptQueue(AcceptQueue::iterator i);
	void deleteRecvQueue(FD2RecvBuffer::iterator i);
	//void deleteSendQueue(FD2SendBuffer::iterator i);
	void addNewEvent(FDEvent *ev);
	FDWatcher();

    public:
	static FDWatcher *getInstance();
	~FDWatcher();

	bool startWatcher();

	FDEvent *nextEvent(int usec);
	void invokeEvents(int usec); // 溜まってるEventを全部 invoke() する
	void addAcceptQueue(int fd, FDReciver *reciver = NULL, void *userData = NULL);
	void addReciveQueue(int fd, FDReciver *reciver = NULL, void *userData = NULL);
	void addNoSizedReciveQueue(int fd, size_t bufSize, FDReciver *reciver = NULL, void *userData = NULL);
	// addSendQueue だけ特殊で、 buf == NULL を突っ込むと、それより前のデータを書き終えたら socket を close する
	void addSendQueue(int fd, StreamBuffer *buf);
	void addSendQueue(int fd, char *buf, size_t size);
	void addNoSizedSendQueue(int fd, char *buf, size_t size);
	void closeAcceptSocket(int fd);
	void closeReciveSocket(int fd);
	void closeSendSocket(int fd);

	void setReadBytes(int fd, size_t size); // 強引に読み込みバイト数を上書きする
    };
};

#endif /* NETPIPE_FDWATCHER_H */
