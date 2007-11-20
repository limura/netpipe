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

class rb_ServiceWrapper : public NetPipe::Service {
private:
    VALUE serviceObject;
public:
    rb_ServiceWrapper(VALUE obj){
	serviceObject = obj;
	// この Service Object も GC から見える位置に置かないと消される
	rb_gc_register_address(&serviceObject);
    }
    ~rb_ServiceWrapper(){
	// このときに serviceObject を GC から見えない位置に置かないと消えない
	rb_gc_unregister_address(&serviceObject);
    }
    bool onEvent(NetPipe::PipeManager *pipeManager, char *portName,
	char *arg, NetPipe::Service::EVENT_TYPE type, char *buf, size_t bufSize){
	Check_Type(serviceObject, T_OBJECT);
	VALUE ret;
	VALUE pmData = rb_data_object_alloc(NULL, pipeManager, NULL, NULL);
	VALUE mNetPipe = rb_define_module("NetPipe");
	VALUE serviceManagerClass = rb_define_class_under(mNetPipe, "PipeManager", rb_cObject);
	VALUE pmObj = rb_funcall(serviceManagerClass, rb_intern("new"), 1, pmData);
	//VALUE pmObj = rb_funcall(rb_cObject, rb_intern("NetPipe::PipeManager.new"), 1, pmData);

	ret = rb_funcall(serviceObject, rb_intern("onEvent"), 5,
	    pmObj, rb_str_new2(portName != NULL ? portName : ""), rb_str_new2(arg != NULL ? arg : ""),
	    INT2NUM(type), rb_str_new(buf, bufSize));
	if(TYPE(ret) == T_TRUE)
	    return true;
	if(TYPE(ret) == T_FALSE)
	    return false;
	return false;
    }
};

class rb_ServiceCreatorWrapper : public NetPipe::ServiceCreator{
private:
    VALUE creatorObject;
public:
    rb_ServiceCreatorWrapper(VALUE obj){
	creatorObject = obj;
	// このときに、この ServiceCreator object が GC から見える位置に置かないと消される
	rb_gc_register_address(&creatorObject);
    }
    ~rb_ServiceCreatorWrapper(){
	// このときに creatorObject を GC の見えない位置に置かないと消えない
	rb_gc_unregister_address(&creatorObject);
    }

    NetPipe::Service *createNewService(void *userData){
	Check_Type(creatorObject, T_OBJECT);
	VALUE ret;
	if(userData != NULL){
	    ret = rb_funcall(creatorObject, rb_intern("createNewService"), 1, (VALUE)userData);
    	}else{
	    ret = rb_funcall(creatorObject, rb_intern("createNewService"), 0);
    	}
	Check_Type(ret, T_OBJECT);
	return new rb_ServiceWrapper(ret);
    }
};

