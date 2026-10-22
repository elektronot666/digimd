/* Full49 logical-track ownership and original Y/X/delay retention.
 * Candidate DDR regions require separate live-memory qualification.
 *
 * Voice routing (2026-10-19). The stock frame keeps each physical voice's last
 * machine selector and track owner (0xa5a) after the voice is released, and the
 * stock allocator may give a track's next note to a different voice. Two frame
 * voices therefore routinely name the same MD track. The renderer now picks
 * exactly one voice per track (the triggered one, else the voice it rendered
 * last) and silences the stale ones. Frame contents never latch a slot fault;
 * only damaged guards do, and a trigger re-arms a damaged slot.
 *
 * Load policy. Translated engines cost up to ~0.8 M instructions per 32-sample
 * block, more than a whole block period on their own. Overrunning the period
 * corrupts every voice on the unit. Two policies keep the DSP inside its
 * deadline; both are bit-exact no-ops while they are not engaged and both can
 * be switched off through MD_ROUTE->policy for original-parity measurement:
 *   bit 0  governor: the block period is averaged from EMUCLK at start-up
 *          (no MD voice is active yet). Each later call interval longer than
 *          the period adds to an estimate of how late the Audio Task runs;
 *          the pacing gate also counts skipped SPORT completions. A skip, or
 *          lateness once a skip has confirmed the EMUCLK model, cuts the MD
 *          cycle budget, which then recovers slowly.
 *          The budget never exceeds what the stock handler leaves: the pacing
 *          gate stamps EMUCLK when the Audio Task starts a block and a hook at
 *          the handler's return stamps its end, so the stock (non-MD) time
 *          of the block is measured on the unit; MD may use the period minus
 *          that time (peak-held) minus a 1/16-period margin.
 *          A track whose last measured cost does not fit is muted for that
 *          block (trigger blocks and other blocks keep separate costs; an
 *          unknown trigger cost counts at least period/8; a machine just
 *          selected uses that machine's last costs, period/8 if it has not
 *          run since boot)
 *          instead of overrunning the deadline for every voice. Lower tracks
 *          are served first, so the highest-numbered tracks go quiet first.
 *   bit 1  dormancy: a track whose dry output stays below -96 dBFS for 96
 *          blocks is not rendered until its next trigger or machine change.
 *   bit 2  clean machine change: a machine selected on a track (live, or by
 *          a sound lock between steps) starts from the state it has when first
 *          selected after boot, instead of inheriting the previous machine's
 *          DSP memory (the original per-track Y/X/delay retention, kept when
 *          the policy is off). The same applies when an MD machine returns to
 *          a track that a stock machine took over.
 *   bit 3  pairing (SIMD, two voices per instruction stream): two tracks next
 *          to each other in render order (no MD track rendering between
 *          them) with the same machine, neither triggered nor changing
 *          machine this block, render their 44.1 kHz blocks with the
 *          machine's pair render. Bit-exact with rendering them one after
 *          the other (fast_pair_gate; whole-handler parity in the fast
 *          qualification). A machine whose measured pair cost per voice does
 *          not beat its single-render cost is not paired for 512 blocks.
 *          The admission estimates stay those of single renders.
 */
#include "dispatch.h"
#include "descriptors.h"
extern void md_runtime_reset(void);
extern void trxbd_build_sine_table(int32_t *);
extern uint32_t md_cycles(void);
const uint32_t md_stock_remap[7]={0u,1u,2u,3u,4u,5u,6u};
#define HISTORY_MAGIC 0x4d444859u
#define HISTORY_GUARD 0x4d445947u
typedef struct {
    uint32_t before;
    uint32_t y[64];
    uint32_t private_words[1600];
    uint32_t after;
} MdTrackHistory;
typedef struct {uint32_t magic,version;MdTrackHistory tracks[16];} MdTrackHistoryPool;
#define HISTORY ((MdTrackHistoryPool *)0x81820000u)
#define ROUTE_MAGIC 0x4d445254u
#define MD_NONE 16u
#define POLICY_GOVERNOR 1u
#define POLICY_DORMANCY 2u
#define POLICY_CLEAN 4u
#ifdef MD_HAS_PAIRING
#define POLICY_PAIR 8u
#else
#define POLICY_PAIR 0u
#endif
#define QUIET_BLOCKS 96u
#define QUIET_LEVEL (1.0f/65536.0f)
#define GOV_SETTLE 16u
#define GOV_CALIBRATION 80u
#define GOV_HOLD 3000u
#define MD_ROUTE_MISSES (*(volatile uint32_t *)0x8181d3f0u)
/* Written by audio_pacing.s: EMUCLK when the Audio Task starts rendering a
 * block, and when the block handler has returned (0x8181d3fc is the hooks'
 * register save word). Zero until the first block. */
