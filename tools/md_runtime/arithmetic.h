/* DSP56300 word/product/accumulator subset proven by the OS 1.63 DSP2
 * sine generator and TRX-BD kernel. Inputs and domains: see CONTRACT.md.
 * Keep pointer-result ABI and truncation points. No struct-return products. */
#ifndef DIGIMD_ARITHMETIC_H
#define DIGIMD_ARITHMETIC_H
#include <stdint.h>
static int32_t sx24(int32_t w) {
    return ((int32_t)((uint32_t)w << 8)) >> 8;
}

static uint32_t u24(int32_t w) {
    return (uint32_t)w & 0xffffffu;
}

#ifdef MD_DSP_HOST_TEST
static int32_t lim24(int32_t ah) {
    if (ah > 0x7fffff) {
        return 0x7fffff;
    }
    if (ah < -0x800000) {
        return -0x800000;
    }
    return ah;
}

#else
extern int32_t lim24(int32_t ah);
#endif

/* The original MD signed24-by-signed24 full product is:
 *   high = floor((s1*s2)/2^23)
 *   low  = (2*s1*s2) & 0xFFFFFF
 * The SHARC mpy_ss_hi leaf already computes the first expression bit-exactly,
 * including all negative products, using the proven direct SSF+SSI path.
 *
 * The second expression depends only on the lowest 23 bits of the product:
 * a uint32_t multiply is EXACT modulo 2^32, including negative operands
 * reinterpreted in two's complement. The subsequent unsigned shift/mask
 * extracts the original lower 24 doubled-product bits.
 *
 * Retain the ORIGINAL four-argument void/pointer C ABI. The previous
 * 2-register tiny-struct experiment (on a separate FAILED branch) passed
 * direct native helper tests but corrupted the sine table at word 4,
 * indicating that direct ABI tests alone cannot validate selcc call sites.
 * Never silently revive that struct-return path. */
#ifdef MD_DSP_HOST_TEST
static int32_t mpy_ss_hi(int32_t s1, int32_t s2);
#else
extern int32_t mpy_ss_hi(int32_t s1, int32_t s2);
#endif
#ifdef MD_DSP_HOST_TEST
static void mpy_ss32(int32_t s1, int32_t s2, int32_t *out_hi, uint32_t *out_lo) {
    uint32_t product_low;
    product_low = (uint32_t)s1 * (uint32_t)s2;
    *out_hi = mpy_ss_hi(s1, s2);
    *out_lo = (product_low << 1) & 0xffffffu;
}
#else
/* Same four-argument pointer ABI as the original C helper. The native leaf
 * writes both outputs directly and avoids a nested call to mpy_ss_hi. */
extern void mpy_ss32(int32_t s1, int32_t s2, int32_t *out_hi, uint32_t *out_lo);
#endif

#ifdef MD_DSP_HOST_TEST
static int32_t mpy_ss_hi(int32_t s1, int32_t s2) {
    int32_t h1;
    uint32_t l1;
    int32_t h2;
    uint32_t l2;
    uint32_t ll;
    int32_t mid;
    /* Also valid for a signed 25-bit second operand: the amplitude stage
     * combines its two products before the one original truncation point. */
    h1 = s1 >> 12;
    l1 = (uint32_t)s1 & 0xfffu;
    h2 = s2 >> 12;
    l2 = (uint32_t)s2 & 0xfffu;
    ll = l1 * l2;
    mid = h1 * (int32_t)l2 + h2 * (int32_t)l1 + (int32_t)(ll >> 12);
    return (h1 * h2 * 2) + (mid >> 11);
}

#else
extern int32_t mpy_ss_hi(int32_t s1, int32_t s2);
#endif

static void mpy_su32(int32_t s1, uint32_t u2, int32_t *out_hi, uint32_t *out_lo) {
    int32_t h1;
    uint32_t l1;
    int32_t h2;
    uint32_t l2;
    uint32_t ll;
    int32_t mid;
    uint32_t lo;
    int32_t hi;
    s1 = sx24(s1);
    u2 = u2 & 0xffffffu;
    h1 = s1 >> 12;
    l1 = (uint32_t)s1 & 0xfffu;
    h2 = (int32_t)(u2 >> 12);
    l2 = u2 & 0xfffu;
    ll = l1 * l2;
    mid = h1 * (int32_t)l2 + h2 * (int32_t)l1 + (int32_t)(ll >> 12);
    lo = (((uint32_t)(mid & 0x7ff) << 13) | ((ll & 0xfffu) << 1)) & 0xffffffu;
    hi = (h1 * h2 * 2) + (mid >> 11);
    *out_hi = hi;
    *out_lo = lo;
}

static void mpy_uu32(uint32_t u1, uint32_t u2, int32_t *out_hi, uint32_t *out_lo) {
    int32_t h1;
    uint32_t l1;
    int32_t h2;
    uint32_t l2;
    uint32_t ll;
    int32_t mid;
    uint32_t lo;
    int32_t hi;
    u1 = u1 & 0xffffffu;
    u2 = u2 & 0xffffffu;
    h1 = (int32_t)(u1 >> 12);
    l1 = u1 & 0xfffu;
    h2 = (int32_t)(u2 >> 12);
    l2 = u2 & 0xfffu;
    ll = l1 * l2;
    mid = h1 * (int32_t)l2 + h2 * (int32_t)l1 + (int32_t)(ll >> 12);
    lo = (((uint32_t)(mid & 0x7ff) << 13) | ((ll & 0xfffu) << 1)) & 0xffffffu;
    hi = (h1 * h2 * 2) + (mid >> 11);
    *out_hi = hi;
    *out_lo = lo;
}

static void add56(int32_t ah, uint32_t al, int32_t bh, uint32_t bl, int32_t *oh, uint32_t *ol) {
    uint32_t sl;
    int32_t sh;
    sl = al + bl;
    sh = (int32_t)((uint32_t)ah + (uint32_t)bh + (sl >> 24));
    *oh = sh;
    *ol = sl & 0xffffffu;
}

static void sub56(int32_t ah, uint32_t al, int32_t bh, uint32_t bl, int32_t *oh, uint32_t *ol) {
    int32_t dl;
    int32_t dh;
    dl = (int32_t)al - (int32_t)bl;
    dh = (int32_t)((uint32_t)ah - (uint32_t)bh);
    if (dl < 0) {
        dl = dl + 0x1000000;
        dh = (int32_t)((uint32_t)dh - 1u);
    }
    *oh = dh;
    *ol = (uint32_t)dl;
}

static void asr56(int32_t ah, uint32_t al, uint32_t n, int32_t *oh, uint32_t *ol) {
    uint32_t mask;
    uint32_t carry;
    /* Full signed56 shift domain 0..55. The hot original cases 1/6/7
     * retain their original path; avoid C shifts by the operand width. */
    if (n == 0u) {
        *oh = ah;
        *ol = al;
        return;
    }
    if (n >= 24u) {
        *ol = (uint32_t)(ah >> (n - 24u)) & 0xffffffu;
        if (n >= 32u) {
            *oh = ah >> 31;
        } else {
            *oh = ah >> n;
        }
        return;
    }
    mask = (1u << n) - 1u;
    carry = (uint32_t)ah & mask;
    *oh = ah >> n;
    *ol = ((al >> n) | (carry << (24u - n))) & 0xffffffu;
}


#endif
