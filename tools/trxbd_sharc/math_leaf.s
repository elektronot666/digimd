/* TRX-BD-only original synthesis stages. Shared leaves are linked once. */
.SECTION/SW seg_swco;
.GLOBAL trxbd_amp_clip.;
trxbd_amp_clip.:
    /* Original MD P:$1004D4..$10050B, 32 samples. R4=voice, R8=shaped,
     * R12=noise, fourth stack argument=noise_silent. Return final envelope.
     * Dispatch ONCE on block-constant noise/curve flags. Four loops retain
     * the same arithmetic/sample order; no silent-buffer reads occur.
     * R9=out, R10=noise, R11=envelope, R12=decay, R13/R14=bounds,
     * R15=curve<<8. Preserve all callee-saved registers and accumulators.
     *
     * Canonical signed24 a<<8 fits signed32. SSF then returns exactly
     * floor((a<<8)*b/2^31) = floor(a*b/2^23), even for signed25 b.
     * SSI sees P<<8: FEXT 7:24 retains exactly (2*P)&0xffffff for the
     * original full-product carry. Products remain within signed64. */
    I7=MODIFY (I7,-0x8)(NW);
    DM(-0x3,I6) = R8;
    DM(-0x4,I6) = R9;
    DM(-0x5,I6) = R10;
    DM(-0x6,I6) = R11;
    DM(-0x7,I6) = R12;
    DM(-0x8,I6) = R13;
    DM(-0x9,I6) = R14;
    DM(-0xA,I6) = R15;
    I4 = I6;
    I4=MODIFY (I4,0x1)(NW);
    R3 = DM(I4,M5);
    R10 = PASS R12;
    R0 = 144;
    R9 = R4 + R0;
    I4 = R4;
    I4=MODIFY (I4,0x1)(NW);
    R12 = DM(I4,M5);
    I4=MODIFY (I4,0x7)(NW);
    R13 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R14 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R15 = DM(I4,M5);
    I4=MODIFY (I4,0x7)(NW);
    R11 = DM(I4,M5);
    R15 = LSHIFT R15 BY 8;
    R0 = 0;
    COMP(R15,R0);
    IF EQ JUMP .L_amp_linear_select;
    COMP(R3,R0);
    IF NE JUMP .L_amp_silent_curve;
    JUMP .L_amp_noise_curve;
.L_amp_linear_select:
    COMP(R3,R0);
    IF NE JUMP .L_amp_silent_linear;

.L_amp_noise_linear:
    /* Active noise, zero curve: exact original stages 8-10. */
    LCNTR = 32, DO .L_amp_noise_linear_end UNTIL LCE;
    I4 = R8;
    R4 = DM(I4,M5);
    I4 = R10;
    R0 = DM(I4,M5);
    R4 = R4 + R0;
    R5 = LSHIFT R11 BY 8;
    R0 = R5 * R4 (SSF);
    R1 = 0x7fffff;
    R6 = MIN(R0,R1);
    R1 = -0x800000;
    R6 = MAX(R6,R1);
    R11 = R5 * R12 (SSF);
    R7 = PASS R6;
    COMP(R6,R13);
    IF GE JUMP .L_amp_noise_linear_high;
    COMP(R6,R14);
    IF GT JUMP .L_amp_noise_linear_no_excess;
    R4 = R6 - R14;
    R7 = PASS R14;
    JUMP .L_amp_noise_linear_excess;
.L_amp_noise_linear_high:
    R4 = R6 - R13;
    R7 = PASS R13;
.L_amp_noise_linear_excess:
    R1 = 0x7fffff;
    R4 = MIN(R4,R1);
    R1 = -0x800000;
    R4 = MAX(R4,R1);
    R4 = LSHIFT R4 BY 8;
    R0 = R4 * R13 (SSF);
    R7 = R7 + R0;
.L_amp_noise_linear_no_excess:
    R0 = PASS R7;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    R0 = R0 + R6;
    R0 = LSHIFT R0 BY 1;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    I4 = R9;
    DM(I4,M5) = R0;
    R1 = 4;
    R8 = R8 + R1;
    R9 = R9 + R1;
.L_amp_noise_linear_end:
    R10 = R10 + R1;
    JUMP .L_amp_done;