#define MD_ROUTE_START (*(volatile uint32_t *)0x8181d3f4u)
#define MD_ROUTE_END (*(volatile uint32_t *)0x8181d3f8u)
#define MD_SELECTORS 49u
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t policy;
    uint32_t voice[16];      /* physical voice rendering each logical track */
    uint32_t quiet[16];      /* consecutive inaudible blocks */
    uint32_t cost[16];       /* last measured EMUCLK cycles of the track */
    uint32_t muted[16];      /* blocks muted by the governor */
    uint32_t dormant;        /* track bits not rendered until a trigger */
    uint32_t stale;          /* track bits whose last trigger used a stock machine */
    uint32_t last;           /* EMUCLK at the previous call */
    uint32_t period;         /* minimum call interval while calibrating */
    uint32_t intervals;
    uint32_t budget;         /* current MD cycle budget per block */
    uint32_t late_blocks;    /* blocks in which a new deadline miss was seen */
    uint32_t refused;        /* stale or duplicate voices silenced */
    uint32_t rearmed;        /* damaged slots re-armed by a trigger */
    uint32_t used;           /* MD cycles used in the last block */
    uint32_t owner[16];      /* scratch: track named by each MD voice */
    uint32_t chosen[16];     /* scratch: voice chosen for each track */
    uint32_t misses_seen;    /* gate miss count already acted on */
    uint32_t hold;           /* blocks before the budget may grow again */
    uint32_t first;          /* EMUCLK when period averaging started */
    int32_t lateness;        /* estimated Audio Task lateness, cycles */
    uint32_t lo,hi;          /* interval range while calibrating */
    uint32_t calibrations;   /* calibration windows started */
    uint32_t trust;          /* lateness model: 1 trusted, 2 refuted */
    uint32_t overdue;        /* blocks since lateness passed a period without a skip */
    uint32_t exit;           /* EMUCLK when the MD dispatcher last returned */
    uint32_t stock;          /* peak-held stock (non-MD) handler cycles per block */
    uint32_t stock_last;     /* stock cycles measured for the last block */
    uint32_t stock_blocks;   /* blocks with a valid stock measurement */
    uint32_t reserve_cap;    /* last MD cap from the stock reserve (0: none) */
    uint32_t machine_cost[MD_SELECTORS]; /* last measured cost per machine (non-trigger block) */
    uint32_t trig_cost[16];  /* last measured cost of the track in a trigger block */
    uint32_t machine_trig[MD_SELECTORS]; /* same per machine */
    uint32_t entry;          /* EMUCLK when the MD dispatcher was entered */
    uint32_t overhead;       /* dispatcher cycles outside engine renders, last block */
    uint32_t fresh;          /* track bits a stock machine took over: next MD machine starts clean */
    uint32_t hzero;          /* track bits whose history is all zero since a clean change */
} MdRoute;
#define MD_ROUTE ((MdRoute *)0x8181d000u)
/* Voice stealing and the learned margin (second 1 KB of the route region; the
 * first one ends with the pacing words at 0x8181d3f0..0x8181d3ff). */
#define ROUTE2_MAGIC 0x4d445232u
#define GOV_MAX_DEFER 8u
typedef struct {
    uint32_t magic;
    uint32_t blocks;         /* dispatcher calls since reset */
    uint32_t trig_at[16];    /* block of the track's last note start (or pending trigger) */
    uint32_t stolen;         /* track bits cut for newer notes: silent until their next trigger */
    uint32_t pending;        /* triggered track bits waiting for budget (at most GOV_MAX_DEFER blocks) */
    uint32_t margin;         /* safety margin learned from deadline misses, cycles */
    uint32_t change_cost;    /* peak cycles of one machine change */
    uint32_t admitted;       /* track bits admitted in the last block */
    uint32_t steals;         /* sounding tracks cut for a newer note */
    uint32_t deferrals;      /* trigger blocks delayed for budget */
    uint32_t dropped;        /* triggers given up after GOV_MAX_DEFER blocks */
    uint32_t tears;          /* blocks that started without a wait: the previous one overran */
    uint32_t need[16];       /* scratch: estimated cycles of each candidate */
    uint32_t refractory;     /* blocks before the next miss may grow the margin again */
    uint32_t pairs;          /* pair renders */
    uint32_t pair_declined;  /* pair renders that returned 1 (rendered singly instead) */
    uint32_t pair_cost[MD_SELECTORS]; /* cycles per voice of the machine's pair renders, average */
    uint32_t pair_retry[MD_SELECTORS];/* block from which a machine that did not pay is tried again */
    uint32_t render_cost[MD_SELECTORS];/* cycles of the machine's single 44.1 kHz renders, average */
    uint32_t pair_n[MD_SELECTORS];    /* renders in the averages since cleared: pairs (low half), singles (high) */
} MdRoute2;
#define MD_ROUTE2 ((MdRoute2 *)0x8181d400u)
/* Generated from the same measured compiler layouts, not hand-copied data. */
@LAYOUT_CONSTANTS@

