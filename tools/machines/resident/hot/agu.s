/* Exact generated md_agu integer semantics, with the existing four-arg C ABI.
 * R4 cursor, R8 modifier, R12 signed step, DM(1,I6) preincrement flag.
 * Only C caller-scratch registers are used. No multiplier or SIMD state. */
md_agu.:
    I4 = R4;
    R2 = DM(1,I6);
    R3 = DM(I4,M5);
    R0 = R3 + R12;
    R1 = 0xFFFFFF;
    R0 = R0 AND R1;
    COMPU(R8,R1);
    IF EQ JUMP .L_md_fast_store;
    R1 = 1;
    COMPU(R8,R1);
    IF LE JUMP .L_md_fast_mask;
    R1 = LEFTZ R8;
    R1 = -R1;
    R5 = -1;
    R1 = LSHIFT R5 BY R1;
.L_md_fast_mask:
    R5 = NOT R1;
    R5 = R3 AND R5;
    R1 = R3 AND R1;
    R1 = R1 + R12;
    R6 = 0;
    COMP(R1,R6);
    IF LT JUMP .L_md_fast_negative;
    COMPU(R1,R8);
    IF LE JUMP .L_md_fast_offset;
    R1 = R1 - R8;
    R7 = 1;
    R1 = R1 - R7;
    JUMP .L_md_fast_offset;
.L_md_fast_negative:
    R1 = R1 + R8;
    R7 = 1;
    R1 = R1 + R7;
.L_md_fast_offset:
    R0 = R5 + R1;
.L_md_fast_store:
    DM(I4,M5) = R0;
    R6 = 0;
    COMP(R2,R6);
    IF NE JUMP .L_md_fast_return;
    R0 = PASS R3;
.L_md_fast_return:
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
