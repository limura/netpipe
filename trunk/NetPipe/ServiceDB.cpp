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
 * $Id: ServiceDB.cpp 95 2007-07-06 07:08:18Z  $
 */

#include "config.h"
#include "ServiceDB.h"
#include "net.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

namespace NetPipe {

    ServiceDB::ServiceDB(){
	serviceData.clear();
	//refreshServiceData();
    }
    ServiceDB *ServiceDB::getInstance(){
	static ServiceDB singleton;
	return &singleton;
    }
    ServiceDB::~ServiceDB(){
	clearCache();
    }

    void ServiceDB::clearCache(){
	for(ServiceNameMap::iterator i = serviceData.begin(); i != serviceData.end(); i++){
	    if(i->second != NULL){
		if(i->second->TCPPort != NULL){
		    free(i->second->TCPPort);
		    i->second->TCPPort = NULL;
		}
		if(i->second->IPHost != NULL){
		    free(i->second->IPHost);
		    i->second->IPHost = NULL;
		}
		free(i->second);
		i->second = NULL;
	    }
	}
	serviceData.clear();
    }

    void ServiceDB::refreshServiceData(){
	int size;
	char *p, *p2, *p3, *http_buf = NULL;
	char *service_map_url = STATIC_SERVICE_MAP_URL;
	if(getenv(SERVICE_MAP_ENV) != NULL)
	    service_map_url = getenv(SERVICE_MAP_ENV);
	int tryNum = 4;
	p = NULL;
	while(tryNum-- > 0 && (http_buf = p = HTTP_post(service_map_url, NULL, 0, NULL, &size)) == NULL)
	    ;
	if(p == NULL || size <= 0)
	    return;
	clearCache();
	p = strstr(p, "\r\n\r\n");
	if(p == NULL){
	    free(http_buf);
	    return;
	}
	p += 4;
	ServiceDB::Service *sv = NULL;
	char *saveP;
	while((p2 = strchr(p, '\n')) != NULL){
	    char *serviceString = NULL;
	    char *host = NULL;
	    char *port = NULL;

	    *p2 = '\0'; // 行末を '\0' terminate
	    p3 = p; // 今のカラム
	    p = ++p2; // 次の行

	    if(*p3 != 'p') // 'p' で始まらない行はしらん
		continue;
	    p3++;
	    while(*p3 == '\t') // とりあえず最初のカラムは 'p' なので読み飛ばす
		p3++;

	    p2 = strtok_r(p3, "\t", &saveP);
	    if(p2 == NULL)
		goto error_retry;
	    serviceString = p2;

	    p2 = strtok_r(NULL, "\t", &saveP);
	    if(p2 == NULL)
		goto error_retry;
	    host = (char *)malloc(strlen(p2) + 1);
	    if(host == NULL)
		goto error_retry;
	    strcpy(host, p2);

	    p2 = strtok_r(NULL, "\t", &saveP);
	    if(p2 == NULL)
		goto error_retry;
	    port = (char *)malloc(strlen(p2) + 1);
	    if(port == NULL)
		goto error_retry;
	    strcpy(port, p2);

	    sv = (ServiceDB::Service *)malloc(sizeof(ServiceDB::Service));
	    if(sv == NULL)
		goto error_retry;
	    sv->TCPPort = port;
	    sv->IPHost = host;
	    serviceData[serviceString] = sv;

	    continue;
error_retry:
	    if(host != NULL)
		free(host);
	    if(port != NULL)
		free(port);
	}
	free(http_buf);
    }

    ServiceDB::Service *ServiceDB::updateServiceData(char *serviceName){
	char urlBuf[1024];
	int size;
	if(serviceName == NULL || strlen(serviceName) + strlen(STATIC_SERVICE_MAP_URL) + 2 > sizeof(urlBuf))
	    return NULL;
	sprintf(urlBuf, "%s?%s", STATIC_SERVICE_MAP_URL, serviceName);
	char *httpReply = HTTP_post(urlBuf, NULL, 0, NULL, &size);
	if(httpReply == NULL)
	    return NULL;
	char *ok200 = strstr(httpReply, " 200 OK\r\n");
	char *enter = strchr(httpReply, '\n');
	if(ok200 == NULL || enter == NULL || ok200 > enter){
	    free(httpReply);
	    return NULL;
	}
	char *begin = strstr(httpReply, "\r\n\r\n");
	begin += 4;
	char *p = strchr(begin, ':');
	if(p == NULL){
	    free(httpReply);
	    return NULL;
	}
	*p = '\0';
	p++;

	ServiceDB::Service *sv = new ServiceDB::Service();
	if(sv == NULL){
	    free(httpReply);
	    return NULL;
	}
	sv->IPHost = strdup(begin);
	if(sv->IPHost == NULL){
	    delete sv;
	    free(httpReply);
	    return NULL;
	}
	sv->TCPPort = (char *)malloc(size - (p -httpReply) + 1);
	if(sv->TCPPort == NULL){
	    delete sv;
	    free(sv->IPHost);
	    free(httpReply);
	    return NULL;
	}
	memcpy(sv->TCPPort, p, size - (p - httpReply));
	sv->TCPPort[size - (p - httpReply)] = '\0';

	serviceData[serviceName] = sv;
	free(httpReply);
	return sv;
    }

    char *ServiceDB::QueryIPHostName(char *serviceName){
	char *p = strchr(serviceName, ' ');
	if(p != NULL)
	    *p = '\0';
	Service *sv = serviceData[serviceName];
	if(sv == NULL){
#if 1
	    sv = updateServiceData(serviceName);
#else
	    refreshServiceData();
	    sv = serviceData[serviceName];
#endif
	    if(p != NULL)
		*p = ' ';
	    if(sv == NULL)
		return NULL;
	}
	if(p != NULL)
	    *p = ' ';
	return sv->IPHost;
    }
    char *ServiceDB::QueryTCPPortName(char *serviceName){
	char *p = strchr(serviceName, ' ');
	if(p != NULL)
	    *p = '\0';
	Service *sv = serviceData[serviceName];
	if(sv == NULL){
#if 1
	    sv = updateServiceData(serviceName);
#else
	    refreshServiceData();
	    sv = serviceData[serviceName];
#endif
	    if(p != NULL)
		*p = ' ';
	    if(sv == NULL)
		return NULL;
	}
	if(p != NULL)
	    *p = ' ';
	return sv->TCPPort;
    }

    bool ServiceDB::Regist(char *ServiceString){
	char *service_update_url = STATIC_SERVICE_UPDATE_URL;
	if(getenv(SERVICE_UPDATE_ENV) != NULL)
	    service_update_url = getenv(SERVICE_UPDATE_ENV);
	// XXXXX ちゃんと送信できたかどうかを見てない
	char *buf = NULL;
	int tryNum = 4;
	while(tryNum-- > 0 && (buf = HTTP_post(service_update_url, ServiceString, strlen(ServiceString), NULL, NULL)) == NULL)
	    ;
	if(buf != NULL)
	    free(buf);
	return buf != NULL;
    }
}; /* namespace NetPipe */
