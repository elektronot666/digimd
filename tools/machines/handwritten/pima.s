/* P-I-MA 44.1 kHz render (pima_render_block_44k1), hand-written SHARC+.
 *
 * Bit-exact replacement for the translated render of the original OS 1.63
 * DSP56300 code at P:$145abb-$145af4 (see docs/FULL49_HANDWRITTEN.md and
 * pima_model.py). Same accumulator convention as pisd.s; mpyr/macr are the
 * convergent rounding at A0 done by hand: selas assembles (SSFR) as (SSF).
 *
 * x:0..31: noise f(k+1), f(-1) = Yd, f(0) = Ye, f(k+1) = f(k) + f(k-1) mod
 * 2^24. Per sample k: env = lim(2*Y8*Y4 (+ Y7 when |noise| <= Y1)) -> Y8;
 * a = rnd(2*env*noise); b = rnd(2*Y2*lim(a)); a = 4b; a = rnd(a - 2*Ya*Y5);
 * a = 2a; a = rnd(a - 2*Yb*Y6); Yb = Ya, Ya = lim(a); block = lim(Ya - Yf)
 * of the old Ya, Yf = old Ya; Y7 = lim(2*Y7*Y3).
 *
 * Calling convention as the translated renders: R4 = voice, R8 = sine.
 */
.EXTERN md_shared_scratch_x.;
.EXTERN md_shared_scratch_y.;
.EXTERN md_shared_y20.;
.EXTERN pima_render_block_44k1_c.;
.SECTION/SW seg_swco;
.GLOBAL pima_render_block_44k1.;
pima_render_block_44k1.:
    I7=MODIFY (I7,-0x34)(NW);
    DM (-3,I6) = R8;
    DM (-4,I6) = R9;
    DM (-5,I6) = R10;
    DM (-6,I6) = R11;
    DM (-7,I6) = R12;
    DM (-8,I6) = R13;
    DM (-9,I6) = R14;
    DM (-10,I6) = R15;
    R0 = I0;
    DM (-11,I6) = R0;
    R0 = I1;
    DM (-12,I6) = R0;
    R0 = I2;
    DM (-13,I6) = R0;
    R0 = I3;
    DM (-14,I6) = R0;
    R0 = I4;
    DM (-15,I6) = R0;
    R0 = I5;
    DM (-16,I6) = R0;
    R0 = M4;
    DM (-17,I6) = R0;
    R0 = M0;
    DM (-18,I6) = R0;
    R0 = M1;
    DM (-19,I6) = R0;
    R0 = M2;
    DM (-20,I6) = R0;
    R0 = M3;
    DM (-21,I6) = R0;
    R0 = MR0F;
    DM (-22,I6) = R0;
    R0 = MR1F;
    DM (-23,I6) = R0;
    R0 = MR2F;
    DM (-24,I6) = R0;
    DM (-28,I6) = R8;
    R0 = L0;
    DM (-46,I6) = R0;
    R0 = L1;
    DM (-47,I6) = R0;
    R0 = L2;
    DM (-48,I6) = R0;
    R0 = L3;
    DM (-49,I6) = R0;
    R0 = L4;
    DM (-50,I6) = R0;
    R0 = L5;
    DM (-51,I6) = R0;
    R0 = 0;
    L0 = R0;
    L1 = R0;
    L2 = R0;
    L3 = R0;
    L4 = R0;
    L5 = R0;
    I0 = R4;
    R14 = 0x7FFFFF;
    R15 = 0xFF800000;
    R13 = @SRC@;
    DM (-39,I6) = R13;


/* ---- noise x:0..31 ($145abb-$145aca) ---------------------------------- */
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R8 = DM (13,I0);
    R8 = LSHIFT R8 BY 8;                /* p << 8 */
    R9 = DM (14,I0);
    R9 = LSHIFT R9 BY 8;                /* q << 8 */
    R7 = -8;
    LCNTR = 0x20 , DO (PC,.L_noise_end)UNTIL LCE;
    R0 = R8 + R9;
    R8 = PASS R9;
    R9 = PASS R0;
    R0 = ASHIFT R0 BY R7;
