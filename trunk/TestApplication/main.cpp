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
 * $Id: main.cpp 96 2007-07-08 11:46:50Z  $
 */

#include <NetPipe.h>
#include <net.h>
#include <tools.h>

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#else
#include <process.h>
#include <io.h>
#endif

void test_ServiceDB(){
    char *service = "hoge";
    char *host, *port;

    NetPipe::ServiceDB *db = NetPipe::ServiceDB::getInstance();

    host = db->QueryIPHostName(service);
    port = db->QueryTCPPortName(service);

    printf("host: %s\r\nport: %s\r\n", host, port);
}

class EchoService : public NetPipe::Service {
private:
public:
    void onEvent(NetPipe::PipeManager *pm, char *portName, char *arg,
	NetPipe::Service::EVENT_TYPE type, char *buf, size_t size){
	switch(type){
	    case NetPipe::Service::RECV:
	    case NetPipe::Service::FD_INPUT:
		pm->write("Output1", buf, size);
		pm->commit("Output1");
		break;
	    case NetPipe::Service::TIMER:
		{
		    char *msg = "TIMER_INPUT";
		    pm->write("Output1", msg, strlen(msg));
		    pm->commit("Output1");
		}
		break;
	    default:
		break;
	};
    }
};

class EchoCreator : public NetPipe::ServiceCreator {
private:
public:
    NetPipe::Service *createNewService(void *userData){
	return new EchoService();
    }
};

class ShService : public NetPipe::Service {
private:
    typedef std::map<std::string, int> ShFDMap;
    ShFDMap shFDMap;
    int callnum;
public:
    ShService(){
	shFDMap.clear();
	callnum = 0;
    }
    virtual ~ShService(){
	printf("delete ShService\n");
    }
    void onEvent(NetPipe::PipeManager *pm, char *portName, char *arg,
	NetPipe::Service::EVENT_TYPE type, char *buf, size_t size){
	int fd = -1;
      printf("onEvent(%p, \"%s\", \"%s\", %d, %p, %d)\n",
	     pm, portName, arg, type, buf, size);
	switch(type){
	    case NetPipe::Service::RECV:
		printf("  RECV\n");
#if 1
		if(shFDMap.find(arg) == shFDMap.end()){ // initialize shell
#ifdef HAVE__POPEN
		    FILE *fp = _popen(arg, "rb+");
#else
		    FILE *fp = popen(arg, "r+");
#endif
		    if(fp == NULL)
			return;

		    shFDMap[arg] = fileno(fp);
		    pm->addReadFD(fileno(fp));
		}
		fd = shFDMap[arg];
		if(fd >= 0 && size > 0){
		    printf("    output to shell (%d bytes)\n", size);
		    write(0, buf, size);
		    write(fd, buf, size);
		}

#if 0
		if(arg != NULL && strncmp(arg, "cat", 3) == 0){
		    int i;
		    printf("recv: ");
		    for(i = 0; i < size; i++)
			printf("%c", buf[i]);
		    printf("\n");
		    return;
		}
		char buf[1024];
		sprintf(buf, "HOGE: %d\n", callnum++);
		pm->write("stdout", buf, strlen(buf));
		pm->commit("stdout");
#endif
#else
		if(arg == NULL)
		    return;

		shFD *sh = shFDMap[arg];
		if(sh == NULL){ // initialize shell
		    printf("    initializing shell\n");
		    int inFDs[2], outFDs[2];
		    if(pipe(inFDs) != 0)
			return;
		    if(pipe(outFDs) != 0)
			return;
		    pid_t cpid = fork();
		    if(cpid < 0)
			return;
		    if(cpid == 0){ // Žq‹Ÿ
			close(0); close(1); close(2);
			if(dup(outFDs[0]) != 0)
			    return;
			if(dup(inFDs[1]) != 1)
			    return;
			if(dup(inFDs[1]) != 2)
			    return;
			close(outFDs[0]);
			close(outFDs[1]);
			close(inFDs[0]);
			close(inFDs[1]);
			execlp("sh", "sh", "-c", arg, NULL);
			exit(0);
		    }else{
			close(outFDs[0]);
			close(inFDs[1]);
		    }
		    sh = new shFD();
		    if(sh == NULL)
			throw "no more memory";
		    sh->inputFD = inFDs[0];
		    sh->outputFD = outFDs[1];
		    printf("sh->input/outputFD: %d, %d\n",
			sh->inputFD, sh->outputFD);
		    shFDMap[arg] = sh;
		    pm->addReadFD(sh->inputFD);
		}
		if(sh->outputFD >= 0 && size > 0){
		    printf("    output to shell (%d bytes)\n", size);
		    write(sh->outputFD, buf, size);
		}
#endif
		break;
	    case NetPipe::Service::RECV_DOWN:
		pm->exit();
		break;
	    case NetPipe::Service::FD_INPUT:
	        printf("  FD_INPUT\n    write to next service %d bytes\n", size);
	        write(0, buf, size);
		pm->write("stdout", buf, size);
		pm->commit("stdout");
		break;
	    case NetPipe::Service::FD_DOWN:
		pm->exit();
		break;
	    default:
		break;
	}
    }
};

class ShCreator : public NetPipe::ServiceCreator {
public:
    NetPipe::Service *createNewService(void *userData){
      printf("create ShService\n");
	return new ShService();
    };
};

