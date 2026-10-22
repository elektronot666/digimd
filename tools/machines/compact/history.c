/* Original MD tracks retain the entire 64-word Y slot when changing machines.
 * Native stream layouts differ, so preserve that bank outside the voice union.
 * The fixed bank lies inside the older fallback's 16 KiB L1 reservation.
 * Original per-machine init writes are applied, never a blanket Y reset. */
#include <stdint.h>
#define HISTORY ((uint32_t *)0x302000u)
extern const uint32_t md_history_words[49];
void md_history_reset(void) {
    uint32_t i;
    for(i=0u;i<1024u;i++) HISTORY[i]=0u;
}
void md_history_restore(uint32_t *state,uint32_t track,uint32_t selector) {
    uint32_t i,n;
    n=md_history_words[selector-5u];
    for(i=0u;i<n;i++)state[i]=HISTORY[64u*track+i];
    if(selector==5u) {state[28]=0u;state[29]=1u;}
    if(selector==6u) {
        state[23]=0x29au;state[24]=1u;
        state[25]=0u;state[26]=0u;state[27]=0u;state[28]=0u;
    }
}
void md_history_save(const uint32_t *state,uint32_t track,uint32_t selector) {
    uint32_t i,n;
    n=md_history_words[selector-5u];
    for(i=0u;i<n;i++)HISTORY[64u*track+i]=state[i];
}
