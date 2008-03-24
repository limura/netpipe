/*
 * Copyright (c) 2008 IIMURA Takuji. All rights reserved.
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

#ifndef UBICIRCUIT_HTTPREADER_H
#define UBICIRCUIT_HTTPREADER_H

#include <map>
#include <string>
#include "net.h"
#include "MainLoop.h"
#include "StreamReader.h"

namespace NetPipe {

#define HTTP_READ_MAX_BLOCK_LENGTH (1024*1024*32)
    class HTTPHeader {
    public:
	string request_method;
	string request_uri;
	string HTTP_version;

	typedef map<std::string, std::string> headerMap;
	headerMap headers;
    };

    class HTTPReader : public StreamReader {
    private:
	MainLoop *parent;
	char *buf;
	size_t bufSize;
	int dataSize;
	int blockLength;
	HTTPHeader header;

	typedef enum {
	    READ_HEADER,
	    READ_BLOCK_LENGTH,
	    READ_BLOCK,
	} ReadState;
	ReadState state;

    public:
	HTTPReader(MainLoop *ml, int fd);
	~HTTPReader();
	bool onRecive();
    };

}; /* namespace NetPipe */

#endif /* UBICIRCUIT_HTTPREADER_H */
