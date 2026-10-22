"""Host C emitter for the fast IR: an independent check of the lifted program.

The emitted function is compiled into the same translation unit as the
generated engine source so it can reuse its voice type and its generic
md_read_*/md_write_* region dispatch for unresolved ('dread'/'dwrite') ops.
"""

class _Bases(dict):
    def __missing__(self, k):
        if k.startswith('T'):
            return 'source_' + k[1:]
        raise KeyError(k)


BASES = _Bases({'Y': 'v->y', 'BLK': 'v->blk44', 'SX': 'md_shared_scratch_x', 'SY': 'md_shared_scratch_y',
         'SINE': 'sine', 'XP': 'v->x_private', 'EXT': 'v->external',
                'SRC': 'md_source_words'})


def val(a):
    if isinstance(a, int):
        return f'0x{a & 0xffffffff:x}u'
    return a


def emit_c(name, voice, ops, fallback=None, generic=None):
    names = set()
    for o in ops:
        for x in o[1:]:
            if isinstance(x, str) and x not in BASES and not x.startswith(('L_', 'X')) and x not in ('x', 'y', 'src') \
                    and not x.startswith('md_') and x not in ('virtual_voice', 'scratch_y20_history'):
                names.add(x)
    for o in ops:
        if o[0] in ('label', 'jmp'):
            names.discard(o[1])
        if o[0] in ('jz', 'jnz'):
            names.discard(o[2])
        if o[0] in ('load', 'store'):
            names.discard(o[2] if o[0] == 'load' else o[1])
    lines = [f'void {name}({voice} *v, const int32_t *sine) {{']
    lines.append('    uint32_t ' + ', '.join(sorted(names)) + ';')
    for o in ops:
        k = o[0]
        if k == 'guard_y':
            w = f'((uint32_t)v->y[{o[1]}] - {o[4]}u * v->virtual_voice)' if o[4] else f'(uint32_t)v->y[{o[1]}]'
            lines.append(f'    if ({w} < 0x{o[2]:x}u || {w} > 0x{o[3]:x}u) {{ {generic}(v, sine); return; }}')
        elif k == 'guard_vv':
            lines.append(f'    if (v->virtual_voice >= {o[1]}u) {{ {fallback}(v, sine); return; }}')
        elif k == 'label':
            lines.append(f'{o[1]}: ;')
        elif k == 'mov':
            lines.append(f'    {o[1]} = {val(o[2])};')
        elif k in ('add', 'sub', 'and', 'or', 'xor'):
            sym = {'add': '+', 'sub': '-', 'and': '&', 'or': '|', 'xor': '^'}[k]
            lines.append(f'    {o[1]} = (uint32_t)({val(o[2])} {sym} {val(o[3])});')
        elif k == 'shl':
            lines.append(f'    {o[1]} = (uint32_t)({val(o[2])} << {o[3]});')
        elif k == 'shr':
            lines.append(f'    {o[1]} = (uint32_t)({val(o[2])} >> {o[3]});')
        elif k == 'sar':
            lines.append(f'    {o[1]} = (uint32_t)((int32_t){val(o[2])} >> {o[3]});')
        elif k in ('min', 'max'):
            cmp = '<' if k == 'min' else '>'
            lines.append(f'    {{ int32_t p=(int32_t){val(o[2])}, q=(int32_t){val(o[3])}; {o[1]} = (uint32_t)(p {cmp} q ? p : q); }}')
        elif k == 'lt':
            lines.append(f'    {o[1]} = (int32_t){val(o[2])} < (int32_t){val(o[3])};')
        elif k == 'ltu':
            lines.append(f'    {o[1]} = (uint32_t){val(o[2])} < (uint32_t){val(o[3])};')
        elif k == 'eq':
            lines.append(f'    {o[1]} = (uint32_t){val(o[2])} == (uint32_t){val(o[3])};')
        elif k == 'satlo':
            h, l, c1, c2, kk = (val(x) for x in o[2:])
            lines.append(f'    {o[1]} = (int32_t){c1} < (int32_t){h} ? {kk} : (int32_t){h} < (int32_t){c2} ? 0u : {l};')
        elif k == 'mulhi':
            lines.append(f'    {o[1]} = (uint32_t)(int32_t)(((int64_t)(int32_t){val(o[2])} * (int64_t)(int32_t){val(o[3])}) >> 23);')
        elif k == 'mullo':
            lines.append(f'    {o[1]} = (uint32_t){val(o[2])} * (uint32_t){val(o[3])};')
        elif k in ('mac24', 'mac24p'):
            a = val(o[3]) if k == 'mac24' else f'(uint32_t)((int32_t){val(o[3])} >> 8)'
            lines.append(f'    {{ int64_t p = (int64_t)(int32_t){a} * (int64_t)(int32_t){val(o[4])}; '
                         f'{o[1]} = (uint32_t)(int32_t)(p >> 23); {o[2]} = ((uint32_t)p << 1) & 0xffffffu; }}')
        elif k == 'acc':
            from tools.machines.translation.fast_opt import acc_terms
            terms = acc_terms(o)
            hs = ' '.join(f'{"+" if g > 0 else "-"} (uint32_t){val(h)}' for g, h, l in terms)
            ls = ' '.join(f'{"+" if g > 0 else "-"} (int32_t){val(l)}' for g, h, l in terms)
            lines.append(f'    {{ int32_t sl = 0 {ls}; {o[1]} = (uint32_t)(0u {hs} + (uint32_t)(sl >> 24)); '
                         f'{o[2]} = (uint32_t)sl & 0xffffffu; }}')
        elif k == 'agu':
            lines.append(f'    {{ uint32_t c = {val(o[2])}; (void)md_agu(&c, {val(o[3])}, (int32_t){val(o[4])}, 0u); {o[1]} = c; }}')
        elif k == 'load':
            lines.append(f'    {o[1]} = (uint32_t)*({BASES[o[2]]} + {val(o[3])});')
        elif k == 'store':
            lines.append(f'    *({BASES[o[1]]} + {val(o[2])}) = (int32_t){val(o[3])};')
        elif k == 'dread':
            d, space, addr, raw = o[1:]
            fn = {'x': 'md_read_x', 'y': 'md_read_y', 'src': 'md_read_source'}[space] + ('_raw' if raw else '')
            if space == 'src':
                lines.append(f'    {d} = {fn}(v, {val(addr)});')
            else:
                lines.append(f'    {d} = {fn}(v, md_shared_scratch_{space}, {val(addr)}, sine);')
        elif k == 'dwrite':
            space, addr, s = o[1:]
            lines.append(f'    md_write_{space}(v, md_shared_scratch_{space}, {val(addr)}, sine, {val(s)});')
        elif k == 'jmp':
            lines.append(f'    goto {o[1]};')
        elif k == 'jz':
            lines.append(f'    if ({val(o[1])} == 0u) goto {o[2]};')
        elif k == 'jnz':
            lines.append(f'    if ({val(o[1])} != 0u) goto {o[2]};')
        elif k == 'ret':
            lines.append('    return;')
        elif k == 'global_load':
            lines.append(f'    {o[1]} = (uint32_t){o[2]};')
        elif k == 'global_store':
            lines.append(f'    {o[1]} = {val(o[2])};')
        elif k == 'field_load':
            lines.append(f'    {o[1]} = (uint32_t)v->{o[2]};')
        elif k == 'field_store':
            lines.append(f'    v->{o[1]} = (int32_t){val(o[2])};')
        else:
            raise ValueError(o)
    lines.append('}')
    return '\n'.join(lines) + '\n'
