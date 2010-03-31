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

struct {
	u16int length;
	u8int sample;
	u16int Y;
	u16int X;
	u8int Nf;
	struct Nf_array* nf_array;
}start_of_frame;

static void handle_marker(FILE* f, unsigned char marker) {
	u16int length = 0;
	switch (marker) {
		case 0xc0:
			/* first read the length */

			printf("sizeof u16int: %u\n", sizeof(length));
			fread(&length, sizeof(length), 1, f);
			/* data with most significant bit first */
			length = htons(length);
			printf(" length: %u\n", length);

			/* rewind a little bit */
			fprintf(stdout, "before: %ld\n", ftell(f));
			fseek(f, -sizeof(length), SEEK_CUR);
			fprintf(stdout, "after: %ld\n", ftell(f));
			if(ferror(f))
				perror("fseek");
#if 1

			/* read all the frame */
			fread(&start_of_frame, length, 1, f);
#else
			fread(&start_of_frame.length, sizeof(length), 1, f);
			fread(&start_of_frame.sample, sizeof(u8int), 1, f);
			fread(&start_of_frame.Y, sizeof(u16int), 1, f);
			fread(&start_of_frame.X, sizeof(u16int), 1, f);
			fread(&start_of_frame.Nf, sizeof(u8int), 1, f);
#endif
			printf(" length: %u\n", htons(start_of_frame.length));
			printf(" sample: %u\n", start_of_frame.sample);
			printf(" Y: %04x\n", htons(start_of_frame.Y));
			printf(" X: %04x\n", htons(start_of_frame.X));
			printf(" #components: %u\n", start_of_frame.Nf);
			//printf(" id: %u\n", start_of_frame.nf_array[0].id);

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
