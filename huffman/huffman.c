#define _POSIX_SOURCE
#define _BSD_SOURCE
#include<features.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<inttypes.h>
#include<assert.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<endian.h>

#include<data_structure/tree.h>
#include<frequency.h>
#include<huffman/huffman.h>
#include<utils/bits.h>


huffman_canon_t* Huffman = NULL;
uint8_t HuffmanLength;
unsigned int HuffmanIdx = 0;

#define GET_NTH(b,n) (((b) & (1 << (n))) ? 1 : 0)

void huffman_code_print(huffman_t row) {
	uint8_t length = row.code_size;
	unsigned int cycle;
	for (cycle = 0 ; cycle < length; cycle++){
		printf("%u", GET_NTH(row.code, length - 1 - cycle));
	}
}

/*
 * Print the huffman code from an array with the last element with
 * code_size field equal to zero.
 */
void huffman_print(huffman_t* h) {
	unsigned int cycle;
	for (cycle = 0 ; h[cycle].code_size ; cycle++){
		printf("%02x\t", h[cycle].symbol);
		printf("%u\t", h[cycle].code_size);
		printf("%"PRIx64"\t", h[cycle].code);
		huffman_code_print(h[cycle]);
		printf("\n");
	}
}

huffman_t* huffman(uint8_t symbol, uint8_t code_size, uint64_t code) {
	/* FIX: check pointer */
	huffman_t* h = malloc(sizeof(huffman_t));
	h->symbol = symbol;
	h->code_size = code_size;
	h->code = code;

	return h;
}

/* temporary struct for build the huffman tree */
struct h_tmp {
	uint8_t symbol;
	uint64_t weight;/* the last one has weight 0 */
};

/* not using a macro we can use it during a gdb session */
static inline struct h_tmp*  node_to_data(node_t n) {
	return ((struct h_tmp*)((n).data));
}

static int cmp_node_weight(const void* a, const void* b) {
	node_t na = **(node_t**)a;
	node_t nb = **(node_t**)b;
	return -(node_to_data(na)->weight - node_to_data(nb)->weight);
}

/*
 * Generate the node by which create the tree.
 *
 * The array returned is terminated with a element of weight zero.
 */
static struct h_tmp* node_create_from_symbol(frequency_table_t t) {
	unsigned int length = t.length + 1;
	struct h_tmp* nodes = malloc(sizeof(struct h_tmp)*length);
	unsigned int cycle;
	frequency_row_t row;

	for (cycle = 0 ; cycle < t.length ; cycle++) {
		row = t.frequencies[cycle];
		nodes[cycle] = (struct h_tmp){
			.symbol = t.frequencies[cycle].symbol,
			.weight = t.frequencies[cycle].frequency
		};
	}

	nodes[length - 1].weight = 0;

	return nodes;
}

/* builds a tree from the frequencies table */
tree_t* tree_from_frequencies_table(frequency_table_t t) {
	struct h_tmp* node_datas = node_create_from_symbol(t);

	unsigned int length = t.length;

	/* create nodes */
	node_t** nodes = malloc(sizeof(node_t)*length);

	unsigned int cycle;
	for (cycle = 0 ; node_datas[cycle].weight ; cycle++) {
		/* FIX: use node_and_memcpy() */
		nodes[cycle] = node(&node_datas[cycle]);
	}

	tree_t* root = NULL;
	node_t* new_node;
	node_t* last_one,* last_two;

	/* main loop */
	while (length > 1) {
		/* order it */
		qsort(nodes, length, sizeof(node_t*), cmp_node_weight);
		last_one = nodes[length - 1];
		last_two = nodes[length - 2];

		/* build a new data node with the last two weight */
		struct h_tmp h_new = (struct h_tmp){
			.weight = node_to_data(*last_one)->weight +
				node_to_data(*last_two)->weight
		};
		/* append this last two */
		new_node = node_and_memcpy(&h_new, sizeof(struct h_tmp));
		root = node_append(new_node, last_one, last_two);
		nodes[length - 2] = root;
		length--;
	}

	/* free useless struct h_tmp array */
	//free(node_datas);/* see above FIX:*/
	free(nodes);

	return root;
}

/* callback to build the huffman code from the tree */
int build_canonical_from_tree(node_t* n, unsigned int depth) {
	if (node_is_leaf(*n)) {
		Huffman[HuffmanIdx++] = (huffman_canon_t){
			.symbol = node_to_data(*n)->symbol,
			.nbits  = depth
		};
	}

	return 0;
}

static int cmp_huffman_nbits(const void* a, const void* b) {
	huffman_canon_t* ha = ((huffman_canon_t*)a);
	huffman_canon_t* hb = ((huffman_canon_t*)b);

	return (ha->nbits - hb->nbits);
}

static int cmp_huffman_symbol(const void* a, const void* b) {
	int status = 0;

	huffman_canon_t* ha = ((huffman_canon_t*)a);
	huffman_canon_t* hb = ((huffman_canon_t*)b);

	if (ha->nbits == hb->nbits)
		status = (ha->symbol - hb->symbol);

	return status;
}

static void Huffman_order_by_nbits(void) {
	qsort(Huffman, HuffmanLength + 1, sizeof(huffman_canon_t), cmp_huffman_nbits);
}

static void Huffman_order_by_symbol(void) {
	qsort(Huffman, HuffmanLength + 1, sizeof(huffman_canon_t), cmp_huffman_symbol);
}

uint64_t huffman_canonicalize_step(
	uint64_t previous_code, uint8_t previous_size, uint8_t new_size) {

	uint64_t new_code;
	new_code = previous_code + 1;

	if (new_size > previous_size)
		new_code <<= (new_size - previous_size);

	return new_code;
}

