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
 * $Id: Kicker.cpp 66 2007-07-04 07:01:15Z  $
 */

#include "config.h"
#include "Kicker.h"
#include "ServiceDB.h"
#include "net.h"
#include "StreamBuffer.h"
#include "VersionChecker.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <map>
#include <string>

namespace NetPipe {
    Kicker::Kicker(){
    }

    Kicker::~Kicker(){
    }

    void Kicker::kick(char *NetPipePathString){
	ServiceDB *db = ServiceDB::getInstance();
	char *buf;
	if(NetPipePathString == NULL)
	    return;
	int NetPipePathStringLen = (int)strlen(NetPipePathString);

	if(NetPipePathString[NetPipePathStringLen - 1] != '\n'){
	    buf = (char *)malloc(NetPipePathStringLen + 2);
	    if(buf == NULL)
		throw "no more memory";
	    memcpy(buf, NetPipePathString, NetPipePathStringLen);
	    buf[NetPipePathStringLen] = '\n';
	    buf[NetPipePathStringLen+1] = '\0';
	}else{
	    buf = strdup(NetPipePathString);
	    if(buf == NULL)
		throw "no more memory";
	}

	stringExistsMap inServiceMap;
	stringExistsMap outServiceMap;
	inServiceMap.clear();
	outServiceMap.clear();

	char *LineStart = buf;
	char *NextLine;
	char *outPortService = NULL;
	char *inPort = NULL;
	char *inService = NULL;
	while((NextLine = strchr(LineStart, '\n')) != NULL){
	    *NextLine = '\0';
	    NextLine++;

	    inService = LineStart;
	    char *p = strchr(LineStart, ';');
	    if(p == NULL)
		goto CONTINUE;
	    *p = '\0';
	    p++;
	    inPort = p;
	    p = strchr(p, ';');
	    if(p == NULL)
		goto CONTINUE;
	    *p = '\0';
	    p++;
	    outPortService = p;
	    p = strchr(p, ';');
	    if(p == NULL)
		goto CONTINUE;
	    p++;
	    inServiceMap[inService] = inPort;
	    outServiceMap[p] = p;
CONTINUE:
	    LineStart = NextLine;
	}

	stringExistsMap firstServiceMap;
	firstServiceMap.clear();
	for(stringExistsMap::iterator i = inServiceMap.begin(); i != inServiceMap.end(); i++){
	    if(outServiceMap[i->first] == NULL)
		firstServiceMap[i->first] = i->second;
	}

	for(stringExistsMap::iterator i = firstServiceMap.begin(); i != firstServiceMap.end(); i++){
	    char *targetService = (char *)i->first.c_str();
	    char *targetPort = i->second;
	    char *IPaddr = db->QueryIPHostName((char *)i->first.c_str());
	    char *port = db->QueryTCPPortName((char *)i->first.c_str());

	    if(targetService == NULL || IPaddr == NULL || port == NULL){
		printf("can not find service name(%s) or IPaddr:port (%s:%s)\n", targetService, IPaddr, port);
		continue;
	    }
	    if(targetPort == NULL)
		targetPort = "";
	    printf("  %s:%s <- %s;%s\n", IPaddr, port, targetPort, targetService);
	    int sock = connect_stream(IPaddr, port);
	    if(sock < 0)
		continue;

	    int size = NetPipePathStringLen + (int)sizeof(uint32_t) * 2 + (int)strlen(targetService) + 3 + (int)strlen(targetPort);
	    StreamBuffer *sm = new StreamBuffer(size + sizeof(uint32_t));
	    if(sm == NULL)
		throw "no more memory";

	    sm->WriteBinary(NETPIPE_HELLO_STRING, strlen(NETPIPE_HELLO_STRING));
	    sm->WriteUint32(size);
	    sm->WriteUint32(0);
	    sm->WriteUint32(size - sizeof(uint32_t) * 2);
	    sm->WriteBinary(targetPort, strlen(targetPort));
	    sm->WriteUint8(';');
	    sm->WriteBinary(targetService, strlen(targetService));
	    sm->WriteUint8('\n');
	    sm->WriteBinary(NetPipePathString, NetPipePathStringLen);
	    sm->WriteUint8('\n');

	    try {
		while(sm->socketWrite(sock))
		    ;
	    }catch (StreamBufferSendError) {
	    }
	    closeSocket(sock);
	}
	free(buf);
    }
}; /* namespace NetPipe */
