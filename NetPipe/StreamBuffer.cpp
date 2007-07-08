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
 * $Id: StreamBuffer.cpp 34 2007-07-01 21:37:34Z  $
 */

#include "StreamBuffer.h"

#include <stdio.h>

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
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif


#include <stdlib.h>
#include <string.h>

namespace NetPipe {

    typedef uint16_t StringSize;

    StreamBuffer::StreamBuffer(size_t Bufsize){
	if(Bufsize != 0){
	    readP = writeP = buf = (char *)malloc(Bufsize);
	    if(buf == NULL)
		throw "malloc error.";
	}else{
	    readP = writeP = buf = NULL;
	}
	remain_size = Bufsize;
    }

    StreamBuffer::~StreamBuffer(){
	if(buf != NULL)
	    free(buf);
	readP = writeP = buf = NULL;
        remain_size = 0;
    }

    bool StreamBuffer::realloc_buffer(){
	size_t new_size = (writeP - buf + remain_size) * 2;
	if(new_size < 1024)
	    new_size = 1024;
	if(new_size > 4096*1024)
	    printf("WARNING!!! new buffer size %d too big!\n", new_size);
	char *tmp_buf = (char *)realloc(buf, new_size);
	if(tmp_buf == NULL)
	    throw "realloc error.";
	readP = tmp_buf + (readP - buf);
	writeP = tmp_buf + (writeP - buf);
	buf = tmp_buf;
	remain_size = new_size - (writeP - buf);
	return true;
    }

    bool StreamBuffer::WriteUint8(uint8_t ui8){
	if(remain_size < sizeof(ui8) && !realloc_buffer())
	    return false;
	memcpy(writeP, &ui8, sizeof(ui8));
	writeP += sizeof(ui8);
	remain_size -= sizeof(ui8);
	return true;
    }

    bool StreamBuffer::WriteUint16(uint16_t ui16){
	if(remain_size < sizeof(ui16) && !realloc_buffer())
	    return false;
	ui16 = htons(ui16);
	memcpy(writeP, &ui16, sizeof(ui16));
	writeP += sizeof(ui16);
	remain_size -= sizeof(ui16);
	return true;
    }

    bool StreamBuffer::WriteUint32(uint32_t ui32){
	if(remain_size < sizeof(ui32) && !realloc_buffer())
	    return false;
	ui32 = htonl(ui32);
	memcpy(writeP, &ui32, sizeof(ui32));
	writeP += sizeof(ui32);
	remain_size -= sizeof(ui32);
	return true;
    }

    bool StreamBuffer::WriteInt8(int8_t i8){
	if(remain_size < sizeof(i8) && !realloc_buffer())
	    return false;
	memcpy(writeP, &i8, sizeof(i8));
	writeP += sizeof(i8);
	remain_size -= sizeof(i8);
	return true;
    }

    bool StreamBuffer::WriteInt16(int16_t i16){
	if(remain_size < sizeof(i16) && !realloc_buffer())
	    return false;
	i16 = htons(i16);
	memcpy(writeP, &i16, sizeof(i16));
	writeP += sizeof(i16);
	remain_size -= sizeof(i16);
	return true;
    }
    bool StreamBuffer::WriteInt32(int32_t i32){
	if(remain_size < sizeof(i32) && !realloc_buffer())
	    return false;
	i32 = htonl(i32);
	memcpy(writeP, &i32, sizeof(i32));
	writeP += sizeof(i32);
	remain_size -= sizeof(i32);
	return true;
    }

    bool StreamBuffer::WriteString(char *nullTerminatedString){
	if(nullTerminatedString == NULL)
	    return false;
	return WriteString(nullTerminatedString, strlen(nullTerminatedString));
    }
    bool StreamBuffer::WriteString(char *str, size_t length){
	if(str == NULL || length <= 0)
	    return true;
	while(remain_size < length + sizeof(StringSize)){
	     if(!realloc_buffer())
		 return false;
	}
	WriteUint16((StringSize)length);
	WriteBinary(str, length);
	return true;
    }
    bool StreamBuffer::WriteBinary(char *dat, size_t length){
	if(dat == NULL || length <= 0)
	    return true;
	while(remain_size < length){
	    if(!realloc_buffer())
		return false;
	}
	memcpy(writeP, dat, length);
	writeP += length;
	remain_size -= length;
	return true;
    }