static uint32_t md_frame(uint32_t offset) {
    uint32_t w;
    w=*((const uint32_t *)0x25c48cu+(offset>>2));
    return (offset&2u)?w>>16:w&0xffffu;
}
static uint32_t md_has_private(uint32_t selector) {
    uint32_t index;
    index=selector-5u;
    if(index<32u)return (MD_PRIVATE_LOW>>index)&1u;
    return (MD_PRIVATE_HIGH>>(index-32u))&1u;
}
static uint32_t md_history_valid(uint32_t track) {
    MdTrackHistory *h;
    if(track>=16u||HISTORY->magic!=HISTORY_MAGIC||HISTORY->version!=1u)return 0u;
    h=HISTORY->tracks+track;
    return h->before==HISTORY_GUARD&&h->after==HISTORY_GUARD;
}
static void md_history_track_reset(uint32_t t) {
    uint32_t k;
    MdTrackHistory *h;
    h=HISTORY->tracks+t;
    for(k=0u;k<64u;k++)h->y[k]=0u;
    for(k=0u;k<1600u;k++)h->private_words[k]=0u;
    h->before=HISTORY_GUARD;h->after=HISTORY_GUARD;
}
static void md_history_reset(void) {
    uint32_t t;
    HISTORY->magic=0u;HISTORY->version=1u;
    for(t=0u;t<16u;t++)md_history_track_reset(t);
    HISTORY->magic=HISTORY_MAGIC;
}
static uint32_t md_history_save(const uint32_t *state,uint32_t track,uint32_t selector) {
    uint32_t i,n,offset;
    MdTrackHistory *h;
    if(selector<5u||selector>53u||!md_history_valid(track))return 0u;
    h=HISTORY->tracks+track;n=md_history_words[selector-5u];
    for(i=0u;i<n;i++)h->y[i]=state[i];
    if(md_has_private(selector)) {
        offset=n+45u;
        for(i=0u;i<1600u;i++)h->private_words[i]=state[offset+i];
    }
    return 1u;
}
static uint32_t md_history_restore(uint32_t *state,uint32_t track,uint32_t selector) {
    uint32_t i,n,offset;
    MdTrackHistory *h;
    if(selector<5u||selector>53u||!md_history_valid(track))return 0u;
    h=HISTORY->tracks+track;n=md_history_words[selector-5u];
    for(i=0u;i<n;i++)state[i]=h->y[i];
    if(md_has_private(selector)) {
        offset=n+45u;
        for(i=0u;i<1600u;i++)state[offset+i]=h->private_words[i];
        state[offset+1600u]=track;
    }
    /* These two optimized legacy initializers otherwise reset the Y bank.
     * Reapply exactly their original DSP initialization stores after restore. */
    if(selector==5u) {state[28]=0u;state[29]=1u;}
    if(selector==6u) {
        state[23]=0x29au;state[24]=1u;
        state[25]=0u;state[26]=0u;state[27]=0u;state[28]=0u;
    }
    return 1u;
}
static void md_clear(MdSlot *slot) {
    uint32_t k;
    for(k=0u;k<MD_SLOT_WORDS;k++)slot->state[k]=0u;
    slot->owner=MD_NO_OWNER;
}
static void md_silence(float *out) {
    uint32_t k;
    for(k=0u;k<32u;k++)out[k]=0.0f;
}
static float *md_out(uint32_t t) {
    return (float *)(*((const uint32_t *)0x254a60u+t));
}
static void md_route_reset(void) {
    MdRoute *r;
    uint32_t t;
    r=MD_ROUTE;r->magic=0u;r->version=2u;r->policy=POLICY_GOVERNOR|POLICY_DORMANCY|POLICY_CLEAN|POLICY_PAIR;
    for(t=0u;t<16u;t++) {r->voice[t]=MD_NONE;r->quiet[t]=0u;r->cost[t]=0u;r->muted[t]=0u;}
    r->dormant=0u;r->stale=0u;r->last=0u;r->period=0xffffffffu;r->intervals=0u;
    r->budget=0u;r->late_blocks=0u;r->refused=0u;r->rearmed=0u;r->used=0u;
    r->misses_seen=0u;r->hold=0u;r->first=0u;r->lateness=0;r->lo=0xffffffffu;r->hi=0u;r->calibrations=0u;r->trust=1u;r->overdue=0u;MD_ROUTE_MISSES=0u;
    r->exit=0u;r->stock=0u;r->stock_last=0u;r->stock_blocks=0u;r->reserve_cap=0u;r->entry=0u;r->overhead=0u;r->fresh=0u;r->hzero=0u;
    for(t=0u;t<MD_SELECTORS;t++) {r->machine_cost[t]=0u;r->machine_trig[t]=0u;}
    for(t=0u;t<16u;t++)r->trig_cost[t]=0u;
    MD_ROUTE_START=0u;MD_ROUTE_END=0u;
    {
        MdRoute2 *x;
        x=MD_ROUTE2;x->magic=0u;x->blocks=0u;x->stolen=0u;x->pending=0u;x->margin=0u;x->change_cost=0u;
        x->admitted=0u;x->steals=0u;x->deferrals=0u;x->dropped=0u;x->tears=0u;x->refractory=0u;
        x->pairs=0u;x->pair_declined=0u;
        for(t=0u;t<MD_SELECTORS;t++) {x->pair_cost[t]=0u;x->pair_retry[t]=0u;x->render_cost[t]=0u;x->pair_n[t]=0u;}
        for(t=0u;t<16u;t++) {x->trig_at[t]=0u;x->need[t]=0u;}
        x->magic=ROUTE2_MAGIC;
    }
    r->magic=ROUTE_MAGIC;
}
void md_generic_init(void) {
    MdPool *pool;
    MdSlot *slot;
    uint32_t t;
    pool=MD_POOL_ADDR;pool->magic=0u;
    md_runtime_reset();md_history_reset();md_route_reset();trxbd_build_sine_table((int32_t *)0x80550000u);
    for(t=0u;t<16u;t++) {
        slot=pool->slots+t;md_clear(slot);slot->fault=0u;
        slot->guard_before=MD_SLOT_GUARD;slot->guard_after=MD_SLOT_GUARD;
    }
    pool->blocks_rendered=0u;pool->magic=MD_POOL_MAGIC;
}
/* Average the block period at start-up (no MD voice is active yet), then
 * adapt the MD budget to measured lateness and to deadline misses counted by
 * the pacing gate. Returns 1 when the budget is enforced for this block. */
/* Stock (non-MD) cycles of the handler: this block's start to the MD
 * dispatcher, plus the previous block's MD return to its handler end and
 * the previous block's dispatcher time outside engine renders. Kept
 * as a peak that decays by period/4096 per block.
 * Single exit on purpose: selcc inlines this function and compiled its early
 * returns as returns from the caller (md_governor_begin), with R0 garbage. */
