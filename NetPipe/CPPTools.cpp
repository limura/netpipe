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

#include "config.h"
#include <stdarg.h>

#include "CPPTools.h"
#include "tools.h"

namespace NetPipe {

    ///
    /*
     * targets に指定された文字列をセパレータとして分割された string の list を返す。
     * max が 0 以下ならば分割できるだけ分割し、0より大きければその数になるだけしか分割しない。
     * NULL 以外が返った場合は delete するのは読んだ側の責任。
     */
    CPPTool::stringList* CPPTool::split(char *buf, size_t size, char **targets, int max){
	int i;
	char *p, *candidate, *target;

	if(buf == NULL || targets == NULL || targets[0] == NULL)
	    return NULL;

	stringList *slist = new stringList();

	while(size > 0){
	    candidate = buf + size;
	    i = 0;
	    while(targets[i] != NULL){
		p = strnstr(buf, targets[i], size);
		if(p != NULL && p < candidate){
		    candidate = p;
		    target = targets[i];
		}
		i++;
	    }
	    if((max <= 0 || --max > 0) && candidate != buf + size){
		slist->push_back(std::string(buf, (int)(candidate - buf)));
		size -= candidate + strlen(target) - buf;
		buf = candidate + strlen(target);

		while(1){
		    int flag = 0;
		    i = 0;
		    while(targets[i] != NULL){
			if(strncmp(buf, targets[i], size) == 0){
			    flag = 1;
			    size -= strlen(targets[i]);
			    buf += strlen(targets[i]);
			    break;
			}
			i++;
		    }
		    if(flag == 0)
			break;
		}
	    }else{
		slist->push_back(std::string(buf, size));
		size = 0;
	    }
	}
	return slist;
    }

}; /* namespace NetPipe */

