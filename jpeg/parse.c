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
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>

#include<jpeg/ljpeg.h>

#define USAGE_STR \
	"usage: parse [options] <jpeg file>\n" \
	"\n" \
	"options:\n" \
	"\n" \
	"  -h\tprint this message\n" \
	"  -q\tquantization table\n" \
	"  -H\thuffman tables\n"     \
	"  -s\tscan data\n"

static void usage(int exit_code) {
	printf(USAGE_STR);
	exit(exit_code);
}

static void handle_JFIF(FILE* f) {
	printf(" JFIF header\n");
	read_JFIF_header(f);
	JFIF_header_print_info();
}

static void handle_start_of_frame(FILE* f) {
	read_start_of_frame(f);
}

static void handle_quantization_table(FILE* f) {
	printf(" QUANTIZATION TABLE\n");
	read_quantization_table_header(f);
	quantization_table_print_info();

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
			handle_start_of_frame(f);
			break;
		case 0xc4:
			handle_huffman_table(f);
			break;
		case 0xe0:
			handle_JFIF(f);
			break;
		case 0xdb:
			handle_quantization_table(f);
			break;
	}

	return delta_idx;
}


int main(int argc, char* argv[]){
	char C;
	int show_quantization_table = 0;
	int show_huffman_table = 0;

	while ( (C = getopt(argc, argv, "hqHs")) != -1) {
		switch (C) {
			case 'h':
				usage(0);
				break;
			case 'H':
				show_huffman_table = 1;
				break;
			case 'q':
				show_quantization_table = 1;
				break;
		}
	}

	FILE* fjpeg = stdin;
	if (optind < argc) {
		char* filename = argv[optind];
		fjpeg = fopen(argv[optind], "r");
		if (!fjpeg) {
			fprintf(stderr, "fatal opening '%s': %s\n", filename, strerror(errno));
			usage(1);
		}
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

