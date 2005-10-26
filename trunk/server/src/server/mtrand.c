#include <global.h>

/* length of state vector */
#define MTRand_N 624

/* period parameter */
#define MTRand_M 397

/* internal state */
uint32 MTRand_state[MTRand_N];

/* next value to get from state */
uint32 *MTRand_pNext;

/* number of values left before reload needed */
int MTRand_left;

uint32 MTRand_twist(const uint32 m, const uint32 s0, const uint32 s1)
{
    return m ^ ((s0 & 0x80000000UL) | (s1 & 0x7fffffffUL) >> 1) ^ (- (sint32)(s1 & 0x00000001UL) & 0x9908b0dfUL);
}

/* Genereate N new values in state */
void MTRand_reload()
{
    register uint32 *p = MTRand_state;
    register sint32 i;
    for(i = MTRand_N - MTRand_M; i--; ++p)
        *p = MTRand_twist(p[MTRand_M], p[0], p[1]);
    for(i = MTRand_M; --i; ++p )
        *p = MTRand_twist(p[MTRand_M - MTRand_N], p[0], p[1]);
    *p = MTRand_twist(p[MTRand_M - MTRand_N], p[0], MTRand_state[0]);
    MTRand_left = MTRand_N, MTRand_pNext = MTRand_state;
}

/* Set seed value like srandom */
void MTRand_init(const uint32 seed)
{
    register uint32 *s = MTRand_state;
    register uint32 *r = MTRand_state;
    register sint32 i = 1;
    *s++ = seed & 0xffffffffUL;
    for(; i < MTRand_N; ++i)
    {
        *s++ = (1812433253UL * (*r ^ (*r >> 30)) + i) & 0xffffffffUL;
        r++;
    }
    MTRand_reload();
}

/* MTRand_randComp is compatible to the standard random function */
/* return must be sint32 (rand() is int return too) */
sint32 MTRand_randComp(void)
{
    register uint32 s1;
    if(!MTRand_left)
        MTRand_reload();
    --MTRand_left;
    s1 = *MTRand_pNext++;
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9d2c5680UL;
    s1 ^= (s1 << 15) & 0xefc60000UL;
    /* Mask out the sign bit */
    return (s1 ^ (s1 >> 18)) & 0x7fffffffUL;
}
