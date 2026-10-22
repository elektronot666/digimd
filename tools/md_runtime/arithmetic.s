/* Exact DSP and resampling helpers with the selcc C ABI. Only caller-saved
 * R0,R1,R3..R7 and scratch DAG I4/I12 are changed. R8 (argument), callee-saved R9..R15,
 * MODE1 and multiplier accumulator registers are preserved. The audio helpers save their callee-saved registers on the stack.
 */
.SECTION/SW seg_swco;
.GLOBAL lim24.;
lim24.:
    /* Exact signed24 saturation using the ADSP-21569 signed ALU MIN/MAX.
     * Unlike CLIP, this preserves -0x800000 (not -0x7fffff).
     * R4 is input; R0 return. Only R0,R1 and the existing I12 return DAG
     * are written; no multiplier accumulators, MODE1, memory or ABI changes.
     * Actual selas support and VM parity are mandatory before accepting. */
    R1 = 0x7FFFFF;
    R0 = MIN(R4,R1);
    R1 = -0x800000;
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    R0 = MAX(R0,R1);

.GLOBAL mpy_ss_hi.;
mpy_ss_hi.:
    /* Exact floor(s1*s2/2^23) with one signed fractional and one signed
     * integer multiply. R4 = signed24, R8 = signed25. Both are in range,
     * so 2*s1*s2 fits signed64 and no SSF result saturation occurs.
     *
     * SHARC Rn = Rx*Ry (SSF) returns upper32 of signed product shifted
     * left 1: floor(P / 2^31). SSI returns low32 of unscaled P.
     * The exact low 8 bits of floor(P/2^23) are bits30:23 of P.
     * ((low32(P) << 1) logical>>24) extracts exactly these 8 bits,
     * discarding the sign bit. Join them with SSF_high << 8.
     *
     * All intermediates fit signed32; this identity handles negative P
     * by arithmetic SSF high and logical SSI low, with FLOOR semantics.
     *
     * No MRF/MRB writes, no callee-saved registers, no MODE1 changes.
     * Register footprint: R0,R1,R3 only; R4/R8 are untouched.
     * Both native opcodes, old/new assembly and exact audio/state must be
     * separately verified in sharcvm before considering production. */
    R0 = R4 * R8 (SSF);
    R3 = R4 * R8 (SSI);
    /* Extract product bits 30:23 directly. Immediate shifts avoid loading
     * three shift counts; finish the join in the return delay slot. */
    R3 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    R0 = R0 OR R3;

.GLOBAL mpy_ss32.;
mpy_ss32.:
    /* Original void/pointer ABI: R4=s1, R8=s2, R12=high pointer,
     * fourth argument at DM(1,I6)=low pointer. Preserve R8..R15, all
     * multiplier accumulators and MODE1. Only I4/I12 and volatile data
     * registers are scratch. No struct return or compiler ABI extension.
     *
     * Reuse the exact SSF/SSI identity above; mask the doubled integer
     * product explicitly to retain exactly its low 24 bits. */
    R0 = R4 * R8 (SSF);
    R3 = R4 * R8 (SSI);
    R1 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R1;
    R3 = LSHIFT R3 BY 1;
    R1 = 0xffffff;
    R3 = R3 AND R1;
    I4 = R12;
    DM(I4,M5) = R0;
    I4 = I6;
    I4=MODIFY (I4,0x1)(NW);
    R5 = DM(I4,M5);
    I4 = R5;
    DM(I4,M5) = R3;
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;


.ENDSEG;
