| DigiMD UI corrections on top of c33's MD control runtime (OS 1.12).
|
| Linked into the cold-boot loader image, which the reset copier places at
| 0x47E20000 and which stays resident. ui.py patches the stock and c33 sites
| that branch here and checks every byte it replaces.
|
| c33 symbols used (trxbd_cf.elf of the pinned c33 build):
|   is_trxbd      0x47e000f8  d0 = selector - 4 on an MD track, else 0
|                             (active track; preserves d1/a0)
|   md_page_ptrs  0x47e0136c  49 SYN page descriptors, selector 5 first:
|                             {title, title, 8 record ids (0 = blank), 10}
|   md_ranges_ptrs 0x47e015b8 49 range lists, selector 5 first
.section .text
.equ IS_MD_ACTIVE,0x47e000f8
.equ MD_PAGE_PTRS,0x47e0136c
.equ RECORDS,0x401f83a4            | parameter record base, record[id] = base + 60 id
.equ MOD_MASK,0x400dc472           | get_param_mod_mask(id)

| LFO DEST list builder (0x4003951e) mask query, called for every parameter
| the track's ParameterSet enumerates. c33 gives an MD track WaveTone's
| parameter set, so the list offered all 40 WaveTone machine parameters, eight
| of them under MD names. On an MD track a WaveTone machine record (page 1) is
| offered only if it is one of that machine's labelled SYN controls; every
| other parameter keeps its stock mask.
.global md_ui_dest_mask
md_ui_dest_mask:
    move.l %d2,-(%sp)
    jsr IS_MD_ACTIVE
    move.l 8(%sp),%d1
    tst.l %d0
    beq.s 9f
    cmpi.l #321,%d1
    bcc.s 9f
    move.l %d1,%d2
    lsl.l #4,%d2
    sub.l %d1,%d2
    lsl.l #2,%d2
    lea RECORDS,%a0
    move.l (0,%a0,%d2.l),%d2
    subq.l #1,%d2
    bne.s 9f
    subq.l #1,%d0
    lsl.l #2,%d0
    lea MD_PAGE_PTRS,%a0
    movea.l (0,%a0,%d0.l),%a0
    addq.l #8,%a0
    moveq #8,%d2
1:  cmp.l (%a0)+,%d1
    beq.s 9f
    subq.l #1,%d2
    bne.s 1b
    moveq #0,%d0
    move.l (%sp)+,%d2
    rts
9:  move.l (%sp)+,%d2
    jmp MOD_MASK

| Control all (0x40066c76) finds each target track's parameter through the page
| view's vtable +0x70 (0x40064e76): page(type, n) of the TARGET track's raw type
| (0x4003134e answers the raw type in c33), then the record under the same knob.
| c33's hook on page(type, n) answers for the ACTIVE track instead, so from an
| MD track every target got MD record ids (on WaveTone the hidden noise
| controls of its second SYN page) and from a stock track an MD target got
| WaveTone's page. This lookup takes the type it is given, as stock does:
| an MD type gets its own SYN page (n = 0) or the stock empty page, a stock type
| the stock table.
.global md_ui_page
md_ui_page:
    move.l 4(%sp),%d1
    move.l 8(%sp),%d0
    cmpi.l #5,%d1
    bcs.s 2f
    cmpi.l #53,%d1
    bhi.s 8f
    tst.l %d0
    bne.s 8f
    subq.l #5,%d1
    lsl.l #2,%d1
    lea MD_PAGE_PTRS,%a0
    move.l (0,%a0,%d1.l),%d0
    rts
2:  tst.l %d0
    blt.s 8f
    lsl.l #4,%d1
    lea SYN_PAGES,%a0
    cmp.l (0,%a0,%d1.l),%d0
    bge.s 8f
    move.l %d2,-(%sp)
    moveq #44,%d2
    muls.l %d2,%d0
    move.l (%sp)+,%d2
    add.l (4,%a0,%d1.l),%d0
    rts
