/* P-I-CC 44.1 kHz render (picc_render_block_44k1), hand-written SHARC+.
 *
 * Bit-exact replacement for the translated render of the original OS 1.63
 * DSP56300 code at P:$145037-$145223 (see docs/FULL49_HANDWRITTEN.md for the
 * section map). It keeps the translated voice layout and the shared scratch
 * contract: every word of voice state, x/y scratch, y:$20 history and the
 * 44.1 kHz block ends as the original leaves it.
 *
 * Accumulators. A DSP56300 accumulator value A (56 bits, A = 2*x*y for a
 * fractional product) is held in a SHARC MR register as A * 2^8: one operand
 * low-aligned (a 24-bit value sign-extended in 32 bits, as stored in memory),
 * the other shifted left 8, multiplied (SSF). Then MR1 = A2:A1 sign-extended,
 * MR0 = A0 << 8, and MR1 is the truncated 24-bit read of A before limiting.
 * Limiting (a move from an accumulator) clamps MR1 to [-2^23, 2^23-1]. A
 * 24-bit integer added at A1 adds to MR1, or multiplies (x << 8) by 2^23.
 * "R = Rx * Ry (SSF)" with the same alignment gives floor(2xy / 2^24), a
 * product moved straight to a register.
 *
 * Declines (calls the reference render) when the voice is outside the
 * states the original program keeps it in: virtual voice >= 16, a delay
 * pointer outside the voice's window, Y4 < 0 (table index), the click
 * phase outside the sine table or its step beyond one wrap, or a filter
 * table address outside the linear source words. Follows picc_model.py
 * (a literal transcription; its sections A-L).
 *
 * Calling convention as the translated renders: R4 = voice, R8 = sine.
 * @SRC@ = byte address of source word 0 (immutable source base - 0x80000).
 * @TMP@ = a 256-word scratch block owned by the hand-written renders.
 */
.EXTERN md_shared_scratch_x.;
.EXTERN md_shared_scratch_y.;
.EXTERN md_shared_y20.;
.EXTERN picc_render_block_44k1_c.;
.SECTION/SW seg_swco;
.GLOBAL picc_render_block_44k1.;
picc_render_block_44k1.:
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

/* ---- section A, evaluated first (registers only) ($145037-$145045). Results: R8 = Y39', R9 = Y12', R10 = Y37', R11 = Y3b',
 * R12 = Y32', slot -36 = Y3a', slot -37 = 1 if the gate set block runs. */
    R0 = DM (57,I0);                    /* x0 = Y39 */
    R1 = DM (9,I0);                     /* y0 = Y9 */
    R1 = LSHIFT R1 BY 8;
    MRF = R1 * R0 (SSF);                /* a = 2*Y9*Y39 */
    R2 = MR1F;
    R3 = MR0F;
    R8 = MIN(R2,R14);
    R8 = MAX(R8,R15);                   /* Y39' = lim(a) */
    R9 = DM (18,I0);                    /* Y12 */
    R10 = DM (55,I0);                   /* Y37 */
    R11 = DM (59,I0);                   /* Y3b */
    R4 = DM (60,I0);                    /* Y3c */
    R5 = 0;
    DM (-37,I6) = R5;                   /* (no gate state machine) */
.L_a_done:
    DM (-38,I6) = R4;                   /* Y3c' */
    R5 = LSHIFT R11 BY 8;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);                   /* Y3b'' = lim(2*Y3b'*Y3c') */
    R12 = DM (50,I0);                   /* Y32 */
    R12 = R12 + R11;
    R12 = MIN(R12,R14);
    R12 = MAX(R12,R15);                 /* Y32' */
    R11 = PASS R5;
    R6 = DM (58,I0);                    /* Y3a */
    R10 = PASS R10;
    IF NE R6 = PASS R12;
    DM (-36,I6) = R6;                   /* Y3a' */

/* ---- guards: nothing is written before these pass ---------------------- */
    R0 = DM (1709,I0);                  /* virtual voice */
    R1 = 16;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    R1 = 0x600;
    R7 = R0 * R1 (SSI);
    R1 = 0x135600;
    R7 = R7 + R1;                       /* the voice's external window */
    DM (-29,I6) = R7;
    R1 = 0x600;
    /* delay pointers Y14..Y17 (256-word modulo lines) inside the window */
    R0 = DM (20,I0);
    R0 = FEXT R0 BY 0:24;
    R0 = R0 - R7;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    R0 = DM (21,I0);
    R0 = FEXT R0 BY 0:24;
    R0 = R0 - R7;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    R0 = DM (22,I0);
    R0 = FEXT R0 BY 0:24;
    R0 = R0 - R7;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    R0 = DM (23,I0);
    R0 = FEXT R0 BY 0:24;
    R0 = R0 - R7;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    /* Y4 >= 0 (table index); click phase Y35 inside the sine table, its step
     * Y5 within one wrap of the 32768-word window */
    R0 = DM (4,I0);
    R0 = PASS R0;
    IF LT JUMP .L_decline;
    R0 = DM (53,I0);
    R0 = FEXT R0 BY 0:24;
    R1 = 0x148000;
    R0 = R0 - R1;
    R1 = 0x8000;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    R0 = DM (5,I0);
    R0 = R0 + R1;
    R1 = 0x10001;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    /* filter tables: D at 4*Y8 + $145224, E at (4*a1(2*Y3a'*Y7) & $fffffc)
     * + $145424, four words each in the linear source words */
    R0 = DM (8,I0);
    R0 = LSHIFT R0 BY 2;
    R1 = 0x145224;
    R0 = R0 + R1;                       /* (unwrapped: in range either way) */
    DM (-30,I6) = R0;
    R1 = 0x13B600;
    R0 = R0 - R1;
    R1 = 0xC9FC;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    R0 = DM (-36,I6);                   /* Y3a' */
    R0 = LSHIFT R0 BY 8;
    R1 = DM (7,I0);
    R1 = LSHIFT R1 BY 2;
    R0 = R0 * R1 (SSF);                 /* a1 of (2*Y3a'*Y7) << 2 */
    R1 = 0xFFFFFFFC;
    R0 = R0 AND R1;
    R1 = 0x145424;
    R0 = R0 + R1;                       /* (unwrapped) */
    DM (-31,I6) = R0;
    R1 = 0x13B600;
    R0 = R0 - R1;
    R1 = 0xC9FC;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;

