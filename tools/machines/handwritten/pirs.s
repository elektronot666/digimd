/* P-I-RS 44.1 kHz render (pirs_render_block_44k1), hand-written SHARC+.
 *
 * Bit-exact replacement for the translated render of the original OS 1.63
 * DSP56300 code at P:$144b01-$144c98 (see docs/FULL49_HANDWRITTEN.md for the
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
 * pointer outside its own 512-word line or two pointers in one line, a
 * pitch word Y2 < 0 (table index), or the sample phase near the ends of the
 * linear source words.
 *
 * Calling convention as the translated renders: R4 = voice, R8 = sine.
 * @SRC@ = byte address of source word 0 (immutable source base - 0x80000).
 * @TMP@ = a 256-word scratch block owned by the hand-written renders.
 */
.EXTERN md_shared_scratch_x.;
.EXTERN md_shared_scratch_y.;
.EXTERN md_shared_y20.;
.EXTERN pirs_render_block_44k1_c.;
.SECTION/SW seg_swco;
.GLOBAL pirs_render_block_44k1.;
pirs_render_block_44k1.:
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

/* ---- guards: nothing is written before these pass ---------------------- */
    R0 = DM (1709,I0);                  /* virtual voice */
    R1 = 16;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;
    R1 = 0x600;
    R9 = R0 * R1 (SSI);
    R1 = 0x135600;
    R9 = R9 + R1;                       /* R9: the voice's external window */
    DM (-29,I6) = R9;
    R7 = 0x1FF;
    R6 = 0x600;
    /* delay pointer Y14 (step Y17), Y15 (Y18), Y16 (Y19): own window, and a
     * stepped start offset within one wrap */
    R0 = DM (20,I0);
    R0 = FEXT R0 BY 0:24;
    R1 = R0 - R9;
    COMPU (R1,R6);
    IF GE JUMP .L_decline;
    R1 = R0 AND R7;
    DM (-33,I6) = R1;
    R10 = R0 - R1;                      /* window base */
    DM (-30,I6) = R10;
    R2 = DM (23,I0);
    R2 = R2 + R1;
    R3 = 0x200;
    R2 = R2 + R3;
    COMPU (R2,R6);
    IF GE JUMP .L_decline;
    R0 = DM (21,I0);
    R0 = FEXT R0 BY 0:24;
    R1 = R0 - R9;
    COMPU (R1,R6);
    IF GE JUMP .L_decline;
    R1 = R0 AND R7;
    DM (-34,I6) = R1;
    R11 = R0 - R1;
    DM (-31,I6) = R11;
    R2 = DM (24,I0);
    R2 = R2 + R1;
    R2 = R2 + R3;
    COMPU (R2,R6);
    IF GE JUMP .L_decline;
    R0 = DM (22,I0);
    R0 = FEXT R0 BY 0:24;
    R1 = R0 - R9;
    COMPU (R1,R6);
    IF GE JUMP .L_decline;
    R1 = R0 AND R7;
    DM (-35,I6) = R1;
    R12 = R0 - R1;
    DM (-32,I6) = R12;
    R2 = DM (25,I0);
    R2 = R2 + R1;
    R2 = R2 + R3;
    COMPU (R2,R6);
    IF GE JUMP .L_decline;
    COMP (R10,R11);
    IF EQ JUMP .L_decline;
    COMP (R10,R12);
    IF EQ JUMP .L_decline;
    COMP (R11,R12);
    IF EQ JUMP .L_decline;
    R0 = DM (2,I0);                     /* Y2 >= 0: table indices in range */
    R0 = PASS R0;
    IF LT JUMP .L_decline;
    R0 = DM (17,I0);                    /* phase Y11: +-128 inside the source */
    R1 = 0x13B680;
    R0 = R0 - R1;
    R1 = 0xC8FE;
    COMPU (R0,R1);
    IF GE JUMP .L_decline;

/* ---- S1/S2 envelopes and pitch ($144b01-$144b19) ----------------------- */
    R0 = DM (15,I0);
    R1 = 0x7F7CEE00;
    R2 = R0 * R1 (SSF);                 /* Y15' */
    R0 = DM (19,I0);
    R1 = 0x7C28F600;
    R3 = R0 * R1 (SSF);                 /* Y19' */
    DM (15,I0) = R2;
    DM (19,I0) = R3;
    /* the original forms 2*Y3*Y19' + 2*Y5*Y15' + 0.5 and drops it; the pitch
     * product is 0.5*Y1, so y:$fe = Y1 and D = (4*0.5*Y1*2^24) >> 22 = 4*Y1 */
    R1 = DM (1,I0);
    R6 = LSHIFT R1 BY 2;                /* D */
    R0 = md_shared_scratch_y.;
    I2 = R0;
    DM (254,I2) = R1;                   /* y:$fe */

