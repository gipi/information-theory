#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<inttypes.h>
#include<assert.h>
#include<getopt.h>

#define _BSD_SOURCE
#include<endian.h>

#include<frequency.h>
#include<huffman/huffman.h>
#include<utils/bits.h>


huffman_t* Huffman = NULL;
unsigned int HuffmanLength;
unsigned int HuffmanIdx = 0;

static int cmp_nodes(const void* a, const void* b) {
	return ( (((node_t*)a)->weight) > (((node_t*)b)->weight) );
}

static void order_tree(tree_t t) {
	qsort(t.nodes, t.length, sizeof(node_t), cmp_nodes);
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

tree_t* tree_init(frequency_table_t t) {
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

tree_t* tree_step(tree_t* t) {
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

void node_walk(node_t n, uint64_t length) {

	if (node_is_leaf(n)) {
		Huffman[HuffmanIdx++] = (huffman_t){
			.symbol = n.symbol,
			.nbits  = length
		};
	} else {
		node_walk(*n.left, length + 1);
		node_walk(*n.right, length + 1);
	}

}

void node_free(node_t* n) {
	if (!node_is_leaf(*n)) {
		node_free(n->left);
		node_free(n->right);
	} else /* valgrind tell me invalid free() */
		;//free(n);
}

int cmp_huffman_nbits(const void* a, const void* b) {
	huffman_t* ha = ((huffman_t*)a);
	huffman_t* hb = ((huffman_t*)b);

	return (ha->nbits > hb->nbits);
}

static void Huffman_order_ny_nbits(void) {
	qsort(Huffman, HuffmanLength, sizeof(huffman_t), cmp_huffman_nbits);
}

#define GET_NTH(b,n) (((b) & (1 << (n))) ? 1 : 0)

void huffman_code_print(huffman_row_t row) {
	uint8_t length = row.code_size;
	unsigned int cycle;
	for (cycle = 0 ; cycle < length; cycle++){
		printf("%u", GET_NTH(row.code, length - 1 - cycle));
	}

}

/* see http://en.wikipedia.org/wiki/Canonical_Huffman_code */
huffman_table_t Huffman_canonicalize(void) {
	Huffman_order_ny_nbits();
	huffman_row_t* hrows = malloc(sizeof(huffman_row_t)*HuffmanLength);

	unsigned int cycle;
	uint64_t nbits = Huffman[0].nbits;

	/* first code */
	hrows[0].code_size = nbits;
	hrows[0].symbol = Huffman[0].symbol;
	hrows[0].code = 0;

	for (cycle = 1 ; cycle < HuffmanLength ; cycle++) {
		hrows[cycle].symbol = Huffman[cycle].symbol;
		hrows[cycle].code = hrows[cycle - 1].code + 1;
		hrows[cycle].code_size = Huffman[cycle].nbits;

		if (hrows[cycle].code_size > hrows[cycle - 1].code_size)
			hrows[cycle].code <<= 1;
	}

	return (huffman_table_t){
		.length = HuffmanLength,
		.rows = hrows
	};
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


/* to avoid O(n^2) we can create an array so to have O(1) */
huffman_row_t huffman_get_code_from_symbol(huffman_table_t t, uint8_t symbol) {
	unsigned int cycle;
	huffman_row_t row;
	for (cycle = 0 ; cycle < t.length; cycle++){

		row = t.rows[cycle];
		if (row.symbol == symbol)
			return row;
	}

	/* this is not possible */
	assert(!"not symbol found in table");

}

size_t huffman_get_encoded_size(
	huffman_table_t t, uint8_t* buffer, size_t size)
{
	unsigned int cycle;
	huffman_row_t row;
	size_t nsize = 0;/* nsize is in bits, size in bytes */
	for (cycle = 0 ; cycle < size; cycle++){
		row = huffman_get_code_from_symbol(t, buffer[cycle]);
		nsize += row.code_size;
	}

	return (float)(nsize/8);

}
