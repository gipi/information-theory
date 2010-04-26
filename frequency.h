#ifndef __FREQUENCY_H__
#define __FREQUENCY_H__

#include<stdio.h>

extern unsigned long long int occurrence[];

typedef enum {
	FREQUENCY_DONT_SAVE_STREAM,
	FREQUENCY_SAVE_STREAM
}frequency_save_t;

void frequency_table_create_from_stream(FILE* ff, frequency_save_t save);
unsigned char* frequency_get_stream_content(void);
unsigned int frequency_get_stream_size(void);

#endif