/* ---- S3-S5 phase ramp, interpolation, gain ($144b1a-$144b4c) ----------
 * P = Y11:Y10; x:k = hi(P + kD), clamp at $144efc;
 * y:$40+k = lim(2*s0 + 2*f*(s1 - s0)), s0/s1 = source words at hi, hi+1,
 * f = lo unsigned; x:$40+k = lim((2*Y3*y:$40+k) << 4). y:k and y:$3f, also
 * written by the original here, are overwritten later in the block. */
    R8 = DM (17,I0);
    R10 = DM (16,I0);
    R10 = LSHIFT R10 BY 8;
    R7 = -24;
    R11 = ASHIFT R6 BY R7;
    R12 = LSHIFT R6 BY 8;
    R0 = LSHIFT R6 BY 13;
    R7 = -19;
    R1 = ASHIFT R6 BY R7;
    R0 = R10 + R0;
    R1 = R8 + R1 + CI;                  /* (R1, R0) = P + 32D */
    R4 = 0x144EFC;
    COMP (R1,R4);
    IF GE R0 = R0 - R0;
    IF GE R1 = PASS R4;                 /* clamp: (R1, R0) = new phase */
    R7 = -8;
    R2 = ASHIFT R0 BY R7;
    DM (17,I0) = R1;
    DM (16,I0) = R2;
    R9 = DM (3,I0);
    R9 = LSHIFT R9 BY 8;                /* Y3 << 8 */
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R1 = 0x100;
    R0 = R0 + R1;
    I3 = R0;                            /* x:$40 */
    R0 = md_shared_scratch_y.;
    R0 = R0 + R1;
    I5 = R0;                            /* y:$40 */
    LCNTR = 0x20 , DO (PC,.L_interp_end)UNTIL LCE;
    DM (I1,M6) = R8;
    R0 = LSHIFT R8 BY 2;
    R0 = R0 + R13;
    I4 = R0;
    R1 = DM (0,I4);
    R2 = DM (1,I4);
    R1 = LSHIFT R1 BY 8;                /* s0 << 8 */
    R2 = LSHIFT R2 BY 8;                /* s1 << 8 */
    /* f (24 bits, unsigned) = 2*(f >> 1) + (f & 1): the products are of
     * 24-bit words, in the translator's form (mulforms.py) */
    R3 = LSHIFT R10 BY -9;              /* f >> 1 */
    R6 = FEXT R10 BY 8:1;               /* f & 1 */
    MRF = R2 * R3 (SSF);
    MRF = MRF + R2 * R3 (SSF);
    MRF = MRF + R2 * R6 (SSF);
    MRF = MRF - R1 * R3 (SSF);
    MRF = MRF - R1 * R3 (SSF);
    MRF = MRF - R1 * R6 (SSF);
    R1 = ASHIFT R1 BY R7;
    R1 = R1 + R1;
    R5 = MR1F;
    R5 = R5 + R1;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (I5,M6) = R5;
    R4 = LSHIFT R5 BY 4;
    R4 = R9 * R4 (SSF);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R10 = R10 + R12 , DM (I3,M6) = R4;
.L_interp_end:
    R8 = R8 + R11 + CI;
    /* the original reads x:(x:$20) (prefetch); an unmapped address sets the
     * memory fault flag */
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R0 = DM (32,I1);
    R0 = FEXT R0 BY 0:24;
    NOP;
    NOP;
    /* (inlined .L_xprobe) */
    R1 = 0x800;
    R1 = R0 - R1;
    R2 = 0x400;
    COMPU (R1,R2);
    IF GE JUMP .L_xprobe_ret_i1;
    R2 = DM (1709,I0);
    R2 = LSHIFT R2 BY 6;
    R1 = R1 - R2;
    R2 = 64;
    COMPU (R1,R2);
    IF LT JUMP .L_xprobe_ret_i1;
    R1 = 1;
    DM (108,I0) = R1;
    JUMP .L_xprobe_ret_i1;
.L_xprobe_ret_i1:
    JUMP .L_s6;
    /* R0: an x address read for nothing (a parallel-move read, which reads
     * peripheral space >= $ffff80 as 0): fault iff in $800-$bff outside the
     * voice's own private window */

/* ---- S6 envelopes, tables ($142de0-$142e01) ----------------------------- */
.L_s6:
    R0 = DM (38,I0);
    R1 = 0x7F5C2900;
    R6 = R0 * R1 (SSF);
    DM (38,I0) = R6;                    /* Y26' */
    R1 = DM (7,I0);
    R1 = LSHIFT R1 BY 8;
    R2 = DM (36,I0);
    R2 = R1 * R2 (SSF);
    R2 = MIN(R2,R14);
    DM (36,I0) = R2;                    /* Y24' */
    R1 = DM (37,I0);
    R3 = 0x47AE1400;
    R1 = R1 * R3 (SSF);
    DM (37,I0) = R1;                    /* Y25' */
    R1 = DM (2,I0);
    R3 = 0x6400;
    R1 = R1 * R3 (SSF);                 /* n0 = floor(200*Y2/2^24) <= 99 */
    R3 = 0x142B49;
    R1 = R1 + R3;
    R1 = LSHIFT R1 BY 2;
    R1 = R1 + R13;
    I4 = R1;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R7 = -8;
    R3 = ASHIFT R1 BY R7;
    DM (0xFB,I1) = R3;
    R3 = 0x5A827A;
    R3 = R1 * R3 (SSF);
    DM (0xFC,I1) = R3;
    R3 = 0xD9999A00;
    R3 = R6 * R3 (SSF);
    R4 = 0x400000;
    R3 = R3 + R4;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15);
    DM (0xF9,I1) = R3;                  /* g, the lowpass coefficient */
    R12 = LSHIFT R3 BY 8;               /* R12 = g << 8 for S7-S10 */

