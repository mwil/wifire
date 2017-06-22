/* taken from rand(3) manpage (POSIX.1-2001) */

static unsigned long next = 1;

int rand2(void)
{
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
}

void srand2(unsigned seed)
{
    next = seed;
}

