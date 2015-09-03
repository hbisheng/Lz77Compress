#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void add(int ticks, int32_t *x, int32_t *y, int scalar, int32_t *s);

int main(void)
{
        const int size = 64;
        int sizeBytes = size * sizeof(int32_t);
        int32_t *x = malloc(sizeBytes);
        int32_t *y = malloc(sizeBytes);
        int32_t *s = malloc(sizeBytes);
        int scalar = 3;

        // Generate input data
        for(int i = 0; i < size; ++i) {
                x[i] = random() % 100;
                y[i] = random() % 100;
        }

        int ticks = size;

        // Call
        add(ticks, x, y, scalar, s);

        return 0;
}
