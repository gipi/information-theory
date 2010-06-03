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
#include<utils/bits.h>

#define USAGE_STR \
	"usage: parse [options] <jpeg file>\n" \
	"\n" \
	"options:\n" \
	"\n" \
	"  -h\tprint this message\n" \
	"  -f\tprint frame info\n" \
	"  -q\tquantization table\n" \
	"  -H\thuffman tables\n"     \
	"  -S\tbits representation of scan data\n" \
	"  -S\tdump of blocks\n"          \
	"  -a\tprint all the above\n"

static void usage(int exit_code) {
	printf(USAGE_STR);
	exit(exit_code);
}

static void handle_JFIF(FILE* f) {
	read_JFIF_header(f);
}

static void handle_start_of_frame(FILE* f) {
	read_start_of_frame(f);
}

static void handle_quantization_table(FILE* f) {
	read_quantization_table_header(f);
}

static void handle_huffman_table(FILE* f) {
	read_huffman_table(f);
}

static void handle_scan_data(FILE* f) {


	ljpeg_read_scan_data(f);
	/* 
	 * TODO: create an auxiliary function
	 * able to save a section in a buffer
	 */
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
		case 0xda:
			handle_scan_data(f);
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
	int show_block = 0;
	int show_data = 0;
	int show_frame = 0;

	while ( (C = getopt(argc, argv, "ahqHsSf")) != -1) {
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
			case 's':
				show_block = 1;
				break;
			case 'S':
				show_data = 1;
				break;
			case 'f':
				show_frame = 1;
				break;
			case 'a':
				show_frame = 1;
				show_data = 1;
				show_quantization_table = 1;
				show_huffman_table = 1;
				show_block = 1;
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

	/* it's a binary file so checking for EOF in fgetc() is useless */
	while (1) {
		one_more_byte(c, fjpeg);
		if (feof(fjpeg))
			break;

		if (c == 0xff) {
			one_more_byte(c, fjpeg);
			/* TODO: create a struct where insert
			 * the marker position*/
			handle_marker(fjpeg, c);
		}else{
			printf(" %02x", c);
		}
	}

	fclose(fjpeg);

	if (show_frame)
		start_of_frame_print_info();

	if (show_quantization_table)
		quantization_table_print_info();

	if (show_huffman_table)
		ljpeg_print_huffman_tables();

	if (show_data)
		ljpeg_print_bits_scan_data();

	if (show_block)
		ljpeg_print_scan_data();

	ljpeg_free();

	return 0;
}
