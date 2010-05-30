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

void ude_zig_zag(uint8_t values[], unsigned int side) {
	unsigned int col, row;
	for (row = 0 ; row < side ; row++) {
		for (col = 0 ; col < side ; col++) {
			printf(" % "PRIu8,
				values[from_col_row_to_idx(row, col, side)]);
		}
		printf("\n");
	}
}

void de_zig_zag(int16_t values[], unsigned int side) {
	unsigned int col, row;
	for (row = 0 ; row < side ; row++) {
		for (col = 0 ; col < side ; col++) {
			printf(" % "PRId16,
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

static uint16_t get_section_length(FILE* f) {
	uint16_t length;
	fread_check(&length, sizeof(length), f);

	return ntohs(length);
}

/* allocate and full a buffer with a section content */
/* you can cast this to a section struct */
void* section_to_buffer(FILE* f) {
	uint16_t length = get_section_length(f);
	uint8_t* buffer = malloc(length);

	fread_check(buffer, length - 2, f);

	return buffer;
}

/* we have to pass a pointer otherwise the compiler doesn't copy
 * all the values of the values field. */
huffman_t* huffman_from_jpeg_header(struct ljpeg_huffman_table* t) {
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

	uint8_t major = htons(gJFIF_header->version) >> 8;
	uint8_t minor = htons(gJFIF_header->version) & 0xff;
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

struct start_of_frame* gstart_of_frame = NULL;

void read_start_of_frame(FILE* f) {
	gstart_of_frame = section_to_buffer(f);
}

void start_of_frame_print_info(void) {
	printf(" %ux%u samples with precision %u bytes and %u components\n",
			htons(gstart_of_frame->X),
			htons(gstart_of_frame->Y),
			gstart_of_frame->sample,
			gstart_of_frame->Nf);

	unsigned int cycle;
	printf( " SUBSAMPLING\n");
	for (cycle = 0 ; cycle < gstart_of_frame->Nf ; cycle++ ) {
		struct Nf_array nfa = gstart_of_frame->nf_array[cycle];
		printf( " id: %u\n"
			" HV sampling %hd:%hd\n"
			" Quant. table id %u\n\n",
				nfa.id,
				nfa.hv_sampling_factor & 15,
				nfa.hv_sampling_factor >> 4,
				nfa.quant_table_number);
	}
}

struct ljpeg_huffman_table* g_ljpeg_ht = NULL;
huffman_t* g_huffman_y_dc = NULL;
huffman_t* g_huffman_y_ac = NULL;
huffman_t* g_huffman_cbcr_dc = NULL;
huffman_t* g_huffman_cbcr_ac = NULL;

static void ljpeg_parse_huffman_table(void) {
	huffman_t* hm = huffman_from_jpeg_header(g_ljpeg_ht);

	if (g_ljpeg_ht->matrix_type) { /* AC */
		if (g_ljpeg_ht->identifier) {/* CbCr */
			g_huffman_cbcr_ac = hm;
		} else {/* Y */
			g_huffman_y_ac = hm;
		}
	} else {/* DC */
		if (g_ljpeg_ht->identifier) {/* CbCr */
			g_huffman_cbcr_dc = hm;
		} else {/* Y */
			g_huffman_y_dc = hm;
		}
	}
}

void read_huffman_table(FILE* f) {
	g_ljpeg_ht = section_to_buffer(f);
	ljpeg_parse_huffman_table();
	free(g_ljpeg_ht);
	g_ljpeg_ht = NULL;
}

void ljpeg_print_huffman_tables(void) {
	huffman_print(g_huffman_y_dc);
	huffman_print(g_huffman_y_ac);
	huffman_print(g_huffman_cbcr_dc);
	huffman_print(g_huffman_cbcr_ac);
}


struct start_of_scan g_start_of_scan;

void ljpeg_read_scan_data(FILE* f) {
	fread(&g_start_of_scan.length, sizeof(uint16_t), 1, f);
	g_start_of_scan.length = htons(g_start_of_scan.length);
	fread(&g_start_of_scan.ncomponents, sizeof(uint8_t), 1, f);

	g_start_of_scan.components =
		malloc(sizeof(uint16_t)*g_start_of_scan.ncomponents);
	fread(g_start_of_scan.components,
		sizeof(uint16_t), g_start_of_scan.ncomponents, f);

	/* 3 useless bytes */
	fseek(f, 3, SEEK_CUR);/* 00 3F 00 ?*/

	g_start_of_scan.data = malloc(1024);

	unsigned char c;
	unsigned int idx = 0;
	while (1) {
		c = fgetc(f);
		switch (c) {
			case 0xff:
				c = fgetc(f);
				if (c != 0x00)
					goto exit;
				c = 0xff;
			default:
				//printf(" %02x ", c);printf_byte(c);
				g_start_of_scan.data[idx++] = c;

		}
	}

exit:
	/* if exits then the last thing read is 0xff 0xXY */
	fseek(f, -2, SEEK_CUR);
}

void print_scan_data(void) {
}
