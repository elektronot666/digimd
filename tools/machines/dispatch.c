#include "dispatch.h"
#include "descriptors.h"
/* Keep verified BD/SD record values: native downstream modulation observes
 * their distinct records even though both bypass stock oscillator kernels. */
const uint32_t md_stock_remap[7] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u };
extern void trxbd_build_sine_table(int32_t *);

static uint32_t md_frame(uint32_t offset) {
    uint32_t w;
    w = *((const uint32_t *)0x25c48cu + (offset >> 2));
    if ((offset & 2u) != 0u) { return w >> 16; }
    return w & 0xffffu;
}

static void md_clear(MdSlot *slot) {
    uint32_t k;
#if MD_HAS_DISPOSAL
    uint32_t previous;
    previous = slot->owner & 0xffu;
    if (slot->owner != MD_NO_OWNER && previous >= 5u && previous <= 53u &&
        slot->fault == 0u && slot->guard_before == MD_SLOT_GUARD && slot->guard_after == MD_SLOT_GUARD) {
        MdInit dispose;
        dispose = *(md_disposals + previous - 5u);
        if (dispose != 0) { dispose(slot->state); }
    }
#endif
    for (k = 0u; k < MD_SLOT_WORDS; k++) { *(slot->state + k) = 0u; }
    slot->owner = MD_NO_OWNER;
}

void md_generic_init(void) {
    MdPool *pool;
    uint32_t t;
    pool = MD_POOL_ADDR;
    pool->magic = 0u;
    trxbd_build_sine_table((int32_t *)0x80550000u);
    for (t = 0u; t < 16u; t++) {
        MdSlot *slot;
        slot = pool->slots + t;
        md_clear(slot);
        slot->fault = 0u;
        slot->guard_before = MD_SLOT_GUARD;
        slot->guard_after = MD_SLOT_GUARD;
    }
    pool->blocks_rendered = 0u;
    pool->magic = MD_POOL_MAGIC;
}

void md_generic_process(void) {
    MdPool *pool;
    uint32_t t;
    uint32_t triggers;
    pool = MD_POOL_ADDR;
    triggers = md_frame(34u);
    for (t = 0u; t < 16u; t++) {
        uint32_t selector;
        uint32_t track;
        uint32_t tag;
        uint32_t k;
        MdSlot *slot;
        const MdDescriptor *engine;
        float *out;
        selector = md_frame(148u + 2u * t);
        slot = pool->slots + t;
        if (selector < 5u) {
            if (pool->magic == MD_POOL_MAGIC && slot->owner != MD_NO_OWNER) {
                md_clear(slot);
            }
            continue;
        }
        out = (float *)(*((const uint32_t *)0x254a60u + t));
        track = md_frame(0xa5au + 2u * t);
        tag = (track << 8) | selector;
        engine = 0;
        if (selector <= 53u) { engine = md_descriptors + (selector - 5u); }
        if (pool->magic != MD_POOL_MAGIC || slot->fault != 0u ||
            slot->guard_before != MD_SLOT_GUARD || slot->guard_after != MD_SLOT_GUARD ||
            track >= 16u || engine == 0 || engine->state_bytes == 0u || engine->state_bytes > sizeof(slot->state) ||
            (engine->state_bytes & 3u) != 0u ||
            engine->init == 0 || engine->process == 0) {
            /* Faults remain silent until FXINIT. Unavailable identities may
             * later be replaced by a verified one without latching a fault. */
            if (pool->magic == MD_POOL_MAGIC) {
                if (slot->guard_before != MD_SLOT_GUARD || slot->guard_after != MD_SLOT_GUARD) {
                    slot->fault = 1u;
                }
                if (slot->owner != MD_NO_OWNER) { md_clear(slot); }
            }
            for (k = 0u; k < 32u; k++) { *(out + k) = 0.0f; }
            continue;
        }
        if (slot->owner != tag) {
#if MD_HAS_DISPOSAL
            md_clear(slot);
#endif
            /* Proven initializers overwrite their entire declared state.
             * Clear only unused tail bytes when binding a smaller engine. */
            for (k = engine->state_bytes >> 2; k < MD_SLOT_WORDS; k++) {
                *(slot->state + k) = 0u;
            }
            engine->init(slot->state);
            slot->owner = tag;
        }
        /* One indirect block callback, never an indirect sample loop. */
        engine->process(slot->state, t, (triggers >> t) & 1u,
                        (const int32_t *)0x80550000u, out);
    }
    if (pool->magic == MD_POOL_MAGIC) { pool->blocks_rendered = pool->blocks_rendered + 1u; }
}
