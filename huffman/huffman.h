#ifndef __HUFFMAN__H_
#define __HUFFMAN__H_

#include<stdint.h>
#include<stddef.h>

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
}tree_t;

typedef struct {
	uint8_t symbol;
	uint8_t nbits;
}__attribute__((__packed__)) huffman_t;

extern huffman_t* Huffman;
/* this contains the number of symbol - 1 to avoid overflow */
extern uint8_t HuffmanLength;

typedef struct {
	uint8_t symbol;
	uint8_t code_size;
	uint64_t code;
}huffman_row_t;

typedef struct {
	unsigned int length;
	huffman_row_t* rows;
}huffman_table_t;

tree_t* tree_init_from_frequencies(frequency_table_t t);
tree_t* tree_step(tree_t* t);
void node_walk(node_t n, uint64_t length);
huffman_table_t Huffman_canonicalize(void);
huffman_table_t huffman_canonical_from_stream(FILE* f);
huffman_table_t huffman_table_read_from_stream(FILE* f);
int huffman_decode_one_symbol(uint8_t*, huffman_table_t t, FILE* f);
void huffman_code_print(huffman_row_t row);
huffman_row_t huffman_get_code_from_symbol(huffman_table_t t, uint8_t symb);
size_t huffman_get_encoded_size(huffman_table_t t,
	uint8_t* buffer, size_t size);
/* write symbols to file pointed by fd using the ht huffman table */
size_t huffman_write_symbols(int fd, huffman_table_t ht,
	uint8_t* symbols, size_t length);
size_t huffman_read_symbols(int fd, huffman_table_t ht, uint8_t* symbols);
uint64_t huffman_canonicalize_step(
	uint64_t previous_code, uint8_t previous_size, uint8_t new_size);

#endif
