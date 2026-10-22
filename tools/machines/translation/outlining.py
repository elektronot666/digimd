"""Outline static instruction groups to bound compiler spill frames.

The parent retains the original native branches and hardware-loop structure.
Every call target is fixed at generation time; no opcode or PC dispatch exists.
"""
import re

def outline(text, code, name, voice_type):
    prefix=name;context=prefix.title()+'Exec'
    entries=list(re.finditer(r'^L_([0-9a-f]+): /\*[^\n]*\*/\n',text,re.M))
    if not entries:return text
    head=text[:entries[0].start()];tail=text[entries[-1].start():]
    declarations=[];initialize=[]
    array_pattern=r'^    (int32_t|uint32_t) (\w+)\[(\d+)\](?: = \{([^;]*)\})?;\n'
    def array(match):
        kind,var,length,values=match.groups();declarations.append(f'    {kind} {var}[{length}];')
        if values:
            for k,val in enumerate(values.split(',')):initialize.append(f'    exec->{var}[{k}] = {val};')
        return ''
    head=re.sub(array_pattern,array,head,flags=re.M)
    names=[re.search(r' (\w+)\[',d)[1] for d in declarations]
    head=head.replace('    uint32_t repeat = 0;\n','');declarations.append('    uint32_t repeat;');names.append('repeat');initialize.append('    exec->repeat = 0u;')
    head=head.replace('{\n',f'{{\n    {context} execution;\n    {context} *exec;\n    exec = &execution;\n'+ '\n'.join(initialize)+'\n',1)
    starts={int(x[1],16):i for i,x in enumerate(entries)}
    # Every explicit destination and loop boundary starts a separate native group.
    leaders={next(iter(code))}
    leaders.update(int(v,16) for v in re.findall(r'goto L_([0-9a-f]+)',text))
    for pc,i in code.items():
        if i.op in ('bra','jmp','bne','beq','bge','bgt','ble','blt','bcc','bcs','bmi','bpl','brset','brclr','jeq','jne'):
            target=re.search(r'(?:func_|int_)([0-9a-f]+)',i.args)
            if target:leaders.add(int(target[1],16))
            else:
                target=re.search(r'>?\$([0-9a-f]+)$',i.args)
                if target:leaders.add(int(target[1],16))
            leaders.add(pc+i.size)
        if i.op=='do':
            target=re.search(r'>?\$([0-9a-f]+)$',i.args);leaders.add(int(target[1],16));leaders.add(pc+i.size)
        if i.op=='dor':
            leaders.add(pc+int(i.args.split('$')[-1],16));leaders.add(pc+i.size)
    controls={'rts','bra','jmp','bne','beq','bge','bgt','ble','blt','bcc','bcs','bmi','bpl','brset','brclr','jeq','jne','do','dor','rep'}
    chunks=[];group=[]
    for idx,match in enumerate(entries):
        pc=int(match[1],16);end=entries[idx+1].start() if idx+1<len(entries) else text.rfind('\n}')
        piece=text[match.start():end]
        # Backedge counter belongs to the native parent, outside helper frames.
        backedge=re.search(r'^    if \(--loop_[^\n]+\n',piece,re.M)
        suffix=''
        if backedge: suffix=piece[backedge.start():];piece=piece[:backedge.start()]
        if pc in leaders or code[pc].op in controls or len(group)>=8:
            if group:chunks.append(('group',group));group=[]
        if code[pc].op in controls:chunks.append(('control',[(pc,piece+suffix)]))
        else:
            group.append((pc,piece))
            if suffix:
                chunks.append(('group',group));group=[];chunks.append(('control',[(None,suffix)]))
    if group:chunks.append(('group',group))
    helpers=[];parent=[]
    for kind,items in chunks:
        if kind=='control':parent.extend(piece for _,piece in items);continue
        pc=items[0][0];helper=f'{name}_group_{pc:x}'
        helpers.append(f'static void {helper}({context} *exec, {voice_type} *v, const int32_t *sine) {{\n'+''.join(piece for _,piece in items)+'}\n')
        parent.append(f'L_{pc:x}:\n    {helper}(exec,v,sine);\n')
    body=''.join(helpers)+head+''.join(parent)+'}\n'
    for var in names:
        # Initializer assignments have already been redirected.
        body=re.sub(r'(?<!->)\b'+var+r'\b',f'exec->{var}',body)
    # declarations cannot be placed after first assignments with the pinned parser.
    header=f'#ifndef {prefix.upper()}_EXEC_DEFINED\n#define {prefix.upper()}_EXEC_DEFINED\ntypedef struct {{\n'+'\n'.join(declarations)+f'\n}} {context};\n#endif\n'
    return header+body
