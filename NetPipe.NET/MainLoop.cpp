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


#include "stdafx.h"

#include "NetPipe.NET.h"
#include <MainLoop.h>

namespace NetPipeDotNET {
    /* MainLoop Wrapper */
    MainLoop::MainLoop(){
	UmMainLoop = new NetPipe::MainLoop();
    }
    MainLoop::~MainLoop(){
	this->!MainLoop();
    }
    MainLoop::!MainLoop(){
	if(UmMainLoop != NULL)
	    delete UmMainLoop;
    }
    void MainLoop::addServiceManager(NetPipeDotNET::ServiceManager ^sm){
	if(UmMainLoop != NULL){
	    UmMainLoop->addServiceManager(sm->getUnmanagedObject());
	}
    }
    void MainLoop::run(int usec){
	if(UmMainLoop != NULL)
	    UmMainLoop->run(usec);
    }
    void MainLoop::run(){
	if(UmMainLoop != NULL)
	    UmMainLoop->run(0);
    }
}; /* namespace NetPipeDotNET */