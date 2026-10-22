"""Source transforms shared by native_gate.py and the Full49 relink
(resident/fast_relink.py) for the two native C ports with hand-written
renders, TRX-SD and TRX-BD: the firmware's transformed C (out/audio-fix-
full49-pacing/<p>_scratch.c) as the relink compiles it, with its render
renamed <p>_render_block_44k1_c (the hand-written render's decline target)
and the tables the hand-written render reads exported."""

VOICE = {'trxsd': 'TrxSdVoice', 'trxbd': 'TrxBdVoice'}
# the hand-written render reads the C port's static tables through these
EXPORTS = {'trxbd': '\nconst int32_t *const trxbd_tables_hw[2] = {trxbd_table_a, trxbd_table_b};\n',
           'trxsd': '\nconst uint32_t *const trxsd_tables_hw[3] = {trxsd_t_filter_cos, trxsd_t_filter_radius, trxsd_t_transient};\n'}


def scratch_source(p, text):
    """The relink's fix-up of the transformed C: TRX-BD's y-scratch noise
    store as a canonical 24-bit word"""
    if p == 'trxbd':
        old = '        else md_shared_scratch_y[i]=noise[i];'
        assert text.count(old) == 1
        text = text.replace(old, '        else md_shared_scratch_y[i]=(int32_t)((uint32_t)noise[i]<<8)>>8; /* canonical 24-bit word */')
    return text


def hand_source(p, text):
    """The C render renamed <p>_render_block_44k1_c, the tables exported"""
    old = f'void {p}_render_block_44k1({VOICE[p]} *v, const int32_t *sine) {{'
    assert text.count(old) == 1
    text = text.replace(old, f'extern void {p}_render_block_44k1({VOICE[p]} *v, const int32_t *sine);\n'
                             f'void {p}_render_block_44k1_c({VOICE[p]} *v, const int32_t *sine) {{')
    return text + EXPORTS[p]
