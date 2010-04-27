#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>

#define NTH(b, n) ((b) & (1 << n) ? 1 : 0)
#define BITIFY(a,n) ((a) ? (1 << (n)): 0)
#define BITS(a,b,c,d,e,f,g,h) ( BITIFY(a, 7) | \
	BITIFY(b,6) | \
	BITIFY(c,5) | \
	BITIFY(d,4) | \
	BITIFY(e,3) | \
	BITIFY(f,2) | \
	BITIFY(g,1) | \
	BITIFY(h,0)  )
#define LONGBITS(a,b,c,d,e,f,g,h,i,l,m,n,o,p,q,r) ( \
	BITIFY(a,15) | \
	BITIFY(b,14) | \
	BITIFY(c,13) | \
	BITIFY(d,12) | \
	BITIFY(e,11) | \
	BITIFY(f,10) | \
	BITIFY(g,9) | \
	BITIFY(h,8) | \
	BITIFY(i,7) | \
	BITIFY(l,6) | \
	BITIFY(m,5) | \
	BITIFY(n,4) | \
	BITIFY(o,3) | \
	BITIFY(p,2) | \
	BITIFY(q,1) | \
	BITIFY(r,0)  )

static void printf_byte(uint8_t bits) {
	printf("%u%u%u%u%u%u%u%u\n",
		NTH(bits, 7),
		NTH(bits, 6), NTH(bits, 5),
		NTH(bits, 4), NTH(bits, 3),
		NTH(bits, 2), NTH(bits, 1),
		NTH(bits, 0));
}

static void printf_16bits(uint16_t bits) {
	printf("%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u\n",
		NTH(bits, 15),
		NTH(bits, 14), NTH(bits, 13),
		NTH(bits, 12), NTH(bits, 11),
		NTH(bits, 10), NTH(bits, 9),
		NTH(bits, 8), NTH(bits, 7),
		NTH(bits, 6), NTH(bits, 5),
		NTH(bits, 4), NTH(bits, 3),
		NTH(bits, 2), NTH(bits, 1),
		NTH(bits, 0));
}

/* create a mask */
static inline uint64_t create_mask_from_msb(
	uint8_t size, uint8_t start, uint8_t delta)
{
	uint64_t mask;
	unsigned int cycle;
	for (cycle = 0 ; cycle < delta; cycle++){
		mask |= (1 << (size - start - cycle));
	}

	return mask;
}

/*
 * Write bits of size 'length' in 'buffer'.
 *
 * EXAMPLE:
 *   1010 has size 4 so we need a mask like 11110000 to insert
 *   it at the start of the byte, like 00111100 to insert it
 *   starting from the third bits of the byte and like 00000111 to insert
 *   it to the end of the byte, remembering to indert
 */
void write_bits(uint8_t* buffer, uint64_t bits, uint8_t length) {
	/* byte position in the buffer */
	static uint64_t buffer_idx = 0;
	/* bit position in the byte (0 = msb, 7 = lsb) */
	static uint8_t bits_idx = 0;

	/*
	 * This function act recursevely saving into buffer
	 * the bits up to complete the byte and calling itself
	 * with the remaining bits.
	 */
	uint8_t nbits_available = (8 - bits_idx);
	uint64_t the_mask;
	uint8_t bits_to_write;

	if (length >= nbits_available) {
		the_mask = create_mask_from_msb(length, 0, nbits_available);
		bits_to_write =
			(bits & the_mask) >> (length - nbits_available);
		buffer[buffer_idx++] |= bits_to_write;
		bits_idx = 0;
		write_bits(buffer,
			bits & ~the_mask, length - nbits_available);
	} else {
		bits_to_write = bits << (nbits_available - length);

		buffer[buffer_idx] |= bits_to_write;
		bits_idx += length;
	}
}

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

	return 0;

	puts(" ---");
	uint64_t tmp = LONGBITS(0,0,0,0,1,0,0,0,0,1,0,1,1,1,1,1);
	write_bits(buffer, tmp, 1);
	printf("01000101   expected\n");
	printf_byte(buffer[1]);
	printf("00001011   expected\n");
	printf_byte(buffer[2]);
	printf("11100000   expected\n");
	printf_byte(buffer[3]);

	return 0;
}
