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
	u16int length;
	char identifier[4];
	u16int version;
	u8int units;
	u16int xdensity;
	u16int ydensity;
	u8int xthumb;
	u8int ythumb;
	u8int RGB[][3];
}__attribute__ ((__packed__));

struct Nf_array {
	u8int id;                 /* Component identifier */
	u8int hv_sampling_factor; /* Horizontal-Vertical sampling factor */
	u8int quant_table_number; 
};

struct start_of_frame{
	u16int length;
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
	u16int length;
	struct quantization quantization[];
}__attribute__ ((__packed__));

struct huffman_table {
	u16int length;
	u8int matrix_type:4;
	u8int identifier:4;
	u8int ncodes[16];
	u8int values[];
}__attribute__ ((__packed__));

huffman_t* huffman_from_jpeg_header(struct huffman_table);

#endif