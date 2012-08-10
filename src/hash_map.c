#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "hash_map.h"

enum
{
	ITEM_EMPTY = 0,
	ITEM_DELETE,
	ITEM_USED,
};

struct hash_item
{
	uint64_t hash_code;
	int8_t flag;
	int8_t key_and_val[0];
};

struct hash_map
{
	hash_func   _hash_function;
	hash_key_eq _key_cmp_function;
	uint32_t slot_size;
	uint32_t size;
	uint32_t key_size;
	uint32_t val_size;
	uint32_t item_size;
	uint32_t expand_size;
	struct hash_item *_items;
};

#define GET_KEY(HASH_MAP,ITEM) ((void*)(ITEM)->key_and_val)

#define GET_VAL(HASH_MAP,ITEM) ((void*)&(ITEM)->key_and_val[(HASH_MAP)->key_size])

#define GET_ITEM(ITEMS,ITEM_SIZE,SLOT) ((struct hash_item*)&((int8_t*)(ITEMS))[ITEM_SIZE*SLOT])

#define HASH_MAP_INDEX(HASH_CODE,SLOT_SIZE) (HASH_CODE % SLOT_SIZE)

int32_t hash_map_size(hash_map_t h)
{
	return h->size;
}

hash_map_t hash_map_create(uint32_t slot_size,uint32_t key_size,
	uint32_t val_size,hash_func hash_function,hash_key_eq key_cmp_function)
{
	hash_map_t h = (hash_map_t)malloc(sizeof(*h));
	if(!h)
		return 0;
	h->slot_size = slot_size;
	h->size = 0;
	h->_hash_function = hash_function;
	h->_key_cmp_function = key_cmp_function;
	h->key_size = key_size;
	h->val_size = val_size;
	h->item_size = sizeof(struct hash_item) + key_size + val_size;
	h->_items = calloc(slot_size,h->item_size);
	h->expand_size = h->slot_size - h->slot_size/4;
	if(!h->_items)
	{
		free(h);
		return 0;
	}
	return h;
}

void hash_map_destroy(hash_map_t *h)
{
	uint32_t i = 0;
	struct hash_item *item;
	free((*h)->_items);
	free(*h);
	*h = 0;
}

static struct hash_item *_hash_map_insert(hash_map_t h,void* key,void* val,uint64_t hash_code)
{
	uint32_t slot = HASH_MAP_INDEX(hash_code,h->slot_size);
	uint32_t check_count = 0;
	struct hash_item *item = 0;
	while(check_count < h->slot_size)
	{
		item = GET_ITEM(h->_items,h->item_size,slot);
		if(item->flag != ITEM_USED)
		{
			//�ҵ���λ����
			item->flag = ITEM_USED;
			memcpy(item->key_and_val,key,h->key_size);
			memcpy(&(item->key_and_val[h->key_size]),val,h->val_size);
			item->hash_code = hash_code;
			//printf("check_count:%d\n",check_count);
			return item;
		}
		else
			if(hash_code == item->hash_code && h->_key_cmp_function(key,GET_KEY(h,item)) == 0)
				break;//�Ѿ��ڱ��д���
		slot = (slot + 1)%h->slot_size;
		check_count++;
	}
	//����ʧ��
	return NULL;
}

static int32_t _hash_map_expand(hash_map_t h)
{
	uint32_t old_slot_size = h->slot_size;
	struct hash_item *old_items = h->_items;
	uint32_t i = 0;
	h->slot_size <<= 1;
	h->_items = calloc(h->slot_size,h->item_size);
	if(!h->_items)
	{
		h->_items = old_items;
		h->slot_size >>= 1;
		return -1;
	}
	for(; i < old_slot_size; ++i)
	{
		struct hash_item *_item = GET_ITEM(old_items,h->item_size,i);
		if(_item->flag == ITEM_USED)
			_hash_map_insert(h,GET_KEY(h,_item),GET_VAL(h,_item),_item->hash_code);
	}
	h->expand_size = h->slot_size - h->slot_size/4;
	free(old_items);
	return 0;
	
}

hash_map_iter hash_map_insert(hash_map_t h,void *key,void *val)
{
	uint64_t hash_code = h->_hash_function(key);
	if(h->slot_size < 0x80000000 && h->size >= h->expand_size)
		//�ռ�ʹ�ó���3/4��չ
		_hash_map_expand(h);
	hash_map_iter iter = {0,0};	
	if(h->size >= h->slot_size)
		return iter;
	struct hash_item *item = _hash_map_insert(h,key,val,hash_code);	
	if( item != NULL)
	{
		++h->size;
		iter.data1 = h;
		iter.data2 = item;
	}
	return iter;
	
}

static struct hash_item *_hash_map_find(hash_map_t h,void *key)
{
	uint64_t hash_code = h->_hash_function(key);
	uint32_t slot = HASH_MAP_INDEX(hash_code,h->slot_size);
	uint32_t check_count = 0;
	struct hash_item *item = 0;
	while(check_count < h->slot_size)
	{
		item = GET_ITEM(h->_items,h->item_size,slot);
		if(item->flag == ITEM_EMPTY)
			return 0;
		if(item->hash_code == hash_code && h->_key_cmp_function(key,GET_KEY(h,item)) == 0)
		{
			if(item->flag == ITEM_DELETE)
				return 0;
			else
				return item;
		}
		slot = (slot + 1)%h->slot_size;
		check_count++;
	}
	return 0;
}

hash_map_iter hash_map_find(hash_map_t h,void* key)
{
	struct hash_item *item = _hash_map_find(h,key);
	hash_map_iter iter = {0,0};
	if(item)
	{
		iter.data1 = h;
		iter.data2 = item;
	}
	return iter;
}

void* hash_map_remove(hash_map_t h,void* key)
{
	struct hash_item *item = _hash_map_find(h,key);
	if(item)
	{
		item->flag = ITEM_DELETE;
		--h->size;
		return GET_VAL(h,item);;
	}
	return NULL;
}

void* hash_map_erase(hash_map_t h,hash_map_iter iter)
{
	if(iter.data1 && iter.data2)
	{
		hash_map_t h = (hash_map_t)iter.data1;
		struct hash_item *item = (struct hash_item *)iter.data2;
		item->flag = ITEM_DELETE;
		--h->size;
		return GET_VAL(h,item);	
	}	
	return NULL;
}

int32_t hash_map_is_vaild_iter(hash_map_iter iter)
{
	return iter.data1 && iter.data2;
}

void* hash_map_iter_get_val(hash_map_iter iter)
{
	hash_map_t h = (hash_map_t)iter.data1;
	struct hash_item *item = (struct hash_item *)iter.data2;
	return GET_VAL(h,item);
}

void hash_map_iter_set_val(hash_map_iter iter,void *data)
{
	hash_map_t h = (hash_map_t)iter.data1;
	struct hash_item *item = (struct hash_item *)iter.data2;
	void *old_val = GET_VAL(h,item);
	if(data != old_val)
	{
		void *ptr = (void*)&((item)->key_and_val[(h)->key_size]);
		memcpy(ptr,data,h->val_size);
	}
}