.L_amp_silent_linear:
    /* Silent noise, zero curve: exact original stages 8-10. */
    LCNTR = 32, DO .L_amp_silent_linear_end UNTIL LCE;
    I4 = R8;
    R4 = DM(I4,M5);
    R5 = LSHIFT R11 BY 8;
    R0 = R5 * R4 (SSF);
    R1 = 0x7fffff;
    R6 = MIN(R0,R1);
    R1 = -0x800000;
    R6 = MAX(R6,R1);
    R11 = R5 * R12 (SSF);
    R7 = PASS R6;
    COMP(R6,R13);
    IF GE JUMP .L_amp_silent_linear_high;
    COMP(R6,R14);
    IF GT JUMP .L_amp_silent_linear_no_excess;
    R4 = R6 - R14;
    R7 = PASS R14;
    JUMP .L_amp_silent_linear_excess;
.L_amp_silent_linear_high:
    R4 = R6 - R13;
    R7 = PASS R13;
.L_amp_silent_linear_excess:
    R1 = 0x7fffff;
    R4 = MIN(R4,R1);
    R1 = -0x800000;
    R4 = MAX(R4,R1);
    R4 = LSHIFT R4 BY 8;
    R0 = R4 * R13 (SSF);
    R7 = R7 + R0;
.L_amp_silent_linear_no_excess:
    R0 = PASS R7;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    R0 = R0 + R6;
    R0 = LSHIFT R0 BY 1;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    I4 = R9;
    DM(I4,M5) = R0;
    R1 = 4;
    R8 = R8 + R1;
.L_amp_silent_linear_end:
    R9 = R9 + R1;
    JUMP .L_amp_done;

.L_amp_noise_curve:
    /* Active noise, nonzero curve: exact original stages 8-10. */
    LCNTR = 32, DO .L_amp_noise_curve_end UNTIL LCE;
    I4 = R8;
    R4 = DM(I4,M5);
    I4 = R10;
    R0 = DM(I4,M5);
    R4 = R4 + R0;
    R5 = LSHIFT R11 BY 8;
    R0 = R5 * R4 (SSF);
    R1 = 0x7fffff;
    R6 = MIN(R0,R1);
    R1 = -0x800000;
    R6 = MAX(R6,R1);
    R11 = R5 * R12 (SSF);
    R7 = PASS R6;
    R5 = 0;
    COMP(R6,R13);
    IF GE JUMP .L_amp_noise_curve_high;
    COMP(R6,R14);
    IF GT JUMP .L_amp_noise_curve_no_excess;
    R4 = R6 - R14;
    R7 = PASS R14;
    JUMP .L_amp_noise_curve_excess;
.L_amp_noise_curve_high:
    R4 = R6 - R13;
    R7 = PASS R13;
.L_amp_noise_curve_excess:
    R1 = 0x7fffff;
    R4 = MIN(R4,R1);
    R1 = -0x800000;
    R4 = MAX(R4,R1);
    R4 = LSHIFT R4 BY 8;
    R0 = R4 * R13 (SSF);
    R3 = R4 * R13 (SSI);
    R7 = R7 + R0;
    R5 = FEXT R3 BY 7:24;
.L_amp_noise_curve_no_excess:
    R1 = 0x7fffff;
    R4 = MIN(R7,R1);
    R1 = -0x800000;
    R4 = MAX(R4,R1);
    R0 = R15 * R4 (SSF);
    R3 = R15 * R4 (SSI);
    R3 = FEXT R3 BY 7:24;
    R5 = R5 + R3;
    R5 = FEXT R5 BY 24:8;
    R0 = R0 + R7;
    R0 = R0 + R5;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    R0 = R0 + R6;
    R0 = LSHIFT R0 BY 1;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    I4 = R9;
    DM(I4,M5) = R0;
    R1 = 4;
    R8 = R8 + R1;
    R9 = R9 + R1;
.L_amp_noise_curve_end:
    R10 = R10 + R1;
    JUMP .L_amp_done;

