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
 * $Id: StreamBuffer.h 51 2007-07-04 01:22:02Z  $
 */

#ifndef NETPIPE_STREAMBUFFER_H
#define NETPIPE_STREAMBUFFER_H

#include "config.h"

#include <stdlib.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

namespace NetPipe {
    class StreamBuffer {
    private:
	char *buf, *writeP, *readP;
	size_t remain_size;
	bool realloc_buffer();
	void setHeader(char *buf, size_t length);

    public:
	StreamBuffer(size_t bufsize = 1024);
	~StreamBuffer();

	/*
	 * 勝手にrealocするバッファ。
	 * write で書き込まれた分を read で読み出せる。
	 *
	 * ただし、
	 * Write: バッファへの書き込み
	 * Read: バッファからの読み込み
	 * socketWrite: バッファからの読み込み
	 * socketRead: バッファへの書き込み
	 * と、socketが頭につくと意味が逆転するので注意。
	 *
	 * また、書き込まれたデータの全てが read されるとバッファの頭から使いなおすのだけれど、
	 * ちょっとでも書き込まれたデータが残っていると頭のほうの残ったデータはもったいないことになるのも注意。
	 */
	bool WriteUint8(uint8_t ui8);
	bool WriteUint16(uint16_t ui16);
	bool WriteUint32(uint32_t ui32);
	bool WriteInt8(int8_t i8);
	bool WriteInt16(int16_t i16);
	bool WriteInt32(int32_t ui32);
	bool WriteString(char *nullTerminatedString);
	bool WriteString(char *str, size_t length);
	bool WriteBinary(char *dat, size_t length);

	uint8_t ReadUint8();
	uint16_t ReadUint16();
	uint32_t ReadUint32();
	int8_t ReadInt8();
	int16_t ReadInt16();
	int32_t ReadInt32();
	char *ReadString();
	int ReadString(char *buf, size_t bufSize);
	int ReadBinary(char *buf, size_t bufSize);

	bool socketWrite(int fd);
	bool socketRead(int fd, size_t minimumBufferSize = 1024);

	char *getBuffer(size_t *size_return = NULL);
	size_t getSize();
	void clearBuffer();
	StreamBuffer *dup();
    };
};

#endif /* NETPIPE_STREAMBUFFER_H */
