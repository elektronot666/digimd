/* Original DSP2 boot generator P:$100068..$10008D; MD_BUILD_SINE names the exported entry. */
void MD_BUILD_SINE(int32_t *sine) {
    int32_t hi[2];
    uint32_t lo[2];
    int32_t c_hi;
    uint32_t c_lo_u;
    uint32_t r1;
    uint32_t k;
    *(sine + 0) = 0;
    *(sine + 1) = 0x000648;
    hi[0] = 0;
    hi[1] = 0x000648;
    lo[0] = 0u;
    lo[1] = 0x7ed46bu;
    c_hi = 0x7fffff;
    c_lo_u = 0xd88586u;
    r1 = 1u;
    for (k = 2u; k < 32768u; k++) {
        int32_t y1;
        uint32_t y0;
        int32_t ah;
        uint32_t al;
        int32_t ph;
        uint32_t pl;
        int32_t prev_h;
        uint32_t prev_l;
        y1 = *(hi + r1);
        y0 = *(lo + r1);
        r1 = r1 ^ 1u;
        mpy_uu32(c_lo_u, y0, &ah, &al);
        mpy_su32(c_hi, y0, &ph, &pl);
        add56(ah >> 24, (uint32_t)ah & 0xffffffu, ph, pl, &ah, &al);
        mpy_su32(y1, c_lo_u, &ph, &pl);
        add56(ah, al, ph, pl, &ah, &al);
        mpy_ss32(c_hi, y1, &ph, &pl);
        add56(ah >> 24, (uint32_t)ah & 0xffffffu, ph, pl, &ah, &al);
        add56(ah, al, ah, al, &ah, &al);
        prev_h = *(hi + r1);
        prev_l = *(lo + r1);
        sub56(ah, al, prev_h, prev_l, &ah, &al);
        if (ah > 0x7fffff) {
            *(hi + r1) = 0x7fffff;
            *(lo + r1) = 0xffffffu;
        } else if (ah < -0x800000) {
            *(hi + r1) = -0x800000;
            *(lo + r1) = 0u;
        } else {
            *(hi + r1) = ah;
            *(lo + r1) = al;
        }
        *(sine + k) = lim24(ah);
    }
}