.L_amp_silent_curve:
    /* Silent noise, nonzero curve: exact original stages 8-10. */
    LCNTR = 32, DO .L_amp_silent_curve_end UNTIL LCE;
    I4 = R8;
    R4 = DM(I4,M5);
    R5 = LSHIFT R11 BY 8;
    R0 = R5 * R4 (SSF);
    R1 = 0x7fffff;
    R6 = MIN(R0,R1);
    R1 = -0x800000;
    R6 = MAX(R6,R1);
    R11 = R5 * R12 (SSF);
    R7 = PASS R6;
    R5 = 0;
    COMP(R6,R13);
    IF GE JUMP .L_amp_silent_curve_high;
    COMP(R6,R14);
    IF GT JUMP .L_amp_silent_curve_no_excess;
    R4 = R6 - R14;
    R7 = PASS R14;
    JUMP .L_amp_silent_curve_excess;
.L_amp_silent_curve_high:
    R4 = R6 - R13;
    R7 = PASS R13;
.L_amp_silent_curve_excess:
    R1 = 0x7fffff;
    R4 = MIN(R4,R1);
    R1 = -0x800000;
    R4 = MAX(R4,R1);
    R4 = LSHIFT R4 BY 8;
    R0 = R4 * R13 (SSF);
    R3 = R4 * R13 (SSI);
    R7 = R7 + R0;
    R5 = FEXT R3 BY 7:24;
.L_amp_silent_curve_no_excess:
    R1 = 0x7fffff;
    R4 = MIN(R7,R1);
    R1 = -0x800000;
    R4 = MAX(R4,R1);
    R0 = R15 * R4 (SSF);
    R3 = R15 * R4 (SSI);
    R3 = FEXT R3 BY 7:24;
    R5 = R5 + R3;
    R5 = FEXT R5 BY 24:8;
    R0 = R0 + R7;
    R0 = R0 + R5;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    R0 = R0 + R6;
    R0 = LSHIFT R0 BY 1;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    I4 = R9;
    DM(I4,M5) = R0;
    R1 = 4;
    R8 = R8 + R1;
.L_amp_silent_curve_end:
    R9 = R9 + R1;

.L_amp_done:
    R0 = PASS R11;
    R8 = DM(-0x3,I6);
    R9 = DM(-0x4,I6);
    R10 = DM(-0x5,I6);
    R11 = DM(-0x6,I6);
    R12 = DM(-0x7,I6);
    R13 = DM(-0x8,I6);
    R14 = DM(-0x9,I6);
    R15 = DM(-0xA,I6);
    I7=MODIFY (I7,0x8)(NW);
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;