/* Non-static on purpose (selcc does not inline it). */
void md_stock_measure(MdRoute *r,uint32_t now) {
    uint32_t start,end,pre,post,stock,decay;
    start=MD_ROUTE_START;end=MD_ROUTE_END;
    if(start!=0u&&end!=0u&&r->exit!=0u&&r->period>=1000u&&r->period<=0x10000000u) {
        pre=now-start;post=end-r->exit;
        if(pre<(r->period<<1)&&post<(r->period<<1)) {
            stock=pre+post+r->overhead;r->stock_last=stock;r->stock_blocks=r->stock_blocks+1u;
            decay=r->period>>12;
            if(stock>=r->stock)r->stock=stock;
            else if(r->stock-stock>decay)r->stock=r->stock-decay;
            else r->stock=stock;
        }
    }
}
/* Non-static on purpose: selcc inlines single-caller static functions and
 * miscompiles their early returns (see md_stock_measure). */
uint32_t md_governor_begin(MdRoute *r) {
    uint32_t now,interval,cap,floor,misses,base,margin,tear,start,end;
    MdRoute2 *x;
    int32_t late;
    now=md_cycles();r->entry=now;
    md_stock_measure(r,now);
    if(r->intervals<GOV_CALIBRATION) {
        /* Accept only a window of steady intervals (within 1/32): a window
         * with overrunning blocks is discarded and measured again. */
        interval=now-r->last;
        if(r->intervals==GOV_SETTLE) {r->first=now;r->lo=0xffffffffu;r->hi=0u;r->calibrations=r->calibrations+1u;}
        else if(r->intervals>GOV_SETTLE) {
            if(interval<r->lo)r->lo=interval;
            if(interval>r->hi)r->hi=interval;
        }
        r->intervals=r->intervals+1u;
        if(r->intervals==GOV_CALIBRATION) {
            r->period=(now-r->first)/(GOV_CALIBRATION-1u-GOV_SETTLE);
            if(r->hi-r->lo>(r->period>>5)) {r->intervals=GOV_SETTLE;r->period=0xffffffffu;}
        }
        r->last=now;r->misses_seen=MD_ROUTE_MISSES;
        return 0u;
    }
    interval=now-r->last;r->last=now;
    if(r->period<1000u||r->period>0x10000000u) {r->intervals=GOV_SETTLE;return 0u;}
    /* Lateness grows by each interval beyond the period; a small leak keeps
     * clock drift and pre-MD jitter from accumulating. Waits clamp it at 0. */
    late=r->lateness+(int32_t)(interval-r->period)-(int32_t)(r->period>>11);
    if(late<0)late=0;
    if(late>(int32_t)(r->period<<2))late=(int32_t)(r->period<<2);
    r->lateness=late;
    /* EMUCLK counts through the idle task on the unit (dn2-cfw prior art
     * measures DSP load from idle-task EMUCLK deltas at 1 GHz). Still
     * cross-check the lateness model against real skipped completions: a skip
     * must find the model at least half a period late, and a model a full
     * period late must be followed by a skip. Otherwise only skips act. */
    misses=MD_ROUTE_MISSES;
    /* An overrun that ends before the following completion is late enough to
     * tear the audio but skips no completion. It shows in the stamps: the
     * Audio Task found that completion already pending and did not sleep,
     * so this block started right after the previous handler ended. */
    start=MD_ROUTE_START;end=MD_ROUTE_END;
    tear=start!=0u&&end!=0u&&start-end<(r->period>>8);
    if(tear)MD_ROUTE2->tears=MD_ROUTE2->tears+1u;
    if(misses!=r->misses_seen) {
        if(late<(int32_t)(r->period>>1))r->trust=2u;
        r->overdue=0u;
    } else if(late>=(int32_t)r->period) {
        r->overdue=r->overdue+1u;
        if(r->overdue>64u)r->trust=2u;
    } else r->overdue=0u;
    if((r->policy&POLICY_GOVERNOR)==0u)return 0u;
    cap=r->period-(r->period>>2);floor=r->period>>6;
    /* Deadline misses (skipped completions, torn blocks, the lateness model)
     * teach the margin: it starts at period/16 and grows by period/16 per
     * event, at most once per 8 blocks, up to period/2 -- it covers wake
     * latency and jitter that the stamps cannot see. After 2 s without an
     * event it shrinks by period/65536 per block (about 11 s back). With
     * the stamps the budget is the cap: period - margin - stock. Without
     * them (no stamps yet) the earlier rule applies: a miss cuts the budget
     * to 3/4 of what MD used, then it recovers slowly. */
    x=MD_ROUTE2;
    if(x->margin<(r->period>>4))x->margin=r->period>>4;
    if(r->budget==0u||r->budget>cap)r->budget=cap;
    /* with the stamps, only direct evidence counts (a skipped completion or
     * a torn block); the lateness model is the fallback without them */
    if(misses!=r->misses_seen||tear||(r->stock_blocks==0u&&r->trust==1u&&late>(int32_t)(r->period>>4)&&interval>r->period)) {
        r->misses_seen=misses;r->late_blocks=r->late_blocks+1u;r->hold=GOV_HOLD;
        if(x->refractory==0u) {
            x->margin=x->margin+(r->period>>4);
            if(x->margin>(r->period>>1))x->margin=r->period>>1;
            x->refractory=8u;
        }
        if(r->stock_blocks==0u) {
            base=r->used<r->budget?r->used:r->budget;   /* MD cycles of the late block */
            r->budget=base-(base>>2);
            if(r->budget<floor)r->budget=floor;
        }
    } else if(r->hold!=0u) r->hold=r->hold-1u;
    else {
        if(x->margin>(r->period>>4)+(r->period>>16))x->margin=x->margin-(r->period>>16);
        if(r->stock_blocks==0u) {
            r->budget=r->budget+(r->period>>11);
            if(r->budget>cap)r->budget=cap;
        }
    }
    if(x->refractory!=0u)x->refractory=x->refractory-1u;
    r->reserve_cap=0u;
    if(r->stock_blocks!=0u) {
        /* MD may use only what the measured stock work leaves, less the margin */
        margin=x->margin;
        if(r->stock+margin>=r->period)cap=0u;
        else if(r->period-margin-r->stock<cap)cap=r->period-margin-r->stock;
        r->reserve_cap=cap;
        r->budget=cap;
    }
    if(r->budget>cap)r->budget=cap;
    return 1u;
}
/* Estimated cycles of a track's render: trigger blocks and other blocks keep
 * separate last costs (a trigger-block spike must not lock out the steady
 * sound); the other kind stands in until measured; an unknown trigger cost
 * counts at least period/8, a machine not measured since boot period/8.
 * Single exit (see md_stock_measure). */
