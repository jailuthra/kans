#include "kans.h"

static uint8_t find_symbol(struct ANSCtx *a)
{
    uint32_t mx = a->state & ANS_MASK;
    for (int i = 0; i < ALPHLEN; i++) {
        if (mx >= a->ftable[i].c && mx < a->ftable[i+1].c) {
            return (uint8_t) i;
        }
    }
    return 0;
}

void ANSCompress(struct ANSCtx *a, uint8_t symbol)
{
    a->state = ((a->state/a->ftable[symbol].f) << ANS_BITS)
               + (a->state % a->ftable[symbol].f)
               + a->ftable[symbol].c;
}

uint8_t ANSDecompress(struct ANSCtx *a)
{
    uint8_t symbol = find_symbol(a);
    a->state = a->ftable[symbol].f * (a->state >> ANS_BITS)
               + (a->state & ANS_MASK)
               - a->ftable[symbol].c;
    return symbol;
}