.GLOBAL trxbd_osc_shape.;
trxbd_osc_shape.:
    /* Original MD oscillator/harmonic/shaper stages, P:$100461..$1004D3.
     * C prepares the ORIGINAL 48-bit increments before ramp decay.
     * R4=y, R8=sine, R12=output, fourth pointer=hi1/lo1/hi2/lo2 increments.
     * Active harmonics stream both oscillators; zero harmonics retain the
     * exact closed-form phase2 fast path in a separate native loop.
     *
     * R8=sine, R9=out, R10:R11=phase1, R12:R13=phase2,
     * R14=harmonic level<<4, R15=shape step, R5/R6=previous fold bounds.
     * In the loops R4=phase1 high increment, R3=signed24 maximum.
     * Preserve mask-before-add phase order, low24 carry, sine2 saturation,
     * exact harmonic truncation, strict fold comparisons and all state.
     * High-phase upper bits cannot affect any low15 index, low carry, or
     * final signed24 state word: compute only those observable low bits.
     * R1=signed24 minimum survives the active harmonic multiply/fold path.
     * No calls, MRF/MRB writes, or MODE1 changes. */
    I7=MODIFY (I7,-0x11)(NW);
    DM(-0x3,I6) = R8;
    DM(-0x4,I6) = R9;
    DM(-0x5,I6) = R10;
    DM(-0x6,I6) = R11;
    DM(-0x7,I6) = R12;
    DM(-0x8,I6) = R13;
    DM(-0x9,I6) = R14;
    DM(-0xA,I6) = R15;
    DM(-0xB,I6) = R4;
    R9 = PASS R12;
    I4 = I6;
    I4=MODIFY (I4,0x1)(NW);
    R0 = DM(I4,M5);
    I4 = R0;
    R1 = DM(I4,M5);
    DM(-0xC,I6) = R1;
    I4=MODIFY (I4,0x1)(NW);
    R1 = DM(I4,M5);
    DM(-0xD,I6) = R1;
    I4=MODIFY (I4,0x1)(NW);
    R1 = DM(I4,M5);
    DM(-0xE,I6) = R1;
    I4=MODIFY (I4,0x1)(NW);
    R1 = DM(I4,M5);
    DM(-0xF,I6) = R1;
    I4 = R4;
    I4=MODIFY (I4,0x16)(NW);
    R10 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R11 = DM(I4,M5);
    I4=MODIFY (I4,0x8)(NW);
    R12 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R13 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R14 = DM(I4,M5);
    I4 = R4;
    I4=MODIFY (I4,0x10)(NW);
    R15 = DM(I4,M5);
    I4=MODIFY (I4,0x4)(NW);
    R5 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R6 = DM(I4,M5);
    R11 = FEXT R11 BY 0:24;
    R13 = FEXT R13 BY 0:24;
    R3 = 104;
    R3 = R4 + R3;
    I4 = R3;
    R0 = DM(I4,M5);
    DM(-0x12,I6) = R0;
    R1 = LSHIFT R0 BY 1;
    DM(-0x10,I6) = R1;
    R0 = R0 + R15;
    R1 = 0x7FFFFF;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    DM(I4,M5) = R0;
    R3 = 108;
    R3 = R4 + R3;
    I4 = R3;
    R0 = DM(I4,M5);
    DM(-0x13,I6) = R0;
    R1 = LSHIFT R0 BY 1;
    DM(-0x11,I6) = R1;
    R0 = R0 - R15;
    R1 = 0x7FFFFF;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    DM(I4,M5) = R0;
    R4 = DM(-0xC,I6);
    R3 = 0x7fffff;
    R14 = LSHIFT R14 BY 4;
    R0 = 0;
    COMP(R14,R0);
    IF EQ JUMP .L_osc_shape_zero;
    LCNTR = 32, DO .L_osc_shape_end UNTIL LCE;
    R0 = FEXT R10 BY 0:15;
    R1 = DM(-0xD,I6);
    R11 = R11 + R1;
    R1 = FEXT R11 BY 24:8;
    R10 = R0 + R4;
    R10 = R10 + R1;
    R11 = FEXT R11 BY 0:24;
    R0 = LSHIFT R0 BY 2;
    R0 = R8 + R0;
    I4 = R0;
    R7 = DM(I4,M5);
    R1 = -2;
    R7 = ASHIFT R7 BY R1;
    R0 = FEXT R12 BY 0:15;
    R1 = DM(-0xF,I6);
    R13 = R13 + R1;
    R1 = FEXT R13 BY 24:8;
    R12 = DM(-0xE,I6);
    R12 = R12 + R0;
    R12 = R12 + R1;
    R13 = FEXT R13 BY 0:24;
    R0 = LSHIFT R0 BY 2;
    R0 = R8 + R0;
    I4 = R0;
    R0 = DM(I4,M5);
    R0 = LSHIFT R0 BY 1;
    R0 = MIN(R0,R3);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    /* Harmonic coefficient is aligned by four, so SSF preserves the
     * original two floors in one exact multiply. R1 remains s24 min. */
    R0 = R0 * R14 (SSF);
    R7 = R7 + R0;
    R7 = -R7;
    R0 = DM(-0x10,I6);
    COMP(R7,R0);
    IF GT JUMP .L_osc_shape_high;
    R0 = DM(-0x11,I6);
    COMP(R7,R0);
    IF LT JUMP .L_osc_shape_low;
    JUMP .L_osc_shape_write;
.L_osc_shape_high:
    R7 = R5 - R15;
    R6 = DM(-0x13,I6);
    R5 = MIN(R7,R3);
    R5 = MAX(R5,R1);
    JUMP .L_osc_shape_fold;
.L_osc_shape_low:
    R7 = R6 + R15;
    R5 = DM(-0x12,I6);
    R6 = MIN(R7,R3);
    R6 = MAX(R6,R1);
.L_osc_shape_fold:
    R7 = LSHIFT R7 BY 1;
    R7 = MIN(R7,R3);
    R7 = MAX(R7,R1);
.L_osc_shape_write:
    I4 = R9;
    DM(I4,M5) = R7;
    R1 = 4;
.L_osc_shape_end:
    R9 = R9 + R1;
    JUMP .L_osc_shape_commit;
    /* Zero harmonics: same 31+1-step low-word carry and high mask as
     * the original C closed form. No sine2 read or multiply is needed.
     * Keep the final step AFTER the high31 mask, including signed increments. */
