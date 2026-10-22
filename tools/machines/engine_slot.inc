/* Typed block adapter. All synthesis and sample-loop calls remain direct. */
void MD_SLOT_INIT(void *memory) {
    MD_VOICE_TYPE *v;
    v = (MD_VOICE_TYPE *)memory;
    MD_VOICE_INIT(v);
}

extern const uint32_t md_param_offsets[8];
void MD_SLOT_PROCESS(void *memory, uint32_t slot, uint32_t trig,
                     const int32_t *sine, float *out) {
    MD_VOICE_TYPE *v;
    uint32_t changed;
    uint32_t k;
    uint32_t p14[8];
    v = (MD_VOICE_TYPE *)memory;
    changed = 0u;
    for (k = 0u; k < 8u; k++) {
        uint32_t offset;
        uint32_t word;
        uint32_t raw;
        offset = *(md_param_offsets + k) + 146u * slot;
        word = *((const uint32_t *)0x25c48cu + (offset >> 2));
        if ((offset & 2u) != 0u) { raw = word >> 16; }
        else { raw = word & 0xffffu; }
        if (raw > 0x7f00u) { raw = 0x7f00u; }
        *(p14 + k) = raw >> 1;
        if (*(v->last_p + k) != *(p14 + k)) { changed = 1u; }
    }
    if (changed != 0u || trig != 0u) {
        for (k = 0u; k < 8u; k++) { *(v->last_p + k) = *(p14 + k); }
#ifdef MD_CONTROL_WITH_TRIGGER
        MD_CONTROL_WITH_TRIGGER(p14, v, trig);
#else
        MD_COEFFICIENTS(p14, v->y);
#endif
    }
    if (trig != 0u) {
        MD_VOICE_TRIGGER(v);
        MD_RENDER44(v, sine);
        v->prev_s24 = *(v->blk44 + 0);
        v->pos_int = 0u;
        v->pos_frac = 0u;
    }
    MD_RENDER48(v, sine, out);
}
