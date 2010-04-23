#include<frequency.h>


unsigned long long int occurrence[0xff];

void frequency_table_create_from_stream(FILE* ff) {
	unsigned char c;
	while(1){
		c = fgetc(ff);
		if (feof(ff))
			break;

		occurrence[(int)c]++;
	}
}
