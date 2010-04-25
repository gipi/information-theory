#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<inttypes.h>

#include<frequency.h>
#include<huffman.h>

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

huffman_t* Huffman = NULL;
unsigned int HuffmanLength;
unsigned int HuffmanIdx = 0;

static int cmp_frequency(const void* a, const void* b) {
	return ( (((frequency_row_t*)a)->frequency) > (((frequency_row_t*)b)->frequency) );
}

static int cmp_nodes(const void* a, const void* b) {
	return ( (((node_t*)a)->weight) > (((node_t*)b)->weight) );
}

static void order_frequencies_table(frequency_table_t t) {
	qsort(t.frequencies, t.length,
		sizeof(frequency_row_t), cmp_frequency);
}

static void order_tree(tree_t t) {
	qsort(t.nodes, t.length, sizeof(node_t), cmp_nodes);
}

static void print_frequencies_table(frequency_table_t t) {
	unsigned int cycle;
	for (cycle = 0 ; cycle < t.length ; cycle++) {
		frequency_row_t f =  t.frequencies[cycle];
		printf(" %c\t%"PRIu64"\n", f.symbol, f.frequency);
	}
}

static void tree_dump(tree_t t) {
	unsigned int cycle;
	printf("WEIGHT\n");
	for (cycle = 0 ; cycle < t.length ; cycle++) {
		printf("%"PRIu64"\n", t.nodes[cycle].weight);
	}
}

static node_t* node_create_from_symbol(frequency_table_t t) {
	/* TODO: memory cleaning */
	node_t* nodes = malloc(sizeof(node_t)*t.length);
	unsigned int cycle;
	for (cycle = 0 ; cycle < t.length ; cycle++) {
		nodes[cycle] = (node_t){
			.symbol = t.frequencies[cycle].symbol,
			.weight = t.frequencies[cycle].frequency
		};
	}

	return nodes;
}

static tree_t* tree_init(frequency_table_t t) {
	tree_t* tree = malloc(sizeof(tree_t));

	tree->length = t.length;
	tree->nodes = node_create_from_symbol(t);

	return tree;
}

static node_t node_create_from_lower_frequencies(node_t* nodes) {
	/* the first two nodes are the candidates */
	node_t n = (node_t){
		.weight = nodes[0].weight + nodes[1].weight,
		.left  = (struct _node_t*)&nodes[0],
		.right = (struct _node_t*)&nodes[1]
	};

	return n;
}

static tree_t* tree_step(tree_t* t) {
	tree_t* new_tree = malloc(sizeof(tree_t));
	new_tree->length = t->length - 1;
	new_tree->nodes = malloc(sizeof(node_t)*new_tree->length);
	/* copy the last nodes minus the first */
	memcpy(new_tree->nodes,
		t->nodes + 1, sizeof(node_t)*new_tree->length);

	/* the two nodes going to make the new node */
	new_tree->nodes[0] = node_create_from_lower_frequencies(t->nodes);
	order_tree(*new_tree);

	/*
	 * We need all the nodes to allow the leaf walking
	 * when do node_walk().
	free(t->nodes);
	*/
	free(t);

	return new_tree;
}

static void node_walk(node_t n, const char* prefix) {
	unsigned int length = strlen(prefix);
	/*
	 * to allocate correctly a string we need to
	 * allocate a byte more than needed cause
	 * the '\0' byte to terminate the string.
	 *
	 * Morever we need calloc to initialize to zero
	 * the new string.
	 */
	char* left_prefix  = calloc(1, length + 2);
	char* right_prefix = calloc(1, length + 2);

	/* maybe use strcat() */;
	memcpy(left_prefix, prefix, length);
	memcpy(right_prefix, prefix, length);

	left_prefix[length]  = '0';
	right_prefix[length] = '1';

	if (node_is_leaf(n)) {
		Huffman[HuffmanIdx++] = (huffman_t){
			.symbol = n.symbol,
			.nbits  = length
		};
		printf("%c\t%s\n", n.symbol, prefix);
	} else {
		node_walk(*n.left, left_prefix);
		node_walk(*n.right, right_prefix);
	}

	free(left_prefix);
	free(right_prefix);
}

void node_free(node_t* n) {
	if (!node_is_leaf(*n)) {
		node_free(n->left);
		node_free(n->right);
	} else /* valgrind tell me invalid free() */
		;//free(n);
}

/*
 * Free all the memory.
 */
void tree_free(tree_t t) {
	unsigned int cycle;
	for (cycle = 0 ; cycle < t.length ; cycle++) {
		node_free(t.nodes);
	}
}

int main(int argc, char* argv[]) {
	FILE* f = stdin;
	if (argc > 1) {
		if (argv[1][0] == '-' && argv[1][1] == 'h')
			usage(0);

		f = fopen(argv[1], "r");
	}

	frequency_table_create_from_stream(f);

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

	node_walk(tree->nodes[0], "");

	for (cycle = 0 ; cycle < HuffmanLength ; cycle++) {
		printf(" %c\t%"PRIu64"\n",
			Huffman[cycle].symbol, Huffman[cycle].nbits);
	}
	
	free(ft);
	tree_free(*tree);
	free(Huffman);

	return 0;
}
