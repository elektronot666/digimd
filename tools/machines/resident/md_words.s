/* Word copy and fill for the Full49 dispatcher's machine-change work (track
 * history save/restore, slot and history clears). fast_relink substitutes
 * them for dispatch49.c's element loops, which selcc -O1 compiles to about
 * 21 instructions per word (1,600 private words per P-I machine change);
 * these take 3.
 *
 *   md_copy_words(dst, src, words): R4 = dst, R8 = src, R12 = words
 *   md_zero_words(dst, words):      R4 = dst, R8 = words
 *
 * Byte addresses (C ABI); DAG modifiers on byte-space addresses are scaled
 * by the access width, so M6 steps one word. Uses R0, R1, R2, R4, I4
 * (scratch) and I0 (kept in R2). L0 and L4 are zeroed for the loop and given
 * back (kept in R1/R4): the caller's circular buffers must not apply to these
 * post-modified pointers (handwritten/lregs.py). Leaf functions: no frame.
 * Hardware loops are kept at three instructions (no 1-2 instruction loops). */
.SECTION/SW seg_swco;
.GLOBAL md_copy_words.;
md_copy_words.:
    R12 = PASS R12;
    IF EQ JUMP .L_copy_done;
    R2 = I0;
    I0 = R4;
    I4 = R8;
    R1 = L0;
    R4 = L4;
    R0 = 0;
    L0 = R0;
    L4 = R0;
    LCNTR = R12 , DO (PC,.L_copy_end)UNTIL LCE;
    R0 = DM (I4,M6);
    NOP;
.L_copy_end:
    DM (I0,M6) = R0;
    I0 = R2;
    L0 = R1;
    L4 = R4;
.L_copy_done:
    I12=DM (M7,I6);
    JUMP (M14,I12) (DB);
    RFRAME;
    NOP;

.GLOBAL md_zero_words.;
md_zero_words.:
    R8 = PASS R8;
    IF EQ JUMP .L_zero_done;
    I4 = R4;
    R4 = L4;
    R0 = 0;
    L4 = R0;
    LCNTR = R8 , DO (PC,.L_zero_end)UNTIL LCE;
    NOP;
    NOP;
.L_zero_end:
    DM (I4,M6) = R0;
    L4 = R4;
.L_zero_done:
    I12=DM (M7,I6);
    JUMP (M14,I12) (DB);
    RFRAME;
    NOP;
.ENDSEG;
