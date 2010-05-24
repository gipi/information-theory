/*
 * Copyright 2010 Gianluca Pacchiella <gianluca.pacchiella@ktln2.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>

#include<jpeg/ljpeg.h>
#include<huffman/huffman.h>

#if 0
#define printf_seed(seed) printf(" (%02u,%02u)", (seed).x, (seed).y)
#else
#define printf_seed(seed)
#endif

struct point {
	unsigned int x;
	unsigned int y;
};

struct point* zigzag = NULL;

static int create_first_half_zigzag(unsigned int side) {
	unsigned int idx = 0, diagonale, row;
	/* who decides when y or x */
	/*
	 *	who =  1     X
	 *	who = -1     Y
	 * */
	int who = 1;

	struct point seed = {0, 0};

	printf_seed(seed);
	zigzag[idx] = seed;
	idx++;


	for (diagonale = 2 ; diagonale <= side ; diagonale++) {
		seed.x = who > 0 ? seed.x + 1 : seed.x;
		seed.y = who < 0 ? seed.y + 1 : seed.y;
		zigzag[idx] = seed;

		printf_seed(seed);
		idx++;

		/* this cycle doens't calculate the seed */
		for(row = 1 ; row < diagonale ; row++) {
			seed.x += -who;
			seed.y +=  who;
			printf_seed(seed);
			zigzag[idx] = seed;

			idx++;
		}
		who *= -1;
	}

	return who;
}

static void create_second_half_zigzag(unsigned int side, int who) {
	/* idx is the number of element calculated yet */
	unsigned int idx = (side * (side + 1))/2, diagonale, row;
	struct point seed = zigzag[idx - 1];

	for (diagonale = 1 ; diagonale < side ; diagonale++) {

		seed.x = who < 0 ? seed.x+1 : seed.x;
		seed.y = who > 0 ? seed.y+1 : seed.y;
		zigzag[idx] = seed;

		printf_seed(seed);
		idx++;

		/* this cycle doens't calculate the seed */
		for(row = side - diagonale  - 1 ; row > 0 ; row--) {
			seed.x += -who;
			seed.y +=  who;
			printf_seed(seed);
			zigzag[idx] = seed;

			idx++;
		}
		who *= -1;
	}
}

void create_zig_zag(unsigned int side) {
	free(zigzag);
	zigzag = malloc(sizeof(struct point) * side * side);
	int who = create_first_half_zigzag(side);
	create_second_half_zigzag(side, who);
}

static unsigned int from_col_row_to_idx(
	unsigned int row,
	unsigned int col,
	unsigned int side)
{
	unsigned int cycle;
	unsigned int max = side * (side + 1);
	for (cycle = 0 ; cycle < max ; cycle++) {
		struct point p = zigzag[cycle];
		if (p.y == row && p.x == col)
			return cycle;
	}
}

void de_zig_zag(u8int values[], unsigned int side) {
	unsigned int col, row;
	for (row = 0 ; row < side ; row++) {
		for (col = 0 ; col < side ; col++) {
			printf(" %02d",
				values[from_col_row_to_idx(row, col, side)]);
		}
		printf("\n");
	}
}

static inline int fread_check(void* buffer, size_t size, FILE* stream) {
	if( fread(buffer, size, 1, stream) < 1 )
		fprintf(stderr, "%s - warning: read \n",__FUNCTION__);

	return 0;/* FIXME*/
}

static u16int get_section_length(FILE* f) {
	u16int length;
	fread_check(&length, sizeof(length), f);

	return ntohs(length);
}

/* allocate and full a buffer with a section content */
/* you can cast this to a section struct */
void* section_to_buffer(FILE* f) {
	u16int length = get_section_length(f);
	u8int* buffer = malloc(length);

	fread_check(buffer, length - 2, f);

	return buffer;
}

/* we have to pass a pointer otherwise the compiler doesn't copy
 * all the values of the values field. */
huffman_t* huffman_from_jpeg_header(struct huffman_table* t) {
	unsigned int nc_cycle, codeidx;

	huffman_canon_t hc[0x100];
	unsigned int ncodes = 0;

	/* loop over the number of code of N nc_cycle bits */
	for (nc_cycle = 0 ; nc_cycle < 16 ; nc_cycle++) {
		for (codeidx = 0 ; codeidx < t->ncodes[nc_cycle] ; codeidx++) {
			hc[ncodes] = (huffman_canon_t) {
				.symbol = t->values[ncodes],
				.nbits  = nc_cycle + 1
			};
			ncodes++;
		}
	}

	Huffman_set(hc, ncodes);

	return Huffman_build_canonicalize_representation();
}

struct JFIF_header* gJFIF_header = NULL;

void read_JFIF_header(FILE* f) {
	gJFIF_header = section_to_buffer(f);
}

void JFIF_header_print_info(void) {
	if (gJFIF_header->identifier[0] != 'J')
		fprintf(stderr, " warning: no JPEG\n");

	u8int major = htons(gJFIF_header->version) >> 8;
	u8int minor = htons(gJFIF_header->version) & 0xff;
	printf(" version: %d.%d\n", major, minor);

	char* units [] = {
		"pixel aspect ratio",
		"dots per inch",
		"dots per cm"
	};
	printf(" %dx%d %s\n",
			htons(gJFIF_header->xdensity),
			htons(gJFIF_header->ydensity),
			units[gJFIF_header->units]);
}

struct quantization_table* gquantization_table = NULL;

void read_quantization_table_header(FILE* f) {
	gquantization_table = section_to_buffer(f);
}

void quantization_table_print_info(void) {
	create_zig_zag(8);
	de_zig_zag(gquantization_table->quantization[0].value, 8);
}