.L_osc_shape_zero:
    R0 = 31;
    R3 = DM(-0xF,I6);
    R0 = R3 * R0 (SSI);
    R13 = R13 + R0;
    R3 = FEXT R13 BY 24:8;
    R0 = 31;
    R1 = DM(-0xE,I6);
    R0 = R1 * R0 (SSI);
    R1 = 0x7FFF;
    R12 = R12 AND R1;
    R12 = R12 + R0;
    R12 = R12 + R3;
    R12 = R12 AND R1;
    R13 = FEXT R13 BY 0:24;
    R0 = DM(-0xF,I6);
    R13 = R13 + R0;
    R0 = FEXT R13 BY 24:8;
    R1 = DM(-0xE,I6);
    R12 = R12 + R1;
    R12 = R12 + R0;
    R13 = FEXT R13 BY 0:24;
    R3 = 0x7fffff;
    LCNTR = 32, DO .L_osc_zero_end UNTIL LCE;
    R0 = FEXT R10 BY 0:15;
    R1 = DM(-0xD,I6);
    R11 = R11 + R1;
    R1 = FEXT R11 BY 24:8;
    R10 = R0 + R4;
    R10 = R10 + R1;
    R11 = FEXT R11 BY 0:24;
    R0 = LSHIFT R0 BY 2;
    R0 = R8 + R0;
    I4 = R0;
    R7 = DM(I4,M5);
    R1 = -2;
    R7 = ASHIFT R7 BY R1;
    R7 = -R7;
    R0 = DM(-0x10,I6);
    COMP(R7,R0);
    IF GT JUMP .L_osc_zero_high;
    R0 = DM(-0x11,I6);
    COMP(R7,R0);
    IF LT JUMP .L_osc_zero_low;
    JUMP .L_osc_zero_write;
.L_osc_zero_high:
    R7 = R5 - R15;
    R6 = DM(-0x13,I6);
    R5 = MIN(R7,R3);
    R1 = -0x800000;
    R5 = MAX(R5,R1);
    JUMP .L_osc_zero_fold;
.L_osc_zero_low:
    R7 = R6 + R15;
    R5 = DM(-0x12,I6);
    R6 = MIN(R7,R3);
    R1 = -0x800000;
    R6 = MAX(R6,R1);
.L_osc_zero_fold:
    R7 = LSHIFT R7 BY 1;
    R7 = MIN(R7,R3);
    R7 = MAX(R7,R1);
.L_osc_zero_write:
    I4 = R9;
    DM(I4,M5) = R7;
    R1 = 4;
.L_osc_zero_end:
    R9 = R9 + R1;
.L_osc_shape_commit:
    R4 = DM(-0xB,I6);
    R1 = -8;
    R0 = LSHIFT R12 BY 8;
    R0 = ASHIFT R0 BY R1;
    I4 = R4;
    I4=MODIFY (I4,0x1f)(NW);
    DM(I4,M5) = R0;
    R0 = LSHIFT R13 BY 8;
    R0 = ASHIFT R0 BY R1;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R0;
    I4 = R4;
    I4=MODIFY (I4,0xe)(NW);
    R3 = DM(I4,M5);
    R0 = LSHIFT R14 BY 4;
    R0 = R0 * R3 (SSF);
    I4 = R4;
    I4=MODIFY (I4,0x21)(NW);
    DM(I4,M5) = R0;
    I4 = R4;
    I4=MODIFY (I4,0x14)(NW);
    DM(I4,M5) = R5;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R6;
    R0 = LSHIFT R10 BY 8;
    R0 = ASHIFT R0 BY R1;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R0;
    R0 = LSHIFT R11 BY 8;
    R0 = ASHIFT R0 BY R1;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R0;
    R8 = DM(-0x3,I6);
    R9 = DM(-0x4,I6);
    R10 = DM(-0x5,I6);
    R11 = DM(-0x6,I6);
    R12 = DM(-0x7,I6);
    R13 = DM(-0x8,I6);
    R14 = DM(-0x9,I6);
    R15 = DM(-0xA,I6);
    I7=MODIFY (I7,0x11)(NW);
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;

