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

#include <NetPipe.h>

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int main(int argc, char *argv[]){
    char *buf, *line, *p;
    size_t bufSize = 0;
    int fd, len;
    char *targetFile = "kicker.txt";
    NetPipe::Kicker *k = new NetPipe::Kicker();
    struct stat st;

    NetPipeInit();

    if(argc > 1)
	targetFile = argv[1];

    fd = open(targetFile, O_RDONLY);
    if(fd < 0){
	printf("can not open %s\n", targetFile);
	return 1;
    }

    if(stat(targetFile, &st) != 0){
	printf("can not stat %s\n", targetFile);
	return 1;
    }
    buf = (char *)malloc(st.st_size + 1);
    if(buf == NULL){
	printf("no more memory.\n");
	return 1;
    }
    bufSize = st.st_size;

    len = read(fd, buf, bufSize - 1);
    if(len <= 0){
	printf("can not read %s\n", targetFile);
	return 2;
    }
    buf[len] = '\0';

    line = p = buf;
    while((p = strstr(p, "\n\n")) != NULL){
	*(++p) = '\0';
	k->kick(line);
	p++;
	while(*p == '\n')
	    p++;
	line = p;
    }
    if(*line != '\0')
	k->kick(line);
    return 0;
}
