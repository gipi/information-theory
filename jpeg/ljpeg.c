#include<jpeg/ljpeg.h>
#include<huffman/huffman.h>

huffman_t* huffman_from_jpeg_header(struct huffman_table t) {
	unsigned int nc_cycle, codeidx;

	huffman_canon_t hc[0x100];
	unsigned int ncodes = 0;

	/* loop over the number of code of N nc_cycle bits */
	for (nc_cycle = 0 ; nc_cycle < 16 ; nc_cycle++) {
		for (codeidx = 0 ; codeidx < t.ncodes[nc_cycle] ; codeidx++) {
			hc[ncodes] = (huffman_canon_t) {
				.symbol = t.values[ncodes],
				.nbits  = nc_cycle
			};
			ncodes++;
		}
	}

	Huffman_set(hc, ncodes);

	return Huffman_build_canonicalize_representation();
}