.GLOBAL trxbd_phase_prepare.;
trxbd_phase_prepare.:
    /* R4=y, R8=four increment words. Full doubled ramp product followed
     * by ASR56(1) is exactly the unscaled signed48 product, including its
     * low24 remainder. Compute once before decaying y[0x13]. */
    I7=MODIFY (I7,-0x2)(NW);
    DM(-0x3,I6) = R8;
    DM(-0x4,I6) = R9;
    R9 = PASS R4;
    I4 = R4;
    I4=MODIFY (I4,0x13)(NW);
    R5 = DM(I4,M5);
    I4 = R4;
    I4=MODIFY (I4,0x4)(NW);
    R6 = DM(I4,M5);
    R0 = R5 * R6 (SSF);
    R3 = R5 * R6 (SSI);
    R1 = FEXT R3 BY 24:7;
    R0 = LSHIFT R0 BY 7;
    R6 = R0 OR R1;
    R1 = 0xffffff;
    R7 = R3 AND R1;
    I4 = R4;
    I4=MODIFY (I4,0x3)(NW);
    R4 = DM(I4,M5);
    R1 = -6;
    R0 = ASHIFT R4 BY R1;
    R3 = FEXT R4 BY 0:6;
    R3 = LSHIFT R3 BY 18;
    R3 = R3 + R7;
    R1 = FEXT R3 BY 24:1;
    R0 = R0 + R6;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 8;
    R1 = -8;
    R0 = ASHIFT R0 BY R1;
    R1 = 0xffffff;
    R3 = R3 AND R1;
    I4 = R8;
    DM(I4,M5) = R0;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R3;
    R1 = -7;
    R0 = ASHIFT R4 BY R1;
    R3 = FEXT R4 BY 0:7;
    R3 = LSHIFT R3 BY 17;
    R3 = R3 + R7;
    R1 = FEXT R3 BY 24:1;
    R0 = R0 + R6;
    R0 = R0 + R1;
    R0 = LSHIFT R0 BY 8;
    R1 = -8;
    R0 = ASHIFT R0 BY R1;
    R1 = 0xffffff;
    R3 = R3 AND R1;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R0;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R3;
    I4 = R9;
    I4=MODIFY (I4,0x5)(NW);
    R6 = DM(I4,M5);
    R0 = R5 * R6 (SSF);
    R3 = R5 * R6 (SSI);
    R3 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R3;
    I4 = R9;
    I4=MODIFY (I4,0x13)(NW);
    DM(I4,M5) = R0;
    R8 = DM(-0x3,I6);
    R9 = DM(-0x4,I6);
    I7=MODIFY (I7,0x2)(NW);
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;

.GLOBAL trxbd_noise_active.;
trxbd_noise_active.:
    /* Original additive modulo-24 generator: eleven recurrence steps,
     * ten triple samples and one final pair. Decay only after rendering. */
    I7=MODIFY (I7,-0x5)(NW);
    DM(-0x3,I6) = R8;
    DM(-0x4,I6) = R9;
    DM(-0x5,I6) = R10;
    DM(-0x6,I6) = R11;
    DM(-0x7,I6) = R12;
    R9 = PASS R4;
    I4 = R4;
    I4=MODIFY (I4,0x1c)(NW);
    R10 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R11 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R12 = DM(I4,M5);
    LCNTR = 10, DO .L_noise_end UNTIL LCE;
    R4 = R10 + R11;
    R4 = LSHIFT R4 BY 8;
    R1 = -8;
    R4 = ASHIFT R4 BY R1;
    R10 = PASS R11;
    R11 = PASS R4;
    R0 = R4 * R12 (SSF);
    R3 = R4 * R12 (SSI);
    R3 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R3;
    I4 = R8;
    DM(I4,M5) = R0;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R0;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R0;
    R1 = 12;
