#include "globals.h"
	
bucket *ST_SCOPE_MAP[MAXSCOPE][HASH_TABLE_SIZE];

int hash(char *key){
	//key is the id name;
	int i=0, tmp =0;
	
	while(key[i]){
		tmp = ((tmp<<SHIFT) + key[i]) % HASH_TABLE_SIZE;
		++i;
	}
	return tmp;
}

// lookup and insert
bucket *lookup(char *key, bucket **ST_HASH_Array){
	int i=0, hashvalue = 0;
	while(key[i]){
		hashvalue = ((hashvalue<<SHIFT) + key[i]) % HASH_TABLE_SIZE;
		++i;
	}
	
	if(ST_HASH_Array[hashvalue] == NULL){
		return NULL;
	}else{
		bucket *tmp = ST_HASH_Array[hashvalue];
		while(strcmp(key, tmp->name) != 0){
			tmp = tmp->next;
			if(tmp == NULL)
				return NULL;
		}
		return tmp;
	}
}

// the parameter of addr was not used in current method
void insert(char *key, int addr, bucket **ST_HASH_Array){
	int i=0, hashvalue = 0;
	while(key[i]){
		hashvalue = ((hashvalue<<SHIFT) + key[i]) % HASH_TABLE_SIZE;
		++i;
	}
	
	if(ST_HASH_Array[hashvalue] == NULL){
		ST_HASH_Array[hashvalue] = (bucket *)malloc(sizeof(bucket));
		strcpy(ST_HASH_Array[hashvalue]->name, key);
		ST_HASH_Array[hashvalue]->addr = addr;
		ST_HASH_Array[hashvalue]->next = NULL;
	}
	else{
		bucket *tmp = ST_HASH_Array[hashvalue];
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = (bucket *)malloc(sizeof(bucket));
		tmp = tmp->next;
		strcpy(tmp->name, key);
		tmp->addr = addr;
		tmp->next = NULL;
	}
}









