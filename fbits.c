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
	int code_size = 0;
	if (argc > 2) {
		while (++idx < argc) {
			code_size = atoi(argv[idx]);
			if(fread_bits(&value, code_size, 1, f) < 1)
				printf(" fread_bits returns < 1\n");
			printf("ftell: %ld ", ftell(f));
			if (code_size > 8)
				printf_16bits(value);
			else
				printf_byte(value);
		}
	}

	return 0;
}
