#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct {
	int minimal:1;
	int binary:1;
	char* fmt;
}options;

#define USAGE_STR \
	"usage: occourence [options] [filename]\n\n" \
	"calculate occourence of bytes\n\n" \
	"options:\n" \
	"\t-p\tshow percentual\n" \
	"\t-m\tminimal output (no zero occourence)\n" \
	"\t-z\tbinary\n" \
	"\t-f FMT\tbyte output format string\n"

unsigned long long int occourrence[0xff];

void usage(int exit_code) {
	fprintf(stderr, USAGE_STR);
	exit(exit_code);
}

int main(int argc, char* argv[]) {

	options.fmt = strdup("0x%02x");
	
	int nargs = 0;
	while (++nargs < argc) {
		if (argv[nargs][0] != '-')
			continue;

		switch (argv[nargs][1]) {
			case 'h':
				usage(0);
				break;
			case 'm':
				options.minimal = 1;
				break;
			case 'f':
				options.fmt = strdup(argv[++nargs]);
				break;
				
		}
	}
	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'h')
		usage(0);

	FILE* ff = stdin;

	char c;
	while(1){
		c = fgetc(ff);
		if (feof(ff))
			break;

		occourrence[(int)c]++;
	}

	char byte[256];

	unsigned int cycle;
	for (cycle = 0 ; cycle < 0xff ; cycle++) {
		if (occourrence[cycle] == 0 && options.minimal)
			continue;
		sprintf(byte, options.fmt, cycle);
		fprintf(stdout, "%s\t%llu\n", byte, occourrence[cycle]);
	}

	return 0;
}