/* ---- A: commit -------------------------------------------------------- */
    DM (57,I0) = R8;                    /* Y39 */
    DM (18,I0) = R9;                    /* Y12 */
    R0 = DM (-37,I6);
    R0 = PASS R0;
    IF EQ JUMP .L_a_keep;
    DM (55,I0) = R10;                   /* Y37 = $020000 */
    R0 = 0xCCCCD;
    DM (16,I0) = R0;                    /* Y10 */
    R0 = 0x75C28F;
    DM (17,I0) = R0;                    /* Y11 */
    R0 = DM (-38,I6);
    DM (60,I0) = R0;                    /* Y3c */
.L_a_keep:
    DM (59,I0) = R11;                   /* Y3b */
    DM (50,I0) = R12;                   /* Y32 */
    R0 = DM (-36,I6);
    DM (58,I0) = R0;                    /* Y3a */
    /* y:$20 = the shared y:$20 history (a 24-bit word) */
    R0 = md_shared_y20.;
    I1 = R0;
    R0 = DM (0,I1);
    R0 = LSHIFT R0 BY 8;
    R1 = -8;
    R0 = ASHIFT R0 BY R1;
    R1 = md_shared_scratch_y.;
    I1 = R1;
    DM (32,I1) = R0;
    /* y:$100..$15f start cleared (the block is formed there) */
    R0 = md_shared_scratch_y.;
    R1 = 0x400;
    R0 = R0 + R1;
    I1 = R0;
    R0 = 0;
    LCNTR = 0x30 , DO (PC,.L_clr_end)UNTIL LCE;
    DM (I1,M6) = R0;
    NOP;
.L_clr_end:
    DM (I1,M6) = R0;

/* ---- B: noise y:0..31 ($1458cc-$1458d9): s(k+1) = s(k) + s(k-1) mod 2^24,
 * s(-1) = Y30, s(0) = Y31 (held << 8) ------------------------------------- */
    R8 = DM (48,I0);
    R8 = LSHIFT R8 BY 8;                /* b1: s(-1) */
    R9 = DM (49,I0);
    R9 = LSHIFT R9 BY 8;                /* x1: s(0) */
    R7 = -8;
    R0 = md_shared_scratch_y.;
    I1 = R0;
    LCNTR = 0x10 , DO (PC,.L_noise_end)UNTIL LCE;
    R0 = ASHIFT R9 BY R7;
    R8 = R8 + R9 , DM (I1,M6) = R0;
    R0 = ASHIFT R8 BY R7;
    R9 = R9 + R8 , DM (I1,M6) = R0;
.L_noise_end:
    NOP;
    R0 = ASHIFT R9 BY R7;
    DM (48,I0) = R0;                    /* Y30 */
    R0 = ASHIFT R8 BY R7;
    DM (49,I0) = R0;                    /* Y31 */

/* ---- C: envelope gated by noise > Y12 -> x:8+k, then x:8+k *= noise ------
 * ($1458da-$1458f9): e = floor(2*e*Y11) + (n(k) > Y12 ? Y10 : 0), limited */
.L_c:
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R0 = md_shared_scratch_x.;
    R1 = 0x20;
    R0 = R0 + R1;
    I2 = R0;                            /* x:8 */
    R8 = DM (52,I0);                    /* e = Y34 */
    R9 = DM (16,I0);                    /* Y10 */
    R10 = DM (17,I0);
    R10 = LSHIFT R10 BY 8;              /* Y11 << 8 */
    R11 = DM (18,I0);                   /* Y12 */
    LCNTR = 0x20 , DO (PC,.L_c1_end)UNTIL LCE;
    R0 = DM (I1,M6);
    COMP (R0,R11);
    IF GT R8 = R8 + R9;
    R8 = MIN(R8,R14);
    R8 = MAX(R8,R15);
    DM (I2,M6) = R8;
.L_c1_end:
    R8 = R8 * R10 (SSF);
    R8 = MIN(R8,R14);
    DM (52,I0) = R8;                    /* Y34 */
    /* x:8+k = lim(2*x:8+k*v(k)), v = y:32, y:0, y:1, .. y:30 */
    R0 = md_shared_scratch_x.;
    R1 = 0x20;
    R0 = R0 + R1;
    I2 = R0;
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R5 = DM (32,I1);
    LCNTR = 0x20 , DO (PC,.L_c2_end)UNTIL LCE;
    R0 = DM (0,I2);
    R5 = LSHIFT R5 BY 8;
    R0 = R0 * R5 (SSF);
    R0 = MIN(R0,R14);
    R5 = DM (I1,M6);
.L_c2_end:
    DM (I2,M6) = R0;

/* ---- D, E: two filter sections over the x scratch ring ($1458fa-$14595d).
 * Per section (coefficients T0..T3 from its table, state words hi/lo/s):
 *   x:r..r+3 <- state copies; a = Y(hi):Y(lo); x1' = x:r+1;
 *   a += T0*x:r + T1*x:r+1; then for j = 0..31, p = r+2+j:
 *   b = a + T1*x1' + T2*x:p + T3*x:p+1 + T2*x:p+2;  x:p:y:p <- b (limited);
 *   a = b + T0*x1' + T1*lim(b);  x1' = lim(b)
 * (the coefficient ring y:$28..$2c always yields the same coefficient per
 * read slot; only the last section's fill, K, stays in it). */
.L_d:
    R0 = DM (-30,I6);                   /* D table */
    NOP;
    NOP;
    /* (inlined .L_table) */
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R9 = DM (0,I4);
    R9 = LSHIFT R9 BY 8;
    R10 = DM (1,I4);
    R10 = LSHIFT R10 BY 8;
    R11 = DM (2,I4);
    R11 = LSHIFT R11 BY 8;
    R12 = DM (3,I4);
    R12 = LSHIFT R12 BY 8;
    JUMP .L_table_ret_i1;
