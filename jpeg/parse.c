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

static void read_length_and_rewind(u16int* length, FILE* f) {
	/* first read the length */
	fread(length, sizeof(*length), 1, f);

	/* data with most significant bit first */
	*length = htons(*length);

	/* rewind a little bit */
	fseek(f, -sizeof(*length), SEEK_CUR);
	if(ferror(f))
		perror("fseek");
	}
}

static unsigned int read_JFIF(FILE* f) {
	printf(" JFIF header\n");
	u16int length;
	read_length_and_rewind(&length, f);

	struct JFIF_header* header = malloc(length);
	/* read all the header */
	fread(header, length, 1, f);

	if (header->identifier[0] != 'J')
		fprintf(stderr, " warning: no JPEG\n");

	u8int major = htons(header->version) >> 8;
	u8int minor = htons(header->version) & 0xff;
	printf(" version: %d.%d\n", major, minor);

	char* units [] = {
		"pixel aspect ratio",
		"dots per inch",
		"dots per cm"
	};
	printf(" %dx%d %s\n",
			htons(header->xdensity),
			htons(header->ydensity),
			units[header->units]);

	return length;
}

static unsigned int read_start_of_frame(FILE* f) {
	u16int length;
	read_length_and_rewind(&length, f);

	/* allocate start of frame */
	struct start_of_frame* sof =
		malloc(length);

	/* read all the frame */
	fread(sof, length, 1, f);

	printf(" length: %u\n", htons(sof->length));
	printf(" %ux%u samples with precision %u bytes and %u components\n",
			htons(sof->X), htons(sof->Y), sof->sample, sof->Nf);

	unsigned int cycle;
	printf( " SUBSAMPLING\n");
	for (cycle = 0 ; cycle < sof->Nf ; cycle++ ) {
		struct Nf_array nfa = sof->nf_array[cycle];
		printf( " id: %u\n"
			" HV sampling %hd:%hd\n"
			" Quant. table id %u\n\n",
				nfa.id,
				nfa.hv_sampling_factor & 15,
				nfa.hv_sampling_factor >> 4,
				nfa.quant_table_number);
	}

	return length;
}

static unsigned int read_quantization_table(FILE* f) {
	u16int length;
	read_length_and_rewind(&length, f);

	printf(" QUANTIZATION TABLE (length: %u)\n", length);

	struct quantization_table* qt = malloc(length);

	fread(qt, length, 1, f);

	create_zig_zag(8);
	de_zig_zag(qt->quantization[0].value, 8);

	return length;
}

static unsigned int read_huffman_table(FILE* f) {
	u16int length;
	read_length_and_rewind(&length, f);

	struct huffman_table* ht = malloc(length);

	fread(ht, length, 1, f);

	printf(" HUFFMAN TABLE (length: %u)\n", length);
	printf(" matrix type: %d\n", ht->matrix_type);
	printf(" identifier: %d\n", ht->identifier);

	/* here goes the code definition */
	/* length (2 bytes) +
	 * 	table class (4 bits) + table destinator (4 bits)
	 */
	huffman_t* h = huffman_from_jpeg_header(*ht);
	huffman_print(h);

	return length;
}

static unsigned int handle_marker(FILE* f, unsigned char marker) {
	unsigned int delta_idx = 0;
	switch (marker) {
		case 0xc0:
			delta_idx = read_start_of_frame(f);
			break;
		case 0xc4:
			delta_idx = read_huffman_table(f);
			break;
		case 0xe0:
			delta_idx = read_JFIF(f);
			break;
		case 0xdb:
			delta_idx = read_quantization_table(f);
			break;
	}

	return delta_idx;
}


int main(int argc, char* argv[]){
	if (argc < 2) {
		fprintf(stderr, "usage: %s <jpeg file>\n", argv[0]);
		exit(1);
	}

	FILE* fjpeg;
	if (!strcmp(argv[1], "-"))
		fjpeg = stdin;
	else
		fjpeg = fopen(argv[1], "r");

	if (!fjpeg) {
		perror("fatal opening jpeg file");
		exit(1);
	}

	unsigned char c;
	unsigned int idx = -1, col = 0, delta = 0;

	/* it's a binary file so checking for EOF in fgetc() is useless */
	while (1) {
		one_more_byte(c, fjpeg, idx);
		if (feof(fjpeg))
			break;

		if (c == 0xff) {
			if(idx)
				printf("\n\n");

			one_more_byte(c, fjpeg, idx);
			printf("0xff%02x at %u", c, idx - 1);
			delta = handle_marker(fjpeg, c);
			col = 0;
		}else{

			if (!(col % 20))
				printf("\n");

			printf(" %02x", c);
			col++;
		}
	}

	fclose(fjpeg);

	return 0;
}

