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

#include <gcroot.h>

#include <map>

#include <NetPipe.h>
#include <MainLoop.h>
#include <ServiceManager.h>
#include <ServiceCreator.h>
#include <Service.h>

#using <mscorlib.dll>

namespace NetPipeDotNET {
    ServiceManager::ServiceManager(System::String ^serviceName){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(serviceName)).ToPointer());
	UmServiceManager = new NetPipe::ServiceManager(name);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
    }
    ServiceManager::~ServiceManager(){
	this->!ServiceManager();
    }
    ServiceManager::!ServiceManager(){
	if(UmServiceManager != NULL)
	    delete UmServiceManager;
    }
    NetPipe::ServiceManager *ServiceManager::getUnmanagedObject(){
	return UmServiceManager;
    }
    System::String ^ServiceManager::getServiceName(){
	if(UmServiceManager == NULL)
	    return gcnew System::String(""); // null を返すにはどうすればいいのー？！
	char *serviceName = UmServiceManager->getServiceName();
	return gcnew System::String(serviceName);
    }
    void ServiceManager::addReadPort(System::String ^portName, System::String ^description){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	char *desc = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(description)).ToPointer());
	UmServiceManager->addReadPort(name, desc);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)desc));
    }
    void ServiceManager::addReadPort(System::String ^portName){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	UmServiceManager->addReadPort(name);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
    }
	void addReadPort(System::String ^portName);
    void ServiceManager::addWritePort(System::String ^portName, System::String ^description){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	char *desc = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(description)).ToPointer());
	UmServiceManager->addWritePort(name, desc);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)desc));
    }
    void ServiceManager::addWritePort(System::String ^portName){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	UmServiceManager->addWritePort(name);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
    }
    //void addReadFD(int fd, string description = "UNDEFINED", size_t bufSize = 4096);
    // void addTimer(int usec, string description = "UNDEFINED");

    class ServiceWrapper : public NetPipe::Service {
    private:
	gcroot<NetPipeDotNET::Service ^> gcService;
    public:
	ServiceWrapper(gcroot<NetPipeDotNET::Service ^>service){
	    gcService = service;
	}
	bool onEvent(NetPipe::PipeManager *pipeManager, char *portName,
	    char *arg, NetPipe::Service::EVENT_TYPE type, char *buf, size_t bufSize){
		array<System::Byte>^ byteArray = gcnew array<System::Byte>(bufSize);
		pin_ptr<System::Byte> pBuf = &byteArray[0];
		memcpy(pBuf, buf, bufSize);
		gcService->onEvent(gcnew NetPipeDotNET::PipeManager(pipeManager),
		    gcnew System::String(portName), gcnew System::String(arg),
		    (NetPipeDotNET::Service::EVENT_TYPE)type,
		    byteArray);
		return false;
	}
    };

    class ServiceCreatorWrapper : public NetPipe::ServiceCreator {
    private:
	gcroot<NetPipeDotNET::ServiceCreator ^> gcServiceCreator;
    public:
	ServiceCreatorWrapper(NetPipeDotNET::ServiceCreator ^serviceCreator){
	    gcServiceCreator = serviceCreator;
	}
	NetPipe::Service *createNewService(void *userData){
	    gcroot<NetPipeDotNET::Service ^> service = gcServiceCreator->createNewService((System::IntPtr)userData);
	    // XXXX ここで作った ServiceWrapper はどこからも参照されてないので GC に殺されないか？
	    return new ServiceWrapper(service);
	}
    };

    void ServiceManager::addServiceCreator(NetPipeDotNET::ServiceCreator ^serviceCreator, System::IntPtr userData){
	ServiceCreatorWrapper *scw = new ServiceCreatorWrapper(serviceCreator);
	// XXXX ここで作った ServiceCreatorWrapper はどこからも参照されてないので GC に殺されないか？
	UmServiceManager->addServiceCreator(scw, (void *)userData);
    }
    void ServiceManager::addServiceCreator(NetPipeDotNET::ServiceCreator ^serviceCreator){
	this->addServiceCreator(serviceCreator, System::IntPtr::Zero);
    }
}; /* namespace NetPipeDotNET */