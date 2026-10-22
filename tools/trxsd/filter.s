/* Original P:$100823..$10082f resonator operation, one sample/stage.
 * R4=older, R8=recent, R12=input; stack=fb2,fb1,feed. All canonical s24.
 * Compute three full signed products, then the original ASL and data limiter.
 * Each product = hi*2^24+lo. Summing three lo24 values fits 26 unsigned bits.
 * floor(2*sum/2^24)=2*sum(hi)+(sum(lo)>>23), including negative products.
 * No intermediate product floor or saturation is discarded.
 * Preserve R8..R15, MODE1 and both multiplier banks; I4/I12 are ABI scratch.
 */
.SECTION/SW seg_swco;
.GLOBAL trxsd_filter.;
trxsd_filter.:
    I4 = I6;
    I4=MODIFY (I4,0x1)(NW);
    R5 = DM(I4,M5);
    R0 = R4 * R5 (SSF);
    R3 = R4 * R5 (SSI);
    R1 = FEXT R3 BY 23:8;
    R6 = LSHIFT R0 BY 8;
    R6 = R6 OR R1;
    R3 = LSHIFT R3 BY 1;
    R1 = 0xffffff;
    R7 = R3 AND R1;
    I4=MODIFY (I4,0x1)(NW);
    R5 = DM(I4,M5);
    R0 = R8 * R5 (SSF);
    R3 = R8 * R5 (SSI);
    R4 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R4;
    R6 = R6 + R0;
    R3 = LSHIFT R3 BY 1;
    R3 = R3 AND R1;
    R7 = R7 + R3;
    I4=MODIFY (I4,0x1)(NW);
    R5 = DM(I4,M5);
    R0 = R12 * R5 (SSF);
    R3 = R12 * R5 (SSI);
    R4 = FEXT R3 BY 23:8;
    R0 = LSHIFT R0 BY 8;
    R0 = R0 OR R4;
    R6 = R6 + R0;
    R3 = LSHIFT R3 BY 1;
    R3 = R3 AND R1;
    R7 = R7 + R3;
    R0 = R6 + R6;
    R1 = FEXT R7 BY 23:3;
    R0 = R0 + R1;
    R1 = 0x7fffff;
    R0 = MIN(R0,R1);
    R1 = -0x800000;
    R0 = MAX(R0,R1);
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
.ENDSEG;

.SECTION/SW seg_swco;
.GLOBAL trxsd_tonal.;
trxsd_tonal.:
    /* v, tonal[32], oscillator2[32], old clip envelope (stack).
     * Original 1007ce..1007e8. The exact linear sum of the two full products
     * is one signed24* signed25 product; extract floor(P/2^17) AFTER ASL6.
     * Positive decay/envelope bounds permit direct SSF amplitude operations.
     */
    I7=MODIFY (I7,-0x8)(NW);
    DM(-0x3,I6)=R8;
    DM(-0x4,I6)=R9;
    DM(-0x5,I6)=R10;
    DM(-0x6,I6)=R11;
    DM(-0x7,I6)=R12;
    DM(-0x8,I6)=R13;
    DM(-0x9,I6)=R14;
    DM(-0xA,I6)=R15;
    R9=PASS R4;
    R10=PASS R12;
    I4=I6;
    I4=MODIFY (I4,0x1)(NW);
    R11=DM(I4,M5);
    I4=R9;
    I4=MODIFY (I4,0x1)(NW);
    R13=DM(I4,M5);
    I4=MODIFY (I4,0xD)(NW);
    R12=DM(I4,M5);
    R14=0x7fffff;
    R15=-0x800000;
    LCNTR=32, DO .L_sd_tonal_end UNTIL LCE;
    I4=R8;
    R3=DM(I4,M5);
    I4=R10;
    R0=DM(I4,M5);
    R3=R3+R0;
    R0=R11*R3(SSF);
    R1=R11*R3(SSI);
    R1=FEXT R1 BY 17:14;
    R0=LSHIFT R0 BY 14;
    R0=R0 OR R1;
    R0=R0+R3;
    R0=MIN(R0,R14);
    R0=MAX(R0,R15);
    R0=LSHIFT R0 BY 8;
    R0=R0*R12(SSF);
    R1=LSHIFT R12 BY 8;
    R12=R1*R13(SSF);
    I4=R8;
    DM(I4,M5)=R0;
    R1=4;
    R8=R8+R1;
.L_sd_tonal_end:
    R10=R10+R1;
    I4=R9;
    I4=MODIFY (I4,0xE)(NW);
    DM(I4,M5)=R12;
    R8=DM(-0x3,I6);
    R9=DM(-0x4,I6);
    R10=DM(-0x5,I6);
    R11=DM(-0x6,I6);
    R12=DM(-0x7,I6);
    R13=DM(-0x8,I6);
    R14=DM(-0x9,I6);
    R15=DM(-0xA,I6);
    I7=MODIFY (I7,0x8)(NW);
    I12=DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
.ENDSEG;
