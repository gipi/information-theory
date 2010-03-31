#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>

typedef unsigned char  u8int;
typedef unsigned short u16int;

struct Nf_array {
	u8int id;
	u8int hv_sampling_factor;
	u8int quant_table_number;
};

struct __attribute__ ((__packed__)){
	u16int length;
	u8int sample;
	u16int Y;
	u16int X;
	u8int Nf;
	struct Nf_array nf_array[];
}__attribute__ ((__packed__));

static void read_start_of_frame(FILE* f) {
	u16int length = 0;
	/* first read the length */
	fread(&length, sizeof(length), 1, f);

	/* data with most significant bit first */
	length = htons(length);

	/* rewind a little bit */
	fseek(f, -sizeof(length), SEEK_CUR);
	if(ferror(f))
		perror("fseek");

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
}

static void handle_marker(FILE* f, unsigned char marker) {
	switch (marker) {
		case 0xc0:
			read_start_of_frame(f);
			break;
	}

}

#define one_more_byte(c,f,idx) do{\
		c = fgetc(f);\
		(idx)++;\
	}while(0)


int main(int argc, char* argv[]){
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
	unsigned int idx = -1, col = 0;

	/*
	printf("%c", fgetc(fjpeg));
	printf("%c", fgetc(fjpeg));
	printf("%c", fgetc(fjpeg));
	printf("%c", fgetc(fjpeg));
	exit(0);*/

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
			handle_marker(fjpeg, c);
			col = 0;
		}else{

			if (!(col % 20))
				printf("\n");

			printf(" %02x", c);
			col++;
		}
	}

	fclose(fjpeg);

	return 0;
}
