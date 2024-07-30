#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

struct line {
    int valid;
    int tag;
    int last_used_time;
};

// E lines each set
//set another name for struct line* which is set
typedef struct line* set;

// 2^s set each cache
set* cache;

int h, v = 0, s, E, b, t, time_stamp = 0;
unsigned hit = 0, miss = 0, eviction = 0;

void initCache() {
    int i, j;
    cache = (set*)malloc(sizeof(set) * (1 << s));
    for (i = 0; i < (1 << s); i++) {
        cache[i] = (set)malloc(sizeof(struct line) * E);
        for (j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].last_used_time = 0;
        }
    }
}

void freeCache() {
    int i;
    for (i = 0; i < (1 << s); i++) {
        free(cache[i]);
    }
    free(cache);
}


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
    int opt;
    FILE* trace_file;
    if (argc == 1) {
        printUsage();
        exit(0);
    }

    //read flag's argument
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h':
                printUsage();
                break;
            case 'v':
                v = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = fopen(optarg, "r");
                break;
            default:
                printUsage();
                exit(0);
        }
    }

    // when somthing go wrong
    if (s < 0 || E < 0 || b < 0||b + s > 64 || trace_file == NULL) {
        printUsage();
        exit(1);
    }

    initCache();


   
    

    printSummary(0, 0, 0);
    return 0;
}
