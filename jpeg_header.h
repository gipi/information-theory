#ifndef __JPEG_HEADER__
#define __JPEG_HEADER__

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
	u16int ncodes;
	u8int value[];
}__attribute__ ((__packed__));

#endif