.L_noise_end:
    DM (I1,M6) = R0;
    R0 = ASHIFT R8 BY R7;
    DM (13,I0) = R0;                    /* Yd = f(31) */
    R0 = ASHIFT R9 BY R7;
    DM (14,I0) = R0;                    /* Ye = f(32) */

/* ---- 32 samples ($145acd-$145af3) --------------------------------------- */
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R0 = 0x40;
    R1 = I0;
    R0 = R0 + R1;
    I2 = R0;                            /* blk44 */
    R4 = DM (4,I0);
    R4 = LSHIFT R4 BY 8;                /* Y4 << 8 */
    R0 = DM (1,I0);
    DM (-30,I6) = R0;                   /* Y1 */
    R5 = DM (2,I0);
    R5 = LSHIFT R5 BY 8;                /* Y2 << 8 */
    R6 = DM (5,I0);
    R6 = LSHIFT R6 BY 8;                /* Y5 << 8 */
    R7 = DM (6,I0);
    R7 = LSHIFT R7 BY 8;                /* Y6 << 8 */
    R3 = DM (3,I0);
    R3 = LSHIFT R3 BY 8;
    DM (-29,I6) = R3;                   /* Y3 << 8 */
    R13 = 1;
    R3 = 0x7FFFFFFF;
    R8 = DM (8,I0);                     /* env */
    R9 = DM (10,I0);                    /* Ya */
    R10 = DM (11,I0);                   /* Yb */
    R11 = DM (15,I0);                   /* Yf */
    R12 = DM (7,I0);                    /* Y7 */
    /* (selas drops the (SSFR) rounding suffix: mpyr/macr round by hand) */
    LCNTR = 0x20 , DO (PC,.L_s_end)UNTIL LCE;
    R8 = R8 * R4 (SSF) , R2 = DM (I1,M6);   /* 2*env*Y4, noise */
    R1 = ABS R2;
    R0 = DM (-30,I6);
    COMP (R1,R0);
    IF LE R8 = R8 + R12;
    R8 = MIN(R8,R14);
    R8 = MAX(R8,R15);                   /* env */
    R1 = LSHIFT R8 BY 8;
    MRF = R1 * R2 (SSF);
    R1 = MR1F;
    R0 = MR0F;
    R2 = R1 AND R13;
    R2 = R2 + R3;
    R0 = R0 + R2;
    R1 = R1 + CI;                       /* convergent rounding at A0 */
    R1 = MIN(R1,R14);                   /* lim(rnd(2*env*noise)) */
    MRF = R5 * R1 (SSF);
    R1 = MR1F;
    R0 = MR0F;
    R2 = R1 AND R13;
    R2 = R2 + R3;
    R0 = R0 + R2;
    R1 = R1 + CI;                       /* convergent rounding at A0 */
    R1 = R1 + R1;
    R1 = R1 + R1;                       /* 4*b */
    MR1F = R1;
    R1 = 0;
    MR0F = R1;
    MRF = MRF - R6 * R9 (SSF);
    R1 = MR1F;
    R0 = MR0F;
    R2 = R1 AND R13;
    R2 = R2 + R3;
    R0 = R0 + R2;
    R1 = R1 + CI;                       /* convergent rounding at A0 */
    R1 = R1 + R1;
    MR1F = R1;
    R1 = 0;
    MR0F = R1;
    MRF = MRF - R7 * R10 (SSF);
    R1 = MR1F;
    R0 = MR0F;
    R2 = R1 AND R13;
    R2 = R2 + R3;
    R0 = R0 + R2;
    R1 = R1 + CI;                       /* convergent rounding at A0 */
    DM (-31,I6) = R1;                   /* new Ya (unlimited) */
    R1 = R9 - R11;
    R1 = MIN(R1,R14);
    R1 = MAX(R1,R15);                   /* block: old Ya - Yf */
    R11 = PASS R9 , DM (I2,M6) = R1;
    R10 = PASS R9;
    R9 = DM (-31,I6);
    R9 = MIN(R9,R14);
    R9 = MAX(R9,R15);
    R0 = DM (-29,I6);
    R12 = R12 * R0 (SSF);
