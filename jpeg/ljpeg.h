#ifndef __LJPEG__
#define __LJPEG__
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

#include<huffman/huffman.h>

typedef unsigned char  u8int;
typedef unsigned short u16int;

#define one_more_byte(c,f,idx) \
	do{\
		c = fgetc(f);\
		(idx)++;\
	}while(0)

struct JFIF_header {
	char identifier[4];
	u16int version;
	u8int units;
	u16int xdensity __attribute__ ((__packed__));
	u16int ydensity __attribute__ ((__packed__));
	u8int xthumb;
	u8int ythumb;
	u8int RGB[][3];
};

struct Nf_array {
	u8int id;                 /* Component identifier */
	u8int hv_sampling_factor; /* Horizontal-Vertical sampling factor */
	u8int quant_table_number; 
};

struct start_of_frame{
	u8int sample; /* Sample precision */
	u16int Y;     /* Number of lines */
	u16int X;     /* Number of samples per line */
	u8int Nf;     /* Number of image components in frame */
	struct Nf_array nf_array[];
}__attribute__ ((__packed__));

struct quantization {
	u8int precision:4;
	u8int index:4;
	u8int value[64];
};

struct quantization_table {
	struct quantization quantization[1];
};

/*
 * If you have to pass as argument this struct and you want
 * to access correctly the values field, you have to pass a
 * pointer to it.
 */
struct ljpeg_huffman_table {/* maybe change the prefix in ljpeg_ */
	u8int identifier:4;   /* high bit */
	u8int matrix_type:4;  /* low bit  */
	u8int ncodes[16];
	/* this MUST be defined like that otherwise the compiler creates
	 * a pointer and when passed to another function the values
	 * are screwed up */
	u8int values[1];/* symbols */
};

struct start_of_scan {
	u16int length;
	u8int ncomponents;
	u16int* components __attribute__ ((__packed__));
	u8int* data;
};

void create_zig_zag(unsigned int side);
void de_zig_zag(int16_t values[], unsigned int side);
void ude_zig_zag(uint8_t values[], unsigned int side);
huffman_t* huffman_from_jpeg_header(struct ljpeg_huffman_table*);

void JFIF_header_print_info(void);
void read_JFIF_header(FILE* f);

void read_quantization_table_header(FILE* f);

void start_of_frame_print_info(void);
void read_start_of_frame(FILE* f);

void read_huffman_table(FILE* f);
void ljpeg_print_huffman_tables(void);

void ljpeg_read_scan_data(FILE* f);

#endif
