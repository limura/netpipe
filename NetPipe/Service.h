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
 * $Id: Service.h 96 2007-07-08 11:46:50Z  $
 */

#ifndef NETPIPE_SERVICE_H
#define NETPIPE_SERVICE_H

#include "config.h"
#include "PipeManager.h"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

namespace NetPipe {
    class ServiceManager;
    class Service {
    public:
	typedef enum {
	    RECV,
	    RECV_DOWN, // 受信ポートが落ちた
	    FD_INPUT,
	    FD_DOWN, // FD_CLOSE はどっかで使われてる
	    TIMER,
	    CREATED,
	} EVENT_TYPE;
	virtual bool onEvent(PipeManager *pipeManager, char *portName,
	    char *arg, EVENT_TYPE type, char *buf, size_t bufSize) = 0;
    };
}; /* namespace NetPipe */

#endif /* NETPIPE_SERVICE_H */
