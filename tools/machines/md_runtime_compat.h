/* Arithmetic/stream bindings shared by statically generated engines. */
#ifndef DIGIMD_GENERATED_RUNTIME_H
#define DIGIMD_GENERATED_RUNTIME_H
#include "../md_runtime/voice.h"
#include "../md_runtime/arithmetic.h"
/* These pure word conversions are expressions so selcc need not emit a call
 * for each original bus transfer. Each argument is evaluated exactly once. */
#define sx24(w) (((int32_t)((uint32_t)(w) << 8)) >> 8)
#define u24(w) ((uint32_t)(w) & 0xffffffu)
#ifndef MD_TRANSLATION_TRACE
#define MD_TRANSLATION_TRACE(pc,ah,al,bh,bl,x0,x1,y0,y1,r2,r4,r7)
#endif
#endif
