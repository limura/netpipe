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
 * $Id: FDSelector.cpp 52 2007-07-04 01:27:20Z  $
 */

#include "config.h"
#include "FDSelector.h"
#include "tools.h"

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
extern int errno;
#endif

namespace NetPipe {
    FDSelector::FDSelector(){
//printf("FDSelector: CLEAR readport\n");
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	maxfd = -1;
	rlmap.clear();
	wlmap.clear();
    }

    FDSelector::~FDSelector(){
	for(streamReaderListMap::iterator i = rlmap.begin(); i != rlmap.end(); i++){
	    for(streamReaderList::iterator j = i->second.begin(); j != i->second.end(); j++){
		del((*j));
	    }
	    i->second.clear();
	}
	rlmap.clear();
	for(streamWriterListMap::iterator i = wlmap.begin(); i != wlmap.end(); i++){
	    for(streamWriterList::iterator j = i->second.begin(); j != i->second.end(); j++){
		del((*j));
	    }
	    i->second.clear();
	}
	wlmap.clear();
	maxfd = -1;
//printf("FDSelector: CLEAR readport\n");
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
    }

    bool FDSelector::add(StreamReader *sr){
	if(sr == NULL)
	    return false;
	int fd = sr->getFD();
//printf("FDSelector: add readport: %d\n", fd);
	FD_SET(fd, &rfds);
	if(maxfd < fd)
	    maxfd = fd;
	rlmap[fd].push_back(sr);
	return true;
    }

    bool FDSelector::add(StreamWriter *sw){
	if(sw == NULL)
	    return false;
	int fd = sw->getFD();
//printf("FDSelector: add writeport: %d\n", fd);
	FD_SET(fd, &wfds);
	if(maxfd < fd)
	    maxfd = fd;
	wlmap[fd].push_back(sw);
	return true;
    }

    bool FDSelector::del(streamReaderList::iterator i){
	int fd = (*i)->getFD();
	delete (*i);
	rlmap[fd].erase(i);
	if(rlmap[fd].size() > 0)
	    return true;
//	printf("FDSelector: DEL readport: %d\n", fd);
	FD_CLR(fd, &rfds);
	rlmap.erase(fd);
	return true;
    }

    bool FDSelector::del(streamWriterList::iterator i){
	int fd = (*i)->getFD();
	delete (*i);
	wlmap[fd].erase(i);
	if(wlmap[fd].size() > 0)
	    return true;
//	printf("FDSelector: DEL writeport: %d\n", fd);
	FD_CLR(fd, &wfds);
	wlmap.erase(fd);
	return true;
    }

    bool FDSelector::delFD(int fd){
	if(fd < 0)
	    return false;
	streamReaderList::iterator ri;
	while((ri = rlmap[fd].begin()) != rlmap[fd].end()){
	    delete (*ri);
	    rlmap[fd].erase(ri);
	}
	FD_CLR(fd, &rfds);
	rlmap.erase(fd);

	streamWriterList::iterator wi;
	while((wi = wlmap[fd].begin()) != wlmap[fd].end()){
	    delete (*wi);
	    wlmap[fd].erase(wi);
	}
	FD_CLR(fd, &wfds);
	wlmap.erase(fd);

	return true;
    }

    bool FDSelector::del(StreamReader *sr){
	if(sr == NULL)
	    return false;
	int fd = sr->getFD();
	
	for(streamReaderList::iterator i = rlmap[fd].begin(); i != rlmap[fd].end(); i++){
	    if(sr == (*i)){
		del(i);
		return true;
	    }
	}
	return false;
    }

    bool FDSelector::del(StreamWriter *sw){
	if(sw == NULL)
	    return false;
	int fd = sw->getFD();
	for(streamWriterList::iterator i = wlmap[fd].begin(); i != wlmap[fd].end(); i++){
	    if(sw == (*i)){
		del(i);
		return true;
	    }
	}
	return false;
    }

