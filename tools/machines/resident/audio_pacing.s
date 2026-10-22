/* Keep codec/TDM ping-pong ownership in step with every SPORT completion.
 * The original notification-take decrements one pending event per call. It
 * skips rendering while the returned count is greater than one, but advances
 * the software DMA half only in the rendered handler. An even backlog thus
 * leaves all later blocks on the wrong half. Advance the half for each skipped
 * completion as well; zero-timeout results are not completions.
 *
 * R0 (pending count) is preserved. R2 is the dead BTGL result from the original
 * gate; the following original C call/next wait overwrites it. The one-event
 * path preserves the original flags and all registers exactly.
 *
 * Each skipped completion is a missed block deadline. Its count is published
 * at 0x8181D3F0 (MdRoute miss word, zero-filled by the loader and by
 * md_generic_init) for the Full49 load governor. This happens before the BTGL,
 * so the flags reaching 0x1C9FFB are still the BTGL result, as before.
 *
 * The render path goes through md_block_start (below), which only stamps the
 * block start; the flags and registers reaching 0x1C9FF4 are unchanged.
 */
.SECTION/SW seg_swco;
.GLOBAL md_audio_pacing_gate.;
md_audio_pacing_gate.:
    IF SZ JUMP md_block_start.;
    R2 = PASS R0;
    IF EQ JUMP 0x1C9FFB;
    R2 = DM(0x8181D3F0);
    R2 = R2 + 1;
    DM(0x8181D3F0) = R2;
    R2 = DM(0x268A38);
    R2 = BTGL R2 BY 0;
    DM(0x268A38) = R2;
    JUMP 0x1C9FFB;

/* Block start for the Full49 governor's stock-time measurement: EMUCLK when
 * the Audio Task goes on to render (one pending completion), at 0x8181D3F4.
 * R0 is saved and restored through 0x8181D3FC; no other register and no flag
 * changes (DM moves and the EMUCLK transfer leave ASTAT alone). */
.GLOBAL md_block_start.;
md_block_start.:
    DM(0x8181D3FC) = R0;
    R0 = EMUCLK;
    DM(0x8181D3F4) = R0;
    R0 = DM(0x8181D3FC);
    JUMP 0x1C9FF4;

/* Handler end: replaces the original 'JUMP 0x1C9FE7' at 0x1C9FFB, where the
 * block handler returns (and the gate's skip path rejoins). Stamps EMUCLK at
 * 0x8181D3F8 the same way, then continues to the notification wait. */
.GLOBAL md_handler_end.;
md_handler_end.:
    DM(0x8181D3FC) = R0;
    R0 = EMUCLK;
    DM(0x8181D3F8) = R0;
    R0 = DM(0x8181D3FC);
    JUMP 0x1C9FE7;
.ENDSEG;
