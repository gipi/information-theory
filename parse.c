#include<stdio.h>
#include<stdlib.h>

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
