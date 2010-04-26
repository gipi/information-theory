#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<frequency.h>

struct {
	int minimal:1;
	int binary:1;
	int percentual:1;
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
			case 'p':
				options.percentual = 1;
				break;
				
		}
	}
	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'h')
		usage(0);

	FILE* ff = stdin;
	frequency_table_create_from_stream(ff, FREQUENCY_DONT_SAVE_STREAM);

	char byte[256];

	unsigned int cycle;

	/* if we want the percentual*/
	if (options.percentual) {
		unsigned long long int total = 0;
		/* first calculate the total */
		for (cycle = 0 ; cycle < 0xff ; cycle++) {
			total += occurrence[cycle];
		}

		fprintf(stderr, "total: %llu\n", total);

		/* second rescale */
		for (cycle = 0 ; cycle < 0xff ; cycle++) {
			occurrence[cycle] = (double)occurrence[cycle]/(double)total*100;
		}
	}

	for (cycle = 0 ; cycle < 0xff ; cycle++) {
		if (occurrence[cycle] == 0 && options.minimal)
			continue;
		sprintf(byte, options.fmt, cycle);
		fprintf(stdout, "%s%c%llu%s%c",
			byte, options.between,
			occurrence[cycle], options.percentual ? "%" : "",
			options.endline);
	}

	return 0;
}
