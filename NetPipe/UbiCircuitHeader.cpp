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

#include "HTTPReader.h"
#include "UbiCircuitHeader.h"

namespace NetPipe {

    UbiCircuitHeader::UbiCircuitHeader(HTTPHeader header){
	HTTPHeader::headerMap::iterator hmi;

	circuit.clear();
	hmi = header.headers.find("x-ubicircuit-version");
	if(hmi == header.headers.end())
	    return;
	if(*hmi != UBICIRCUIT_HTTP_PROTOCOL_VERSION)
	    return;

	hmi = header.headers.find("x-ubicircuit-circuit");
	if(hmi == header.headers.end())
	    return;
	circuit = *hmi;

	if(header.request_uri.size() <= 1){
	    circuit.clear();
	    return;
	}
	serviceName = header.request_uri.substr(1);

	inputPort.clear();
	hmi = header.headers.find("x-ubicircuit-inputport");
	if(hmi == header.headers.end()){
	    serviceName.clear();
	    circuit.clear();
	    return;
	}
	inputPort = *hmi;

	inputArg.clear();
	hmi = header.headers.find("x-ubicircuit-inputarg");
	if(hmi != header.headers.end())
	    inputArg = *hmi;
	serviceArg.clear();
	hmi = header.headers.find("x-ubicircuit-servicearg");
	if(hmi != header.headers.end())
	    serviceArg = *hmi;
    }

    UbiCircuitHeader::UbiCircuitHeader(std::string Circuit, std::string ServiceName, std::string InputPort,
	std::string InputArg, std::string ServiceArg){

	if(Circuit.size() <= 0 || ServiceName.size() <= 0 || InputPort.size() <= 0){
	    circuit.clear();
	    return;
	}
	circuit = Circuit;
	serviceName = ServiceName;
	inputPort = InputPort;
	inputArg = InputArg;
	serviceArg = ServiceArg;
    }

    std::string UbiCircuitHeader::getHTTPHeader(){
	if(!isVAlidData())
	    return std::string("");
	std::string buf("POST /");
	buf += serviceName;
	buf += " HTTP/1.1\r\nTransfer-Encoding: chunked\r\n"
	    "X-UbiCircuit-Version: " UBICIRCUIT_HTTP_PROTOCOL_VERSION "\r\n"
	    "X-UbiCircuit-Circuit: ";
	buf += circuit;
	buf += "\r\n"
	    "X-UbiCircuit-InputPort: ";
	buf += inputPort;
	if(inputArg.size() > 0){
	    buf += "\r\nX-UbiCircuit-InputArg: ";
	    buf += inputArg;
	}
	if(serviceArg.size() > 0){
	    buf += "\r\nX-UbiCircuit-ServiceArg: ";
	    buf += serviceArg;
	}
	buf += "\r\n\r\n";

	return buf;
    }

    bool UbiCircuitHeader::isValidData(){
	if(circuit.size() <= 0)
	    return false;
	return true;
    }

    std::string UbiCircuitHeader::getInputArg(){
	return inputArg;
    }
    std::string UbiCircuitHeader::getInputPort(){
	return inputPort;
    }
    std::string UbiCircuitHeader::getServiceName(){
	return serviceName;
    }
    std::string UbiCircuitHeader::getServiceArg(){
	return serviceArg;
    }

}; /* namespace NetPipe */
