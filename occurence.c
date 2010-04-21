#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct {
	int minimal:1;
	int binary:1;
	char* fmt;
	char endline;
	char between;
	int argc;
	char* argv[];
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
	options.argc = 1;
	options.argv[0] = argv[0];
	options.endline = '\n';
	options.between = '\t';
	
	int nargs = 0;

	/* parse options:
	 *
	 * arg which are not options are copied
	 * in options.argc and options.argv
	 *
	 */
	while (++nargs < argc) {
		if (argv[nargs][0] != '-') {
			options.argv[++options.argc] = argv[nargs];
			continue;
		}

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
			case 'z':
				options.endline = '\0';
				options.between = '\0';
				break;
				
		}
	}
	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'h')
		usage(0);

	FILE* ff = stdin;

	unsigned char c;
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
		fprintf(stdout, "%s%c%llu%c",
			byte, options.between,
			occourrence[cycle],
			options.endline);
	}

	return 0;
}
