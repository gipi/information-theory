#ifndef __LJPEG_H__
#define __LJPEG_H__
/*
 * Copyright 2010 Gianluca Pacchiella <gianluca.pacchiella@ktln2.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include"config.h"
#endif

#ifdef HAVE_ARPA_INET_H /* ntohs */
#include<arpa/inet.h>
#elif HAVE_WINSOCK2_H
#include<winsock2.h>
#endif

#ifdef WIN32
#include<windows.h>
#endif

#include<huffman/huffman.h>

#define one_more_byte(c,f) \
	do{\
		c = fgetc(f);\
	}while(0)

struct JFIF_header {
	char identifier[4];
	uint16_t version;
	uint8_t units;
	uint16_t xdensity __attribute__ ((__packed__));
	uint16_t ydensity __attribute__ ((__packed__));
	uint8_t xthumb;
	uint8_t ythumb;
	uint8_t RGB[][3];
};

struct Nf_array {
	uint8_t id;                 /* Component identifier */
	uint8_t hsampl:4; /* Horizontal-Vertical sampling factor */
	uint8_t vsampl:4; /* Horizontal-Vertical sampling factor */
	uint8_t quant_table_number; 
};

struct start_of_frame{
	uint8_t sample; /* Sample precision */
	uint16_t Y;     /* Number of lines */
	uint16_t X;     /* Number of samples per line */
	uint8_t Nf;     /* Number of image components in frame */
	struct Nf_array nf_array[];
}__attribute__ ((__packed__));

struct quantization {
	uint8_t precision:4;
	uint8_t index:4;
	uint8_t value[64];
}__attribute__ ((__packed__));

struct quantization_table {
	struct quantization quantization[1];
};

/*
 * If you have to pass as argument this struct and you want
 * to access correctly the values field, you have to pass a
 * pointer to it.
 */
struct ljpeg_huffman_table {/* maybe change the prefix in ljpeg_ */
	uint8_t identifier:4;   /* high bit */
	uint8_t matrix_type:4;  /* low bit  */
	uint8_t ncodes[16];
	/* this MUST be defined like that otherwise the compiler creates
	 * a pointer and when passed to another function the values
	 * are screwed up */
	uint8_t values[1];/* symbols */
};

struct component {
	uint8_t id;
	uint8_t ac:4;
	uint8_t dc:4;
};

struct start_of_scan {
	uint16_t length;
	uint8_t ncomponents;
	struct component* components __attribute__ ((__packed__));
	uint8_t* data;
};

void create_zig_zag(unsigned int side);
void de_zig_zag(int16_t values[], unsigned int side);
void ude_zig_zag(uint8_t values[], unsigned int side);
huffman_t* huffman_from_jpeg_header(struct ljpeg_huffman_table*);

void JFIF_header_print_info(void);
void read_JFIF_header(FILE* f);

void read_quantization_table_header(FILE* f);
void quantization_table_print_info(void);

void start_of_frame_print_info(void);
void read_start_of_frame(FILE* f);

extern huffman_t* g_huffman_y_dc;
extern huffman_t* g_huffman_y_ac;
extern huffman_t* g_huffman_cbcr_dc;
extern huffman_t* g_huffman_cbcr_ac;

void read_huffman_table(FILE* f);
void ljpeg_print_huffman_tables(void);

void ljpeg_read_scan_data(FILE* f);
void ljpeg_print_scan_data(void);
void ljpeg_print_bits_scan_data(void);

void ljpeg_free(void);

#endif
