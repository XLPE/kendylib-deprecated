/*
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _KENDYNET_H
#define _KENDYNET_H
#include <stdint.h>
#include "util/link_list.h"
//����ϵͳ֧�ֵ�����׽��ֺ�engine������
#define MAX_ENGINE 64
#define MAX_SOCKET 4096

/*IO�������ɶ���ʹ�õĽṹ*/
typedef struct
{
	LIST_NODE;
	struct iovec *iovec;
	int32_t    iovec_count;
	uint32_t err_code;
}st_io;


//��ʼ������ϵͳ
int32_t      InitNetSystem();

typedef int32_t HANDLE;
#ifndef INVAILD_HANDLE
#define INVAILD_HANDLE -1
#endif
struct block_queue;

//recv�������ʱcallback
typedef void (*OnRead)(int32_t,st_io*);
//send�������ʱcallback
typedef void (*OnWrite)(int32_t,st_io*);
//���ӹر�ʱ,������δ��ɵ�����ִ�е�callback
typedef void (*OnClear_pending)(st_io*);

HANDLE   CreateEngine();
void     CloseEngine(HANDLE);
int32_t  EngineRun(HANDLE,int32_t timeout);
int32_t  Bind2Engine(HANDLE,HANDLE,OnRead,OnWrite,OnClear_pending);

enum
{
	RECV_NOW = 1,
	RECV_POST = 0,
	SEND_NOW = 1,
	SEND_POST = 0,
};

int32_t WSASend(HANDLE,st_io*,int32_t flag,uint32_t *err_code);
int32_t WSARecv(HANDLE,st_io*,int32_t flag,uint32_t *err_code);

#endif
