#include <stdio.h>
#include <stdlib.h>

void add(int ticks, int32_t *x, int32_t *y, int scalar, int32_t *s) {
    for(int i = 0; i < ticks; i++)
        s[i] = x[i] + y[i] + scalar;
}


