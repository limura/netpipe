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
    struct rb_MainLoop_struct {
	NetPipe::MainLoop *ml;
    };

    void rb_cMainLoop_delete(rb_MainLoop_struct *mls){
	printf("NetPipe::MainLoop free. mls: %p\r\n", mls);
	if(mls != NULL){
	    printf("  mls->ml: %p\r\n", mls->ml);
	    if(mls->ml != NULL){
		delete mls->ml;
	    }
	    xfree(mls);
	}
    }

    VALUE rb_cMainLoop_alloc(VALUE self){
	struct rb_MainLoop_struct *mls = ALLOC(struct rb_MainLoop_struct);
	mls->ml = new NetPipe::MainLoop();
	printf("NetPipe::MainLoop alloc(); mls: %p msl->ml: %p\r\n", mls, mls->ml);
	return Data_Wrap_Struct(self, 0, rb_cMainLoop_delete, mls);
    }
    VALUE rb_cMainLoop_addServiceManager(VALUE obj, VALUE sm){
	printf("MainLoop.addServiceManager: TYPE(obj): %d\r\n", TYPE(sm));
	Check_Type(sm, T_DATA);
	struct rb_MainLoop_struct *mls;
	Data_Get_Struct(obj, struct rb_MainLoop_struct, mls);
	if(mls->ml == NULL)
	    rb_raise(rb_eTypeError, "NetPipe::MainLoop not initialized");

	VALUE vData = rb_funcall(sm, rb_intern("get_cServiceManager"), 0);
	Check_Type(vData, T_DATA);
	NetPipe::ServiceManager *smp = (NetPipe::ServiceManager *)RDATA(vData)->data;

	printf("NetPipe::MainLoop addServiceManager: %p\r\n", smp);
	mls->ml->addServiceManager(smp);
	return Qundef;
    }
    VALUE rb_cMainLoop_run(int argc, VALUE *arg, VALUE obj){
	int usec = 0;
	if(argc > 1)
	    rb_raise(rb_eTypeError, "usage: NetPipe::MainLoop.run([usec])");
	if(argc == 1){
	    Check_Type(argc, T_FIXNUM);
	    usec = FIX2INT(arg[0]);
	}

	struct rb_MainLoop_struct *mls;
	Data_Get_Struct(obj, struct rb_MainLoop_struct, mls);
	if(mls->ml == NULL)
	    rb_raise(rb_eTypeError, "NetPipe::MainLoop not initialized");

	printf("NetPipe::MainLoop run(%d)\r\n", usec);
	mls->ml->run(usec);
	return Qundef;
    }

    void Init_MainLoop(VALUE toplevel){
	VALUE mainLoopClass = rb_define_class_under(toplevel, "MainLoop", rb_cObject);
	rb_define_method(mainLoopClass, "addServiceManager", (VALUE (*)(...))rb_cMainLoop_addServiceManager, 1);
	rb_define_method(mainLoopClass, "run", (VALUE (*)(...))rb_cMainLoop_run, -1);
	rb_define_alloc_func(mainLoopClass, rb_cMainLoop_alloc);
    }
}
