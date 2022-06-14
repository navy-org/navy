#include <stdlib.h>

static unsigned int random_next = 1;

void srandom(unsigned int seed)
{
    random_next = seed;
}

long random(void)
{
    random_next = random_next * 1103515245 + 12345;
    return (unsigned int) (random_next / 65536) % 32768;
}