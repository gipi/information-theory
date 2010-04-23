#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>

#include<frequency.h>
#include<huffman.h>

#define USAGE_STR \
	"usage: huffman [options] [filename]\n" \
	"\n" \
	"encode/decode a stream by huffman coding\n"

static void usage(int exit_code) {
	printf(USAGE_STR);
	exit(exit_code);
}


static cmp(const frequency_row_t* a, const frequency_row_t* b) {
	return ( (a->frequency) > (b->frequency) );
}

void order_frequencies_table(frequency_table_t t) {
	qsort(t.frequencies, t.length, sizeof(frequency_row_t), cmp);
}

void print_frequencies_table(frequency_table_t t) {
	unsigned int cycle;
	for (cycle = 0 ; cycle < t.length ; cycle++) {
		frequency_row_t f =  t.frequencies[cycle];
		printf(" %c\t%"PRIu64"\n", f.symbol, f.frequency);
	}
}

int main(int argc, char* argv[]) {
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'h') {
		usage(0);
	}
	frequency_table_create_from_stream(stdin);

	unsigned int cycle, idx = 0;
	frequency_row_t* ft = NULL;
	for (cycle = 0 ; cycle < 0xff ; cycle++) {
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
	print_frequencies_table(table);

	return 0;
}
