#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>

#include<utils/bits.h>


void printf_byte(uint8_t bits) {
	printf("%u%u%u%u%u%u%u%u\n",
		NTH(bits, 7),
		NTH(bits, 6), NTH(bits, 5),
		NTH(bits, 4), NTH(bits, 3),
		NTH(bits, 2), NTH(bits, 1),
		NTH(bits, 0));
}

void printf_16bits(uint16_t bits) {
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

/*
 * create a mask of size bits starting from the start-th most
 * significant bit for the following delta bits.
 *
 * EXAMPLES:
 *
 * 	create_mask_from_msb(8, 0, 8) ---> 11111111
 * 	create_mask_from_msb(8, 2, 6) ---> 00111111
 * 	create_mask_from_msb(8, 2, 3) ---> 00111000
 */
inline uint64_t create_mask_from_msb(
	uint8_t size, uint8_t start, uint8_t delta)
{
	uint64_t mask = 0;
	unsigned int cycle;
	for (cycle = 0 ; cycle < delta; cycle++){
		mask |= (1 << (size - start - cycle - 1));
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

/*
 * Read the following length bits from buffer. If advance is set
 * then update the internal indexes to the following bits.
 */
uint64_t read_bits(uint8_t* buffer, uint8_t length, int advance) {
	static size_t buffer_idx = 0;
	/* bit position in the byte (0 = msb, 7 = lsb) */
	static uint8_t bits_idx = 0;

	uint8_t old_bits_idx = bits_idx;
	uint8_t old_buffer_idx = buffer_idx;

	uint64_t value = 0;
	uint8_t nbits_available = (8 - bits_idx);
	uint64_t the_mask = 0;

	if (length >= nbits_available) {
		the_mask = create_mask_from_msb(
			length, bits_idx, nbits_available);
		value = buffer[buffer_idx++] & the_mask;
		bits_idx = 0;
		value <<= length - nbits_available;
		value |= read_bits(
			buffer, length - nbits_available, advance);
	} else {
		the_mask = create_mask_from_msb(8, bits_idx, length);
		value = buffer[buffer_idx] & the_mask;
		value >>= (nbits_available - length);
		bits_idx += length;
	}

	if (!advance) {
		buffer_idx = old_buffer_idx;
		bits_idx = old_bits_idx;
	}

	return value;
}
