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


// NetPipe.NET.h

#pragma once

namespace NetPipe {
    class MainLoop;
    class ServiceManager;
    class ServiceCreator;
    class PipeManager;
    class Kicker;
};
namespace NetPipeDotNET {
    public ref class Initializer {
    public:
	static void Initialize();
    };

    public ref class Kicker {
    private:
	NetPipe::Kicker *original;
    public:
	Kicker();
	~Kicker();
	!Kicker();

	void kick(System::String ^pipePath, System::String ^CircuitID);
    };

    public ref class PipeManager {
    private:
	NetPipe::PipeManager *original;
    public:
	PipeManager(NetPipe::PipeManager *Original);

	// void addReadFD(int fd, size_t bufSize = 4096);
	bool write(System::String ^portName, array<System::Byte> ^buf);
	bool write(System::String ^portName, array<System::Byte> ^buf, int length);
	bool write(System::String ^portName, System::String ^str);
	bool commit(System::String ^portName);
	void exit();
    };

    public ref class Service abstract {
    public:
	enum class EVENT_TYPE {
	    RECV,
	    RECV_DOWN,
	    FD_INPUT,
	    FD_DOWN,
	    TIMER,
	};
	virtual void onEvent(NetPipeDotNET::PipeManager ^pipeManager,
	    System::String ^portName, System::String ^arg,
	    EVENT_TYPE type, array<System::Byte> ^buf) = 0;
    };

    public ref class ServiceCreator abstract {
    public:
	virtual Service ^createNewService(System::IntPtr userData) = 0;
    };

    public ref class ServiceManager {
    private:
	NetPipe::ServiceManager *UmServiceManager;
    public:
	ServiceManager(System::String ^serviceName);
	~ServiceManager();
	!ServiceManager();

	NetPipe::ServiceManager *getUnmanagedObject();

	System::String ^getServiceName();
	void addReadPort(System::String ^portName, System::String ^description);
	void addReadPort(System::String ^portName);
	void addWritePort(System::String ^portName, System::String ^description);
	void addWritePort(System::String ^portName);
	//void addReadFD(int fd, string description = "UNDEFINED", size_t bufSize = 4096);
	// void addTimer(int usec, string description = "UNDEFINED");
	void addServiceCreator(NetPipeDotNET::ServiceCreator ^serviceCreator, System::IntPtr userData);
	void addServiceCreator(NetPipeDotNET::ServiceCreator ^serviceCreator);
    };

    public ref class MainLoop
    {
    private:
	NetPipe::MainLoop *UmMainLoop;
    public:
	MainLoop();
	~MainLoop();
	!MainLoop();
	void addServiceManager(NetPipeDotNET::ServiceManager ^sm);
	void run(int usec);
	void run();
    };
}
