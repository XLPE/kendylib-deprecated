#include "allocator.h"
struct first_fit_chunk
{
	//״̬1Ϊ����,0Ϊ����
	int32_t tag;               //��ǰ���״̬
	uint32_t size;     //��ǰ��Ĵ�С
	union{
		struct{
			struct first_fit_chunk *next;//��һ�����п�
			struct first_fit_chunk *pre; //ǰһ�����п�
		};
		uint8_t buf[1];
	};
};

struct first_fit_pool
{
	IMPLEMEMT(allocator);
	struct first_fit_chunk ava_head;
	uint32_t pool_size;
	uint8_t *begin;
	uint8_t *end;
	uint8_t buf[1];
};



#define FREE_TAG 0XFEDCBA98
#define USE_TAG  0X89ABCDEF
#define CHUNK_HEAD (sizeof(int32_t) + sizeof(uint32_t))
#define RESERVESIZE (sizeof(struct first_fit_chunk) + sizeof(int32_t))
