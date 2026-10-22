/* GND-SIN 44.1 kHz render, hand-written, restricted to the instruction
 * vocabulary of the translated renders that run on the unit
 * (translation/fast_sharc.py VOCABULARY). Bisection probe (2026-10-21): the
 * regular hand-written renders stop the unit's audio although sharcvm runs
 * them bit-exact. Same arithmetic as ../gndsin.s, written with:
 *   - the translator's prologue/epilogue (frame 0x15; R8-R15, I0-I3, M4, I5);
 *   - pointers set only through R registers (I = R), never I = sym / I = I;
 *   - memory only as DM(n,I0), DM(n,I6) and DM(M4,Ii) with M4 = R (no
 *     post-modify);
 *   - no parallel moves, no MRB, no `+ CI` (the 48-bit phase keeps its low
 *     word as 24 unsigned bits; the carry is FEXT 24:8, as the translator);
 *   - no CALL/RTS, no SIMD, no decline (GND-SIN has no guard);
 *   - the caller's MR preserved, as the translator does when a render uses
 *     the multiplier result register (saved MR0F/MR1F/MR2F, restored MR1F
 *     first since writing it sign-extends into MR2).
 * R4 = voice, R8 = sine (signed 24-bit words).
 */
.EXTERN md_shared_scratch_x.;
.SECTION/SW seg_swco;
.GLOBAL gndsin_render_block_44k1.;
gndsin_render_block_44k1.:
    I7=MODIFY (I7,-0x18)(NW);
    DM (-5,I6) = R8;
    DM (-6,I6) = R9;
    DM (-7,I6) = R10;
    DM (-8,I6) = R11;
    DM (-9,I6) = R12;
    DM (-10,I6) = R13;
    DM (-11,I6) = R14;
    DM (-12,I6) = R15;
    R0 = I0;
    DM (-13,I6) = R0;
    R0 = I1;
    DM (-14,I6) = R0;
    R0 = I2;
    DM (-15,I6) = R0;
    R0 = I3;
    DM (-16,I6) = R0;
    R0 = M4;
    DM (-17,I6) = R0;
    R0 = I5;
    DM (-18,I6) = R0;
    DM (-19,I6) = R8;                   /* sine */
    DM (-20,I6) = R4;                   /* voice (byte address) */
    R0 = MR0F;
    DM (-21,I6) = R0;
    R0 = MR1F;
    DM (-22,I6) = R0;
    R0 = MR2F;
    DM (-23,I6) = R0;
    I0 = R4;
    R14 = 0x7FFFFF;
    R15 = 0xFF800000;
    /* step = Y1 * 2^24 + 2*Y3*Y6 (Y6 before its decay); Y6 = lim(2*Y6*Y4) */
    R0 = DM (3,I0);
    R1 = DM (6,I0);
    R1 = LSHIFT R1 BY 8;
    MRF = R0 * R1 (SSF);
    R2 = DM (4,I0);
    R2 = R1 * R2 (SSF);
    R2 = MIN(R2,R14);
    DM (6,I0) = R2;
    R0 = DM (1,I0);
    R0 = LSHIFT R0 BY 8;
    R1 = 0x800000;
    MRF = MRF + R0 * R1 (SSF);
    R0 = MR0F;
    R1 = MR1F;
    R0 = LSHIFT R0 BY -14;
    R0 = LSHIFT R0 BY 8;
    R2 = LSHIFT R1 BY 26;
    R3 = R0 OR R2;
    R3 = LSHIFT R3 BY -8;               /* step lo (24 unsigned bits) */
    R2 = -6;
    R4 = ASHIFT R1 BY R2;               /* step hi */
    R0 = DM (-19,I6);
    I4 = R0;                            /* sine table */
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R1 = DM (7,I0);                     /* phase hi */
    R2 = DM (8,I0);
    R2 = FEXT R2 BY 0:24;               /* phase lo (24 unsigned bits) */
    R5 = 0x7FFF;
    R7 = -10;
    R9 = 0;                             /* k */
    /* x:k = sine(hi & $7fff) / 4, then phase += step */
    LCNTR = 0x20 , DO (PC,.L_osc_end)UNTIL LCE;
    R1 = R1 AND R5;
    M4 = R1;
    R0 = DM (M4,I4);
    R2 = R2 + R3;
    R6 = FEXT R2 BY 24:8;               /* carry */
    R2 = FEXT R2 BY 0:24;
    R1 = R1 + R4;
    R1 = R1 + R6;
    R0 = LSHIFT R0 BY 8;
    R0 = ASHIFT R0 BY R7;
    M4 = R9;
    DM (M4,I1) = R0;
.L_osc_end:
    R9 = R9 + 1;
    R8 = -8;
    R1 = LSHIFT R1 BY 8;
    R1 = ASHIFT R1 BY R8;
    DM (7,I0) = R1;
    R2 = LSHIFT R2 BY 8;
    R2 = ASHIFT R2 BY R8;
    DM (8,I0) = R2;
    /* block(k) = lim(x:k * env), env = Ya:Y9 (double precision), decayed by Y2 */
    R1 = DM (0xA,I0);                   /* env hi */
    R2 = DM (0x9,I0);
    R2 = FEXT R2 BY 0:24;               /* env lo (unsigned, a valid SSF operand) */
    R3 = DM (0x2,I0);
    R7 = LSHIFT R3 BY 8;
    R4 = 0x80;
    R0 = DM (-20,I6);
    R5 = 0x2C;
    R0 = R0 + R5;
    I4 = R0;                            /* the block (voice word 11) */
    R10 = 0;                            /* k */
    LCNTR = 0x20 , DO (PC,.L_env_end)UNTIL LCE;
    M4 = R10;
    R0 = DM (M4,I1);
    R5 = LSHIFT R1 BY 8;
    R9 = LSHIFT R0 BY 8;
    R6 = R9 * R2 (SSF);                 /* floor(2*s*lo / 2^24) */
    MRF = R0 * R5 (SSF);
    MRF = MRF + R6 * R4 (SSF);
    R6 = MR1F;
    R6 = MIN(R6,R14);
    R6 = MAX(R6,R15);
    DM (M4,I4) = R6;
    R6 = R7 * R2 (SSF);
    MRF = R3 * R5 (SSF);
    MRF = MRF + R6 * R4 (SSF);
    R1 = MR1F;
    R2 = MR0F;
    R2 = LSHIFT R2 BY -8;
.L_env_end:
    R10 = R10 + 1;
    DM (0xA,I0) = R1;
    R2 = LSHIFT R2 BY 8;
    R8 = -8;
    R2 = ASHIFT R2 BY R8;
    DM (0x9,I0) = R2;
    R0 = DM (-22,I6);
    MR1F = R0;
    R0 = DM (-23,I6);
    MR2F = R0;
    R0 = DM (-21,I6);
    MR0F = R0;
    R0 = DM (-13,I6);
    I0 = R0;
    R0 = DM (-14,I6);
    I1 = R0;
    R0 = DM (-15,I6);
    I2 = R0;
    R0 = DM (-16,I6);
    I3 = R0;
    R0 = DM (-17,I6);
    M4 = R0;
    R0 = DM (-18,I6);
    I5 = R0;
    R8 = DM (-5,I6);
    R9 = DM (-6,I6);
    R10 = DM (-7,I6);
    R11 = DM (-8,I6);
    R12 = DM (-9,I6);
    R13 = DM (-10,I6);
    R14 = DM (-11,I6);
    R15 = DM (-12,I6);
    I12=DM (M7,I6);
    JUMP (M14,I12) (DB);
    RFRAME;
    NOP;
.ENDSEG;
