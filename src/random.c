#include "headers/random.h"
#include "headers/time.h"


int rand() {
    uint64_t val = (uint64_t)RAND_SEED * rdtsc() - 1327;
    return (int)(val % ((uint64_t)RAND_MAX + 1));
}