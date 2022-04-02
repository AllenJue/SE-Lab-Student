#include "cache.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#define ADDRESS_LENGTH 64

char* trace_file = NULL;

int verbosity_cache = 0;

/* Counters used to record cache statistics */
extern int miss_count;
extern int hit_count;
extern int dirty_eviction_count;
extern int clean_eviction_count;

/*
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded.
 */
void printSummary(int hits, int misses, int dirty_evictions, int clean_evictions)
{
    printf("hits:%d misses:%d dirty evictions:%d clean evictions:%d\n", hits, misses, dirty_evictions, clean_evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d %d\n", hits, misses, dirty_evictions, clean_evictions);
    fclose(output_fp);
}


/*
 * replayTrace - replays the given trace file against the cache
 */
void replayTrace(cache_t *cache, char* trace_fn)
{
    char buf[1000];
    uword_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while( fgets(buf, 1000, trace_fp) != NULL) {
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);

            if( verbosity_cache)
                printf("%c %llx,%u ", buf[1], addr, len);

            switch (buf[1]) {
                case 'S':
                    access_data(cache, addr, WRITE);
                    break;
                case 'L':
                    access_data(cache, addr, READ);
                    break;
                case 'M':
                    access_data(cache, addr, READ);
                    access_data(cache, addr, WRITE);
                    break;
                default:
                    printf("Bad trace operation: %c\n", buf[1]);

            }

            if ( verbosity_cache)
                printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * main - Main routine
 */
int main(int argc, char* argv[])
{
    int s = -1, E = -1, b = -1;
    char c;
    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
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
            trace_file = optarg;
            break;
        case 'v':
             verbosity_cache = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == -1 || E == -1 || b == -1 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    /* Compute S, E and B from command line args */

    /* Initialize cache */
    cache_t *cache = create_cache(s, b, E, 0);

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
    printf("DEBUG: set_index_mask: %llu\n", set_index_mask);
#endif

    replayTrace(cache, trace_file);

    /* Free allocated memory */
    free_cache(cache);

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, dirty_eviction_count, clean_eviction_count);
    return 0;
}
