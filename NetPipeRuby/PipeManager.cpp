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
 * $Id: Acceptor.cpp 51 2007-07-04 01:22:02Z  $
 */

#include <NetPipe.h>
#include <ruby.h>

extern "C" {
    struct rb_PipeManager_struct {
	NetPipe::PipeManager *pm;
    };

    void rb_PipeManager_delete(rb_PipeManager_struct *pms){
	if(pms != NULL){
	    if(pms->pm != NULL)
		delete pms->pm;
	    xfree(pms);
	}
    }

    VALUE rb_cPipeManager_alloc(VALUE self){
	struct rb_PipeManager_struct *pms = ALLOC(struct rb_PipeManager_struct);
	pms->pm = NULL;
	return Data_Wrap_Struct(self, 0, rb_PipeManager_delete, pms);
    }

    VALUE rb_cPipeManager_initialize(VALUE obj, VALUE pmData){
	Check_Type(pmData, T_DATA);
	if(RDATA(pmData)->data == NULL)
	    rb_raise(rb_eTypeError, "PipeManager Initialize error. user can not use this initializer");

	struct rb_PipeManager_struct *pms;
	Data_Get_Struct(obj, struct rb_PipeManager_struct, pms);
	pms->pm = (NetPipe::PipeManager *)RDATA(pmData)->data;
	return obj;
    }

    VALUE rb_cPipeManager_write(int argc, VALUE *arg, VALUE obj){
	if(argc < 1 || argc > 3)
	    rb_raise(rb_eTypeError, "usage PipeManager.write(\"portName\", data, [length])");
	struct rb_PipeManager_struct *pms;
	Data_Get_Struct(obj, struct rb_PipeManager_struct, pms);
	if(pms->pm == NULL)
	    rb_raise(rb_eTypeError, "PipeManager not initialized");
	Check_Type(arg[0], T_STRING);
	Check_Type(arg[1], T_STRING);
#undef write
	if(argc == 2){
	    pms->pm->write(RSTRING(arg[0])->ptr, RSTRING(arg[1])->ptr, RSTRING(arg[1])->len);
	}else{
	    size_t size;
	    size = RSTRING(arg[1])->len;
	    if(TYPE(arg[2]) == T_FIXNUM){
		if(FIX2ULONG(arg[2]) < size)
		    size = FIX2ULONG(arg[2]);
		pms->pm->write(RSTRING(arg[0])->ptr, RSTRING(arg[1])->ptr, size);
	    }else if(TYPE(arg[2]) == T_BIGNUM){
		pms->pm->write(RSTRING(arg[0])->ptr, RSTRING(arg[1])->ptr, size);
	    }else{
		rb_raise(rb_eTypeError, "PipeManager.write arg 3 is not length");
	    }
	}
	return Qundef;
    }
    VALUE rb_cPipeManager_commit(VALUE obj, VALUE portName){
	Check_Type(portName, T_STRING);
	struct rb_PipeManager_struct *pms;
	Data_Get_Struct(obj, struct rb_PipeManager_struct, pms);
	if(pms->pm == NULL)
	    rb_raise(rb_eTypeError, "PipeManager not initialized");
	pms->pm->commit(RSTRING(portName)->ptr);
	return Qundef;
    }
    VALUE rb_cPipeManager_exit(VALUE obj){
	struct rb_PipeManager_struct *pms;
	Data_Get_Struct(obj, struct rb_PipeManager_struct, pms);
	if(pms->pm == NULL)
	    rb_raise(rb_eTypeError, "PipeManager not initialized");
	pms->pm->exit();
	return Qundef;
    }
    VALUE rb_cPipeManager_addTimer(VALUE obj, VALUE tick){
	Check_Type(tick, T_FIXNUM);
	struct rb_PipeManager_struct *pms;
	Data_Get_Struct(obj, struct rb_PipeManager_struct, pms);
	if(pms->pm == NULL)
	    rb_raise(rb_eTypeError, "PipeManager not initialized");
	pms->pm->addTimer(FIX2INT(tick));
	return Qundef;

    }
    VALUE rb_cPipeManager_reconnect(VALUE obj, VALUE portName){
	Check_Type(portName, T_STRING);
	struct rb_PipeManager_struct *pms;
	Data_Get_Struct(obj, struct rb_PipeManager_struct, pms);
	if(pms->pm == NULL)
	    rb_raise(rb_eTypeError, "PipeManager not initialized");
	pms->pm->reconnect(RSTRING(portName)->ptr);
	return Qundef;
    }

    void Init_PipeManager(VALUE toplevel){
	VALUE pipeManagerClass = rb_define_class_under(toplevel, "PipeManager", rb_cObject);
	rb_define_method(pipeManagerClass, "initialize", (VALUE (*)(...))rb_cPipeManager_initialize, 1);
	rb_define_method(pipeManagerClass, "write", (VALUE (*)(...))rb_cPipeManager_write, -1);
	rb_define_method(pipeManagerClass, "commit", (VALUE (*)(...))rb_cPipeManager_commit, 1);
	rb_define_method(pipeManagerClass, "exit", (VALUE (*)(...))rb_cPipeManager_exit, 0);
	rb_define_method(pipeManagerClass, "addTimer", (VALUE (*)(...))rb_cPipeManager_addTimer, 1);
	rb_define_method(pipeManagerClass, "reconnect", (VALUE (*)(...))rb_cPipeManager_reconnect, 1);
	rb_define_alloc_func(pipeManagerClass, rb_cPipeManager_alloc);
    }
}