#ifndef __BITS__
#define __BITS__
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

void printf_byte(uint8_t bits);
void printf_16bits(uint16_t bits);
void write_bits(uint8_t* buffer, uint64_t bits, uint8_t length);

#endif