8:  move.l #EMPTY_PAGE,%d0
    rts
.equ SYN_PAGES,0x42433ad4          | per type {count, pages}, 16 bytes; 44-byte pages
.equ EMPTY_PAGE,0x42433bd4

| Machine setter (0x4004cc08), type changed: wraps its call of the machine
| parameter reset 0x4004c30c(this, 1, old, 0) (the `jsr` and the following
| `movea.l (%a2),%a0`). There d2 = new type, d6 = old type, a2 = this.
| Choosing an MD machine on a track that had none sets the AMP envelope to
| ATK 0, HOLD NOTE, DEC INF, AHD. Choosing a stock machine on an MD track puts
| back the OS 1.12 record defaults (ATK 8, HOLD NOTE, DEC 32, ADSR) if those
| four values are still the MD ones. MD to MD and stock to stock leave AMP as
| it is, as stock does.
.global md_ui_machine_reset
md_ui_machine_reset:
    move.l 16(%sp),-(%sp)
    move.l 16(%sp),-(%sp)
    move.l 16(%sp),-(%sp)
    move.l 16(%sp),-(%sp)
    jsr 0x4004c30c
    lea 16(%sp),%sp
    move.l %d2,%d0
    subq.l #5,%d0
    cmpi.l #48,%d0
    bhi.s 1f
    move.l %d6,%d0
    subq.l #5,%d0
    cmpi.l #48,%d0
    bls.s 9f
    lea md_amp_md,%a1
    moveq #1,%d0
    bsr.s md_ui_amp
    bra.s 9f
1:  move.l %d6,%d0
    subq.l #5,%d0
    cmpi.l #48,%d0
    bhi.s 9f
    lea md_amp_md,%a1
    moveq #0,%d0
    bsr.s md_ui_amp
    tst.l %d0
    beq.s 9f
    lea md_amp_stock,%a1
    moveq #1,%d0
    bsr.s md_ui_amp
9:  movea.l (%a2),%a0
    rts

| a1 = four {word slot, word value}, a2 = the sound's parameter set.
| Slot values are words at sound + 0x14 + 2*slot: the reset's store
| `move.w (%a0),(0x14,%fp,%d2.l*2)` at 0x4004c38a and the getter 0x4003713c.
| (objdump prints that displacement as "14": it is hex.)
| d0 = 0: answer 1 if every slot holds its value, else 0.
| d0 = 1: write each value to sound+0x14+2*slot and post the same change
| event (0x401dbb44, slot, 1) through vtable +0x10 as the reset does.
md_ui_amp:
    lea -24(%sp),%sp
    movem.l %d3-%d4/%a3,(%sp)
    movea.l %a1,%a3
    move.l %d0,%d4
    moveq #4,%d3
1:  movea.l (%a2),%a0
    move.l %a2,-(%sp)
    movea.l 40(%a0),%a0
    jsr (%a0)
    addq.l #4,%sp
    movea.l %d0,%a0
    lea 20(%a0),%a0
    moveq #0,%d0
    move.w (%a3)+,%d0
    move.w (%a3)+,%d1
    tst.l %d4
    bne.s 3f
    cmp.w (0,%a0,%d0.l*2),%d1
    bne.s 4f
    bra.s 2f
3:  move.w %d1,(0,%a0,%d0.l*2)
    move.l #0x401dbb44,%d1
    move.l %d1,12(%sp)
    move.l %d0,16(%sp)
    moveq #1,%d1
    move.b %d1,20(%sp)
    pea 12(%sp)
    move.l %a2,-(%sp)
    movea.l (%a2),%a0
    movea.l 16(%a0),%a0
    jsr (%a0)
    addq.l #8,%sp
2:  subq.l #1,%d3
    bne.s 1b
    moveq #1,%d0
    bra.s 5f
4:  moveq #0,%d0
5:  movem.l (%sp),%d3-%d4/%a3
    lea 24(%sp),%sp
    rts

