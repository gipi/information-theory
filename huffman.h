#ifndef __HUFFMAN__H_
#define __HUFFMAN__H_

#include<stdint.h>

#define node_is_leaf(n) (!(n).left)

typedef struct _node_t{
	uint8_t symbol;
	uint64_t weight;
	struct _node_t* left;
	struct _node_t* right;
}node_t;

typedef struct {
	unsigned int length;
	node_t* nodes;
}three_t;

typedef struct {
	uint8_t symbol;
	uint64_t frequency;
}frequency_row_t;

typedef struct {
	unsigned int length;
	frequency_row_t* frequencies;
}frequency_table_t;

typedef struct {
	uint8_t symbol;
	uint8_t code_size;
	uint64_t code;
}huffman_row_t;

typedef struct {
	huffman_row_t row;
}huffman_table_t;

/* write symbols to file pointed by fd using the ht huffman table */
size_t huffman_write_symbols(int fd, huffman_table_t ht,
	uint8_t* symbols, size_t length);
size_t huffman_read_symbols(int fd, huffman_table_t ht, uint8_t* symbols);

#endif
