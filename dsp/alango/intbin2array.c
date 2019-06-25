//-----------------------------------------------------------------------------
// convert bin to C-style hex array
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define BUF_SIZE    (16*1024)
#define ARG_BINARY_FILE    1
#define ARG_C_ARRAY_FILE   2

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    FILE *pf_bin = NULL, *pf_txt = NULL;
    unsigned char buf[BUF_SIZE];
    unsigned char s[16 * 1024];
    int i, n_bin;

    if (argc < 3)
    {
        printf("few arguments: \r\n");
        printf("<array name> <input bin> <output c>\r\n");
        goto quit;
    }

    //------- files
    pf_bin = fopen(argv[ARG_BINARY_FILE], "rb");
    if (pf_bin == NULL)
    {
        printf("can't open '%s'\r\n", argv[ARG_BINARY_FILE]);
        goto quit;
    }
    else
    {
        printf(" open '%s'\r\n", argv[ARG_BINARY_FILE]);
    }

    pf_txt = fopen(argv[ARG_C_ARRAY_FILE], "wb");
    if (pf_txt == NULL)
    {
        printf("can't open '%s'\r\n", argv[ARG_C_ARRAY_FILE]);
        goto quit;
    }
    else
    {
        printf("open '%s'\r\n", argv[ARG_C_ARRAY_FILE]);
    }

    fseek(pf_bin,0L,SEEK_END);
    int size = ftell(pf_bin);
    printf("pf_txt read buff size:%d ",size);

    n_bin = fread(buf, sizeof(unsigned int), size, pf_bin);

    sprintf(s, "unsigned char %s[%d] = \r\n{", argv[ARG_BINARY_FILE], n_bin);
    fwrite(s, 1, strlen(s), pf_txt);

    for (i = 0; i < n_bin; i++)
    {

        sprintf(s, "%s0x%02X%s", i % 16 ? "" : "\r\n    ", buf[i], i < n_bin - 1 ? "," : "");

        fwrite(s, 1, strlen(s), pf_txt);
    }
    sprintf(s, "\r\n};\r\n");
    fwrite(s, 1, strlen(s), pf_txt);

quit:
    if (pf_bin)
    {
        printf(" close '%s'\r\n", argv[ARG_BINARY_FILE]);
        fclose(pf_bin);
    }

    if (pf_txt)
    {
        printf(" close '%s'\r\n", argv[ARG_C_ARRAY_FILE]);
        fclose(pf_txt);
    }

    return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