.L_table_ret_i1:
    R0 = DM (35,I0);                    /* x:4..7 <- Y23, Y22, Y25, Y24 */
    R1 = DM (34,I0);
    R2 = DM (37,I0);
    R3 = DM (36,I0);
    R4 = md_shared_scratch_x.;
    I1 = R4;
    DM (4,I1) = R0;
    DM (5,I1) = R1;
    DM (6,I1) = R2;
    DM (7,I1) = R3;
    R0 = DM (39,I1);
    DM (36,I0) = R0;                    /* Y24 = x:$27 */
    R0 = DM (38,I1);
    DM (37,I0) = R0;                    /* Y25 = x:$26 */
    R0 = 4;                             /* ring start */
    R1 = 34;                            /* hi = Y22, lo = Y21, s = Y23 */
    R2 = 33;
    R3 = 35;
    NOP;
    NOP;
    /* (inlined .L_section) */
    DM (-32,I6) = R1;
    DM (-33,I6) = R2;
    DM (-34,I6) = R3;
    R4 = md_shared_scratch_x.;
    R5 = LSHIFT R0 BY 2;
    R4 = R4 + R5;
    I1 = R4;                            /* x:r */
    R4 = md_shared_scratch_y.;
    R4 = R4 + R5;
    R6 = 8;
    R4 = R4 + R6;
    I2 = R4;                            /* y:r+2 */
    M4 = R1;
    R4 = DM (M4,I0);                    /* Y(hi) */
    MR1F = R4;
    M4 = R2;
    R4 = DM (M4,I0);                    /* Y(lo) */
    R4 = LSHIFT R4 BY 8;
    MR0F = R4;
    R0 = DM (I1,M6);                    /* x:r */
    R1 = DM (I1,M6);                    /* x:r+1 = x1' */
    MRF = MRF + R9 * R0 (SSF);
    MRF = MRF + R10 * R1 (SSF);
    R2 = DM (0,I1);                     /* x:p */
    R3 = DM (1,I1);                     /* x:p+1 */
    R7 = -8;
    R8 = -1;
    LCNTR = 0x20 , DO (PC,.L_sec_end_i2)UNTIL LCE;
    MRF = MRF + R10 * R1 (SSF) , R4 = DM (2,I1);
    MRF = MRF + R11 * R2 (SSF);
    MRF = MRF + R12 * R3 (SSF);
    MRF = MRF + R11 * R4 (SSF);         /* b */
    R5 = MR1F;
    R6 = MR0F;
    R13 = ASHIFT R6 BY R7;              /* b0 (raw) */
    R0 = MIN(R5,R14);
    R0 = MAX(R0,R15);                   /* lim(b) */
    R6 = PASS R13;
    R5 = R5 - R0;                       /* limited: lo = $ffffff / 0 */
    IF GT R6 = PASS R8;
    IF LT R6 = R6 - R6;
    MRF = MRF + R9 * R1 (SSF) , DM (I1,M6) = R0;
    MRF = MRF + R10 * R0 (SSF) , DM (I2,M6) = R6;
    R1 = PASS R0;
    R2 = PASS R3;
.L_sec_end_i2:
    R3 = PASS R4;
    /* Y(hi) = x:p of j = 31, Y(lo) = b0 (raw), Y(s) = x:p of j = 30 */
    R4 = DM (-32,I6);
    M4 = R4;
    DM (M4,I0) = R1;
    R4 = DM (-33,I6);
    M4 = R4;
    DM (M4,I0) = R13;
    R4 = DM (-2,I1);
    R5 = DM (-34,I6);
    M4 = R5;
    DM (M4,I0) = R4;
    JUMP .L_section_ret_i2;
.L_section_ret_i2:
.L_e:
    R0 = DM (-31,I6);                   /* E table */
    NOP;
    NOP;
    /* (inlined .L_table) */
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R9 = DM (0,I4);
    R9 = LSHIFT R9 BY 8;
    R10 = DM (1,I4);
    R10 = LSHIFT R10 BY 8;
    R11 = DM (2,I4);
    R11 = LSHIFT R11 BY 8;
    R12 = DM (3,I4);
    R12 = LSHIFT R12 BY 8;
    JUMP .L_table_ret_i3;
.L_table_ret_i3:
    R0 = DM (40,I0);                    /* x:0..3 <- Y28, Y27, Y2a, Y29 */
    R1 = DM (39,I0);
    R2 = DM (42,I0);
    R3 = DM (41,I0);
    R4 = md_shared_scratch_x.;
    I1 = R4;
    DM (0,I1) = R0;
    DM (1,I1) = R1;
    DM (2,I1) = R2;
    DM (3,I1) = R3;
    R0 = DM (35,I1);
    DM (41,I0) = R0;                    /* Y29 = x:$23 */
    R0 = DM (34,I1);
    DM (42,I0) = R0;                    /* Y2a = x:$22 */
    R0 = 0;
    R1 = 39;                            /* hi = Y27, lo = Y26, s = Y28 */
    R2 = 38;
    R3 = 40;
    NOP;
    NOP;
    /* (inlined .L_section) */
    DM (-32,I6) = R1;
    DM (-33,I6) = R2;
    DM (-34,I6) = R3;
    R4 = md_shared_scratch_x.;
    R5 = LSHIFT R0 BY 2;
    R4 = R4 + R5;
    I1 = R4;                            /* x:r */
    R4 = md_shared_scratch_y.;
    R4 = R4 + R5;
    R6 = 8;
    R4 = R4 + R6;
    I2 = R4;                            /* y:r+2 */
    M4 = R1;
    R4 = DM (M4,I0);                    /* Y(hi) */
    MR1F = R4;
    M4 = R2;
    R4 = DM (M4,I0);                    /* Y(lo) */
    R4 = LSHIFT R4 BY 8;
    MR0F = R4;
    R0 = DM (I1,M6);                    /* x:r */
    R1 = DM (I1,M6);                    /* x:r+1 = x1' */
    MRF = MRF + R9 * R0 (SSF);
    MRF = MRF + R10 * R1 (SSF);
    R2 = DM (0,I1);                     /* x:p */
    R3 = DM (1,I1);                     /* x:p+1 */
    R7 = -8;
    R8 = -1;
    LCNTR = 0x20 , DO (PC,.L_sec_end_i4)UNTIL LCE;
    MRF = MRF + R10 * R1 (SSF) , R4 = DM (2,I1);
    MRF = MRF + R11 * R2 (SSF);
    MRF = MRF + R12 * R3 (SSF);
    MRF = MRF + R11 * R4 (SSF);         /* b */
    R5 = MR1F;
    R6 = MR0F;
    R13 = ASHIFT R6 BY R7;              /* b0 (raw) */
    R0 = MIN(R5,R14);
    R0 = MAX(R0,R15);                   /* lim(b) */
    R6 = PASS R13;
    R5 = R5 - R0;                       /* limited: lo = $ffffff / 0 */
    IF GT R6 = PASS R8;
    IF LT R6 = R6 - R6;
    MRF = MRF + R9 * R1 (SSF) , DM (I1,M6) = R0;
    MRF = MRF + R10 * R0 (SSF) , DM (I2,M6) = R6;
    R1 = PASS R0;
    R2 = PASS R3;
