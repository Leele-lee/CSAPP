#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>

void printUsage() {
    puts("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>");
    puts("Options:");
    puts("  -h         Print this help message.");
    puts("  -v         Optional verbose flag.");
    puts("  -s <num>   Number of set index bits.");
    puts("  -E <num>   Number of lines per set.");
    puts("  -b <num>   Number of block offset bits.");
    puts("  -t <file>  Trace file.");
    puts("");
    puts("Examples:");
    puts("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace");
    pups("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace");
}

int main(int argc, char* argv[])
{
    FILE* trace_file;
    if (argc == 1) {
        printUsage();
        exit(0);
    }

    printSummary(0, 0, 0);
    return 0;
}
