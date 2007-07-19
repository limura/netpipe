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
 * $Id$
 */

#include "stdafx.h"
#include "NetPipe.NET.h"

#include <NetPipe.h>
#include <PipeManager.h>

namespace NetPipeDotNET {
    PipeManager::PipeManager(NetPipe::PipeManager *Original){
	original = Original;
    }

    bool PipeManager::write(System::String ^portName, array<System::Byte> ^buf){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	pin_ptr<System::Byte> pbuf = &buf[0];
	bool ret = original->write(name, (char *)pbuf, buf->Length);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
	return ret;
    }
    bool PipeManager::write(System::String ^portName, array<System::Byte> ^buf, int length){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	pin_ptr<System::Byte> pbuf = &buf[0];
	bool ret = original->write(name, (char *)pbuf, length);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
	return ret;
    }
    bool PipeManager::write(System::String ^portName, System::String ^str){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	char *mStr = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str)).ToPointer());
	bool ret = original->write(name, mStr, strlen(mStr));
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)mStr));
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
	return ret;
    }
    bool PipeManager::commit(System::String ^portName){
	char *name = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(portName)).ToPointer());
	bool ret = original->commit(name);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)name));
	return ret;
    }
    void PipeManager::exit(){
	return original->exit();
    }
};