.L_sec_end_i4:
    R3 = PASS R4;
    /* Y(hi) = x:p of j = 31, Y(lo) = b0 (raw), Y(s) = x:p of j = 30 */
    R4 = DM (-32,I6);
    M4 = R4;
    DM (M4,I0) = R1;
    R4 = DM (-33,I6);
    M4 = R4;
    DM (M4,I0) = R13;
    R4 = DM (-2,I1);
    R5 = DM (-34,I6);
    M4 = R5;
    DM (M4,I0) = R4;
    JUMP .L_section_ret_i4;
.L_section_ret_i4:
    JUMP .L_f;

    /* R0: table address (DSP x:) -> R9..R12 = T0..T3 << 8 */

    /* the section: R0 ring start r, R1/R2/R3 state words hi/lo/s; T0..T3 << 8
     * in R9..R12. x/y:p are written from p = r+2. */

/* ---- F: click ($145960-$14598c). x0 = Y1 (Y37 != 0: $733333);
 * t = floor(2*$49*x0); if Y13 < t: Y13 = lim(Y13 + $20), v = t - Y13 - $20
 * (at most $20), n = |v| >> 1 pairs: x:k = lim(x:k + 2*$19999a*sine(phase)),
 * phase = Y35 + k*Y5 in the 32768-word sine table; Y35 += 2n*Y5 ---------- */
.L_f:
    R0 = DM (1,I0);
    R1 = DM (55,I0);
    R2 = 0x733333;
    R1 = PASS R1;
    IF NE R0 = PASS R2;
    R1 = 0x4900;
    R0 = R0 * R1 (SSF);                 /* t */
    R1 = DM (19,I0);                    /* Y13 */
    COMP (R1,R0);
    IF GE JUMP .L_g;
    R2 = 0x20;
    R1 = R1 + R2;
    R3 = MIN(R1,R14);
    R3 = MAX(R3,R15);
    DM (19,I0) = R3;
    R0 = R0 - R1;                       /* v */
    R0 = MIN(R0,R2);
    R0 = ABS R0;
    R0 = LSHIFT R0 BY -1;               /* n */
    R1 = DM (53,I0);
    R1 = FEXT R1 BY 0:24;
    R2 = 0x148000;
    R4 = R1 - R2;                       /* sine index */
    R5 = DM (5,I0);                     /* step */
    R6 = 0x7FFF;
    R0 = R0 + R0;                       /* 2n samples */
    R1 = DM (-28,I6);
    I4 = R1;                            /* sine */
    R7 = 0;                             /* gain: 0 below Y3 = $19999a */
    R1 = DM (3,I0);
    R2 = 0x19999A;
    COMP (R1,R2);
    IF LT JUMP .L_f_gain;
    R2 = 0x19999A00;
    R7 = R1 * R2 (SSF);
.L_f_gain:
    R1 = md_shared_scratch_x.;
    I1 = R1;
    R0 = PASS R0;
    IF EQ JUMP .L_f_done;
    LCNTR = R0 , DO (PC,.L_f_end)UNTIL LCE;
    M4 = R4;
    R1 = DM (M4,I4);                    /* sine word (24-bit) */
    R1 = LSHIFT R1 BY 8;
    R1 = R1 * R7 (SSF) , R2 = DM (0,I1);
    R2 = R2 + R1;
    R2 = MIN(R2,R14);
    R2 = MAX(R2,R15);
    R4 = R4 + R5 , DM (I1,M6) = R2;
.L_f_end:
    R4 = R4 AND R6;
.L_f_done:
    R2 = 0x148000;
    R4 = R4 + R2;
    DM (53,I0) = R4;                    /* Y35 */

/* ---- G: x:k *= envelope: attack (Y33 != 0: Y32, Y32, Y32+Y33, ..; Y32 +=
 * 32*Y33, done at $7fffff) or decay (Y32, Y32, then e = floor(2*e*x0), x0 =
 * Y2, or $7f8d50 while Y39 < $54) ($14598e-$1459b6) --------------------- */
.L_g:
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R8 = DM (51,I0);                    /* Y33 */
    R9 = DM (50,I0);                    /* a = Y32 */
    R8 = PASS R8;
    IF EQ JUMP .L_g_decay;
    R10 = MIN(R9,R14);
    R10 = MAX(R10,R15);                 /* y1 */
    LCNTR = 0x20 , DO (PC,.L_ga_end)UNTIL LCE;
    R0 = DM (0,I1);
    R11 = LSHIFT R10 BY 8;
    R0 = R0 * R11 (SSF);
    R0 = MIN(R0,R14);
    R10 = MIN(R9,R14) , DM (I1,M6) = R0;
    R10 = MAX(R10,R15);
.L_ga_end:
    R9 = R9 + R8;
    R0 = MIN(R9,R14);
    R0 = MAX(R0,R15);
    DM (50,I0) = R0;                    /* Y32 */
    COMP (R9,R14);
    IF LT JUMP .L_h;
    R0 = 0;
    DM (51,I0) = R0;                    /* Y33 = 0 */
    JUMP .L_h;
.L_g_decay:
    R8 = DM (2,I0);
    R0 = DM (57,I0);                    /* Y39 (this block's) */
    R1 = 0x54;
    R2 = 0x666666;
    COMP (R0,R1);
    IF LT R8 = PASS R2;
    R8 = LSHIFT R8 BY 8;                /* x0 << 8 */
    R10 = MIN(R9,R14);
    R10 = MAX(R10,R15);                 /* y0 = lim(a) */
    /* per sample: m = y0; (a, y0) = (2*y0*x0, lim(a)): each factor twice */
    LCNTR = 0x20 , DO (PC,.L_gd_end)UNTIL LCE;
    R0 = DM (0,I1);
    R11 = LSHIFT R10 BY 8;
    R0 = R0 * R11 (SSF);
    R0 = MIN(R0,R14);
    R12 = MIN(R9,R14) , DM (I1,M6) = R0;
    R12 = MAX(R12,R15);
    R9 = R10 * R8 (SSF);
