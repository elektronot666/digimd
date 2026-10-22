/* Core-clock cycle counter for the Full49 load governor. EMUCLK counts every
 * core clock on hardware; the sharcvm interpreter sources it from executed
 * instructions. Leaf: returns EMUCLK in R0 and touches no other register. */
.SECTION/SW seg_swco;
.GLOBAL md_cycles.;
md_cycles.:
    R0 = EMUCLK;
    I12 = DM(M7,I6);
    JUMP(M14,I12)(DB);
    RFRAME;
    NOP;
.ENDSEG;
