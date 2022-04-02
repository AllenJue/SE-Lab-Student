#include <stdio.h>
#include <stdbool.h>
#include "common.h"

/*
 * A possible hierarchy for the cache. The helper functions defined below
 * are based on this cache structure.
 * lru is a counter used to implement LRU replacement policy.
 */
typedef struct cache_line {
    bool valid;
    uword_t tag;
    bool dirty;
    uword_t lru;
    byte_t *data;
} cache_line_t;

typedef struct cache_set {
    cache_line_t *lines;
} cache_set_t;

typedef struct cache {
    cache_set_t *sets;
    unsigned int s; /* set index bits */
    unsigned int b; /* block offset bits */
    unsigned int E; /* associativity */
    unsigned int d; /* cache delay */
} cache_t;


typedef enum {
    READ,
    WRITE
} operation_t;

typedef struct {
    bool valid;
    bool dirty;
    uword_t addr;
    byte_t *data;
} evicted_line_t;


cache_t *create_cache(int s_in, int b_in, int E_in, int d_in);
void free_cache(cache_t *cache);
void access_data(cache_t *cache, uword_t addr, operation_t operation);

evicted_line_t *handle_miss(cache_t *cache, uword_t addr, operation_t operation, byte_t *incoming_data);
bool check_hit(cache_t *cache, uword_t addr, operation_t operation);

void get_byte_cache(cache_t *cache, uword_t addr, byte_t *dest);
void get_word_cache(cache_t *cache, uword_t addr, word_t *dest);
void set_byte_cache(cache_t *cache, uword_t addr, byte_t val);
void set_word_cache(cache_t *cache, uword_t addr, word_t val);

cache_t *create_checkpoint(cache_t *cache);
void display_set(cache_t *cache, unsigned int set_index);