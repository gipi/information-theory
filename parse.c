#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>

#include<jpeg_header.h>

static void read_length_and_rewind(u16int* length, FILE* f) {
	/* first read the length */
	fread(length, sizeof(*length), 1, f);

	/* data with most significant bit first */
	*length = htons(*length);

	/* rewind a little bit */
	fseek(f, -sizeof(*length), SEEK_CUR);
	if(ferror(f))
		perror("fseek");
}

#if 0
#define printf_seed(seed) printf(" (%02u,%02u)", (seed).x, (seed).y)
#else
#define printf_seed(seed)
#endif

struct point {
	unsigned int x;
	unsigned int y;
};

struct point* zigzag = NULL;

static int create_first_half_zigzag(unsigned int side) {
	unsigned int idx = 0, diagonale, row;
	/* who decides when y or x */
	/*
	 *	who =  1     X
	 *	who = -1     Y
	 * */
	int who = 1;

	struct point seed = {0, 0};

	printf_seed(seed);
	printf("\n");
	zigzag[idx] = seed;
	idx++;


	for (diagonale = 2 ; diagonale <= side ; diagonale++) {
		seed.x = who > 0 ? seed.x+1 : seed.x;
		seed.y = who < 0 ? seed.y+1 : seed.y;
		zigzag[idx] = seed;

		printf_seed(seed);
		idx++;

		/* this cycle doens't calculate the seed */
		for(row = 1 ; row < diagonale ; row++) {
			seed.x += -who;
			seed.y +=  who;
			printf_seed(seed);
			zigzag[idx] = seed;

			idx++;
		}
		printf("\n");
		who *= -1;
	}

	return who;
}

static void create_second_half_zigzag(unsigned int side, int who) {
	/* idx is the number of element calculated yet */
	unsigned int idx = (side * (side + 1))/2, diagonale, row;
	struct point seed = zigzag[idx - 1];

	for (diagonale = 1 ; diagonale < side ; diagonale++) {

		seed.x = who < 0 ? seed.x+1 : seed.x;
		seed.y = who > 0 ? seed.y+1 : seed.y;
		zigzag[idx] = seed;

		printf_seed(seed);
		idx++;

		/* this cycle doens't calculate the seed */
		for(row = side - diagonale  - 1 ; row > 0 ; row--) {
			seed.x += -who;
			seed.y +=  who;
			printf_seed(seed);
			zigzag[idx] = seed;

			idx++;
		}
		printf("\n");
		who *= -1;
	}
}

static void create_zig_zag(unsigned int side) {
	free(zigzag);
	zigzag = malloc(side * side);
	int who = create_first_half_zigzag(side);
	create_second_half_zigzag(side, who);
}

static unsigned int from_col_row_to_idx(
	unsigned int row,
	unsigned int col,
	unsigned int side)
{
	unsigned int cycle;
	unsigned int max = side * (side + 1);
	for (cycle = 0 ; cycle < max ; cycle++) {
		struct point p = zigzag[cycle];
		if (p.y == row && p.x == col)
			return cycle;
	}
}

static void de_zig_zag(u8int values[], unsigned int side) {
	unsigned int col, row;
	for (row = 0 ; row < side ; row++) {
		for (col = 0 ; col < side ; col++) {
			printf(" %02d",
				values[from_col_row_to_idx(row, col, side)]);
		}
		printf("\n");
	}
}

static unsigned int read_JFIF(FILE* f) {
	printf(" JFIF header\n");
	u16int length;
	read_length_and_rewind(&length, f);

	struct JFIF_header* header = malloc(length);
	/* read all the header */
	fread(header, length, 1, f);

	if (header->identifier[0] != 'J')
		fprintf(stderr, " warning: no JPEG\n");

	u8int major = htons(header->version) >> 8;
	u8int minor = htons(header->version) & 0xff;
	printf(" version: %d.%d\n", major, minor);

	char* units [] = {
		"pixel aspect ratio",
		"dots per inch",
		"dots per cm"
	};
	printf(" %dx%d %s\n",
			htons(header->xdensity),
			htons(header->ydensity),
			units[header->units]);

	return length;
}

static unsigned int read_start_of_frame(FILE* f) {
	u16int length;
	read_length_and_rewind(&length, f);

	/* allocate start of frame */
	struct start_of_frame* sof =
		malloc(length);

	/* read all the frame */
	fread(sof, length, 1, f);

	printf(" length: %u\n", htons(sof->length));
	printf(" sample: %u\n", sof->sample);
	printf(" Y: %04x\n", htons(sof->Y));
	printf(" X: %04x\n", htons(sof->X));
	printf(" #components: %u\n", sof->Nf);

	unsigned int cycle;
	printf( " SUBSAMPLING\n");
	for (cycle = 0 ; cycle < sof->Nf ; cycle++ ) {
		struct Nf_array nfa = sof->nf_array[cycle];
		printf( " id: %u\n"
			" HV sampling %hd:%hd\n\n",
				nfa.id,
				nfa.hv_sampling_factor & 15,
				nfa.hv_sampling_factor >> 4);
	}

	return length;
}

static unsigned int read_quantization_table(FILE* f) {
	u16int length;
	read_length_and_rewind(&length, f);

	printf(" QUANTIZATION TABLE (length: %u)\n", length);

	struct quantization_table* qt = malloc(length);

	fread(qt, length, 1, f);

	create_zig_zag(8);
	de_zig_zag(qt->quantization[0].value, 8);

	return length;
}

static unsigned int handle_marker(FILE* f, unsigned char marker) {
	unsigned int delta_idx = 0;
	switch (marker) {
		case 0xc0:
			delta_idx = read_start_of_frame(f);
			break;
		case 0xe0:
			delta_idx = read_JFIF(f);
			break;
		case 0xdb:
			delta_idx = read_quantization_table(f);
			break;
	}

	return delta_idx;
}


int main(int argc, char* argv[]){
#ifdef __TEST_ZIG_ZAG
	u8int values[] = {
		1,2,3,4,5,6,7,8,9,10,
		11,12,13,14,15,16,17,18,19,20,
		21,22,23,24,25,26,27,28,29,30,
		31,32,33,34,35,36,37,38,39,40,
		41,42,43,44,45,46,47,48,49,50,
		51,52,53,54,55,56,57,58,59,60,
		61,62,63,64
	};
	create_zig_zag(8);
	de_zig_zag(values, 8);
#else
	if (argc < 2) {
		fprintf(stderr, "usage: %s <jpeg file>\n", argv[0]);
		exit(1);
	}

	FILE* fjpeg = fopen(argv[1], "r");
	if (!fjpeg) {
		perror("fatal opening jpeg file");
		exit(1);
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

#endif
	return 0;
}

