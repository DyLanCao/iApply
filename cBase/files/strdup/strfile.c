#include <stdio.h>

#include <string.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>


static struct option main_options[] = {
        { "help",       0, 0, 'h' },
        { "verbose",    0, 0, 'v' },
        { "msbc",       0, 0, 'm' },
        { "subbands",   1, 0, 's' },
        { "bitpool",    1, 0, 'b' },
        { "joint",      0, 0, 'j' },
        { "dualchannel",0, 0, 'd' },
        { "snr",        0, 0, 'S' },
        { "blocks",     1, 0, 'B' },
        { 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
        char *output = NULL;
        int i, opt, tofile = 0;
        bool msbc = false;

        while ((opt = getopt_long(argc, argv, "+hmvd:f:",
                                                main_options, NULL)) != -1) {
                switch(opt) {
                case 'h':
                        exit(0);

                case 'v':
                        break;

                case 'm':
                        msbc = true;
                        break;

                case 'd':
                        free(output);
                        output = strdup(optarg);
			//printf("%s",output);
                        tofile = 0;
                        break;

                case 'f' :
                        free(output);
                        output = strdup(optarg);
			//printf("%s",output);
                        tofile = 1;
                        break;

                default:
                        exit(1);
                }
        }

        argc -= optind;
        argv += optind;
        optind = 0;

        if (argc < 1) {
                exit(1);
        }
	

	for (i = 0; i < argc; i++)
			printf("%s \n\t",argv[i]);

        free(output);

        return 0;
}
              
