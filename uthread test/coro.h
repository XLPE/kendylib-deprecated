#ifndef _CORO_H
#define _CORO_H
#include <stdint.h>
#include "uthread.h"

struct coro
{
	struct coro *next;
	uthread_t ut;
	uint32_t id;
	start_fun st_fun;
	uint32_t is_end;
};

struct testarg
{
	struct coro *sche;
	struct coro *co;
};

void* yield(struct coro*,struct coro*,void *);
void* resume(struct coro*,struct coro*,void *);


struct scheduler
{
	struct coro *active;  //���ڼ���̬��coro
	struct coro *self;
};

struct scheduler *scheduler_create();
//����һ��coro����start_run
void spawn(struct scheduler*,void *stack,uint32_t stack_size,start_fun);
//����coro����
void schedule(struct scheduler*);

#endif