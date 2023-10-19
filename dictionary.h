#ifndef DICTIONARY_H_INCLUDED
#define DICTIONARY_H_INCLUDED

typedef struct dictionary dictionary;

dictionary* dictionaryOpen(char* filename);
void dictionaryClose(dictionary* d);
void dictionaryAdd(dictionary* d,void* key,unsigned key_size, void* data,unsigned data_size);
void dictionaryGet(dictionary* d,void* key,unsigned key_size,void** data,unsigned* data_size);
void dictionarySafeWrite(dictionary* d,void* key,unsigned key_size,void* data,unsigned data_size);
void dictionaryGenerateHashTable(dictionary* d);
unsigned dictionaryGetSize(dictionary* d);

#endif