| AMP slots (OS 1.12 stock control fixture): 81 ATK (record 62), 82 HOLD (63),
| 83 DEC (64), 91 MODE (73: 0 AHD, 0x100 ADSR).
.balign 2
.global md_amp_md, md_amp_stock
md_amp_md:
    .short 81, 0x0000, 82, 0x7f00, 83, 0x7f00, 91, 0x0000
md_amp_stock:
    .short 81, 0x0800, 82, 0x7f00, 83, 0x2000, 91, 0x0100

| Limits ({min, max, default} at a0) for a parameter of a given track.
| c33's hook on limits_getter(id) answers for the ACTIVE track, so a value set
| on any other track was clamped by the active track's machine: from a stock
| track an MD target's knob H (record 259, WaveTone's Noise Type, stock max 2)
| was held to 0..2, and from an MD track a stock target got MD ranges.
|
| Control all clamps each target at 0x40066da0. There d2 = target track and
| a2 = the page view, whose +120 is the model that 0x4003134e(model, track)
| asks for the track's raw machine type.
.global md_ui_target_limits
md_ui_target_limits:
    move.l %a0,-(%sp)
    move.l %d2,-(%sp)
    move.l 120(%a2),-(%sp)
    jsr 0x4003134e
    addq.l #8,%sp
    movea.l (%sp)+,%a0
    bra.s md_ui_limits

| ParameterSet::setValue (0x40037326) clamps at 0x40037350 with a2 = the
| parameter set. For a SoundParameterSet (vtable 0x401dbc0c) the machine is
| its own sound's: +16 holds the sound owner, whose vtable +0x28 gives the
| sound (as its value getter 0x4003713c does). Other sets keep c33's answer.
.global md_ui_set_limits
md_ui_set_limits:
    move.l (%a2),%d0
    cmpi.l #0x401dbc0c,%d0
    bne.s 8f
    move.l 16(%a2),%d0
    beq.s 8f
    move.l %a0,-(%sp)
    movea.l %d0,%a0
    move.l %a0,-(%sp)
    movea.l (%a0),%a0
    movea.l 40(%a0),%a0
    jsr (%a0)
    addq.l #4,%sp
    movea.l (%sp)+,%a0
    tst.l %d0
    beq.s 8f
    movea.l %d0,%a1
    mvs.b 222(%a1),%d0
    bra.s md_ui_limits
8:  jmp 0x400dc154

| ParameterSet::isValueValid(this, id, value) (0x40036bac, SoundParameterSet
| +0x24 ends there) checks min <= value <= max with the getter it loads into a2
| at 0x40036bce; setValue drops a value that fails. Both calls push id and
| leave `this` at 36(%sp) here.
.global md_ui_valid_limits
md_ui_valid_limits:
    move.l %a2,-(%sp)
    movea.l 40(%sp),%a2
    move.l 8(%sp),-(%sp)
    bsr.s md_ui_set_limits
    addq.l #4,%sp
    movea.l (%sp)+,%a2
    rts

| d0 = machine type, 4(%sp) = id, a0 = result. An MD machine's own control gets
| its c33 range; anything else the stock getter's body, past c33's hook.
md_ui_limits:
    move.l 4(%sp),%d1
    subq.l #5,%d0
    cmpi.l #48,%d0
    bhi.s 9f
    lsl.l #2,%d0
    lea MD_RANGES_PTRS,%a1
    movea.l (0,%a1,%d0.l),%a1
1:  move.l (%a1)+,%d0
    beq.s 9f
    cmp.l %d0,%d1
    beq.s 2f
    addq.l #4,%a1
    bra.s 1b
2:  movea.l (%a1),%a1
    move.l (%a1)+,(%a0)
    move.l (%a1)+,4(%a0)
    move.l (%a1),8(%a0)
    move.l %a0,%d0
    rts
9:  move.l %d2,-(%sp)
    move.l 8(%sp),%d1
    jmp 0x400dc15a
.equ MD_RANGES_PTRS,0x47e015b8     | c33: 49 x {record, &{min, max, default}}..., 0
