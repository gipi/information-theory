#include<stdlib.h>

#include<frequency.h>


unsigned long long int occurrence[0xff];

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
