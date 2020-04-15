// BinToTxt.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#include "base_types.h"

#define OUT_CHAR
//#define OUT_DWORD
typedef char CHAR;
typedef int INT;
typedef unsigned int DWORD;
typedef unsigned char BYTE;
static void Usage(CHAR *argv[]) {
    fprintf(stderr, "\r\nEXPLAIN:\n");
    fprintf(stderr, "\tThis BinToTxt.exe will covert pack bin file into BYTE(1 byte) or Double Word(4 bytes) format (ex: 0x12 or 0x12345678)\r\n");
    fprintf(stderr, "\r\nUSAGE:\n");
    fprintf(stderr, "\tBinToTxt.exe <Output format> <Pack bin file> <Output file>\r\n");
}

// return output file format in 1 or 4 bytes.
static int ParseCommLine(INT argc, CHAR *argv[]) {
    
    if (argc != 4) {
        Usage(argv);
        system("pause");
        exit(-1);
    }

    if (!strcmp(argv[1], "1")) {
        return 1;
    }
    else if (!strcmp(argv[1], "4")) {
        return 4;
    }
    else {
        printf("Error: Wrong parameter for output format.\n");
        system("pause");
        exit(-1);
    }
}


int main(INT argc, CHAR *argv[]) {
    int outPutSize = 0;
    int iNum = 0;

    unsigned int dwordFromBinary[2];
    unsigned char byteFromBinary[2];

    FILE *binary, *wFile;

    outPutSize = ParseCommLine(argc, argv);

    binary = fopen(argv[2], "rb");
    wFile = fopen(argv[3], "w");
        
    if (binary == NULL) {
        printf("Error: Can not open bin file.\n");
        system("pause");
        exit(-1);
    }

    if (wFile == NULL) {
        printf("Error: Can not open write file.\n");
        system("pause");
        exit(-1);
    }
    
    if (outPutSize == 4) {
        while (fread(&dwordFromBinary, sizeof(DWORD), 1, binary) == 1) {
            if ((iNum % 6) == 0) {
                fprintf(wFile, "\t0x%08X, ", *dwordFromBinary);
            }
            else if ((iNum % 6) == 5) {
                fprintf(wFile, "0x%08X, \n", *dwordFromBinary);
            }
            else {
                fprintf(wFile, "0x%08X, ", *dwordFromBinary);
            }
            iNum++;
        }
        printf("Total size: %d DWORD(4 bytes)\n", iNum);
    }
    else if (outPutSize == 1) {
        while (fread(&byteFromBinary, sizeof(BYTE), 1, binary) == 1) {
            if ((iNum % 16) == 0) {
                fprintf(wFile, "\t0x%02X, ", *byteFromBinary);
            }
            else if ((iNum % 16) == 15) {
                fprintf(wFile, "0x%02X, \n", *byteFromBinary);
            }
            else {
                fprintf(wFile, "0x%02X, ", *byteFromBinary);
            }
            iNum++;
        }
        printf("Total size: %d CHAR(1 byte)\n", iNum);
    }
    printf("Convert Successfully.\n");

    fclose(binary);
    fclose(wFile);
    
    //printf("argc = %d, argv = %s\n", argc, argv[0]);
    //system("pause");

    return 0;
}