.L_gd_end:
    R10 = PASS R12;
    R7 = 8;
    R0 = LSHIFT R9 BY R7;
    R7 = -8;
    R0 = ASHIFT R0 BY R7;
    DM (50,I0) = R0;                    /* Y32 = a1 (unlimited) */

/* ---- H: tables, comb delay offsets y:2..5, gains y:0, y:7 ($1459b7-$1459e2) */
.L_h:
    R0 = DM (4,I0);                     /* Y4, or $ccccd while Y39 < $54 */
    R1 = DM (57,I0);
    R2 = 0x54;
    R3 = 0xCCCCD;
    COMP (R1,R2);
    IF LT R0 = PASS R3;
    R1 = 0x1F400;
    R0 = R0 * R1 (SSF);                 /* n0 <= $1f3 */
    R1 = 0x142B49;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R0 = DM (0,I4);
    R0 = LSHIFT R0 BY 8;
    R1 = 0x5A827A;
    R0 = R0 * R1 (SSF);
    R2 = md_shared_scratch_y.;
    I2 = R2;
    DM (0,I2) = R0;                     /* y:0 */
    R0 = DM (6,I0);
    R1 = 0x0CCCCD00;
    R0 = R0 * R1 (SSF);
    R1 = 0x733333;
    R0 = R0 + R1;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    DM (7,I2) = R0;                     /* y:7 */
    R0 = DM (1,I0);                     /* x0 = Y1 */
    R1 = 0xFFFF0700;
    R2 = R0 * R1 (SSF);
    DM (2,I2) = R2;
    R1 = 0xFFFF7B00;
    R2 = R0 * R1 (SSF);
    DM (3,I2) = R2;
    R1 = 0xFFFF8B00;
    R2 = R0 * R1 (SSF);
    DM (4,I2) = R2;
    R1 = 0xFFFFB700;
    R2 = R0 * R1 (SSF);
    DM (5,I2) = R2;
    R2 = 0;
    DM (6,I2) = R2;

/* ---- I: 4 x 8 samples ($1459e3-$145a57). With x1 = y:7, c = $7eb852,
 * g = y:0, s = y:6 (an x scratch index), per sub-block i:
 *  1 lowpass of line Y14 (+y:2) -> x:$89.. (state Y1c), of Y17 (+y:5) ->
 *    x:$c9.. (Y1f);  2 Y16 (+y:4) -> x:$c0.. (Y1e), Y15 (+y:3) -> x:$80.. (Y1d)
 *  3 y:$100+8i+e = lim(2c*x:$c9+e + 2c*x:$c0+e + 2*$ccccd*x:s+e)
 *  4 line Y17 <- lim(-2g*x:$80+e - 2g*x:$c0+e + 2*$666666*x:s+e),
 *    line Y14 <- lim(-2g*x:$80+e + 2g*x:$c0+e + ..)
 *  5 line Y16 <- lim(2g*x:$89+e - 2g*x:$c9+e + ..), Y15 <- lim(.. + ..)
 *  s += 8. The lines are 256-word rings; reads and writes keep the
 * original's order. */
    /* y:$ff goes through an accumulator (the block's first output store
     * re-stores it): a 24-bit word */
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R0 = DM (255,I1);
    R0 = LSHIFT R0 BY 8;
    R1 = -8;
    R0 = ASHIFT R0 BY R1;
    DM (255,I1) = R0;
    R0 = 0;
    DM (-35,I6) = R0;                   /* sub-block */
