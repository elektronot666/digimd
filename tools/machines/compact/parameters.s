/* Exact serialized SYN extraction, once per block. C ABI arguments:
 * R4=physical slot, R8=p14[8], R12=last_p[8]. Only volatile R0,R1,R3..R7,
 * I4/I12 and flags change. No MODE1, multiplier accumulator or fixed save.
 * The original 16-bit clamp and truncating /2 remain unchanged. */
.SECTION/SW seg_swco;
.GLOBAL md_compact_parameters.;
md_compact_parameters.:
    R0 = 146;
    R4 = R4 * R0 (SSI);
    R0 = 0x25C48C;
    R4 = R4 + R0;
    R5 = R8;
    R6 = R12;
    R7 = md_param_offsets.;
    LCNTR = 8, DO .L_md_parameters_end UNTIL LCE;
    I4 = R7;
    R0 = DM(I4,M5);
    R1 = FEXT R0 BY 2:30;
    R1 = LSHIFT R1 BY 2;
    R1 = R4 + R1;
    I4 = R1;
    R1 = DM(I4,M5);
    R3 = 2;
    R0 = R0 AND R3;
    R0 = PASS R0;
    IF EQ JUMP .L_md_parameters_low;
    R1 = FEXT R1 BY 16:16;
.L_md_parameters_low:
    R1 = FEXT R1 BY 0:16;
    R0 = 0x7F00;
    R1 = MIN(R1,R0);
    R0 = FEXT R1 BY 1:15;
    I4 = R5;
    DM(I4,M5) = R0;
    I4 = R6;
    DM(I4,M5) = R0;
    R1 = 4;
    R5 = R5 + R1;
    R6 = R6 + R1;
.L_md_parameters_end:
    R7 = R7 + R1;
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
.ENDSEG;