.L_noise_end:
    R8 = R8 + R1;
    R4 = R10 + R11;
    R4 = LSHIFT R4 BY 8;
    R1 = -8;
    R4 = ASHIFT R4 BY R1;
    I4 = R9;
    I4=MODIFY (I4,0x1c)(NW);
    DM(I4,M5) = R11;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R4;
    R0 = R4 * R12 (SSF);
    R3 = R4 * R12 (SSI);
    R3 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R3;
    I4 = R8;
    DM(I4,M5) = R0;
    I4=MODIFY (I4,0x1)(NW);
    DM(I4,M5) = R0;
    I4 = R9;
    I4=MODIFY (I4,0xc)(NW);
    R4 = DM(I4,M5);
    R0 = R4 * R12 (SSF);
    R3 = R4 * R12 (SSI);
    R3 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R3;
    I4 = R9;
    I4=MODIFY (I4,0x1e)(NW);
    DM(I4,M5) = R0;
    R8 = DM(-0x3,I6);
    R9 = DM(-0x4,I6);
    R10 = DM(-0x5,I6);
    R11 = DM(-0x6,I6);
    R12 = DM(-0x7,I6);
    I7=MODIFY (I7,0x5)(NW);
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;

.GLOBAL trxbd_transient.;
trxbd_transient.:
    /* R4=out, R8=table_a, R12=table_b; stack=y, noise, silent.
     * Original sum is (8*tb*y[2] + 2*ta*y[15]) >> 24, then noise
     * and saturation. Join exact high parts with the low24 carry. */
    I7=MODIFY (I7,-0x8)(NW);
    DM(-0x3,I6) = R8;
    DM(-0x4,I6) = R9;
    DM(-0x5,I6) = R10;
    DM(-0x6,I6) = R11;
    DM(-0x7,I6) = R12;
    DM(-0x8,I6) = R13;
    DM(-0x9,I6) = R14;
    DM(-0xA,I6) = R15;
    R10 = PASS R4;
    R9 = PASS R12;
    I4 = I6;
    I4=MODIFY (I4,0x1)(NW);
    R4 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R11 = DM(I4,M5);
    I4=MODIFY (I4,0x1)(NW);
    R14 = DM(I4,M5);
    I4 = R4;
    I4=MODIFY (I4,0x2)(NW);
    R13 = DM(I4,M5);
    I4 = R4;
    I4=MODIFY (I4,0xf)(NW);
    R12 = DM(I4,M5);
    R15 = 0xffffff;
    LCNTR = 32, DO .L_transient_end UNTIL LCE;
    I4 = R9;
    R4 = DM(I4,M5);
    R0 = R4 * R13 (SSF);
    R3 = R4 * R13 (SSI);
    R1 = FEXT R3 BY 21:10;
    R0 = LSHIFT R0 BY 10;
    R6 = R0 OR R1;
    R3 = LSHIFT R3 BY 3;
    R7 = R3 AND R15;
    I4 = R8;
    R4 = DM(I4,M5);
    R0 = R4 * R12 (SSF);
    R3 = R4 * R12 (SSI);
    R1 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R1;
    R3 = LSHIFT R3 BY 1;
    R3 = R3 AND R15;
    R7 = R7 + R3;
    R7 = FEXT R7 BY 24:1;
    R6 = R6 + R0;
    R6 = R6 + R7;
    R0 = 0;
    COMP(R14,R0);
    IF NE JUMP .L_transient_silent;
    I4 = R11;
    R0 = DM(I4,M5);
    R6 = R6 + R0;
.L_transient_silent:
    R1 = 0x7fffff;
    R0 = MIN(R6,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    I4 = R10;
    DM(I4,M5) = R0;
    R1 = 4;
    R8 = R8 + R1;
    R9 = R9 + R1;
    R10 = R10 + R1;
.L_transient_end:
    R11 = R11 + R1;
    R8 = DM(-0x3,I6);
    R9 = DM(-0x4,I6);
    R10 = DM(-0x5,I6);
    R11 = DM(-0x6,I6);
    R12 = DM(-0x7,I6);
    R13 = DM(-0x8,I6);
    R14 = DM(-0x9,I6);
    R15 = DM(-0xA,I6);
    I7=MODIFY (I7,0x8)(NW);
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;

.GLOBAL trxbd_transient_fade.;
trxbd_transient_fade.:
    /* Retrigger crossfade: preserve the sequential floor at every sample.
     * Original y[0x18] is only read; this path must not commit its tail. */
    R5 = 0x733333;
    I4 = R8;
    LCNTR = 32, DO .L_transient_fade_end UNTIL LCE;
    R0 = R4 * R5 (SSF);
    R3 = R4 * R5 (SSI);
    R3 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R4 = R0 OR R3;
    DM(I4,M5) = R4;
.L_transient_fade_end:
    I4=MODIFY (I4,0x1)(NW);
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
.ENDSEG;
