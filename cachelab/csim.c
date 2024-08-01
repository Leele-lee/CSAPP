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
FILE* trace_file;
char* trace_name = NULL;
int modify = 0;

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

// properly deal with load, store, modify situation
void usingCache(size_t address) {
    int set_index;
    int tag_index;
    int i;
    int min_timestamp_line = 0;
    int need_eviction = 1; // default need eviction
    set_index = (address << (s + b)) >> (64 - s);
    tag_index = (address >> (s + b));
    set current_set = cache[set_index];

    //check every line in given set, 
    for (i = 0; i < E; i++) {
        if (current_set[i].last_used_time < min_timestamp_line) {
            min_timestamp_line = i;
        }
        if (current_set[i].valid == 0) {
            need_eviction = 0;
            continue;
        } else if (tag_index == current_set[i].tag) { 
            // check t 
            hit++;
            if (v = 1) {
                printf(" hit");
            }
            current_set[i].last_used_time = time_stamp++;
            hit += modify;
            if (modify) {
                printf(" hit\n");
            }
            return;
        }
    }
    // determine miss or eviction
    miss++;
    eviction += need_eviction;
    if (v && need_eviction && modify) {
        printf(" miss eviction hit\n");
    } else if (v && need_eviction) {
        printf(" miss eviction\n");
    } else if (v){
        printf(" miss\n");
    }
    hit += modify;
    // add data into cache line which has the largest timestamp
    current_set[min_timestamp_line].valid = 1;
    current_set[min_timestamp_line].tag = tag_index;
    current_set[min_timestamp_line].last_used_time = time_stamp++;
}

//I 0400d7d4,8
// M 0421c7f0,4
// L 04f6b868,8
// S 7ff0005c8,8
void replayTrace() {
    int size;
    char identifier;
    size_t address;
    
    trace_file = fopen(trace_name, "r");;
    while (fscanf(trace_file, "%s, %lx, %d", &identifier, &address, &size)) {
        if (v) {
            printf("%c, %lx, %d", identifier, address, size);
        }
        switch (identifier) {
            case 'I': continue;
            case 'L': usingCache(address); modify = 0; break;
            case 'S': usingCache(address); modify = 0; break;
            case 'M': usingCache(address); modify = 1; break;
            default: break;
        } 
    }
    fclose(trace_file);
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
                trace_name = optarg;
                break;
            default:
                printUsage();
                exit(0);
        }
    }

    // when somthing go wrong
    if (s < 0 || E < 0 || b < 0||b + s > 64 || trace_name == NULL) {
        printUsage();
        exit(1);
    }

    initCache();
    //use cache



   
    

    printSummary(0, 0, 0);
    return 0;

}