.L_i_loop:
    R0 = 20;                            /* part 1: Y14 (+y:2), Y1c, x:$89 */
    R1 = 2;
    R2 = 28;
    R3 = 0x89;
    R4 = 23;                            /*         Y17 (+y:5), Y1f, x:$c9 */
    R5 = 5;
    R6 = 31;
    R7 = 0xC9;
    NOP;
    NOP;
    /* (inlined .L_lp2) */
    DM (-32,I6) = R4;
    DM (-33,I6) = R5;
    DM (-34,I6) = R6;
    DM (-36,I6) = R7;
    DM (-37,I6) = R2;
    DM (-38,I6) = R3;
    R2 = md_shared_scratch_y.;
    I4 = R2;
    M4 = R1;
    R1 = DM (M4,I4);
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i5_i1;
.L_line_ret_i5_i1:
    I5 = R12;
    DM (-30,I6) = R10;
    R0 = DM (-32,I6);
    R1 = DM (-33,I6);
    R2 = md_shared_scratch_y.;
    I4 = R2;
    M4 = R1;
    R1 = DM (M4,I4);
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i6_i1;
.L_line_ret_i6_i1:
    I4 = R12;
    R7 = PASS R10;
    R6 = DM (-30,I6);
    /* the two lowpasses one after the other, each in MRF (no background
     * accumulator, and k*(x - lim(a)) as two products: mulforms.py) */
    R0 = DM (-37,I6);
    M4 = R0;
    R0 = DM (M4,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R8 = DM (7,I1);
    R8 = LSHIFT R8 BY 8;                /* x1 << 8 */
    R0 = md_shared_scratch_x.;
    R1 = DM (-38,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I1 = R1;
    R1 = DM (-36,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I2 = R1;
    R9 = 0xFF;
    LCNTR = 8 , DO (PC,.L_lp2_end_i1a)UNTIL LCE;
    R2 = MR1F , M4 = R6;
    R2 = MIN(R2,R14);
    R2 = MAX(R2,R15) , R0 = DM (M4,I5);
    MRF = MRF + R8 * R0 (SSF) , DM (I1,M6) = R2;
    MRF = MRF - R8 * R2 (SSF);
    R6 = R6 + 1;
.L_lp2_end_i1a:
    R6 = R6 AND R9;
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    R1 = DM (-37,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    R0 = DM (-34,I6);
    M4 = R0;
    R0 = DM (M4,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    LCNTR = 8 , DO (PC,.L_lp2_end_i1)UNTIL LCE;
    R3 = MR1F , M4 = R7;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15) , R1 = DM (M4,I4);
    MRF = MRF + R8 * R1 (SSF) , DM (I2,M6) = R3;
    MRF = MRF - R8 * R3 (SSF);
    R7 = R7 + 1;
.L_lp2_end_i1:
    R7 = R7 AND R9;
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    R1 = DM (-34,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    JUMP .L_lp2_ret_i1;
.L_lp2_ret_i1:
    R0 = 22;                            /* part 2: Y16 (+y:4), Y1e, x:$c0 */
    R1 = 4;
    R2 = 30;
    R3 = 0xC0;
    R4 = 21;                            /*         Y15 (+y:3), Y1d, x:$80 */
    R5 = 3;
    R6 = 29;
    R7 = 0x80;
    NOP;
    NOP;
    /* (inlined .L_lp2) */
    DM (-32,I6) = R4;
    DM (-33,I6) = R5;
    DM (-34,I6) = R6;
    DM (-36,I6) = R7;
    DM (-37,I6) = R2;
    DM (-38,I6) = R3;
    R2 = md_shared_scratch_y.;
    I4 = R2;
    M4 = R1;
    R1 = DM (M4,I4);
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i5_i2;
.L_line_ret_i5_i2:
    I5 = R12;
    DM (-30,I6) = R10;
    R0 = DM (-32,I6);
    R1 = DM (-33,I6);
    R2 = md_shared_scratch_y.;
    I4 = R2;
    M4 = R1;
    R1 = DM (M4,I4);
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i6_i2;
.L_line_ret_i6_i2:
    I4 = R12;
    R7 = PASS R10;
    R6 = DM (-30,I6);
    /* the two lowpasses one after the other, each in MRF (no background
     * accumulator, and k*(x - lim(a)) as two products: mulforms.py) */
    R0 = DM (-37,I6);
    M4 = R0;
    R0 = DM (M4,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R8 = DM (7,I1);
    R8 = LSHIFT R8 BY 8;                /* x1 << 8 */
    R0 = md_shared_scratch_x.;
    R1 = DM (-38,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I1 = R1;
    R1 = DM (-36,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I2 = R1;
    R9 = 0xFF;
    LCNTR = 8 , DO (PC,.L_lp2_end_i2a)UNTIL LCE;
    R2 = MR1F , M4 = R6;
    R2 = MIN(R2,R14);
    R2 = MAX(R2,R15) , R0 = DM (M4,I5);
    MRF = MRF + R8 * R0 (SSF) , DM (I1,M6) = R2;
    MRF = MRF - R8 * R2 (SSF);
    R6 = R6 + 1;
.L_lp2_end_i2a:
    R6 = R6 AND R9;
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    R1 = DM (-37,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    R0 = DM (-34,I6);
    M4 = R0;
    R0 = DM (M4,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    LCNTR = 8 , DO (PC,.L_lp2_end_i2)UNTIL LCE;
    R3 = MR1F , M4 = R7;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15) , R1 = DM (M4,I4);
    MRF = MRF + R8 * R1 (SSF) , DM (I2,M6) = R3;
    MRF = MRF - R8 * R3 (SSF);
    R7 = R7 + 1;
.L_lp2_end_i2:
    R7 = R7 AND R9;
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    R1 = DM (-34,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    JUMP .L_lp2_ret_i2;
.L_lp2_ret_i2:
    /* part 3 */
    R0 = md_shared_scratch_x.;
    R1 = 0x324;
    R1 = R1 + R0;
    I1 = R1;                            /* x:$c9 */
    R1 = 0x300;
    R1 = R1 + R0;
    I2 = R1;                            /* x:$c0 */
    R2 = md_shared_scratch_y.;
    I4 = R2;
    R1 = DM (6,I4);                     /* s */
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I3 = R1;                            /* x:s */
    R1 = DM (-35,I6);
    R1 = LSHIFT R1 BY 5;
    R3 = 0x400;
    R1 = R1 + R3;
    R1 = R1 + R2;
    I5 = R1;                            /* y:$100 + 8i */
    R8 = 0x7EB85200;
    R9 = 0x0CCCCD00;
    LCNTR = 8 , DO (PC,.L_p3_end)UNTIL LCE;
    R0 = DM (I1,M6);
    R1 = DM (I2,M6);
    R2 = DM (I3,M6);
    MRF = R8 * R0 (SSF);
    MRF = MRF + R8 * R1 (SSF);
    MRF = MRF + R9 * R2 (SSF);
    R3 = MR1F;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15);
.L_p3_end:
    DM (I5,M6) = R3;
    R0 = 0x80;                          /* part 4: P = x:$80, Q = x:$c0 */
    R1 = 0xC0;
    R2 = 23;                            /* b -> Y17 */
    R3 = 20;                            /* a -> Y14 */
    R4 = -1;                            /* P*-1 */
    NOP;
    NOP;
    /* (inlined .L_wr2) */
    DM (-32,I6) = R0;
    DM (-33,I6) = R1;
    DM (-34,I6) = R3;
    DM (-36,I6) = R4;
    DM (-37,I6) = R2;
    R0 = PASS R2;
    R1 = 0;
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i7_i3;
.L_line_ret_i7_i3:
    I5 = R12;
    R6 = PASS R10;
    DM (-38,I6) = R13;
    R0 = DM (-34,I6);
    R1 = 0;
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i8_i3;
.L_line_ret_i8_i3:
    I4 = R12;
    R7 = PASS R10;
    DM (-30,I6) = R13;
    R0 = md_shared_scratch_x.;
    R1 = DM (-32,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I1 = R1;                            /* P */
    R1 = DM (-33,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I2 = R1;                            /* Q */
    R2 = md_shared_scratch_y.;
    I3 = R2;
    R1 = DM (6,I3);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    R8 = DM (0,I3);
    R8 = LSHIFT R8 BY 8;                /* g << 8 */
    I3 = R1;                            /* x:s */
    R4 = DM (-36,I6);
    R11 = 0x66666600;
    R12 = 0xFF;
    /* the sign on P is this copy's constant (R4 = -1): b, then a,
     * each in MRF with P accumulated subtracted (no background accumulator) */
    LCNTR = 8 , DO (PC,.L_wr2_end_i3)UNTIL LCE;
    R0 = DM (I1,M6);
    R1 = DM (I2,M6);
    R2 = DM (I3,M6);
    MRF = R11 * R2 (SSF);
    MRF = MRF - R8 * R0 (SSF);
    MRF = MRF - R8 * R1 (SSF);
    R3 = MR1F , M4 = R6;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15);
    DM (M4,I5) = R3;
    MRF = R11 * R2 (SSF);
    MRF = MRF - R8 * R0 (SSF);
    MRF = MRF + R8 * R1 (SSF);
    R3 = MR1F , M4 = R7;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15);
    DM (M4,I4) = R3;
    R6 = R6 + 1;
    R6 = R6 AND R12;
    R7 = R7 + 1;
.L_wr2_end_i3:
    R7 = R7 AND R12;
    R0 = DM (-38,I6);
    R0 = R0 + R6;
    R1 = DM (-37,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    R0 = DM (-30,I6);
    R0 = R0 + R7;
    R1 = DM (-34,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    JUMP .L_wr2_ret_i3;
.L_wr2_ret_i3:
    R0 = 0x89;                          /* part 5: P = x:$89, Q = x:$c9 */
    R1 = 0xC9;
    R2 = 22;                            /* b -> Y16 */
    R3 = 21;                            /* a -> Y15 */
    R4 = 1;
    NOP;
    NOP;
    /* (inlined .L_wr2) */
    DM (-32,I6) = R0;
    DM (-33,I6) = R1;
    DM (-34,I6) = R3;
    DM (-36,I6) = R4;
    DM (-37,I6) = R2;
    R0 = PASS R2;
    R1 = 0;
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i7_i4;
.L_line_ret_i7_i4:
    I5 = R12;
    R6 = PASS R10;
    DM (-38,I6) = R13;
    R0 = DM (-34,I6);
    R1 = 0;
    NOP;
    NOP;
    /* (inlined .L_line) */
    M4 = R0;
    R0 = DM (M4,I0);
    R0 = FEXT R0 BY 0:24;
    R10 = 0xFF;
    R11 = R0 AND R10;
    R13 = R0 - R11;
    R12 = DM (-29,I6);
    R12 = R13 - R12;
    R0 = 173;
    R12 = R12 + R0;
    R12 = LSHIFT R12 BY 2;
    R0 = I0;
    R12 = R12 + R0;
    R11 = R11 + R1;
    R10 = R11 AND R10;
    JUMP .L_line_ret_i8_i4;
.L_line_ret_i8_i4:
    I4 = R12;
    R7 = PASS R10;
    DM (-30,I6) = R13;
    R0 = md_shared_scratch_x.;
    R1 = DM (-32,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I1 = R1;                            /* P */
    R1 = DM (-33,I6);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    I2 = R1;                            /* Q */
    R2 = md_shared_scratch_y.;
    I3 = R2;
    R1 = DM (6,I3);
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R0;
    R8 = DM (0,I3);
    R8 = LSHIFT R8 BY 8;                /* g << 8 */
    I3 = R1;                            /* x:s */
    R4 = DM (-36,I6);
    R11 = 0x66666600;
    R12 = 0xFF;
    /* the sign on P is this copy's constant (R4 = 1): b, then a,
     * each in MRF with P accumulated added (no background accumulator) */
    LCNTR = 8 , DO (PC,.L_wr2_end_i4)UNTIL LCE;
    R0 = DM (I1,M6);
    R1 = DM (I2,M6);
    R2 = DM (I3,M6);
    MRF = R11 * R2 (SSF);
    MRF = MRF + R8 * R0 (SSF);
    MRF = MRF - R8 * R1 (SSF);
    R3 = MR1F , M4 = R6;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15);
    DM (M4,I5) = R3;
    MRF = R11 * R2 (SSF);
    MRF = MRF + R8 * R0 (SSF);
    MRF = MRF + R8 * R1 (SSF);
    R3 = MR1F , M4 = R7;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15);
    DM (M4,I4) = R3;
    R6 = R6 + 1;
    R6 = R6 AND R12;
    R7 = R7 + 1;
.L_wr2_end_i4:
    R7 = R7 AND R12;
    R0 = DM (-38,I6);
    R0 = R0 + R6;
    R1 = DM (-37,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    R0 = DM (-30,I6);
    R0 = R0 + R7;
    R1 = DM (-34,I6);
    M4 = R1;
    DM (M4,I0) = R0;
    JUMP .L_wr2_ret_i4;
.L_wr2_ret_i4:
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R0 = DM (6,I1);
    R1 = 8;
    R0 = R0 + R1;
    R1 = 0xFF;
    R0 = R0 AND R1;
    DM (6,I1) = R0;                     /* s += 8 (mod 256) */
    R0 = DM (-35,I6);
    R0 = R0 + 1;
    DM (-35,I6) = R0;
    R1 = 4;
    COMP (R0,R1);
    IF LT JUMP .L_i_loop;
    JUMP .L_j;

    /* R0 = pointer word, R1 = offset: R12 = byte address of the line's first
     * external word, R13 = line base (DSP address), R10 = start index */

    /* parts 1/2: two lowpasses (coefficient y:7), 8 samples. R0..R3 = the
     * first line's pointer word, offset word (y:), state word, output (x:);
     * R4..R7 the second's */

    /* parts 4/5: 8 samples into two lines. R0/R1 = x: of P/Q, R2/R3 = the
     * b/a lines' pointer words, R4 = sign on P: b = 2g*(R4*P - Q) + 2c*x:s,
     * a = 2g*(R4*P + Q) + 2c*x:s, c = $666666; pointers advance 8 */

/* ---- J: x:4..9 = $333333 while Y37 != 0, else 0; x:10..35 = 0 ---------- */
.L_j:
    R0 = md_shared_scratch_x.;
    R1 = 0x10;
    R0 = R0 + R1;
    I1 = R0;
    R1 = DM (55,I0);
    R2 = 0;
    R3 = 0x333333;
    R1 = PASS R1;
    IF NE R2 = PASS R3;
    LCNTR = 6 , DO (PC,.L_j1_end)UNTIL LCE;
    NOP;
    NOP;                                /* (no 2-instruction hardware loops) */
.L_j1_end:
    DM (I1,M6) = R2;
    R2 = 0;
    LCNTR = 0x1A , DO (PC,.L_j2_end)UNTIL LCE;
    NOP;
    NOP;                                /* (no 2-instruction hardware loops) */
.L_j2_end:
    DM (I1,M6) = R2;

/* ---- K: output filter section, state Y2b..Y2f ($1451da-$145216); its
 * coefficient fill stays in y:$28..$2c ----------------------------------- */
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R0 = DM (45,I0);
    DM (0,I1) = R0;                     /* x:0..3 <- Y2d, Y2c, Y2f, Y2e */
    R0 = DM (44,I0);
    DM (1,I1) = R0;
    R0 = DM (47,I0);
    DM (2,I1) = R0;
    R0 = DM (46,I0);
    DM (3,I1) = R0;
    /* coefficients: filter D's table T at 4*Y8 + $145224: T0, T1,
     * lim(2*w*T2), lim(2*w*T3), w = Y3 - $19999a (0 at or below) */
    R0 = DM (8,I0);
    R0 = LSHIFT R0 BY 2;
    R1 = 0x145224;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R0 = DM (3,I0);
    R1 = 0x19999A;
    R0 = R0 - R1;
    R1 = 0;
    R0 = MAX(R0,R1);                    /* w */
    R2 = md_shared_scratch_y.;
    I2 = R2;
    R7 = -8;
    R9 = DM (0,I4);
    R9 = LSHIFT R9 BY 8;
    R1 = ASHIFT R9 BY R7;
    DM (40,I2) = R1;
    R10 = DM (1,I4);
    R10 = LSHIFT R10 BY 8;
    R1 = ASHIFT R10 BY R7;
    DM (41,I2) = R1;
    R11 = DM (2,I4);
    R11 = LSHIFT R11 BY 8;
    R11 = R0 * R11 (SSF);
    R11 = MIN(R11,R14);
    DM (42,I2) = R11;
    DM (44,I2) = R11;
    R11 = LSHIFT R11 BY 8;
    R12 = DM (3,I4);
    R12 = LSHIFT R12 BY 8;
    R12 = R0 * R12 (SSF);
    R12 = MIN(R12,R14);
    DM (43,I2) = R12;
    R12 = LSHIFT R12 BY 8;
    R0 = DM (35,I1);
    DM (46,I0) = R0;                    /* Y2e = x:$23 */
    R0 = DM (34,I1);
    DM (47,I0) = R0;                    /* Y2f = x:$22 */
    R0 = 0;
    R1 = 44;                            /* hi = Y2c, lo = Y2b, s = Y2d */
    R2 = 43;
    R3 = 45;
    NOP;
    NOP;
    /* (inlined .L_section) */
    DM (-32,I6) = R1;
    DM (-33,I6) = R2;
    DM (-34,I6) = R3;
    R4 = md_shared_scratch_x.;
    R5 = LSHIFT R0 BY 2;
    R4 = R4 + R5;
    I1 = R4;                            /* x:r */
    R4 = md_shared_scratch_y.;
    R4 = R4 + R5;
    R6 = 8;
    R4 = R4 + R6;
    I2 = R4;                            /* y:r+2 */
    M4 = R1;
    R4 = DM (M4,I0);                    /* Y(hi) */
    MR1F = R4;
    M4 = R2;
    R4 = DM (M4,I0);                    /* Y(lo) */
    R4 = LSHIFT R4 BY 8;
    MR0F = R4;
    R0 = DM (I1,M6);                    /* x:r */
    R1 = DM (I1,M6);                    /* x:r+1 = x1' */
    MRF = MRF + R9 * R0 (SSF);
    MRF = MRF + R10 * R1 (SSF);
    R2 = DM (0,I1);                     /* x:p */
    R3 = DM (1,I1);                     /* x:p+1 */
    R7 = -8;
    R8 = -1;
    LCNTR = 0x20 , DO (PC,.L_sec_end_i9)UNTIL LCE;
    MRF = MRF + R10 * R1 (SSF) , R4 = DM (2,I1);
    MRF = MRF + R11 * R2 (SSF);
    MRF = MRF + R12 * R3 (SSF);
    MRF = MRF + R11 * R4 (SSF);         /* b */
    R5 = MR1F;
    R6 = MR0F;
    R13 = ASHIFT R6 BY R7;              /* b0 (raw) */
    R0 = MIN(R5,R14);
    R0 = MAX(R0,R15);                   /* lim(b) */
    R6 = PASS R13;
    R5 = R5 - R0;                       /* limited: lo = $ffffff / 0 */
    IF GT R6 = PASS R8;
    IF LT R6 = R6 - R6;
    MRF = MRF + R9 * R1 (SSF) , DM (I1,M6) = R0;
    MRF = MRF + R10 * R0 (SSF) , DM (I2,M6) = R6;
    R1 = PASS R0;
    R2 = PASS R3;
.L_sec_end_i9:
    R3 = PASS R4;
    /* Y(hi) = x:p of j = 31, Y(lo) = b0 (raw), Y(s) = x:p of j = 30 */
    R4 = DM (-32,I6);
    M4 = R4;
    DM (M4,I0) = R1;
    R4 = DM (-33,I6);
    M4 = R4;
    DM (M4,I0) = R13;
    R4 = DM (-2,I1);
    R5 = DM (-34,I6);
    M4 = R5;
    DM (M4,I0) = R4;
    JUMP .L_section_ret_i9;
.L_section_ret_i9:

/* ---- L: the block y:$100+k = lim(2*(y:$100+k + x:4+k)) ($145a93-$145aa3),
 * Y37 = 0; blk44 = y:$100.., the voice's y:$20 history -------------------- */
    R0 = md_shared_scratch_x.;
    R1 = 0x10;
    R0 = R0 + R1;
    I1 = R0;
    R0 = md_shared_scratch_y.;
    R1 = 0x400;
    R0 = R0 + R1;
    I2 = R0;
    R0 = 0x100;
    R1 = I0;
    R0 = R0 + R1;
    I3 = R0;                            /* blk44 */
    LCNTR = 0x20 , DO (PC,.L_l_end)UNTIL LCE;
    R0 = DM (I1,M6);
    R1 = DM (0,I2);
    R1 = R1 + R0;
    R1 = R1 + R1;
    R1 = MIN(R1,R14);
    R1 = MAX(R1,R15);
    DM (I3,M6) = R1;
.L_l_end:
    DM (I2,M6) = R1;
    R0 = 0;
    DM (55,I0) = R0;                    /* Y37 */
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R0 = DM (32,I1);
    DM (1710,I0) = R0;                  /* the voice's y:$20 history */
    R0 = FEXT R0 BY 0:24;
    R1 = md_shared_y20.;
    I1 = R1;
    DM (0,I1) = R0;
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
    JUMP picc_render_block_44k1_c.;
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
