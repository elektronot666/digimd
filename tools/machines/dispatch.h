#ifndef DIGIMD_MACHINE_DISPATCH_H
#define DIGIMD_MACHINE_DISPATCH_H
#include <stdint.h>
/* This is the measured BD/SD bound, never a claim about future P-I state.
 * A new engine exceeding it needs an independently validated reservation. */
#define MD_SLOT_WORDS 80u
#define MD_SLOT_GUARD 0x4d444752u
#define MD_POOL_MAGIC 0x4d443439u
#define MD_NO_OWNER 0xffffffffu
typedef struct {
    uint32_t owner; /* native track << 8 | raw selector; MD_NO_OWNER if released */
    uint32_t fault;
    uint32_t guard_before;
    uint32_t state[MD_SLOT_WORDS];
    uint32_t guard_after;
} MdSlot;
typedef struct {
    uint32_t magic;
    uint32_t blocks_rendered;
    MdSlot slots[16];
} MdPool;
typedef void (*MdInit)(void *);
typedef void (*MdProcess)(void *, uint32_t, uint32_t, const int32_t *, float *);
typedef struct {
    uint32_t md_id;
    uint32_t state_bytes;
    MdInit init;
    MdProcess process;
} MdDescriptor;
#define MD_POOL_ADDR ((MdPool *)0x00300000u)
#endif
