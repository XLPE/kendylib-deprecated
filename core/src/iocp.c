#ifdef _WIN
#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include <stdint.h>
#include "Engine.h"
#include "Socket.h"

int32_t  iocp_init(engine_t e)
{
	e->complete_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if(NULL == e->complete_port)
	{
		printf("\nError occurred while creating IOCP: %d.", WSAGetLastError());
		return -1;
	}
	return 0;
}

int32_t iocp_register(engine_t e, socket_t s)
{
	HANDLE hTemp;
	hTemp = CreateIoCompletionPort((HANDLE)s->fd, e->complete_port,(ULONG_PTR)s, 1);
	if (NULL == hTemp)
		return -1;
	s->engine = e;
	return 0;
}


int32_t iocp_unregister(engine_t e,socket_t s)
{
	s->engine = NULL;
	return 0;
}
typedef void (*CallBack)(int32_t,st_io*);
int32_t iocp_loop(engine_t n,int32_t timeout)
{
	assert(n);
	int32_t        bytesTransfer;
	socket_t       socket;
	st_io *overLapped = 0;
	uint32_t lastErrno = 0;
	BOOL bReturn;
	CallBack call_back;
	uint32_t ms;
	uint32_t tick = GetSystemMs();
	uint32_t _timeout = tick + timeout;
	do
	{
		ms = _timeout - tick;
		call_back = 0;
		lastErrno = 0;
		bReturn = GetQueuedCompletionStatus(
			n->complete_port,(PDWORD)&bytesTransfer,
			(LPDWORD)&socket,
			(OVERLAPPED**)&overLapped,ms);

		if(FALSE == bReturn && !overLapped)// || socket == NULL || overLapped == NULL)
		{
			break;
		}
		if(0 == bytesTransfer)
		{
			//�����жϻ����
			lastErrno = WSAGetLastError();			
			if(bReturn == TRUE && lastErrno == 0)
			{
			}
			else
			{
				if(overLapped->m_Type & IO_RECV)
					call_back = socket->OnRead;	
				else
					call_back = socket->OnWrite;
				if(FALSE == bReturn)
					bytesTransfer = -1;
			}
			overLapped->err_code = lastErrno;
		}
		else
		{	
			if(overLapped->m_Type & IO_RECV)
				call_back = socket->OnRead;
			else if(overLapped->m_Type & IO_SEND)
				call_back = socket->OnWrite;
			else
			{
				//����
				continue;
			}
		}

		if(call_back)
			call_back(bytesTransfer,overLapped);
		tick = GetSystemMs();
	}while(tick < _timeout);
	return 0;
}


#endif