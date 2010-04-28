#include<stdlib.h>

#include<frequency.h>


unsigned long long int occurrence[0x100];

unsigned char* Depot = NULL;
unsigned int DepotIdx = 0;

#define DEPOT_SIZE 1024

static void Depot_save(unsigned char c) {
	if ( !(DepotIdx % DEPOT_SIZE) ) {
		size_t ns = (float)(DepotIdx / DEPOT_SIZE + 1) * DEPOT_SIZE;
		Depot = realloc(Depot, ns);
	}
	Depot[DepotIdx++] = c;
}

static void Depot_free(void) {
	free(Depot);
	Depot = NULL;
	DepotIdx = 0;
}

inline unsigned char* frequency_get_stream_content(void) {
	return Depot;
}

inline unsigned int frequency_get_stream_size(void) {
	return DepotIdx;
}

void frequency_table_create_from_stream(FILE* ff, frequency_save_t save) {
	unsigned char c;
	Depot = malloc(DEPOT_SIZE);
	while(1){
		c = fgetc(ff);
		if (feof(ff))
			break;

		occurrence[(int)c]++;

		/* save in Depot the content of the stream */
		if (save) {
			Depot_save(c);
		}
	}
}

void print_frequencies_table(frequency_table_t t) {
	unsigned int cycle;
	for (cycle = 0 ; cycle < t.length ; cycle++) {
		frequency_row_t f =  t.frequencies[cycle];
		printf(" %c\t%"PRIu64"\n", f.symbol, f.frequency);
	}
}

static int cmp_frequency(const void* a, const void* b) {
	return ( (((frequency_row_t*)a)->frequency) > (((frequency_row_t*)b)->frequency) );
}

void order_frequencies_table(frequency_table_t t) {
	qsort(t.frequencies, t.length,
		sizeof(frequency_row_t), cmp_frequency);
}

