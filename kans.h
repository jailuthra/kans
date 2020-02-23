/**
 * @file
 * KANS Header
 */

#include <stdint.h>

#ifndef KANS_H
#define KANS_H

#define ALPHLEN 256
#define ANS_BITS 12
#define ANS_RANGE (uint32_t)(1UL << (ANS_BITS))
#define ANS_MASK (uint32_t)(ANS_RANGE - 1U)
#define STREAM_BITS 16
#define STREAM_RANGE (uint32_t)(1UL << STREAM_BITS)
#define MAX_X (uint32_t)(1UL << ((2*STREAM_BITS) - ANS_BITS))

struct ftnode {
    uint64_t n; ///< frequency count in source
    uint32_t f; ///< Fs in working range
    uint32_t c; ///< Cumulative Fs (Cs) in working range
    uint32_t maxX; ///< max state for symbol (stream)
};

struct ANSCtx {
    struct ftnode *ftable;  
    uint32_t b; ///< chunk size for streaming
    uint32_t state;
};

void ANSCompress(struct ANSCtx *a, uint8_t symbol);
uint8_t ANSDecompress(struct ANSCtx *a);
#endif
