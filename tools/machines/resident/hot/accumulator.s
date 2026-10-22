/* Existing six-argument pointer ABI; preserve unsigned carry and signed borrow
 * exactly. High-word arithmetic wraps modulo32, low24 truncation is unchanged.
 * Stack args: bl, output-high pointer, output-low pointer. */
add56.:
    R0 = DM(1,I6);
    R5 = DM(2,I6);
    R6 = DM(3,I6);
    R1 = R8 + R0;
    R0 = -24;
    R0 = LSHIFT R1 BY R0;
    R3 = R4 + R12;
    R3 = R3 + R0;
    R0 = 0xFFFFFF;
    R1 = R1 AND R0;
    I4 = R5;
    DM(I4,M5) = R3;
    I4 = R6;
    DM(I4,M5) = R1;
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
sub56.:
    R0 = DM(1,I6);
    R5 = DM(2,I6);
    R6 = DM(3,I6);
    R1 = R8 - R0;
    R3 = R4 - R12;
    R0 = 0;
    COMP(R1,R0);
    IF GE JUMP .L_md_fast_sub_store;
    R0 = 0x1000000;
    R1 = R1 + R0;
    R0 = 1;
    R3 = R3 - R0;
.L_md_fast_sub_store:
    I4 = R5;
    DM(I4,M5) = R3;
    I4 = R6;
    DM(I4,M5) = R1;
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