uint32_t md_need(const MdRoute *r,uint32_t track,uint32_t trig) {
    uint32_t need;
    if(trig) {
        need=r->trig_cost[track];
        if(need==0u)need=r->cost[track]>(r->period>>3)?r->cost[track]:r->period>>3;
    } else {
        need=r->cost[track];
        if(need==0u)need=r->trig_cost[track];
        if(need==0u)need=r->period>>3;
    }
    return need;
}
/* After a track's render: its costs and dormancy. spent: its cycles, or with
 * paired its share of a pair render. Non-static (see md_stock_measure). */
void md_after_render(MdRoute *r,uint32_t track,uint32_t selector,uint32_t trig,uint32_t spent,const float *out,uint32_t paired) {
    uint32_t k,m;
    float peak,x;
    m=selector-5u;
    if(paired) {
        /* the admission estimate stays a single render's */
        if(m<MD_SELECTORS&&r->machine_cost[m]!=0u)r->cost[track]=r->machine_cost[m];
        else r->cost[track]=spent<<1;
    } else if(trig) {
        r->trig_cost[track]=spent;
        if(m<MD_SELECTORS)r->machine_trig[m]=spent;
        /* a steady cost measured while the voice was silent says nothing
         * about the sound this trigger starts */
        if(r->cost[track]<(spent>>1))r->cost[track]=spent;
    } else {
        r->cost[track]=spent;
        if(m<MD_SELECTORS)r->machine_cost[m]=spent;
    }
    if((r->policy&POLICY_DORMANCY)!=0u) {
        peak=0.0f;
        for(k=0u;k<32u;k++) {x=out[k];if(x<0.0f)x=-x;if(x>peak)peak=x;}
        if(peak<QUIET_LEVEL) {
            if(r->quiet[track]<QUIET_BLOCKS)r->quiet[track]=r->quiet[track]+1u;
            if(r->quiet[track]>=QUIET_BLOCKS)r->dormant=r->dormant|(1u<<track);
        } else r->quiet[track]=0u;
    }
}
#ifdef MD_HAS_PAIRING
#define MD_SPLIT_DUE 0x100u
#define MD_SINE ((const int32_t *)0x80550000u)
/* the pair render's private scratch: 2 x (x + y scratch words); twice that
 * plus one word in merge mode (4 x 608 + 1 words; reserved by fast_relink) */
#define md_pair_scratch ((uint32_t *)0x8183b000u)
/* running average over about 8 values; 0 is empty */
static void md_average(uint32_t *a,uint32_t x) {
    if(*a==0u)*a=x;
    else *a=*a-(*a>>3)+(x>>3);
}
/* a single render of machine m: into its average; the first 4 are probes */
static void md_single(MdRoute2 *q,uint32_t m,uint32_t c) {
    md_average(&q->render_cost[m],c);
    if((q->pair_n[m]>>16)<4u)q->pair_n[m]=q->pair_n[m]+0x10000u;
}
/* Two adjacent tracks with the same machine: each voice's parameters and
 * resampling up to its 44.1 kHz render, both renders at once (singly when only
 * one is due, the pair render declines or the machine's single render cost is
 * being probed), then each voice's remaining resampling -- the statements of
 * the two single renders, in their order for each voice. cost[0], cost[1]:
 * each voice's cycles (half of the pair render each). Whether pairing pays is
 * judged render against render (a block's whole cost depends on how many
 * renders fall due in it): the average of half the pair render against the
 * average single 44.1 kHz render, both over about 8 renders (their cost
 * varies with the sound and, for a pair, with how often the voices diverge).
 * Returns 1 when the pair render ran. Non-static (see md_stock_measure). */
