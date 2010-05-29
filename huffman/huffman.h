#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include<stdint.h>
#include<stddef.h>


typedef struct {
	uint8_t symbol;
	uint8_t nbits;
}__attribute__((__packed__)) huffman_canon_t;

typedef struct {
	uint8_t size;
	huffman_canon_t codes[];
}__attribute__((__packed__)) huffman_header_t;

extern huffman_canon_t* Huffman;
/* this contains the number of symbol - 1 to avoid overflow */
extern uint8_t HuffmanLength;

typedef struct {
	uint8_t symbol;
	uint8_t code_size;/* the last element has zero code_size */
	uint64_t code;
}huffman_t;

void huffman_print(huffman_t* h);
huffman_t* huffman(uint8_t symbol, uint8_t code_size, uint64_t code);

huffman_t* Huffman_build_canonicalize_representation(void);
int Huffman_build_from_stream(FILE* f);
void Huffman_load_from_stream(FILE* f);
int huffman_decode_one_symbol(uint8_t*, huffman_t* h, FILE* f);
void huffman_code_print(huffman_t h);
huffman_t huffman_get_code_from_symbol(huffman_t* h, uint8_t symb);
size_t huffman_get_encoded_size(huffman_t* h,
	uint8_t* buffer, size_t size);
/* write symbols to file pointed by fd using the ht huffman table */
size_t huffman_write_symbols(int fd, huffman_t* ht,
	uint8_t* symbols, size_t length);
size_t huffman_read_symbols(int fd, huffman_t* ht, uint8_t* symbols);
uint64_t huffman_canonicalize_step(
	uint64_t previous_code, uint8_t previous_size, uint8_t new_size);

void Huffman_set(huffman_canon_t* huffman_array, uint8_t length);

int huffman_look_for_code_from_xio(uint8_t* symbol, huffman_t* t, xio_t* xio);

#endif
