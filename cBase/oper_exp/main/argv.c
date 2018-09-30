#include <stdio.h>
#include <unistd.h>

int main (int argc, char *argv[]) {
	int opt = 0;
	char *in_fname = NULL;
	char *out_fname = NULL;
	char test[] = "1";

	printf("\nInput option value=%d", opt);

	opt = getopt(argc, argv, "i:o:");
	printf("\nInput option .... value=%d", opt);
	while ((opt = getopt(argc, argv, "i:o:")) != -1) {

		printf("\nInput option value=%s", opt);
		switch(opt) {
			case 'i':
				in_fname = optarg;
				int ret = strcmp(in_fname,test);
					printf("\nInput option value=%s ret:%d", in_fname, ret);
				if(!strcmp(in_fname,test))
				{
					printf("\nInput option value=%s ret:%d", in_fname, ret);
				}
				break;
			case 'o':
				out_fname = optarg;
				printf("\nOutput option value=%s", out_fname);
				break;
			case '?':
				/* Case when user enters the command as
				 *      * $ ./cmd_exe -i
				 *           */
				if (optopt == 'i') {
					printf("\nMissing mandatory input option");
					/* Case when user enters the command as
					 *      * # ./cmd_exe -o
					 *           */
				} else if (optopt == 'o') {
					printf("\nMissing mandatory output option");
				} else {
					printf("\nInvalid option received");
				}
				break;
		}
	}

	printf("\n");
	return 0;
}