/* ---- S7 oscillator lowpass and delay line 1 lowpass ($142e03-$142e18) --- */
    /* x:$60+k from y:$40+k (coefficient Y4), x:$80+k from line Y14 at +Y17 */
    R11 = 0x7FFCB900;                   /* coefficient ffcb9 */
    R0 = DM (-30,I6);                   /* line base -> external word index */
    R9 = DM (-29,I6);
    R0 = R0 - R9;
    R1 = 173;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = I0;
    R0 = R0 + R1;
    I4 = R0;
    R0 = DM (-33,I6);
    R1 = DM (23,I0);
    R0 = R0 + R1;
    R7 = 0x1FF;
    R10 = R0 AND R7;                    /* read offset */
    R0 = md_shared_scratch_x.;
    R1 = 0x180;
    R0 = R0 + R1;
    I1 = R0;                            /* x:$60 */
    R1 = 0x80;
    R0 = R0 + R1;
    I3 = R0;                            /* x:$80 */
    R0 = md_shared_scratch_x.;
    R1 = 0x100;
    R0 = R0 + R1;
    I2 = R0;                            /* x:$40 */
    /* the two lowpasses one after the other, each in MRF (no background accumulator, and
     * k*(x - lim(a)) as two products of 24-bit words: mulforms.py) */
    R0 = DM (18,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    LCNTR = 0x20 , DO (PC,.L_s7a_end)UNTIL LCE;
    R0 = MR1F;
    R0 = MIN(R0,R14) , R1 = DM (I2,M6);
    R0 = MAX(R0,R15);
    MRF = MRF + R11 * R1 (SSF) , DM (I1,M6) = R0;
.L_s7a_end:
    MRF = MRF - R11 * R0 (SSF);
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    DM (18,I0) = R0;                    /* Y12 */
    R0 = DM (29,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    LCNTR = 0x20 , DO (PC,.L_s7_end)UNTIL LCE;
    R2 = MR1F , M4 = R10;
    R2 = MIN(R2,R14) , R5 = DM (M4,I4);
    R2 = MAX(R2,R15);
    MRF = MRF + R12 * R5 (SSF) , DM (I3,M6) = R2;
    MRF = MRF - R12 * R2 (SSF);
    R10 = R10 + 1;
.L_s7_end:
    R10 = R10 AND R7;
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    DM (29,I0) = R0;                    /* Y1d */

/* ---- S8 delay line 2 lowpass ($142e18-$142e26): x:$a0+k ----------------- */
    R0 = DM (30,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    R0 = DM (-31,I6);
    R0 = R0 - R9;
    R1 = 173;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = I0;
    R0 = R0 + R1;
    I4 = R0;
    R0 = DM (-34,I6);
    R1 = DM (24,I0);
    R0 = R0 + R1;
    R10 = R0 AND R7;
    R0 = md_shared_scratch_x.;
    R1 = 0x280;
    R0 = R0 + R1;
    I3 = R0;                            /* x:$a0 */
    LCNTR = 0x20 , DO (PC,.L_s8_end)UNTIL LCE;
    R2 = MR1F , M4 = R10;
    R2 = MIN(R2,R14) , R5 = DM (M4,I4);
    R2 = MAX(R2,R15);
    MRF = MRF + R12 * R5 (SSF) , DM (I3,M6) = R2;
    MRF = MRF - R12 * R2 (SSF);
    R10 = R10 + 1;
.L_s8_end:
    R10 = R10 AND R7;
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    DM (30,I0) = R0;                    /* Y1e */

/* ---- S9/S10 noise, delay line 3 lowpass, noise filter ($142e26-$142e4d) - */
    /* noise s(k+1) = s(k) + s(k-1) mod 2^24, s(-1) = Y22, s(0) = Y23, held
     * << 8; x:$c0+k = delay 3 lowpass, y:k = filtered noise */
    R0 = DM (31,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    R0 = DM (-32,I6);
    R0 = R0 - R9;
    R1 = 173;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = I0;
    R0 = R0 + R1;
    I4 = R0;
    R0 = DM (-35,I6);
    R1 = DM (25,I0);
    R0 = R0 + R1;
    R10 = R0 AND R7;
    R8 = DM (34,I0);
    R8 = LSHIFT R8 BY 8;                /* s(k-1) << 8 */
    R9 = DM (35,I0);
    R9 = LSHIFT R9 BY 8;                /* s(k) << 8 */
    R11 = -8;
    R6 = 0xD9999A00;
    R0 = md_shared_scratch_x.;
    R1 = 0x300;
    R0 = R0 + R1;
    I3 = R0;                            /* x:$c0 */
    R0 = md_shared_scratch_y.;
    I2 = R0;                            /* y:0 */
    LCNTR = 0x20 , DO (PC,.L_s10a_end)UNTIL LCE;
    R2 = MR1F , M4 = R10;
    R2 = MIN(R2,R14) , R5 = DM (M4,I4);
    R2 = MAX(R2,R15);
    MRF = MRF + R12 * R5 (SSF) , DM (I3,M6) = R2;
    MRF = MRF - R12 * R2 (SSF);
    R10 = R10 + 1;
.L_s10a_end:
    R10 = R10 AND R7;
    R1 = MR1F;
    R1 = MIN(R1,R14);
    R1 = MAX(R1,R15);
    DM (31,I0) = R1;                    /* Y1f */
    /* noise filter b (Y27): b += s(k) at A1 (an add to MR1, not a multiply
     * by 2^23), then b += lim(b) * R6 */
    R0 = DM (39,I0);
    MR1F = R0;
    R0 = 0;
    MR0F = R0;
    LCNTR = 0x20 , DO (PC,.L_s10_end)UNTIL LCE;
    R1 = MR1F;
    R3 = MIN(R1,R14);
    R3 = MAX(R3,R15);
    R2 = ASHIFT R9 BY R11;
    R1 = R1 + R2 , DM (I2,M6) = R3;
    MR1F = R1;
    MRF = MRF + R6 * R3 (SSF);
    R0 = R9 + R8;
    R8 = PASS R9;
.L_s10_end:
    R9 = PASS R0;
    DM (-36,I6) = R3;                   /* the last lim(b): x0 for S11 */
    R7 = -8;
    R0 = ASHIFT R9 BY R7;
    DM (34,I0) = R0;                    /* Y22 = s(32) */
    R0 = ASHIFT R8 BY R7;
    DM (35,I0) = R0;                    /* Y23 = s(31) */
    R1 = md_shared_scratch_y.;
    I2 = R1;
    DM (250,I2) = R0;                   /* y:$fa */
    R1 = MR1F;
    R1 = MIN(R1,R14);
    R1 = MAX(R1,R15);
    DM (39,I0) = R1;                    /* Y27 */

/* ---- S11 noise level trigger, noise mix ($144bbf-$144bdc) --------------- */
    R0 = ABS R0;
    R1 = DM (36,I0);                    /* Y24 */
    R2 = DM (-36,I6);
    COMP (R0,R1);
    IF LT JUMP .L_s11_keep;
    R2 = PASS R14;
    DM (37,I0) = R14;                   /* Y25 = $7fffff */
.L_s11_keep:
    R1 = LSHIFT R1 BY 8;
    R2 = R1 * R2 (SSF);
    R2 = MIN(R2,R14);
    R1 = DM (6,I0);
    R1 = LSHIFT R1 BY 8;
    R2 = R1 * R2 (SSF);
    R2 = MIN(R2,R14);
    R1 = 0x26666600;
    R3 = R2 * R1 (SSF);
    R8 = LSHIFT R3 BY 8;                /* x1 << 8 */
    /* x:$40+k and x:$60+k each += 2*x1*y:k (limited) */
    R0 = md_shared_scratch_x.;
    R1 = 0x180;
    R0 = R0 + R1;
    I1 = R0;                            /* x:$60 */
    R1 = 0x80;
    R0 = R0 - R1;
    I3 = R0;                            /* x:$40 */
    R0 = md_shared_scratch_y.;
    I2 = R0;                            /* y:0 */
    LCNTR = 0x20 , DO (PC,.L_s11_end)UNTIL LCE;
    R0 = DM (I2,M6);
    R1 = R8 * R0 (SSF) , R2 = DM (0,I1);
    R3 = DM (0,I3);
    R2 = R2 + R1;
    R2 = MIN(R2,R14);
    R2 = MAX(R2,R15);
    R3 = R3 + R1 , DM (I1,M6) = R2;
    R3 = MIN(R3,R14);
    R3 = MAX(R3,R15);
.L_s11_end:
    DM (I3,M6) = R3;

/* ---- S12/S13 junction ($142e72-$142eb3) ----------------------------------
 * o = x:$60+k, d1..d3 = x:$80/$a0/$c0+k, c = $7eb852:
 *   delay 3 <- lim(2*o*c + 2*x:$fb*d1)
 *   y:k     <- lim(2*o*c + 2*x:$fc*(d2+d3)) -> delay 2
 *   y:$20+k <- lim(2*o*c + 2*x:$fc*(d3-d2)) -> delay 1 */
    R0 = md_shared_scratch_x.;
    R1 = 0x3EC;
    R1 = R1 + R0;
    I1 = R1;
    R8 = DM (0,I1);
    R8 = LSHIFT R8 BY 8;                /* x:$fb << 8 */
    R9 = DM (1,I1);
    R9 = LSHIFT R9 BY 8;                /* x:$fc << 8 */
    R10 = 0x7EB85200;
    R1 = 0x180;
    R0 = R0 + R1;
    I1 = R0;                            /* x:$60 */
    M0 = 32;
    M1 = 64;
    M2 = 96;
    M3 = 32;
    R0 = md_shared_scratch_y.;
    I2 = R0;                            /* y:0 */
    R1 = 0x100;
    DM (255,I2) = R1;                   /* y:$ff = r7 */
    R6 = DM (-29,I6);
    R7 = 0x1FF;
    R0 = DM (-30,I6);
    R0 = R0 - R6;
    R1 = 173;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = I0;
    R0 = R0 + R1;
    I4 = R0;                            /* line 1 */
    R0 = DM (-31,I6);
    R0 = R0 - R6;
    R1 = 173;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = I0;
    R0 = R0 + R1;
    I5 = R0;                            /* line 2 */
    R0 = DM (-32,I6);
    R0 = R0 - R6;
    R1 = 173;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 2;
    R1 = I0;
    R0 = R0 + R1;
    I3 = R0;                            /* line 3 */
    R11 = DM (-33,I6);                  /* write offsets */
    R12 = DM (-34,I6);
    R13 = DM (-35,I6);
    LCNTR = 0x20 , DO (PC,.L_s12_end)UNTIL LCE;
    R1 = DM (M0,I1);                    /* d1 */
    R2 = DM (M1,I1);                    /* d2 */
    R3 = DM (M2,I1);                    /* d3 */
    R0 = DM (I1,M6);                    /* o */
    MRF = R10 * R0 (SSF);
    MRF = MRF + R8 * R1 (SSF);
    R4 = MR1F , M4 = R13;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (M4,I3) = R4;                    /* delay 3 */
    MRF = R10 * R0 (SSF);
    MRF = MRF + R9 * R2 (SSF);
    MRF = MRF + R9 * R3 (SSF);
    R4 = MR1F , M4 = R12;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (M4,I5) = R4;                    /* delay 2 */
    MRF = R10 * R0 (SSF);
    MRF = MRF + R9 * R3 (SSF);
    MRF = MRF - R9 * R2 (SSF);
    R5 = MR1F , M4 = R11;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (M4,I4) = R5;                    /* delay 1 */
    DM (M3,I2) = R5;                    /* y:$20+k */
    DM (I2,M6) = R4;                    /* y:k */
    R11 = R11 + 1;
    R11 = R11 AND R7;
    R12 = R12 + 1;
    R12 = R12 AND R7;
    R13 = R13 + 1;
.L_s12_end:
    R13 = R13 AND R7;
    /* pointers Y14..Y16 advance 32 in their lines */
    R0 = DM (-30,I6);
    R0 = R0 + R11;
    DM (20,I0) = R0;
    R0 = DM (-31,I6);
    R0 = R0 + R12;
    DM (21,I0) = R0;
    R0 = DM (-32,I6);
    R0 = R0 + R13;
    DM (22,I0) = R0;

/* ---- S14 body ($144c1c-$144c2d): x:$60+k, Y20, Y21 ----------------------
 * x:$60+k = v(k-1), v(-1) = Y20; S = d1+d2+d3;
 * v(k) = lim(b); b += S - y0; y0 = lim(S); b -= 2*v(k)*Y1c */
    R0 = md_shared_scratch_x.;
    R1 = 0x180;
    R0 = R0 + R1;
    I1 = R0;                            /* x:$60 */
    M0 = 32;
    M1 = 64;
    M2 = 96;
    R8 = DM (32,I0);                    /* Y20: v(-1) */
    MR1F = R8;
    R0 = 0;
    MR0F = R0;
    R9 = DM (33,I0);                    /* Y21: y0 */
    R10 = DM (28,I0);
    R10 = LSHIFT R10 BY 8;              /* Y1c << 8 */
    LCNTR = 0x20 , DO (PC,.L_s14_end)UNTIL LCE;
    R1 = DM (M0,I1);
    R2 = DM (M1,I1);
    R3 = DM (M2,I1);
    DM (I1,M6) = R8;                    /* x:$60+k = v(k-1) */
    R1 = R1 + R2;
    R1 = R1 + R3;                       /* S */
    R2 = R1 - R9;                       /* S - y0 */
    R9 = MIN(R1,R14);
    R9 = MAX(R9,R15);                   /* y0 = lim(S) */
    R3 = MR1F;
    R8 = MIN(R3,R14);
    R8 = MAX(R8,R15);                   /* v(k) */
    R3 = R3 + R2;
    MR1F = R3;
.L_s14_end:
    MRF = MRF - R10 * R8 (SSF);
    DM (33,I0) = R9;
    R0 = MR1F;
    R0 = MIN(R0,R14);
    R0 = MAX(R0,R15);
    DM (32,I0) = R0;
/* ---- S15 mode tuning: y:$b4+i = lim(2*T(i)*(y:$fe | Y1)), T at x:$144c9a */
/* ---- S16 cosine polynomial: y:$a0+i -------------------------------- */
    R0 = md_shared_scratch_y.;
    I2 = R0;
    R8 = DM (254,I2);                   /* y:$fe */
    R9 = DM (1,I0);                     /* Y1 */
    R12 = 0x15555500;
    R13 = 0x7FFFFF;
    R1 = 0x02B54200;
    R0 = R8 * R1 (SSF);
    DM (180,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (160,I2) = R5;
    R1 = 0x04607000;
    R0 = R8 * R1 (SSF);
    DM (181,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (161,I2) = R5;
    R1 = 0x05482F00;
    R0 = R8 * R1 (SSF);
    DM (182,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (162,I2) = R5;
    R1 = 0x07BE0F00;
    R0 = R8 * R1 (SSF);
    DM (183,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (163,I2) = R5;
    R1 = 0x0C5F3400;
    R0 = R9 * R1 (SSF);
    DM (184,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (164,I2) = R5;
    R1 = 0x0C955800;
    R0 = R9 * R1 (SSF);
    DM (185,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (165,I2) = R5;
    R1 = 0x12E4F800;
    R0 = R9 * R1 (SSF);
    DM (186,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (166,I2) = R5;
    R1 = 0x13D74700;
    R0 = R9 * R1 (SSF);
    DM (187,I2) = R0;
    R1 = -R0;
    R2 = LSHIFT R0 BY 8;
    MRF = R2 * R1 (SSF);                /* -2q^2 */
    R3 = MR1F;                          /* yq */
    R4 = R3 * R12 (SSF);                /* lim(2*$155555*yq) */
    R3 = LSHIFT R3 BY 8;
    MRF = MRF + R3 * R4 (SSF);
    R5 = MR1F;
    R5 = R5 + R13;
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (167,I2) = R5;
/* ---- S17 mode gains: x:0..3 = y:$144caa.., x:4..7 = lim(2*Y8*g) */
    R0 = md_shared_scratch_x.;
    I1 = R0;
    R0 = 0xFFEB3333;
    DM (0,I1) = R0;
    R0 = 0xFFF2B021;
    DM (1,I1) = R0;
    R0 = 0xFFF95810;
    DM (2,I1) = R0;
    R0 = 0xFFFCAC08;
    DM (3,I1) = R0;
    R9 = DM (8,I0);                     /* Y8 */
    R1 = 0xFDB22D00;
    R0 = R9 * R1 (SSF);
    DM (4,I1) = R0;
    R1 = 0xFED91700;
    R0 = R9 * R1 (SSF);
    DM (5,I1) = R0;
    R1 = 0xFFC50500;
    R0 = R9 * R1 (SSF);
    DM (6,I1) = R0;
    R1 = 0xFFF14100;
    R0 = R9 * R1 (SSF);
    DM (7,I1) = R0;
/* ---- S18 mode coefficients: y:$64+4i+0..2, indices from x:$144ca2 */
    R9 = DM (2,I0);                     /* Y2 >= 0 */
    R10 = 0x800000;
    R11 = 0x142949;
    R7 = -8;
    R1 = 0x00000D00;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (160,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (101,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (100,I2) = R5;
    R5 = DM (0,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (102,I2) = R5;
    R1 = 0x00002200;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (161,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (105,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (104,I2) = R5;
    R5 = DM (1,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (106,I2) = R5;
    R1 = 0x00002500;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (162,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (109,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (108,I2) = R5;
    R5 = DM (2,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (110,I2) = R5;
    R1 = 0x00001A00;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (163,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (113,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (112,I2) = R5;
    R5 = DM (3,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (114,I2) = R5;
    R1 = 0x00001500;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (164,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (117,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (116,I2) = R5;
    R5 = DM (4,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (118,I2) = R5;
    R1 = 0x00010000;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (165,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (121,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (120,I2) = R5;
    R5 = DM (5,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (122,I2) = R5;
    R1 = 0x00010000;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (166,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (125,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (124,I2) = R5;
    R5 = DM (6,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (126,I2) = R5;
    R1 = 0x0001FF00;
    R0 = R9 * R1 (SSF);                /* n0 */
    R0 = R0 + R11;
    R0 = LSHIFT R0 BY 2;
    R1 = DM (-39,I6);
    R0 = R0 + R1;
    I4 = R0;
    R1 = DM (0,I4);
    R1 = LSHIFT R1 BY 8;                /* x1 << 8 */
    R2 = ASHIFT R1 BY R7;               /* x1 */
    R3 = DM (167,I2);
    MRF = R1 * R3 (SSF);
    MRF = MRF + R1 * R3 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    DM (129,I2) = R4;
    MRF = MRF - R1 * R2 (SSF);
    R4 = MR1F;
    R4 = R4 - R10;
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);                   /* lim(a) */
    R5 = -R2;
    R5 = R1 * R5 (SSF);                /* -2*x1^2 */
    DM (128,I2) = R5;
    R5 = DM (7,I1);
    R5 = LSHIFT R5 BY 8;
    R4 = LSHIFT R4 BY 1;
    R5 = R5 * R4 (SSF);
    R5 = MIN(R5,R14);
    R5 = MAX(R5,R15);
    DM (130,I2) = R5;

/* ---- S19 modal bank ($142f14-$142f32) ------------------------------------
 * 8 modes i: state a = X:Y(r6+$29+2i), x1 = X(r6+$2a+2i), coefficients
 * c0..c2 = y:$64+4i..: per sample a += 2*x1*c0 + 2*lim(a)*c1 + 2*x(k)*c2,
 * then x1 = lim(a) from before, x(k) = x:$60+k. The modes then sum into
 * x:$40+k in order, s = lim(s + floor(a)); x:$3f ends as the sum after
 * mode 6 at k = 31 (each mode starts by writing the previous mode's last
 * sum there).
 * Pass 1, SIMD: modes 2j (PEx) and 2j+1 (PEy) together, x(k) broadcast;
 * floor(a) of sample k goes to @TMP@[8k + 2j], [8k + 2j + 1].
 * Pass 2, SISD: per sample the 8 saturating sums. */
    R14 = 0x7FFFFF;
    S14 = R14;
    S15 = R15;
    R7 = -8;
    S7 = R7;
    R8 = -1;
    S8 = R8;
    R0 = md_shared_scratch_y.;
    R1 = 0x190;
    R0 = R0 + R1;
    I5 = R0;                            /* y:$64 */
    R0 = 0x25C;
    R1 = I0;
    R0 = R0 + R1;
    I4 = R0;                            /* voice word 151 = X(r6+$2a) */
    R0 = @TMP@;
    DM (-40,I6) = R0;
    M4 = 8;
    LCNTR = 4 , DO (PC,.L_pair_end)UNTIL LCE;
    R0 = DM (0,I5);
    R9 = LSHIFT R0 BY 8;                /* c0 << 8 */
    R0 = DM (1,I5);
    R10 = LSHIFT R0 BY 8;               /* c1 << 8 */
    R0 = DM (2,I5);
    R11 = LSHIFT R0 BY 8;               /* c2 << 8 */
    R0 = DM (4,I5);
    R0 = LSHIFT R0 BY 8;
    S9 = R0;
    R0 = DM (5,I5);
    R0 = LSHIFT R0 BY 8;
    S10 = R0;
    R0 = DM (6,I5);
    R0 = LSHIFT R0 BY 8;
    S11 = R0;
    R0 = DM (0,I4);                     /* a hi */
    S0 = DM (2,I4);
    R1 = DM (-109,I4);                  /* a lo: Y(r6+$29+2i) */
    R1 = LSHIFT R1 BY 8;
    R2 = DM (-107,I4);
    R2 = LSHIFT R2 BY 8;
    S1 = R2;
    R2 = DM (1,I4);                     /* x1 */
    S2 = DM (3,I4);
    R3 = md_shared_scratch_x.;
    R4 = 0x180;
    R3 = R3 + R4;
    I1 = R3;                            /* x:$60, broadcast */
    R3 = DM (-40,I6);
    I3 = R3;                            /* @TMP@ + 2j */
    R4 = 8;
    R3 = R3 + R4;
    DM (-40,I6) = R3;
    /* (was SIMD) PEx pass */
    DM (-41,I6) = I1;
    DM (-42,I6) = I3;
    MR0F = R1;
    MR1F = R0;
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R2 (SSF);
    R1 = MIN(R0,R14);
    R1 = MAX(R1,R15);
    MRF = MRF + R10 * R1 (SSF);
    MRF = MRF + R11 * R3 (SSF);
    LCNTR = 0xF , DO (PC,.L_res_end)UNTIL LCE;
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R1 (SSF) , DM (I3,M4) = R0;
    R2 = MIN(R0,R14);
    R2 = MAX(R2,R15);
    MRF = MRF + R10 * R2 (SSF);
    MRF = MRF + R11 * R3 (SSF);
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R2 (SSF) , DM (I3,M4) = R0;
    R1 = MIN(R0,R14);
    R1 = MAX(R1,R15);
    MRF = MRF + R10 * R1 (SSF);
.L_res_end:
    MRF = MRF + R11 * R3 (SSF);
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R1 (SSF) , DM (I3,M4) = R0;
    R2 = MIN(R0,R14);
    R2 = MAX(R2,R15);
    MRF = MRF + R10 * R2 (SSF);
    MRF = MRF + R11 * R3 (SSF);
    R0 = MR1F;
    DM (I3,M4) = R0;                    /* floor(a) of k = 31 */
    /* state: a limited to 48 bits, x1 */
    R5 = MR0F;
    R5 = LSHIFT R5 BY R7;               /* lo (A0, 24 bits) */
    R5 = LSHIFT R5 BY 8;
    R5 = ASHIFT R5 BY R7;
    COMP (R0,R14);
    IF GT R5 = PASS R8;
    COMP (R0,R15);
    IF LT R5 = R5 - R5;
    R1 = MIN(R0,R14);
    R1 = MAX(R1,R15);
    R0 <-> S0;
    R1 <-> S1;
    R2 <-> S2;
    R3 <-> S3;
    R4 <-> S4;
    R5 <-> S5;
    R6 <-> S6;
    R7 <-> S7;
    R8 <-> S8;
    R9 <-> S9;
    R10 <-> S10;
    R11 <-> S11;
    R12 <-> S12;
    R13 <-> S13;
    R14 <-> S14;
    R15 <-> S15;
    /* PEy pass */
    I1 = DM (-41,I6);
    I3 = DM (-42,I6);
    I3=MODIFY (I3,0x1)(NW);
    MR0F = R1;
    MR1F = R0;
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R2 (SSF);
    R1 = MIN(R0,R14);
    R1 = MAX(R1,R15);
    MRF = MRF + R10 * R1 (SSF);
    MRF = MRF + R11 * R3 (SSF);
    LCNTR = 0xF , DO (PC,.L_res_end_y)UNTIL LCE;
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R1 (SSF) , DM (I3,M4) = R0;
    R2 = MIN(R0,R14);
    R2 = MAX(R2,R15);
    MRF = MRF + R10 * R2 (SSF);
    MRF = MRF + R11 * R3 (SSF);
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R2 (SSF) , DM (I3,M4) = R0;
    R1 = MIN(R0,R14);
    R1 = MAX(R1,R15);
    MRF = MRF + R10 * R1 (SSF);
.L_res_end_y:
    MRF = MRF + R11 * R3 (SSF);
    R0 = MR1F , R3 = DM (I1,M6);
    MRF = MRF + R9 * R1 (SSF) , DM (I3,M4) = R0;
    R2 = MIN(R0,R14);
    R2 = MAX(R2,R15);
    MRF = MRF + R10 * R2 (SSF);
    MRF = MRF + R11 * R3 (SSF);
    R0 = MR1F;
    DM (I3,M4) = R0;                    /* floor(a) of k = 31 */
    /* state: a limited to 48 bits, x1 */
    R5 = MR0F;
    R5 = LSHIFT R5 BY R7;               /* lo (A0, 24 bits) */
    R5 = LSHIFT R5 BY 8;
    R5 = ASHIFT R5 BY R7;
    COMP (R0,R14);
    IF GT R5 = PASS R8;
    COMP (R0,R15);
    IF LT R5 = R5 - R5;
    R1 = MIN(R0,R14);
    R1 = MAX(R1,R15);
    R0 <-> S0;
    R1 <-> S1;
    R2 <-> S2;
    R3 <-> S3;
    R4 <-> S4;
    R5 <-> S5;
    R6 <-> S6;
    R7 <-> S7;
    R8 <-> S8;
    R9 <-> S9;
    R10 <-> S10;
    R11 <-> S11;
    R12 <-> S12;
    R13 <-> S13;
    R14 <-> S14;
    R15 <-> S15;
    I3=MODIFY (I3,-0x1)(NW);
    DM (0,I4) = R1;
    DM (2,I4) = S1;
    DM (-109,I4) = R5;
    DM (-107,I4) = S5;
    DM (1,I4) = R2;
    DM (3,I4) = S2;
    I5=MODIFY (I5,0x8)(NW);
.L_pair_end:
    I4=MODIFY (I4,0x4)(NW);
    /* pass 2: per sample, s = lim(s + floor(a_i)) for i = 0..7 */
    R0 = md_shared_scratch_x.;
    R1 = 0x100;
    R1 = R1 + R0;
    I2 = R1;                            /* x:$40 */
    R1 = 0xFC;
    R1 = R1 + R0;
    I4 = R1;                            /* x:$3f */
    R0 = @TMP@;
    I3 = R0;
    LCNTR = 0x20 , DO (PC,.L_sum_end)UNTIL LCE;
    R4 = DM (0,I2);
    R5 = DM (0,I3);
    R4 = R4 + R5 , R5 = DM (1,I3);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R4 = R4 + R5 , R5 = DM (2,I3);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R4 = R4 + R5 , R5 = DM (3,I3);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R4 = R4 + R5 , R5 = DM (4,I3);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R4 = R4 + R5 , R5 = DM (5,I3);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R4 = R4 + R5 , R5 = DM (6,I3);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R4 = R4 + R5 , R5 = DM (7,I3);
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    R4 = R4 + R5 , DM (0,I4) = R4;      /* x:$3f <- sum after mode 6 */
    R4 = MIN(R4,R14);
    R4 = MAX(R4,R15);
    I3=MODIFY (I3,0x8)(NW);
.L_sum_end:
    DM (I2,M6) = R4;

/* ---- the block: blk44 = x:$40.., y:$20 history ------------------------- */
    R0 = md_shared_scratch_x.;
    R1 = 0x100;
    R0 = R0 + R1;
    I1 = R0;
    R0 = 0x100;
    R1 = I0;
    R0 = R0 + R1;
    I2 = R0;                            /* voice word 64: blk44 */
    LCNTR = 0x10 , DO (PC,.L_out_end)UNTIL LCE;
    R0 = DM (I1,M6);
    R1 = DM (I1,M6);
    DM (I2,M6) = R0;
.L_out_end:
    DM (I2,M6) = R1;
    R0 = md_shared_scratch_y.;
    I1 = R0;
    R0 = DM (32,I1);
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
    JUMP pirs_render_block_44k1_c.;
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
