#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include<utils/bits.h>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "usage: fbits <filename> [nbits1 nbits2]\n");
		exit(1);
	}

	FILE* f = fopen(argv[1], "r");
	if (!f) {
		perror("fatal: I could not open the file");
		exit(1);
	}
	uint64_t value = 0;

	fread_init(f);

	unsigned int idx = 1;
	if (argc > 2) {
		while (++idx < argc) {
			if(fread_bits(&value, atoi(argv[idx]), 1, f) < 1)
				printf(" fread_bits returns < 1\n");
			printf("ftell: %ld ", ftell(f));
			printf_byte(value);
		}
	}

	return 0;
}
