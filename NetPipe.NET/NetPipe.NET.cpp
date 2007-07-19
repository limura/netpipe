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
#include <Kicker.h>

#using <mscorlib.dll>

namespace NetPipeDotNET {
    void Initializer::Initialize(){
	NetPipeInit();
    }

#if 0
    char *Util::SystemString2Char(System::String ^str, char *buf, size_t size){
	size_t len = str->Length;
	if(len <= 0)
	    return NULL;
	if(len * 2 > size)
	    return NULL;
	const char *mStr = static_cast<const char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str)).ToPointer());
	strncpy_s(buf, len + 1, mStr, _TRUNCATE);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)mStr));
	return buf;
    }
    char *Util::SystemString2Char(System::String ^str){
	static char buf[1024];
	return SystemString2Char(str, buf, sizeof(buf));
    }
#endif

    Kicker::Kicker(){
	original = new NetPipe::Kicker();
    }
    Kicker::~Kicker(){
	this->!Kicker();
    }
    Kicker::!Kicker(){
	if(original != NULL)
	    delete original;
    }

    void Kicker::kick(System::String ^pipePath){
	if(original != NULL){
	    char *mStr = static_cast<char*>((System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(pipePath)).ToPointer());
	    original->kick(mStr);
	    System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void *)mStr));
	}
    }

} /* */