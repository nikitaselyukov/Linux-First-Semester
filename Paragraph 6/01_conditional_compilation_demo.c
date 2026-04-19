#include <stdio.h>

static void process(void)
{
#if defined(USE_GPU) && defined(USE_FLOAT)
    printf("USE_GPU and USE_FLOAT\n");
#elif defined(USE_GPU)
    printf("USE_GPU\n");
#elif defined(USE_FLOAT)
    printf("USE_FLOAT\n");
#else
    printf("None\n");
#endif
}

int main(void)
{
    process();
    return 0;
}