extern "C" {
    struct rb_ServiceManager_struct {
	NetPipe::ServiceManager *sm;
    };

    void rb_cServiceManager_delete(rb_ServiceManager_struct *sms){
	printf("NetPipe::ServiceManager delete. sms: %p\r\n", sms);
	if(sms != NULL){
	    printf("  sms->sm: %p\r\n", sms->sm);
	    if(sms->sm != NULL){
		delete sms->sm;
	    }
	    xfree(sms);
	}
    }

    VALUE rb_cServiceManager_alloc(VALUE self){
	struct rb_ServiceManager_struct *sms = ALLOC(struct rb_ServiceManager_struct);
	printf("NetPipe::ServiceManager alloc. sms: %p\r\n", sms);
	sms->sm = NULL;
	return Data_Wrap_Struct(self, 0, rb_cServiceManager_delete, sms);
    }

    VALUE rb_cServiceManager_initialize(VALUE obj, VALUE name){
	printf("NetPipe::ServiceManager initialize: TYPE(name): %d\r\n", TYPE(name));
	Check_Type(name, T_STRING);
	if(RSTRING(name)->len <= 0)
	    rb_raise(rb_eTypeError, "ServiceManager can not use null Name");

	struct rb_ServiceManager_struct *sms;
	Data_Get_Struct(obj, struct rb_ServiceManager_struct, sms);
	sms->sm = new NetPipe::ServiceManager(RSTRING(name)->ptr);
	printf("NetPipe::ServiceManager initialize sms->sm: %p\r\n", sms->sm);
	return obj;
    }

    VALUE rb_cServiceManager_addReadPort(int argc, VALUE *arg, VALUE obj){
	if(argc < 0 || argc > 2)
	    rb_raise(rb_eTypeError, "usage ServiceManager.addReadPort(\"name\" [\"description\"])");
	struct rb_ServiceManager_struct *sms;
	Data_Get_Struct(obj, struct rb_ServiceManager_struct, sms);
	if(sms->sm == NULL)
	    rb_raise(rb_eTypeError, "ServiceManager not initialized");
	Check_Type(arg[0], T_STRING);
	if(argc == 1){
	    sms->sm->addReadPort(RSTRING(arg[0])->ptr);
	}else{
	    Check_Type(arg[1], T_STRING);
	    sms->sm->addReadPort(RSTRING(arg[0])->ptr, RSTRING(arg[1])->ptr);
	}
	return Qundef;
    }
    VALUE rb_cServiceManager_addWritePort(int argc, VALUE *arg, VALUE obj){
	if(argc < 0 || argc > 2)
	    rb_raise(rb_eTypeError, "usage ServiceManager.addWritePort(\"name\" [\"description\"])");
	struct rb_ServiceManager_struct *sms;
	Data_Get_Struct(obj, struct rb_ServiceManager_struct, sms);
	if(sms->sm == NULL)
	    rb_raise(rb_eTypeError, "ServiceManager not initialized");
	Check_Type(arg[0], T_STRING);
	if(argc == 1){
	    sms->sm->addWritePort(RSTRING(arg[0])->ptr);
	}else{
	    Check_Type(arg[1], T_STRING);
	    sms->sm->addWritePort(RSTRING(arg[0])->ptr, RSTRING(arg[1])->ptr);
	}
	return Qundef;
    }
    VALUE rb_cServiceManager_addServiceCreator(int argc, VALUE *arg, VALUE obj){
	if(argc < 0 || argc > 2)
	    rb_raise(rb_eTypeError, "usage ServiceManager.addServiceCreator(ServiceCreatorObject, [userData])");
	struct rb_ServiceManager_struct *sms;
	Data_Get_Struct(obj, struct rb_ServiceManager_struct, sms);
	if(sms->sm == NULL)
	    rb_raise(rb_eTypeError, "ServiceManager not initialized");
	Check_Type(arg[0], T_OBJECT);
	rb_ServiceCreatorWrapper *scw = new rb_ServiceCreatorWrapper(arg[0]);
	if(argc == 1){
	    sms->sm->addServiceCreator(scw);
	}else{
	    sms->sm->addServiceCreator(scw, (void *)arg[1]);
	}
	return Qundef;
    }
    VALUE rb_cServiceManager_get_cServiceManager(VALUE obj){
	struct rb_ServiceManager_struct *sms;
	Data_Get_Struct(obj, struct rb_ServiceManager_struct, sms);
	if(sms->sm == NULL)
	    rb_raise(rb_eTypeError, "ServiceManager not initialized");

	return rb_data_object_alloc(NULL, sms->sm, NULL, NULL);
    }

    void Init_ServiceManager(VALUE toplevel){
	VALUE serviceManagerClass = rb_define_class_under(toplevel, "ServiceManager", rb_cObject);
	rb_define_method(serviceManagerClass, "initialize", (VALUE (*)(...))rb_cServiceManager_initialize, 1);
	rb_define_method(serviceManagerClass, "addReadPort", (VALUE (*)(...))rb_cServiceManager_addReadPort, -1);
	rb_define_method(serviceManagerClass, "addWritePort", (VALUE (*)(...))rb_cServiceManager_addWritePort, -1);
	rb_define_method(serviceManagerClass, "addServiceCreator", (VALUE (*)(...))rb_cServiceManager_addServiceCreator, -1);
	rb_define_method(serviceManagerClass, "get_cServiceManager", (VALUE (*)(...))rb_cServiceManager_get_cServiceManager, 0);
	rb_define_alloc_func(serviceManagerClass, rb_cServiceManager_alloc);

	VALUE serviceModule = rb_define_module_under(toplevel, "Service");
	rb_define_const(serviceModule, "RECV", INT2NUM(NetPipe::Service::RECV));
	rb_define_const(serviceModule, "RECV_DOWN", INT2NUM(NetPipe::Service::RECV_DOWN));
	rb_define_const(serviceModule, "SEND_DOWN", INT2NUM(NetPipe::Service::SEND_DOWN));
	rb_define_const(serviceModule, "FD_INPUT", INT2NUM(NetPipe::Service::FD_INPUT));
	rb_define_const(serviceModule, "FD_DOWN", INT2NUM(NetPipe::Service::FD_DOWN));
	rb_define_const(serviceModule, "TIMER", INT2NUM(NetPipe::Service::TIMER));
	rb_define_const(serviceModule, "CREATED", INT2NUM(NetPipe::Service::CREATED));
    }
}