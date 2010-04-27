#include<stdio.h>
#include<stdlib.h>

#include<utils/bits.h>

int main(int argc, char* argv[]) {
	if(argc == 2) {
		unsigned int value = atoi(argv[1]);
		if (value > 0xff) {
			fprintf(stderr, "value too much\n");
			exit(1);
		}
		printf_byte(value);
		return 0;
	}

	printf("0000100001011111   expected\n");
	printf_16bits(LONGBITS(0,0,0,0,1,0,0,0,0,1,0,1,1,1,1,1));

	unsigned char* buffer = calloc(1, 3);
	write_bits(buffer, BITS(0,1,0,1,1,0,0,0), 8);

	printf("01011000   expected\n");
	printf_byte(buffer[0]);

	puts(" ---");
	write_bits(buffer, BITS(0,0,0,0,0,1,0,0), 4);
	printf("01011000   expected\n");
	printf_byte(buffer[0]);
	printf("01000000   expected\n");
	printf_byte(buffer[1]);

	puts(" ---");
	write_bits(buffer, BITS(0,0,0,0,0,0,1,0), 3);
	printf("01000100   expected\n");
	printf_byte(buffer[1]);

	puts(" ---");
	write_bits(buffer, BITS(0,0,0,0,0,0,0,1), 1);
	printf("01000101   expected\n");
	printf_byte(buffer[1]);

	puts(" ---");
	write_bits(buffer, BITS(0,0,0,0,0,0,0,1), 1);
	printf("01000101   expected\n");
	printf_byte(buffer[1]);
	printf("10000000   expected\n");
	printf_byte(buffer[2]);

	puts(" ---");
	write_bits(buffer, BITS(0,0,0,0,0,0,0,0), 1);
	printf("10000000   expected\n");
	printf_byte(buffer[2]);

	puts(" ---");
	write_bits(buffer, BITS(0,0,0,0,0,0,0,1), 1);
	printf("10100000   expected\n");
	printf_byte(buffer[2]);

	puts(" ---");
	write_bits(buffer, LONGBITS(0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1), 12);
	printf("10110000   expected\n");
	printf_byte(buffer[2]);
	printf("10000110   expected\n");
	printf_byte(buffer[3]);

	return 0;
}
