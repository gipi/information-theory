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
#include<getopt.h>

#include<frequency.h>
#include<huffman/huffman.h>
#include<utils/bits.h>

#define USAGE_STR \
	"usage: huffman [options] [filename]\n" \
	"\n" \
	"encode/decode a stream by huffman coding\n" \
	"\n" \
	"options:\n" \
	"\t-h\tprint this help\n" \
	"\t-f\tprint frequencies for symbol\n" \
	"\t-c\tprint canonical code\n" \
	"\t-d\tdecompress the filename\n"

static void usage(int exit_code) {
	printf(USAGE_STR);
	exit(exit_code);
}

int main(int argc, char* argv[]) {
	FILE* f = stdin;

	char opt;
	unsigned int print_canonical = 0,
		decompress = 0;
	while ((opt = getopt(argc, argv, "fcdh")) != -1) {
		switch (opt) {
			case 'h':
				usage(0);
				break;
			case 'f':
				print_frequencies = 1;
				break;
			case 'c':
				print_canonical = 1;
				break;
			case 'd':
				decompress = 1;
				break;
			default:
				usage(1);
		}
	}

	if(argc > optind) {
		f = fopen(argv[optind], "r");
		if (!f) {
			perror("error opening file");
			exit(1);
		}
	}

	if (decompress) {
		uint8_t symbol;
		size_t size = 0;
		/* read huffman canonical table from file */
		huffman_table_t t = huffman_table_read_from_stream(f);
		/* decode stream */
		while (!feof(f)) {
			if(huffman_decode_one_symbol(&symbol, t, f) < 0)
				break;
			if(fwrite(&symbol, sizeof(uint8_t), 1, stdout) < 1)
				if(ferror(f))
					perror("error decoding");
			size++;
		}

		fprintf(stderr, " decoded %u bytes\n", size);

		goto exit;
	}

	huffman_table_t final = huffman_canonical_from_stream(f);

	unsigned int cycle;

	if (print_canonical) {
		for (cycle = 0 ; cycle < HuffmanLength; cycle++){
			printf("%c\t", final.rows[cycle].symbol);
			huffman_code_print(final.rows[cycle]);
			printf("\n");
		}
	}


	if (print_canonical || print_frequencies)
		goto exit;

	uint8_t* content = frequency_get_stream_content();
	size_t stream_length = frequency_get_stream_size();

	size_t new_size = huffman_get_encoded_size(
			final, content, stream_length);

	/* first print huffman canonical coding */
	fwrite(&HuffmanLength, sizeof(uint8_t), 1, stdout);
	fwrite(Huffman, sizeof(huffman_t), HuffmanLength, stdout);

	uint8_t* buffer = calloc(1, new_size + 1);
	huffman_row_t hr;
	for (cycle = 0 ; cycle < stream_length ; cycle++) {
		hr = huffman_get_code_from_symbol(final, content[cycle]);
		write_bits(buffer, hr.code, hr.code_size);
	}
	fprintf(stderr, " table size: %u\n", (HuffmanLength * 2) + 1);
	fprintf(stderr, " encoded in %u bytes\n", new_size);

	fwrite(buffer, sizeof(uint8_t), new_size + 1, stdout);
exit:
	free(Huffman);

	return 0;
}
