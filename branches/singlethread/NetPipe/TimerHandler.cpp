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

#include "TimerHandler.h"

namespace NetPipe{

TimerHandler::TimerHandler(){
    prev_called_time.tv_sec = 0;
    prev_called_time.tv_usec = 0;
    tick_time = 0;
    myName = NULL;
}

int64_t TimerHandler::diffTimeout(struct timeval *now){
    int64_t sec, usec;
    if(now == NULL)
	return -1;
    sec = prev_called_time.tv_sec - now->tv_sec;
    usec = (prev_called_time.tv_usec + tick_time) - now->tv_usec;
//    printf("  TimerHandler diffing %ld.%ld - %ld.%ld\r\n", prev_called_time.tv_sec, now->tv_sec, prev_called_time.tv_usec + tick_time, now->tv_usec);
    while(usec < 0){
	sec--;
	usec += 1000000;
    }
    while(usec >= 1000000){
	sec++;
	usec -= 1000000;
    }
    return (int64_t)sec * 1000000 + usec;
}

void TimerHandler::setTickTime(int tick_time){
    this->tick_time = tick_time;
}

int TimerHandler::getTickTime(){
    return tick_time;
}

void TimerHandler::updateTimeout(struct timeval *now){
    if(now == NULL)
	return;
    prev_called_time.tv_sec = now->tv_sec;
    prev_called_time.tv_usec = now->tv_usec;
}

char *TimerHandler::getName(){
    return myName;
}

}; /* namespace NetPipe */