class DataCreator : public NetPipe::Service {
private:
public:
    void onEvent(NetPipe::PipeManager *pm, char *portName, char *arg,
	NetPipe::Service::EVENT_TYPE type, char *buf, size_t size){
	char writeBuf[1024];
	int i;
	switch(type){
	    case NetPipe::Service::RECV:
		if(arg != NULL && sscanf(arg, "%d", &i) == 1){
#ifndef HAVE_TIMEGETTIME
		    struct timeval tv;
		    if(gettimeofday(&tv, NULL) == 0){
			pm->write("stdout", (char *)&tv, sizeof(tv));
		    }
#endif
		    memset(writeBuf, '!', sizeof(writeBuf));
		    while(i > 0){
			if(i > sizeof(writeBuf)){
			    pm->write("stdout", writeBuf, sizeof(writeBuf));
			    i -= sizeof(writeBuf);
			}else{
			    pm->write("stdout", writeBuf, i);
			    i = 0;
			}
		    }
		    pm->commit("stdout");
		}
		//pm->exit();
		break;
	    case NetPipe::Service::RECV_DOWN:
		printf("DataCreator got RECV_DOWN. exit\n");
		pm->exit();
		break;
	    default:
		break;
	};
    }
};
class Pipe : public NetPipe::Service {
private:
public:
    void onEvent(NetPipe::PipeManager *pm, char *portName, char *arg,
	NetPipe::Service::EVENT_TYPE type, char *buf, size_t size){
	switch(type){
	    case NetPipe::Service::RECV:
		if(buf != NULL && size > 0){
#ifndef HAVE_TIMEGETTIME
		    if(size > sizeof(struct timeval)){
			struct timeval tv1, tv2;
			memcpy(&tv1, buf, sizeof(tv1));
			if(gettimeofday(&tv2, NULL) == 0){
			    printf("DIFF TIME: %f\n", diff_time(&tv1, &tv2));
			}
		    }
#endif
		    pm->write("stdout", buf, size);
		    pm->commit("stdout");
		}
		break;
	    case NetPipe::Service::RECV_DOWN:
		printf("Pipe got RECV_DOWN. exit\n");
		pm->exit();
		break;
	    default:
		break;
	};
    }
};
class DevNull : public NetPipe::Service {
private:
public:
    void onEvent(NetPipe::PipeManager *pm, char *portName, char *arg,
	NetPipe::Service::EVENT_TYPE type, char *buf, size_t size){
	switch(type){
	    case NetPipe::Service::RECV:
#ifndef HAVE_TIMEGETTIME
		if(size > sizeof(struct timeval) && buf != NULL){
		    struct timeval tv1, tv2;
		    memcpy(&tv1, buf, sizeof(tv1));
		    if(gettimeofday(&tv2, NULL) == 0){
			printf("DIFF TIME: %f\n", diff_time(&tv1, &tv2));
		    }
		}
#endif
		break;
	    case NetPipe::Service::RECV_DOWN:
		printf("DevNull got RECV_DOWN. exit\n");
		pm->exit();
		break;
	    default:
		break;
	};
    }
};
class DataCreatorCreator : public NetPipe::ServiceCreator {
public:
    NetPipe::Service *createNewService(void *userData){
	return new DataCreator();
    };
};
class PipeCreator : public NetPipe::ServiceCreator {
public:
    NetPipe::Service *createNewService(void *userData){
	return new Pipe();
    };
};
class DevNullCreator : public NetPipe::ServiceCreator {
public:
    NetPipe::Service *createNewService(void *userData){
	return new DevNull();
    };
};


#ifndef _WIN32
void sigchld(int sig){
    wait(NULL);
}
void sigalrm(int sig){

}
#endif

int main(int argc, char *argv[]){
    NetPipeInit();

#ifndef _WIN32
    if(argc > 2){
	chdir(argv[2]);
    }
#endif
    if(argc > 1 && strcmp(argv[1], "-d") == 0){
	char filename[1024];
	sprintf(filename, "TestApplicationLog_IP-%s_PID-%d.txt", getGlobalIP4Addr(), getpid());
	if(freopen(filename, "a+", stdout) == NULL)
	    printf("can not freopen(%s)\n", filename);
	close(0);
	close(2);
#ifndef _WIN32
	daemon(1, 1);
#endif
    }
#ifndef _WIN32
    signal(SIGCHLD, sigchld);
    signal(SIGALRM, sigalrm);
#endif
    setbuf(stdout, NULL);

    NetPipe::MainLoop *ml = new NetPipe::MainLoop();
    NetPipe::ServiceManager *sm;

#if 0
    sm = new NetPipe::ServiceManager("echo");
    sm->addReadPort("Input1");
    sm->addWritePort("Output1");
    sm->addReadFD(0);
    sm->addTimer(1000);
    sm->addServiceCreator(new EchoCreator());
    ml->addServiceManager(sm);

    sm = new NetPipe::ServiceManager("sh");
    sm->addReadPort("stdin");
    sm->addWritePort("stdout");
    sm->addServiceCreator(new ShCreator());
    ml->addServiceManager(sm);
#endif

    char buf[1024];
    char *ip4addr = getGlobalIP4Addr();

    if(argc > 3)
	ip4addr = argv[3];

    sprintf(buf, "%s:%s", ip4addr, "DataCreator");
    sm = new NetPipe::ServiceManager(buf);
    sm->addWritePort("stdout");
    sm->addServiceCreator(new DataCreatorCreator());
    ml->addServiceManager(sm);

    sprintf(buf, "%s:%s", ip4addr, "Pipe");
    sm = new NetPipe::ServiceManager(buf);
    sm->addReadPort("stdin");
    sm->addWritePort("stdout");
    sm->addServiceCreator(new PipeCreator());
    ml->addServiceManager(sm);

    sprintf(buf, "%s:%s", ip4addr, "DevNull");
    sm = new NetPipe::ServiceManager(buf);
    sm->addReadPort("stdin");
    sm->addServiceCreator(new DevNullCreator());
    ml->addServiceManager(sm);

    ml->run();

    return 0;
}
