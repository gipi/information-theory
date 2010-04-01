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

static void read_JFIF(FILE* f) {
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
}

static void read_start_of_frame(FILE* f) {
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
}

static void read_quantization_table(FILE* f) {
}

static void handle_marker(FILE* f, unsigned char marker) {
	switch (marker) {
		case 0xc0:
			read_start_of_frame(f);
			break;
		case 0xe0:
			read_JFIF(f);
			break;
	}
}


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
