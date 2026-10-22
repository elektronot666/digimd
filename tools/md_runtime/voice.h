/* Word-aligned stream storage, following any engine-specific DSP state.
 * Keep these fields contiguous. Native stream assembly specializes only the
 * byte offset of blk44; there is no runtime layout lookup. */
#ifndef DIGIMD_VOICE_H
#define DIGIMD_VOICE_H
#include <stdint.h>
#define MD_STREAM_FIELDS \
    int32_t blk44[32]; \
    int32_t prev_s24; \
    uint32_t pos_int; \
    uint32_t pos_frac;
typedef struct {
    MD_STREAM_FIELDS
} MdStream;
#endif
