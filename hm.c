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
	unsigned int print_canonical = 0, print_frequencies = 0;
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

	frequency_table_create_from_stream(f, FREQUENCY_SAVE_STREAM);
	fprintf(stderr, " read %u bytes\n", frequency_get_stream_size());

	unsigned int cycle, idx = 0;
	frequency_row_t* ft = NULL;
	for (cycle = 0 ; cycle < 0x100 ; cycle++) {
		if (occurrence[cycle] == 0)
			continue;

		ft = realloc(ft, sizeof(frequency_row_t)*++idx);
		ft[idx - 1] = (frequency_row_t){
			.symbol = cycle,
			.frequency = occurrence[cycle]
		};
	}

	frequency_table_t table = {
		.length = idx,
		.frequencies = ft
	};

	order_frequencies_table(table);
	if (print_frequencies)
		print_frequencies_table(table);

	tree_t* tree = tree_init(table);
	/* TODO: check for memory leak */
	while (tree->length > 1) {
		tree = tree_step(tree);
	}

	Huffman = malloc(sizeof(huffman_t)*table.length);
	HuffmanLength = table.length;

	if(!Huffman) {
		perror("error allocating memory");
		exit(1);
	}

	/* this fulls Huffman */
	node_walk(tree->nodes[0], 0);

	huffman_table_t final = Huffman_canonicalize();

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
	free(ft);
	tree_free(*tree);
	free(Huffman);

	return 0;
}