.L_s_end:
    R12 = MIN(R12,R14);
    DM (8,I0) = R8;
    DM (10,I0) = R9;
    DM (11,I0) = R10;
    DM (15,I0) = R11;
    DM (7,I0) = R12;
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R1 = md_shared_y20.;
    I3 = R1;
    R0 = DM (0,I3);                     /* y:$20 = the shared history: kept */
    R0 = LSHIFT R0 BY 8;
    R1 = -8;
    R0 = ASHIFT R0 BY R1;
    DM (32,I1) = R0;
    R0 = FEXT R0 BY 0:24;
    DM (0,I3) = R0;
    JUMP .L_exit;
.L_decline:
    /* tail call: the C reference render runs in this render's place, in
     * its caller's frame (not nested under this one: the audio stack is
     * 509 words) */
    R4 = I0;
    R0 = DM (-11,I6);
    I0 = R0;
    R0 = DM (-12,I6);
    I1 = R0;
    R0 = DM (-13,I6);
    I2 = R0;
    R0 = DM (-14,I6);
    I3 = R0;
    R0 = DM (-15,I6);
    I4 = R0;
    R0 = DM (-16,I6);
    I5 = R0;
    R0 = DM (-17,I6);
    M4 = R0;
    R0 = DM (-18,I6);
    M0 = R0;
    R0 = DM (-19,I6);
    M1 = R0;
    R0 = DM (-20,I6);
    M2 = R0;
    R0 = DM (-21,I6);
    M3 = R0;
    R0 = DM (-23,I6);
    MR1F = R0;
    R0 = DM (-24,I6);
    MR2F = R0;
    R0 = DM (-22,I6);
    MR0F = R0;
    R0 = DM (-46,I6);
    L0 = R0;
    R0 = DM (-47,I6);
    L1 = R0;
    R0 = DM (-48,I6);
    L2 = R0;
    R0 = DM (-49,I6);
    L3 = R0;
    R0 = DM (-50,I6);
    L4 = R0;
    R0 = DM (-51,I6);
    L5 = R0;
    R8 = DM (-3,I6);
    R9 = DM (-4,I6);
    R10 = DM (-5,I6);
    R11 = DM (-6,I6);
    R12 = DM (-7,I6);
    R13 = DM (-8,I6);
    R14 = DM (-9,I6);
    R15 = DM (-10,I6);
    I7=MODIFY (I7,0x34)(NW);
    JUMP pima_render_block_44k1_c.;
.L_exit:
    R0 = DM (-11,I6);
    I0 = R0;
    R0 = DM (-12,I6);
    I1 = R0;
    R0 = DM (-13,I6);
    I2 = R0;
    R0 = DM (-14,I6);
    I3 = R0;
    R0 = DM (-15,I6);
    I4 = R0;
    R0 = DM (-16,I6);
    I5 = R0;
    R0 = DM (-17,I6);
    M4 = R0;
    R0 = DM (-18,I6);
    M0 = R0;
    R0 = DM (-19,I6);
    M1 = R0;
    R0 = DM (-20,I6);
    M2 = R0;
    R0 = DM (-21,I6);
    M3 = R0;
    R0 = DM (-23,I6);
    MR1F = R0;
    R0 = DM (-24,I6);
    MR2F = R0;
    R0 = DM (-22,I6);
    MR0F = R0;
    R0 = DM (-46,I6);
    L0 = R0;
    R0 = DM (-47,I6);
    L1 = R0;
    R0 = DM (-48,I6);
    L2 = R0;
    R0 = DM (-49,I6);
    L3 = R0;
    R0 = DM (-50,I6);
    L4 = R0;
    R0 = DM (-51,I6);
    L5 = R0;
    R8 = DM (-3,I6);
    R9 = DM (-4,I6);
    R10 = DM (-5,I6);
    R11 = DM (-6,I6);
    R12 = DM (-7,I6);
    R13 = DM (-8,I6);
    R14 = DM (-9,I6);
    R15 = DM (-10,I6);
    I12=DM (M7,I6);
    JUMP (M14,I12) (DB);
    RFRAME;
    NOP;
.ENDSEG;