    uint8_t StreamBuffer::ReadUint8(){
	uint8_t i;
	if((int)(writeP - readP) < (int)sizeof(i))
	    return 0;
	i = *((uint8_t *)readP);
	readP += sizeof(i);
	if(readP == writeP)
	    clearBuffer();
	return i;
    }
    uint16_t StreamBuffer::ReadUint16(){
	uint16_t i;
	if((int)(writeP - readP) < (int)sizeof(i))
	    return 0;
	i = *((uint16_t *)readP);
	readP += sizeof(i);
	if(readP == writeP)
	    clearBuffer();
	return i;
    }
    uint32_t StreamBuffer::ReadUint32(){
	uint32_t i;
	if((int)(writeP - readP) < (int)sizeof(i))
	    return 0;
	i = *((uint32_t *)readP);
	readP += sizeof(i);
	if(readP == writeP)
	    clearBuffer();
	return i;
    }
    int8_t StreamBuffer::ReadInt8(){
	int8_t i;
	if((int)(writeP - readP) < (int)sizeof(i))
	    return 0;
	i = *((int8_t *)readP);
	readP += sizeof(i);
	if(readP == writeP)
	    clearBuffer();
	return i;
    }
    int16_t StreamBuffer::ReadInt16(){
	int16_t i;
	if((int)(writeP - readP) < (int)sizeof(i))
	    return 0;
	i = *((int16_t *)readP);
	readP += sizeof(i);
	if(readP == writeP)
	    clearBuffer();
	return i;
    }
    int32_t StreamBuffer::ReadInt32(){
	int32_t i;
	if((int)(writeP - readP) < (int)sizeof(i))
	    return 0;
	i = *((int32_t *)readP);
	readP += sizeof(i);
	if(readP == writeP)
	    clearBuffer();
	return i;
    }

    char *StreamBuffer::ReadString(){
	StringSize ssize = ReadUint16();
	if(ssize <= 0)
	    return NULL; // XXXX 長さの無い文字列だったということで、readP を元の位置には戻していない。
	if(writeP - readP < ssize){
	    readP -= sizeof(StringSize);
	    return NULL;
	}
	char *buf = (char *)malloc(ssize);
	memcpy(buf, readP, ssize);
	readP += ssize;
	if(readP == writeP)
	    clearBuffer();
	return buf;
    }

    // return code: rc
    // rc < 0: error
    // rc == 0: more buffer need or 0 byte string.
    // rc > 0; read size;
    // string は '\0' で終端される。
    int StreamBuffer::ReadString(char *recvBuf, size_t bufSize){
	if(buf == NULL)
	    return -1;
	StringSize ssize = ReadUint16();
	if(bufSize - 1 < ssize){
	    readP -= sizeof(ssize);
	    return 0;
	}
	if(writeP - readP < ssize){ // 文字列データが読み込みきれてないようだ？
	    readP -= sizeof(ssize);
	    return -1;
	}
	memcpy(recvBuf, readP, ssize);
	recvBuf[ssize] = '\0';
	readP += ssize;
	if(readP == writeP)
	    clearBuffer();
	return ssize;
    }

    int StreamBuffer::ReadBinary(char *recvBuf, size_t bufSize){
	if(buf == NULL)
	    return -1;
	if((int)(writeP - readP) < (int)bufSize)
	    return 0;
	memcpy(recvBuf, readP, bufSize);
	readP += bufSize;
	if(readP == writeP)
	    clearBuffer();
	return (int)bufSize;
    }

    char *StreamBuffer::getBuffer(size_t *size_return){
	if(size_return != NULL)
	    *size_return = (writeP - buf);
	return buf;
    }
    size_t StreamBuffer::getSize(){
	return writeP - buf;
    }

    void StreamBuffer::clearBuffer(){
	remain_size += writeP - buf;
	readP = writeP = buf;
    }

    StreamBuffer *StreamBuffer::dup(){
	StreamBuffer *newBuf = new StreamBuffer(writeP - buf);
	if(newBuf == NULL)
	    return NULL;
	newBuf->WriteBinary(buf, writeP - buf);
	return newBuf;
    }

    // return code: false; もう何も送るものは無い or ERROR true: まだ送るものがある
    bool StreamBuffer::socketWrite(int fd){
	if(fd < 0)
	    return false;
	if(writeP - readP <= 0)
	    return false;
	int write_length = send(fd, buf, (int)(writeP - readP), 0);
	if(write_length <= 0)
	    return false;
	if(write_length < (writeP - readP)){
	    readP += write_length;
	    return true;
	}
	remain_size += writeP - buf;
	readP = writeP = buf;
	return false;
    }

    // return code: false: error or 読めなかった true: 読めた
    bool StreamBuffer::socketRead(int fd, size_t minBufferSize){
	if(fd < 0)
	    return false;
	while(remain_size < minBufferSize){
	    if(!realloc_buffer())
		return false;
	}
	int read_length = recv(fd, writeP, (int)remain_size, 0);
	if(read_length <= 0)
	    return false;
	writeP += read_length;
	remain_size -= read_length;
	return true;
    }

}; /* namespace NetPipe */
