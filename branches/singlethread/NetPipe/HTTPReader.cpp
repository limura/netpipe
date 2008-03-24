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

#include "config.h"
#include "StreamReader.h"
#include "MainLoop.h"
#include "HTTPReader.h"
#include "CPPTools.h"

#include <vector>
#ifdef HAVE_ERRNO_H
#include <errno.h>
extern int errno;
#endif
#include <algorithm>
#include <string.h>

#include "net.h"
#include "tools.h"

namespace NetPipe {

    HTTPReader::HTTPReader(MainLoop *ml, int fd){
	parent = ml;
	this->fd = fd;
	static char thisName[] = "HTTPReader";
	this->myName = thisName;
	dataSize = 0;
	buf = (char *)malloc(4096);
	if(buf == NULL)
	    throw "no more memory";
	bufSize = 4096;
	state = HTTPReader::READ_HEADER;
	blockLength = 0;
    }

    HTTPReader::~HTTPReader(){
	if(buf != NULL)
	    free(buf);
	buf = NULL;
	bufSize = 0;
	dataSize = 0;
	blockLength = 0;
	//if(fd >= 0) // éÛÇØë§Ç™ closeSocket() ÇÇ∑ÇÈÇ±Ç∆ÇÕÇ»Ç¢ÅB
	//    closeSocket(fd);
	fd = -1;
	parent = NULL;
    }

    bool HTTPReader::onRecive(){
	if(bufSize - dataSize <= 64){
	    char *tmp;
	    if((state != HTTPReader::READ_BLOCK && bufSize + 4096 > 1024*1024)
		|| (state == HTTPReader::READ_BLOCK && bufSize + 4096 > HTTP_READ_MAX_BLOCK_LENGTH)){
		char *msg = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nRequest data size too big.";
		send(fd, msg, strlen(msg), 0);
		closeSocket(fd);
		return false;
	    }
	    tmp = (char *)realloc(buf, bufSize + 4096);
	    if(tmp == NULL)
		throw "no more memory";
	    buf = tmp;
	    bufSize += 4096;
	}
	    
	int len = recv(fd, &buf[dataSize], (int)(bufSize - dataSize), 0);
	if(len <= 0){ // EOF or Socket error
	    closeSocket(fd);
	    return false;
	}
	dataSize += len;

STATE_START:
	switch(state){
	    case HTTPReader::READ_HEADER:
		{
		    char *p = NULL;
		    char *line_separator[] = { "\r\n", "\r", "\n", NULL };
		    char *space_separator[] = { " ", NULL };
		    char *header_separator[] = { ":", " ", NULL };
		    std::string line;
		    CPPTool::stringList *lines = NULL;
		    CPPTool::stringList *items = NULL;
		    char *headerEnd = p = strnstr(buf, "\r\n\r\n", dataSize);
		    if(p == NULL)
			break;

		    lines = CPPTool::split(buf, (size_t)(p - buf), line_separator);
		    if(lines == NULL){
			closeSocket(fd);
			return false;
		    }

		    line = lines->front();
		    items = CPPTool::split((char *)line.c_str(), line.length(), space_separator, 3);
		    if(items == NULL || items->size() != 3){
			closeSocket(fd);
			return false;
		    }
		    header.request_method = items->front(); items->pop_front();
		    header.request_uri = items->front(); items->pop_front();
		    header.HTTP_version = items->front(); items->pop_front();
		    delete items; items = NULL;
		    lines->pop_front();

		    header.headers.clear();
		    std::string prev_field("");
		    while(lines->size() > 0){
			line = lines->front();
			if(line.find_first_of(" \t") == 0){ // for multiple line header field.
			    if(prev_field.size() <= 0)
				continue;
			    int i = 0;
			    while(line.find_first_of(" \t", i) == i && i < line.size())
				i++;
			    line = line.substr(i);
			    header.headers[prev_field] += line;
			    continue;
			}
			items = CPPTool::split((char *)line.c_str(), line.length(), header_separator, 2);
			if(items == NULL || items->size() != 2)
			    continue;
			std::string name = items->front(); items->pop_front();
			std::string value = items->front();
			std::transform(name.begin(), name.end(), name.begin(), tolower);
			if(header.headers.find(name) == header.headers.end()){
			    header.headers[name] = value;
			}else{
			    header.headers[name] += ", " + value;
			}
			delete items;
			lines->pop_front();
		    }

		    headerEnd += 4;
		    dataSize -= headerEnd - buf;
		    if(dataSize > 0)
			memmove(buf, headerEnd, dataSize);

		    HTTPHeader::headerMap::iterator chunked = header.headers.find(std::string("transfer-encoding"));
		    if(chunked != header.headers.end() && chunked->second == "chunked"
			&& header.request_method == "POST"){
			state = HTTPReader::READ_BLOCK_LENGTH;
		    }else{
			parent->onHttpRecive(NULL, 0, header, fd);
			return false;
		    }
		}
	    case HTTPReader::READ_BLOCK_LENGTH:
		if(dataSize <= 0)
		    break;
		{
		    char *p = NULL;
		    char *next = strnstr(buf, "\r\n", dataSize);
		    if(next == NULL)
			break;
		    blockLength = 0;
		    p = buf;
		    while(*p != '\0' && *p != '\r' && (p - buf) >= dataSize){
			switch(*p){
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			    blockLength *= 16;
			    blockLength += *p - '0';
			    break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			    blockLength *= 16;
			    blockLength += *p - 'a';
			    break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			    blockLength *= 16;
			    blockLength += *p - 'A';
			    break;
			default:
			    break;
			}
			p++;
		    }
		    next += 2;
		    dataSize -= next - buf;
		    if(dataSize > 0)
			memmove(buf, next, dataSize);
		    if(blockLength == 0){ // EOF request.
			if(parent != NULL)
			    parent->onHttpEof(header);
			closeSocket(fd);
			return false;
		    }
		    if(blockLength < 0 || blockLength > HTTP_READ_MAX_BLOCK_LENGTH){
			char *msg = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nRequest block size too big.";
			send(fd, msg, strlen(msg), 0);
			closeSocket(fd);
			return false;
		    }
		    if(bufSize < blockLength){
			p = (char *)realloc(buf, blockLength);
			if(p == NULL)
			    throw "no more memory";
			buf = p;
			bufSize = blockLength;
		    }
		}
		state = HTTPReader::READ_BLOCK;
	    case HTTPReader::READ_BLOCK:
		if(dataSize >= blockLength){
		    if(parent != NULL)
			parent->onHttpRecive(buf, blockLength, header, fd);
		    dataSize -= blockLength;
		    if(dataSize > 0)
			memmove(buf, buf + blockLength, dataSize);
		    state = HTTPReader::READ_BLOCK_LENGTH;
		    goto STATE_START;
		}
		break;
	    default:
		break;
	}

	return true;
    }

}; /* namespace NetPipe */