/* see http://en.wikipedia.org/wiki/Canonical_Huffman_code */
huffman_t* Huffman_build_canonicalize_representation(void) {
	huffman_t* hrows =
		malloc(sizeof(huffman_t)*(HuffmanLength + 2));

	unsigned int cycle;
	uint64_t nbits = Huffman[0].nbits;

	/* first code */
	hrows[0].code_size = nbits;
	hrows[0].symbol = Huffman[0].symbol;
	hrows[0].code = 0;

	for (cycle = 1 ; cycle < HuffmanLength + 1 ; cycle++) {
		hrows[cycle].symbol = Huffman[cycle].symbol;
		hrows[cycle].code_size = Huffman[cycle].nbits;
		hrows[cycle].code = huffman_canonicalize_step(
			hrows[cycle - 1].code,
			hrows[cycle - 1].code_size,
			hrows[cycle].code_size);
	}

	hrows[HuffmanLength + 1].code_size = 0;

	/* TODO: add last element with code_size = 0 */

	return hrows;
}

/* this function sets the Huffman{,Length} to predefined value */
/* the array hc has to be nbits zero terminated */
void Huffman_set(huffman_canon_t* hc, uint8_t length) {
	/* first release the memory */
	free(Huffman);

	/* second allocate memory for the  maximum */
	/* TODO: check return value of malloc() */
	Huffman = malloc(sizeof(huffman_canon_t)*0x100);

	memcpy(Huffman, hc, sizeof(huffman_canon_t) * length);
	HuffmanLength = length - 1;
}

/* to avoid O(n^2) we can create an array so to have O(1) */
huffman_t huffman_get_code_from_symbol(huffman_t* t, uint8_t symbol) {
	huffman_t row;

	unsigned int cycle;
	for (cycle = 0 ; t[cycle].code_size ; cycle++){
		row = t[cycle];
		if (row.symbol == symbol)
			return row;
	}

	/* this is not possible */
	assert(!"not symbol found in table");
}

size_t huffman_get_encoded_size(
	huffman_t* t, uint8_t* buffer, size_t size)
{
	unsigned int cycle;
	huffman_t row;
	size_t nsize = 0;/* nsize is in bits, size in bytes */
	for (cycle = 0 ; cycle < size; cycle++){
		row = huffman_get_code_from_symbol(t, buffer[cycle]);
		nsize += row.code_size;
	}

	return (float)(nsize/8);
}

/* build the Huffman array from the stream */
int Huffman_build_from_stream(FILE* f) {
	int nbytes = frequency_table_create_from_stream(
		f, FREQUENCY_SAVE_STREAM);

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

	/* build the huffman tree */
	/* TODO: create a EOS global symbol */
	tree_t* tree = tree_from_frequencies_table(table);

	Huffman = malloc(sizeof(huffman_t)*table.length);
	HuffmanLength = table.length - 1;

	if(!Huffman) {
		perror("error allocating memory");
		exit(1);
	}

	/* now we build the canonical representation */
	/* the result is stored in Huffman and HuffmanLength */
	tree_traverse(tree, 0, build_canonical_from_tree);

	tree_free(tree);

	/* move */
	Huffman_order_by_nbits();
	Huffman_order_by_symbol();

	return nbytes;
}

/* build Huffman from the table read from the file */
void Huffman_load_from_stream(FILE* f) {
	/* FIXME: 0x100 ---> HuffmanLength */
	if (!Huffman)
		Huffman = calloc(sizeof(huffman_t), 0x100);
	fread(&HuffmanLength, sizeof(HuffmanLength), 1, f);
	fread(Huffman, sizeof(huffman_canon_t), HuffmanLength + 1, f);
}

/* look and advance */
/*
 * On error return -1.
 */
static int huffman_look_for_code(
	uint8_t* symbol, huffman_t* t, FILE* f)
{
	unsigned int cycle, status = 0;
	uint64_t value = 0;
	uint8_t old_code_size = 0;
	huffman_t row;
	for (cycle = 0 ; t[cycle].code_size ; cycle++) {
		row = t[cycle];

		/* is useless to re-read the value*/
		if (row.code_size != old_code_size) {
			old_code_size = row.code_size;
			status = fread_bits(&value, row.code_size, 0, f);
		}

		if(status == -1)
			return status;

		if (value != row.code)
			continue;

		fread_bits(&value, row.code_size, 1, f);
		if (ferror(f)) {
			perror(__FILE__);
			clearerr(f);
		}

		*symbol = row.symbol;

		return 0;
	}

	fprintf(stderr, "fatal: code not found at 0x%lx\n"
		" value: ", ftell(f));
	fprintf(stderr, "EOF: %u\tError: %u\n", feof(f), ferror(f));
	//printf_16bits(value);

	return -1;
}

int huffman_decode_one_symbol(uint8_t* symbol, huffman_t* t, FILE* f) {
	return huffman_look_for_code(symbol, t, f);
}

int huffman_look_for_code_from_xio(uint8_t* symbol, huffman_t* t, xio_t* xio) {
	unsigned int cycle, status = 0;
	uint64_t value = 0;
	uint8_t old_code_size = 0;
	huffman_t row;
	for (cycle = 0 ; t[cycle].code_size ; cycle++) {
		row = t[cycle];

		/* is useless to re-read the value*/
		if (row.code_size != old_code_size) {
			old_code_size = row.code_size;
			status = readbits(&value, row.code_size, 0, xio);
		}

		if(status == -1)
			return status;

		if (value != row.code)
			continue;

		readbits(&value, row.code_size, 1, xio);

		*symbol = row.symbol;

		return 0;
	}


	return -1;
}
