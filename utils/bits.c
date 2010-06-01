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

/* create a mask with the initial bits all zero starting from MSB */
/* create_complementary_mask(8) =
 * 	1111111111111111111111111111111111111111111111111111111100000000 */
inline uint64_t create_complementary_mask(uint8_t size) {
	uint64_t mask = 0;
	unsigned int cycle;
	for (cycle = 0 ; cycle < size; cycle++){
		mask |= (1 << (size - cycle - 1));
	}

	return ~mask;
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

/******* FILE API ***************************/

/* this contains private data */
struct {
	uint8_t byte;
	/* bit position in the byte (0 = msb, 7 = lsb) */
	uint8_t bits_idx;
	uint8_t eof;
}_file;

static int _file_read_a_byte(FILE* f) {
	return fread(&_file.byte, sizeof(uint8_t), 1, f);
}

/*
 * Move internal pointer.
 *
 * TODO: make bits_offset works.
 */
int fseek_bits(FILE* f, long byte_offset, long bits_offset, int whence) {
	return fseek(f, byte_offset, whence);
}

int fread_init(FILE* f) {
	_file.bits_idx = 0;
	return 0;//_file_read_a_byte(f);
}

/* TODO: the stream has to be seekable */
size_t fread_bits(uint64_t* value, uint8_t length, int advance, FILE* f) {
	static uint8_t status = 0;
	uint8_t old_seek_bits = _file.bits_idx;
	uint8_t old_byte = _file.byte;
	uint8_t seek = 0;

	uint8_t nbits_available = (8 - _file.bits_idx);
	uint64_t the_mask = 0;

	/* if bits_idx == 0 then read a byte from the stream */
	if (!_file.bits_idx) {
		status = _file_read_a_byte(f);
		if (status < 1)
			return -1;
		seek++;
	}

	if (length > nbits_available) {
		the_mask = create_mask_from_msb(
			8, _file.bits_idx, nbits_available);
		*value = _file.byte & the_mask;

		/* update the internal file position */
		_file.bits_idx = 0;

		/* adjust value */
		*value <<= length - nbits_available;
		/* read remaining bits */
		uint64_t new_value;
		status = fread_bits(&new_value,
			length - nbits_available, advance, f);
		*value |= new_value;
	} else {
		the_mask = create_mask_from_msb(8, _file.bits_idx, length);
		*value = _file.byte & the_mask;
		*value >>= (nbits_available - length);
		_file.bits_idx += length;
	}

	/* */
	if (!advance) {
		if (fseek_bits(f, -seek, 0, SEEK_CUR) < 0)
			perror("fseek_bits");
		_file.bits_idx = old_seek_bits;
		_file.byte = old_byte;
	}

	return status;
}

static uint64_t _xio_readinternalbytevalue(xio_t xio) {
	uint64_t value;
	switch (xio.type) {
		case XIO_TYPE_BUFFER:
			value = xio.buffer.data[xio.buffer.bytes_idx];
			break;
		case XIO_TYPE_STREAM:
			/* TODO: some check */
			fseek(xio.stream.file, xio.stream.seek, SEEK_SET);
			fread(&value, sizeof(value),1, xio.stream.file);
			break;
	}

	return value;
}

static uint8_t _xio_get_nbits_available(xio_t xio) {
	uint8_t nbitsavailable = 0;
	switch (xio.type) {
		case XIO_TYPE_BUFFER:
			nbitsavailable = 8 - xio.buffer.bits_idx;
			break;
		case XIO_TYPE_STREAM:
			break;
	}

	return nbitsavailable;
}

/* update the internal index to the next byte */
static void _xio_next(xio_t* xio) {
	switch (xio->type) {
		case XIO_TYPE_BUFFER:
			xio->buffer.bytes_idx++;
			xio->buffer.bits_idx = 0;
			break;
		case XIO_TYPE_STREAM:
			xio->stream.seek++;
			xio->stream.bits_idx = 0;
			break;
	}
}

static void _xio_update_bits_idx(xio_t* xio, int delta) {
	switch (xio->type) {
		case XIO_TYPE_BUFFER:
			xio->buffer.bits_idx += delta;
			break;
		case XIO_TYPE_STREAM:
			xio->stream.bits_idx += delta;
			break;
	}
}

/* create a mask starting from bits_idx to delta bits*/
uint64_t _xio_mask(xio_t xio, uint8_t delta) {
	/* the start position is always bits_idx */
	uint8_t start;
	switch (xio.type) {
		case XIO_TYPE_BUFFER:
			start = xio.buffer.bits_idx;
			break;
		case XIO_TYPE_STREAM:
			start = xio.stream.bits_idx;
			break;
	}

	uint64_t mask = 0;
	unsigned int cycle;
	for (cycle = 0 ; cycle < delta; cycle++){
		mask |= (1 << (8 - start - cycle - 1));
	}

	return mask;
}

size_t readbits(uint64_t* value, uint8_t length, int advance, xio_t* xio) {
	static uint8_t status = 0;
	xio_t old_xio = *xio;

	uint8_t nbits_available = _xio_get_nbits_available(*xio);
	uint64_t the_mask = 0;

	if (length > nbits_available) {
		/* read recursevely */
		the_mask = _xio_mask(*xio, nbits_available);
		*value = _xio_readinternalbytevalue(*xio) & the_mask;

		/* adjust value */
		*value <<= length - nbits_available;

		/* update the internal file position */
		_xio_next(xio);

		/* read remaining bits */
		uint64_t new_value;
		/* advance index */
		status = readbits(&new_value,
			length - nbits_available, advance, xio);
		*value |= new_value;
	} else {
		the_mask = _xio_mask(*xio, length);
		*value = _xio_readinternalbytevalue(*xio) & the_mask;
		*value >>= (nbits_available - length);
		_xio_update_bits_idx(xio, length);
	}

	/* */
	if (!advance)
		*xio = old_xio;

	return status;
}

#ifdef __TEST__

int main(int argc, char* argv[]) {
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'h') {
		printf("usage: %s [<nbits1> ... ]\n", argv[0]);
		exit(0);
	}

	uint8_t buffer[1024];
	size_t totalread = 0;
	while (!feof(stdin)) {
		totalread +=
			fread(buffer + totalread, 1, 1024 - totalread, stdin);
	}

	uint64_t value = 0;
	xio_t xio;
	xio.buffer = (struct Buffer){
		.type = XIO_TYPE_BUFFER,
		.data = buffer
	};

	unsigned int cycle = 0;
	for (cycle = 1 ; cycle < argc ; cycle++) {
		readbits(&value, atoi(argv[cycle]), 1, &xio);
		printf_byte(value);
		printf(" ");
	}

	return 0;
}
#endif /* __TEST__ */