uint32_t md_render_pair(MdRoute *r,const MdDescriptor *engine,uint32_t sel,MdSlot *sa,uint32_t ta,float *oa,
                        MdSlot *sb,uint32_t tb,float *ob,uint32_t *cost) {
    uint32_t c,d,ra,rb,m,pair;
    MdRoute2 *q;
    q=MD_ROUTE2;m=sel-5u;pair=0u;
    c=md_cycles();
    ra=engine->begin(sa->state,ta,0u,MD_SINE,oa)!=0u?engine->run(sa->state,oa,0u):32u;
    cost[0]=md_cycles()-c;c=md_cycles();
    rb=engine->begin(sb->state,tb,0u,MD_SINE,ob)!=0u?engine->run(sb->state,ob,0u):32u;
    cost[1]=md_cycles()-c;
    if((ra&MD_SPLIT_DUE)!=0u&&(rb&MD_SPLIT_DUE)!=0u&&(q->pair_n[m]>>16)>=4u) {
        c=md_cycles();
        pair=engine->pair(sa->state,sb->state,md_pair_scratch)==0u;
        if(pair) {engine->rendered(sa->state);engine->rendered(sb->state);q->pairs=q->pairs+1u;}
        else q->pair_declined=q->pair_declined+1u;
        c=md_cycles()-c;cost[0]=cost[0]+(c>>1);cost[1]=cost[1]+c-(c>>1);
        if(pair) {
            md_average(&q->pair_cost[m],c>>1);
            if((q->pair_n[m]&0xffffu)<8u)q->pair_n[m]=q->pair_n[m]+1u;
            /* pairs that do not beat single renders wait 512 blocks */
            else if(q->pair_cost[m]>=q->render_cost[m]) {
                q->pair_retry[m]=q->blocks+512u;q->pair_cost[m]=0u;q->pair_n[m]=q->pair_n[m]&0xffff0000u;
            }
        }
    }
    while((ra&MD_SPLIT_DUE)!=0u) {
        c=md_cycles();
        if(!pair) {engine->render(sa->state,MD_SINE);md_single(q,m,md_cycles()-c);}
        ra=engine->run(sa->state,oa,ra&31u);
        if((ra&MD_SPLIT_DUE)!=0u) {d=md_cycles();engine->render(sa->state,MD_SINE);md_single(q,m,md_cycles()-d);ra=engine->run(sa->state,oa,ra&31u);}
        cost[0]=cost[0]+md_cycles()-c;
    }
    while((rb&MD_SPLIT_DUE)!=0u) {
        c=md_cycles();
        if(!pair) {engine->render(sb->state,MD_SINE);md_single(q,m,md_cycles()-c);}
        rb=engine->run(sb->state,ob,rb&31u);
        if((rb&MD_SPLIT_DUE)!=0u) {d=md_cycles();engine->render(sb->state,MD_SINE);md_single(q,m,md_cycles()-d);rb=engine->run(sb->state,ob,rb&31u);}
        cost[1]=cost[1]+md_cycles()-c;
    }
    return pair;
}
/* Whether a track that renders this block can wait for a pair partner. */
uint32_t md_pairable(const MdRoute *r,const MdDescriptor *engine,uint32_t selector,uint32_t trig,uint32_t changed) {
    uint32_t m,ok;
    m=selector-5u;ok=0u;
    if((r->policy&POLICY_PAIR)!=0u&&engine->pair!=0&&trig==0u&&changed==0u&&m<MD_SELECTORS&&
       (int32_t)(MD_ROUTE2->blocks-MD_ROUTE2->pair_retry[m])>=0)ok=1u;
    return ok;
}
#endif
void md_generic_process(void) {
    MdPool *pool;
    MdRoute *r;
    MdSlot *slot;
    const MdDescriptor *engine;
    float *out;
    uint32_t t,k,selector,track,tag,triggers,valid,previous,trig,bit;
    uint32_t *owner,*chosen,mdvoices,stocktrig,governing,used,start,spent,need,change,changes,clean;
    uint32_t admit,cand,planned,rank,left,best,changed;
#ifdef MD_HAS_PAIRING
    uint32_t held,held_t,held_sel,both,pc[2];
    float *held_out;
    const MdDescriptor *held_engine;
#endif
    MdRoute2 *q;
    pool=MD_POOL_ADDR;r=MD_ROUTE;triggers=md_frame(34u);
    if(r->magic!=ROUTE_MAGIC)md_route_reset();
    owner=r->owner;chosen=r->chosen;
    governing=md_governor_begin(r);
    /* 1. Classify physical voices from this frame. */
    mdvoices=0u;stocktrig=0u;
    for(t=0u;t<16u;t++) {
        owner[t]=MD_NONE;chosen[t]=MD_NONE;
        selector=md_frame(148u+2u*t);track=md_frame(0xa5au+2u*t);
        if(selector<5u) {
            if(track<16u&&((triggers>>t)&1u))stocktrig=stocktrig|(1u<<track);
            continue;
        }
        if(track>=16u) {md_silence(md_out(t));continue;}
        owner[t]=track;mdvoices=mdvoices|(1u<<t);
    }
    /* 2. One voice per logical track: a triggered voice wins, then the voice
     * rendered last, then the lowest-numbered voice naming the track. A stock
     * trigger on the track retires its MD voices until the next MD trigger. */
    r->stale=r->stale|stocktrig;r->fresh=r->fresh|stocktrig;
    for(t=0u;t<16u;t++) {
        if(((mdvoices>>t)&1u)==0u||((triggers>>t)&1u)==0u)continue;
        track=owner[t];
        if(chosen[track]==MD_NONE)chosen[track]=t;
    }
    for(track=0u;track<16u;track++) {
        bit=1u<<track;
        if(chosen[track]!=MD_NONE) {
            r->voice[track]=chosen[track];r->stale=r->stale&~bit;
            r->dormant=r->dormant&~bit;r->quiet[track]=0u;
            continue;
        }
        if(r->stale&bit)continue;
        t=r->voice[track];
        if(t<16u&&((mdvoices>>t)&1u)!=0u&&owner[t]==track) {chosen[track]=t;continue;}
        for(t=0u;t<16u;t++)if(((mdvoices>>t)&1u)!=0u&&owner[t]==track) {chosen[track]=t;break;}
        r->voice[track]=chosen[track];
    }
    for(t=0u;t<16u;t++) {
        if(((mdvoices>>t)&1u)==0u||chosen[owner[t]]==t)continue;
        md_silence(md_out(t));r->refused=r->refused+1u;
    }
    /* 3. Admission (governor engaged). The newest notes get the budget
     * first: this block's triggers, then triggers already waiting, then the
     * sounding tracks by note start, newest first. A sounding track that does
     * not fit is cut and stays silent until its next trigger (MD tracks are
     * monophonic and may cut each other); a trigger that does not fit waits
     * up to GOV_MAX_DEFER blocks (5 ms), then is dropped. Each estimate
     * carries 1/32 for jitter (the learned margin covers the rest) and the
     * machine-change work if a change is due. */
    q=MD_ROUTE2;
    if(q->magic!=ROUTE2_MAGIC)md_route_reset();
    q->blocks=q->blocks+1u;
#ifdef MD_HAS_PAIRING
    /* single render costs drift with the sound: probe them again */
    if((q->blocks&1023u)==0u)for(k=0u;k<MD_SELECTORS;k++) {q->render_cost[k]=0u;q->pair_n[k]=q->pair_n[k]&0xffffu;}
#endif
    admit=0xffffu;
    if(governing) {
        cand=0u;
        for(track=0u;track<16u;track++) {
            bit=1u<<track;t=chosen[track];
            if(t==MD_NONE) {q->pending=q->pending&~bit;continue;}
            trig=((triggers>>t)&1u)|((q->pending>>track)&1u);
            if(!trig&&((q->stolen&bit)!=0u||((r->policy&POLICY_DORMANCY)!=0u&&(r->dormant&bit)!=0u)))continue;
            need=md_need(r,track,trig);
            selector=md_frame(148u+2u*t);
            if(pool->slots[track].owner!=((track<<8)|selector)||((r->policy&POLICY_CLEAN)!=0u&&(r->fresh&bit)!=0u))
                need=need+(q->change_cost!=0u?q->change_cost:r->period>>4);
            q->need[track]=need+(need>>5);
            cand=cand|bit;
        }
        admit=0u;planned=0u;
        for(rank=0u;rank<3u;rank++) {
            left=cand;
            while(left!=0u) {
                /* pick the best remaining candidate of this rank */
                best=16u;
                for(track=0u;track<16u;track++) {
                    bit=1u<<track;
                    if((left&bit)==0u)continue;
                    t=chosen[track];
                    trig=(triggers>>t)&1u;
                    k=trig?0u:((q->pending&bit)!=0u?1u:2u);
                    if(k!=rank) {left=left&~bit;continue;}
                    if(best==16u)best=track;
                    else if(rank==1u&&q->trig_at[track]<q->trig_at[best])best=track;
                    else if(rank==2u&&q->trig_at[track]>q->trig_at[best])best=track;
                }
                if(best==16u)break;
                bit=1u<<best;left=left&~bit;
                if(planned+q->need[best]<=r->budget) {admit=admit|bit;planned=planned+q->need[best];}
            }
        }
        for(track=0u;track<16u;track++) {
            bit=1u<<track;
            if((cand&bit)==0u||(admit&bit)!=0u)continue;
            t=chosen[track];
            r->muted[track]=r->muted[track]+1u;
            if(((triggers>>t)&1u)!=0u||(q->pending&bit)!=0u) {
                if((q->pending&bit)==0u) {q->pending=q->pending|bit;q->trig_at[track]=q->blocks;}
                if(q->blocks-q->trig_at[track]>=GOV_MAX_DEFER) {
                    q->pending=q->pending&~bit;q->stolen=q->stolen|bit;q->dropped=q->dropped+1u;
                } else q->deferrals=q->deferrals+1u;
            } else {q->stolen=q->stolen|bit;q->steals=q->steals+1u;}
        }
        q->admitted=admit;
    } else {q->stolen=0u;q->pending=0u;}
    /* 4. Render in logical track order: original MD shares scratch between
     * algorithms in that order. Stock processing still receives voice t. */
    used=0u;changes=0u;
#ifdef MD_HAS_PAIRING
    /* a pairable track waits (held) until the next track that renders: the
     * same machine pairs with it, anything else renders it first */
    held=MD_NONE;held_t=0u;held_sel=0u;held_out=0;held_engine=0;
#endif
    for(track=0u;track<16u;track++) {
        t=chosen[track];
        if(t==MD_NONE)continue;
        bit=1u<<track;
        selector=md_frame(148u+2u*t);trig=((triggers>>t)&1u)|((q->pending>>track)&1u);
        out=md_out(t);slot=pool->slots+track;
        if((admit&bit)==0u||(!trig&&(q->stolen&bit)!=0u)) {md_silence(out);continue;}
        if(trig) {q->pending=q->pending&~bit;q->stolen=q->stolen&~bit;q->trig_at[track]=q->blocks;}
        tag=(track<<8)|selector;engine=0;
        if(selector<=53u)engine=md_descriptors+(selector-5u);
        if(engine==0) {md_silence(out);continue;}
        if(pool->magic!=MD_POOL_MAGIC||HISTORY->magic!=HISTORY_MAGIC) {md_silence(out);continue;}
        if(trig&&(slot->fault!=0u||slot->guard_before!=MD_SLOT_GUARD||slot->guard_after!=MD_SLOT_GUARD||!md_history_valid(track))) {
            /* Re-arm a damaged slot on a fresh note rather than staying mute. */
            md_clear(slot);slot->fault=0u;slot->guard_before=MD_SLOT_GUARD;slot->guard_after=MD_SLOT_GUARD;
            md_history_track_reset(track);r->rearmed=r->rearmed+1u;
        }
        valid=slot->fault==0u&&slot->guard_before==MD_SLOT_GUARD&&slot->guard_after==MD_SLOT_GUARD&&md_history_valid(track);
        if(valid)valid=engine->state_bytes!=0u&&engine->state_bytes<=sizeof(slot->state)&&
                       (engine->state_bytes&3u)==0u&&engine->init!=0&&engine->process!=0;
        if(!valid) {
            slot->fault=1u;md_clear(slot);
            md_silence(out);
            continue;
        }
        if((r->policy&POLICY_CLEAN)!=0u&&(r->fresh&bit)!=0u) {
            /* back from a stock machine: start again as a new selection */
            r->fresh=r->fresh&~bit;
            if(slot->owner==tag)slot->owner=MD_NO_OWNER;
        }
        changed=slot->owner!=tag;
        if(slot->owner!=tag) {
            /* machine-change work is MD work of this block, not stock load:
             * it is kept out of the governor's stock estimate */
            change=md_cycles();
            clean=(r->policy&POLICY_CLEAN)!=0u;
            if(slot->owner!=MD_NO_OWNER) {
                previous=slot->owner&255u;
                if((slot->owner>>8)!=track||(!clean&&!md_history_save(slot->state,track,previous))) {
                    slot->fault=1u;md_clear(slot);
                    md_silence(out);
                    continue;
                }
                if(!clean)r->hzero=r->hzero&~bit;
            }
            /* clean change: restore the first-selection (all-zero) history;
             * a history left zero by the last clean change needs no reset */
            if(clean&&(r->hzero&bit)==0u) {md_history_track_reset(track);r->hzero=r->hzero|bit;}
            for(k=engine->state_bytes>>2;k<MD_SLOT_WORDS;k++)slot->state[k]=0u;
            engine->init(slot->state);
            if(!md_history_restore(slot->state,track,selector)) {
                slot->fault=1u;md_clear(slot);
                md_silence(out);
                continue;
            }
            slot->owner=tag;
            /* a machine's first block is judged by its last measured costs */
            r->cost[track]=selector-5u<MD_SELECTORS?r->machine_cost[selector-5u]:0u;
            r->trig_cost[track]=selector-5u<MD_SELECTORS?r->machine_trig[selector-5u]:0u;
            r->quiet[track]=0u;r->dormant=r->dormant&~bit;
            change=md_cycles()-change;changes=changes+change;
            if(change>=q->change_cost)q->change_cost=change;
            else q->change_cost=q->change_cost-(q->change_cost>>6);
        }
        if((r->policy&POLICY_DORMANCY)!=0u&&(r->dormant&bit)!=0u&&!trig) {md_silence(out);continue;}
#ifdef MD_HAS_PAIRING
        if(held!=MD_NONE) {
            if(held_sel==selector&&md_pairable(r,engine,selector,trig,changed)) {
                both=md_render_pair(r,engine,selector,pool->slots+held,held_t,held_out,slot,t,out,pc);
                used=used+pc[0]+pc[1];
                md_after_render(r,held,selector,0u,pc[0],held_out,both);
                md_after_render(r,track,selector,0u,pc[1],out,both);
                held=MD_NONE;
                continue;
            }
            /* not a partner: the held track renders first, as it would have */
            start=md_cycles();
            held_engine->process(pool->slots[held].state,held_t,0u,MD_SINE,held_out);
            spent=md_cycles()-start;used=used+spent;
            md_after_render(r,held,held_sel,0u,spent,held_out,0u);
            held=MD_NONE;
        }
        if(md_pairable(r,engine,selector,trig,changed)) {
            held=track;held_t=t;held_sel=selector;held_out=out;held_engine=engine;
            continue;
        }
#endif
        start=md_cycles();
        engine->process(slot->state,t,trig,(const int32_t *)0x80550000u,out);
        spent=md_cycles()-start;used=used+spent;
        md_after_render(r,track,selector,trig,spent,out,0u);
    }
#ifdef MD_HAS_PAIRING
    if(held!=MD_NONE) {
        start=md_cycles();
        held_engine->process(pool->slots[held].state,held_t,0u,MD_SINE,held_out);
        spent=md_cycles()-start;used=used+spent;
        md_after_render(r,held,held_sel,0u,spent,held_out,0u);
    }
#endif
    r->used=used;
    if(pool->magic==MD_POOL_MAGIC)pool->blocks_rendered=pool->blocks_rendered+1u;
    r->exit=md_cycles();
    r->overhead=r->exit-r->entry>used+changes?r->exit-r->entry-used-changes:0u;
}
const uint32_t md_history_pool_bytes=sizeof(MdTrackHistoryPool);
const uint32_t md_history_track_bytes=sizeof(MdTrackHistory);
const uint32_t md_route_bytes=sizeof(MdRoute);
const uint32_t md_route2_bytes=sizeof(MdRoute2);
