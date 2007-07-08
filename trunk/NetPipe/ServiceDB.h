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
 * $Id: ServiceDB.h 96 2007-07-08 11:46:50Z  $
 */

#ifndef NETPIPE_SERVICEDB_H
#define NETPIPE_SERVICEDB_H

#include "config.h"

#include <string>
#include <map>

#define SERVICE_MAP_ENV "NETPIPE_SERVICE_MAP_URL"
#define SERVICE_UPDATE_ENV "NETPIPE_SERVICE_UPDATE_URL"
#define STATIC_SERVICE_MAP_URL "http://uirou.no-ip.org/s/s.cgi"
#define STATIC_SERVICE_UPDATE_URL "http://uirou.no-ip.org/s/s.cgi"
//#define STATIC_SERVICE_MAP_URL "http://172.16.2.33:8472/s.fcgi"
//#define STATIC_SERVICE_UPDATE_URL "http://172.16.2.33:8472/s.fcgi"

namespace NetPipe {
    class ServiceDB {
    private:
	typedef struct {
	    char *IPHost;
	    char *TCPPort;
	} Service;
	typedef std::map<std::string, Service *> ServiceNameMap;
	ServiceNameMap serviceData;

	void refreshServiceData();
	ServiceDB::Service *updateServiceData(char *serviceName);
	ServiceDB();
	void clearCache();
    public:
	~ServiceDB();
	static ServiceDB *getInstance();

	char *QueryIPHostName(char *serviceName);
	char *QueryTCPPortName(char *serviceName);

	bool Regist(char *serviceString); // XXXX ServiceDB::Service ÇÃì‡óeÇ ServiceDBäOÇÃâΩÇ© Ç…èëÇ©ÇπÇƒÇ¢ÇÈ
    };
};

#endif /* NETPIPE_SERVICEDB_H */
