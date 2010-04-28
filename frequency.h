#ifndef __FREQUENCY_H__
#define __FREQUENCY_H__

#include<stdio.h>
#include<stdint.h>
#include<inttypes.h>

extern unsigned long long int occurrence[];

typedef struct {
	uint8_t symbol;
	uint64_t frequency;
}frequency_row_t;

typedef struct {
	unsigned int length;
	frequency_row_t* frequencies;
}frequency_table_t;

typedef enum {
	FREQUENCY_DONT_SAVE_STREAM,
	FREQUENCY_SAVE_STREAM
}frequency_save_t;

void frequency_table_create_from_stream(FILE* ff, frequency_save_t save);
void print_frequencies_table(frequency_table_t t);
void order_frequencies_table(frequency_table_t t);
unsigned char* frequency_get_stream_content(void);
unsigned int frequency_get_stream_size(void);

#endif