    bool FDSelector::add(TimerHandler *th){
	if(th == NULL)
	    return false;
	int64_t tickTime = th->getTickTime();
	if(tickTime <= 0)
	    return false;
	thlist.push_back(th);

	struct timeval tv;
	gettimeofday(&tv, NULL);
	th->updateTimeout(&tv);
	return true;
    }
    bool FDSelector::del(TimerHandler *th){
	for(timerHandlerList::iterator i = thlist.begin(); i != thlist.end(); i++){
	    if(*(i) == th){
		thlist.erase(i);
		return true;
	    }
	}
	return false;
    }

    // search timed out TimerHandler. and return next Timer wait time.
    int64_t FDSelector::timerCheck(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t min, tmpi64;
	timerHandlerList::iterator i;
    TimerCheck_Start:
	min = 999999999999UL;
	for(i = thlist.begin(); i != thlist.end(); i++){
	    tmpi64 = (*(i))->diffTimeout(&tv);
	    if(tmpi64 <= 0){
		TimerHandler *th = *(i);
		if(th->onTimeout()){
		    th->updateTimeout(&tv);
		}else{
printf("TIMER delete %p\r\n", th);
		    thlist.erase(i);
		    delete th;
		}
		goto TimerCheck_Start;
	    }else{
		if(min > tmpi64)
		    min = tmpi64;
	    }
	}
	return min;
    }

    bool FDSelector::run(int usec){
	fd_set read_fds, write_fds;

	if(maxfd < 0)
	    return false;
	memcpy(&read_fds, &rfds, sizeof(rfds));
	memcpy(&write_fds, &wfds, sizeof(wfds));

	DPRINTF(10, ("select sleep %d usec.\n", usec));

	int64_t next_timeout = this->timerCheck();
	if(usec > 0 && usec < next_timeout)
	    next_timeout = usec;

	int selectRet;
#ifdef HAVE_ERRNO_H
	errno = 0;
#endif
//printf("FDSelector selecting(%d) (thlist: %d)\r\n", next_timeout, thlist.size());
	if(usec <= 0 && next_timeout <= 0){
	    selectRet = select(maxfd + 1, &read_fds, &write_fds, NULL, NULL);
	}else{
	    struct timeval tv;
	    tv.tv_sec = (long)(next_timeout / 1000000);
	    tv.tv_usec = (long)(next_timeout % 1000000);
	    selectRet = select(maxfd + 1, &read_fds, &write_fds, NULL, &tv);
	}
//printf("FDSelector select out: %d\r\n", selectRet);
	this->timerCheck();

	if(selectRet < 0){
#ifdef HAVE_ERRNO_H
#ifdef EINTR
	    if(errno == EINTR)
		return true;
#endif
#endif
	    printf("select() return %d.\n", selectRet);
	    //perror("select:");
	    throw "select return error. ";
	}
	for(streamReaderListMap::iterator i = rlmap.begin(); selectRet > 0 && i != rlmap.end(); i++){
	    if(i->second.size() > 0 && FD_ISSET(i->first, &read_fds)){
		streamReaderList::iterator j = i->second.begin();
//printf("FDSelector recv: %d\r\n", i->first);
		try{
		    if((*j)->onRecive() == false){
			del(*j);
			return true;
		    }
		}catch (char *){
		    delFD(i->first);
		    //del(*j);
		    return true;
		}
		selectRet--;
	    }
	}
	for(streamWriterListMap::iterator i = wlmap.begin(); selectRet > 0 && i != wlmap.end(); i++){
	    if(i->second.size() > 0 && FD_ISSET(i->first, &write_fds)){
		streamWriterList::iterator j = i->second.begin();
		streamWriterList::iterator k;
//printf("FDSelector send: %d\r\n", i->first);
		try{
		    if((*j)->onWrite() == false){
			//delFD(i->first);
			del(*j);
			return true;
		    }
		}catch (char *){
		    delFD(i->first);
		    //del(*j);
		    return true;
		}
		selectRet--;
	    }
	}
	return true;
    }

    void FDSelector::deleteService(Service *targetService){
	for(streamWriterListMap::iterator i = wlmap.begin(); i != wlmap.end(); i++){
	    for(streamWriterList::iterator j = i->second.begin(); j != i->second.end(); j++){
		(*j)->deleteService(targetService);
	    }
	}
    }
};
