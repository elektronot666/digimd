"""DSP56300 source-to-C lowering. Static labels/loops; no runtime decoder.

Unknown operations fail closed. Successful lowering alone is never parity.
"""
import re
from dataclasses import dataclass


class Unsupported(ValueError):
    pass


@dataclass(frozen=True)
class Instruction:
    pc: int
    size: int
    words: tuple
    text: str

    @property
    def op(self):
        return self.text.split()[0]

    @property
    def args(self):
        return self.text[len(self.op):].strip()


def decode(path):
    instructions = {}
    for line in path.read_text().splitlines():
        pc, n, a, b, text = line.split('\t', 4)
        i = Instruction(int(pc, 16), int(n, 16), (int(a,16), int(b,16)), text.strip())
        instructions[i.pc] = i
    return instructions


def target(text):
    match = re.fullmatch(r'(?:int_|func_)([0-9a-f]+)', text)
    if match:
        return int(match[1],16)
    if re.fullmatch(r'>?\$[0-9a-f]+', text):
        return int(text.lstrip('>$'),16)
    raise Unsupported('Indirect/unresolved branch: '+text)


def reachable(instructions, entry):
    pending, seen = [entry], {}
    while pending:
        pc = pending.pop()
        if pc in seen:
            continue
        if pc not in instructions:
            raise Unsupported(f'Unmapped instruction {pc:#x}')
        i = instructions[pc]
        if not i.size or i.op in ('dc','dc24','illegal'):
            raise Unsupported(f'Invalid instruction {pc:#x}: {i.text}')
        seen[pc] = i
        if i.op == 'rts':
            continue
        if i.op in ('bra','jmp','beq','bne','bge','bgt','ble','blt','bcc','bcs','jeq','jne','bmi','bpl'):
            pending.append(target(i.args))
            if i.op in ('bra','jmp'):
                continue
        if i.op in ('jsr','bsr'):
            raise Unsupported(f'Subroutine lowering required {pc:#x}: {i.text}')
        if i.op == 'do':
            pending.append(target(i.args.split(',')[1]))
        if i.op in ('brset','brclr'):
            pending.append(target(i.args.split(',')[-1]))
        if i.op=='dor':
            match=re.fullmatch(r'>\*\+\$([0-9a-f]+)',i.args.split(',')[1])
            if not match:raise Unsupported('DOR target: '+i.args)
            pending.append(i.pc+int(match[1],16))
        pending.append(pc+i.size)
    occupied = {}
    for pc,i in seen.items():
        for a in range(pc,pc+i.size):
            if a in occupied and occupied[a] != pc:
                raise Unsupported(f'Overlapping instruction starts {pc:#x}/{occupied[a]:#x}')
            occupied[a] = pc
    return dict(sorted(seen.items()))


class Lowering:
    def __init__(self, instructions, image=None):
        self.instructions = instructions
        self.slot_words = set()
        self.sine = False
        self.loops = {}
        self.image = image
        self.tables = {}
        self.direct_addresses = {}
        self.requires_source_image = False
        self.private_memory = False
        self.buffered_output = False
        self.scratch_x_words = 128
        self.scratch_y_words = 128
        self.outline_native_groups = False

    def memory(self, operand):
        match=re.fullmatch(r'([xy]):\(r([0-7])\+n([0-7])\)',operand)
        if match:
            space,reg,n=match.groups()
            return f'md_read_{space}(v, scratch{space}, md_index(r{reg}, m{reg}, sx24((int32_t)n{n})), sine)'
        m=re.fullmatch(r'([xy]):[<>]?\$([0-9a-f]+)',operand)
        if m:
            address=int(m[2],16)
            if address>=(self.scratch_x_words if m[1]=='x' else self.scratch_y_words): raise Unsupported('Absolute scratch bound: '+operand)
            return f'scratch{m[1]} + {address}u'
        m=re.fullmatch(r'([xy]):\(r([0-7])(?:\+\$([0-9a-f]+))?\)([+-]?)(?:n([0-7]))?',operand)
        pre=False;stride=None
        if not m:
            m=re.fullmatch(r'([xy]):(-)\(r([0-7])\)',operand)
            if not m:raise Unsupported('Memory operand: '+operand)
            space,reg,offset,update=m[1],m[3],None,'-';pre=True
        else:space,reg,offset,update,stride=m.groups()
        if offset and int(offset,16)==0x148000:
            self.sine=True
            return f'sine + r{reg}'
        if offset:
            base=int(offset,16)
            if base>=0x100000:
                self.requires_source_image=True
                return f'md_read_source(v, r{reg} + {base}u)'
            return f'md_read_{space}(v, scratch{space}, r{reg} + {base}u, sine)'
        address=f'r{reg}'
        if update:
            if not pre and stride:
                address=f'md_agu(&r{reg}, m{reg}, {"" if update=="+" else "-"}sx24((int32_t)n{stride}), 0u)'
            elif reg in self.direct_addresses:
                sign='++' if update=='+' else '--'
                address=f'{sign}r{reg}' if pre else f'r{reg}{sign}'
            else:
                address=f'md_agu(&r{reg}, m{reg}, {"1" if update=="+" else "-1"}, {int(pre)}u)'
        if reg in self.direct_addresses and self.direct_addresses[reg]<128:
            base=self.direct_addresses[reg]
            return f'scratch{space} + ({base}u + (({address}) & 31u))'
        return f'md_read_{space}(v, scratch{space}, {address}, sine)'

    def word(self, operand):
        if self.private_memory and operand=='y:>$142':return 'v->virtual_voice'
        if self.private_memory and operand=='y:>$140':return '0x100u'
        if re.fullmatch(r'#[<>]?\$[0-9a-f]+',operand):
            return '0x'+operand.split('$')[1]+'u'
        if operand in ('a','b'):
            return f'u24(lim24({operand}h))'
        if operand in ('a0','b0'):
            return operand[0]+'l'
        if operand in ('a1','b1'):
            return f'u24({operand[0]}h)'
        if re.fullmatch(r'[xy][01]|[rmn][0-7]',operand):
            return operand
        m = re.fullmatch(r'y:\(r6(?:\+\$([0-9a-f]+))?\)',operand)
        if m:
            offset = int(m[1] or '0',16)
            if offset >= 64:
                raise Unsupported('Outside original voice slot')
            self.slot_words.add(offset)
            return f'u24(*(v->y + {offset}u))'
        if operand.startswith(('x:','y:')):
            mem=self.memory(operand)
            if mem.startswith('md_read_'):return mem
            return f'u24(*({mem}))'
        raise Unsupported('Operand: '+operand)

    def store(self, operand, value):
        if operand in ('a','b'):
            return f'{operand}h = sx24((int32_t)({value})); {operand}l = 0u;'
        if operand in ('a0','b0'):
            return f'{operand[0]}l = ({value}) & 0xffffffu;'
        if operand in ('a1','b1'):
            # Replacing A1/B1 leaves the accumulator extension byte intact.
            return f'{operand[0]}h = (int32_t)(((uint32_t){operand[0]}h & 0xff000000u) | (({value}) & 0xffffffu));'
        if re.fullmatch(r'[xy][01]|[rmn][0-7]',operand):
            return f'{operand} = ({value}) & 0xffffffu;'
        if operand.startswith('y:(r6'):
            dest = self.word(operand)[4:-1]
            return f'{dest} = sx24((int32_t)({value}));'
        if operand.startswith(('x:','y:')):
            mem=self.memory(operand)
            if mem.startswith('md_read_'):
                if mem.startswith('md_read_source'):raise Unsupported('Immutable source write')
                # Writable indirect accesses are scratch-only for this stage.
                return mem.replace('md_read_','md_write_')[:-1]+f', {value});'
            return f'*({mem}) = sx24((int32_t)({value}));'
        raise Unsupported('Destination: '+operand)

    def pair(self, operand):
        if operand in ('a','b'):
            return operand+'h', operand+'l'
        if operand in ('x','y'):
            return f'sx24((int32_t){operand}1)', operand+'0'
        return f'sx24((int32_t)({self.word(operand)}))', '0u'

    def instruction(self,i):
        text=self._instruction(i)
        # Original parallel bus moves read RAM, including its aliased guard.
        # Peripheral MOVEP/other nonparallel forms retain distinct semantics.
        opcode=i.words[0]
        if opcode>=0x100000 or (opcode & 0xfe4000)==0x080000:
            text=re.sub(r'\bmd_read_(x|y|source)\(',lambda match:'md_read_'+match[1]+'_raw(',text)
        if len(re.split(r'\s{2,}',i.args))>1:return text
        if i.op in ('add','sub','mpy','mpysu','mpyuu','mpyr','mac','macr','macsu','mpyi','maci','dmac','asr','asl','abs','neg','clr','tst'):
            dest=i.args.split(',')[-1]
            if dest in ('a','b'):
                text+=f' negative = ({dest}h < 0); zero = ({dest}h == 0 && {dest}l == 0u); less = negative; greater = (!zero && !negative);'
        return text

    def _instruction(self, i):
        op, args = i.op, i.args
        parallel=re.split(r'\s{2,}',args)
        if len(parallel)>1:
            if parallel[1].startswith('if'):
                cond={'ifeq':'zero','ifne':'!zero','ifle':'!greater','iflt':'less','ifge':'!less','ifgt':'greater','ifmi':'negative','ifpl':'!negative'}.get(parallel[1])
                if cond is None:raise Unsupported('Conditional ALU: '+i.text)
                base=Instruction(i.pc,i.size,i.words,op+' '+parallel[0])
                return f'if ({cond}) {{ '+self.instruction(base)+' }'
            before=[];after=[]
            for k,move in enumerate(parallel[1:]):
                src,dest=move.split(',')
                if src.startswith('l:') or dest.startswith('l:'):
                    snapshot,write=self.long_move(src,dest,k)
                    before.append(snapshot);after.append(write)
                else:
                    before.append(f'pm{k} = {self.word(src)};');after.append(self.store(dest,f'pm{k}'))
            base=Instruction(i.pc,i.size,i.words,op+' '+parallel[0])
            return ' '.join(before+[self.instruction(base)]+after)
        p = args.split(',')
        if op in ('nop','rts'):
            return '' if op == 'nop' else 'return;'
        if op == 'move' and len(p)==1:
            agu=re.fullmatch(r'\(r([0-7])\)([+-])(?:n([0-7]))?',args)
            if agu:
                reg,sign,stride=agu.groups()
                step=f'sx24((int32_t)n{stride})' if stride else '1'
                return f'(void)md_agu(&r{reg},m{reg},{"-" if sign=="-" else ""}{step},0u);'
        if op == 'move' and len(p)==2:
            if p[0].startswith('l:') or p[1].startswith('l:'):
                before,after=self.long_move(*p,0);return before+' '+after
            value=self.word(p[0])
            # The one-word MOVE immediate stores an 8-bit fractional value
            # left-aligned on data buses. Address/modifier immediates are
            # integers. The disassembler distinguishes full words with #>.
            if re.fullmatch(r'#\$[0-9a-f]+',p[0]) and re.fullmatch(r'[ab]|[xy][01]',p[1]):
                value=f'(({value}) << 16)'
            return self.store(p[1],value)
        if op=='clr' and p[0] in ('a','b'):
            return f'{p[0]}h = 0; {p[0]}l = 0u;'
        if op in ('mpy','mpysu','mpyuu','dmac','mpyr','mac','macr','mpyi','maci','macsu'):
            if op=='dmac':
                if not args.startswith('ss '):
                    raise Unsupported('DMAC signedness: '+args)
                p=args[3:].split(',')
            if len(p)!=3 or p[2] not in ('a','b'):
                raise Unsupported('Multiply form: '+args)
            d=p[2]
            negate=p[0].startswith('-');p[0]=p[0].lstrip('-')
            a,b=self.word(p[0]),self.word(p[1])
            fn={'mpy':'mpy_ss32','mpyr':'mpy_ss32','mac':'mpy_ss32','macr':'mpy_ss32','mpyi':'mpy_ss32','maci':'mpy_ss32','macsu':'mpy_su32','mpysu':'mpy_su32','mpyuu':'mpy_uu32','dmac':'mpy_ss32'}[op]
            a=f'sx24((int32_t)({a}))' if op!='mpyuu' else a
            b=f'sx24((int32_t)({b}))' if op not in ('mpysu','mpyuu','macsu') else b
            text=f'{fn}({a}, {b}, &th, &tl);'
            if negate:text+=' th = (int32_t)(0u - (uint32_t)th - (uint32_t)(tl != 0u)); tl = (0u - tl) & 0xffffffu;'
            if op=='dmac':
                text+=f' {d}l = u24({d}h); {d}h = {d}h >> 24; add56(th, tl, {d}h, {d}l, &{d}h, &{d}l);'
            elif op in ('mac','macr','maci','macsu'):
                text+=f' add56({d}h, {d}l, th, tl, &{d}h, &{d}l);'
            else:
                text+=f' {d}h = th; {d}l = tl;'
            if op in ('mpyr','macr'):
                text+=f' {d}h = (int32_t)((uint32_t){d}h + (uint32_t)({d}l > 0x800000u || ({d}l == 0x800000u && ({d}h & 1)))); {d}l = 0u;'
            return text
        if op in ('add','sub') and len(p)==2 and p[1] in ('a','b'):
            h,l=self.pair(p[0]); d=p[1]
            return f'{op}56({d}h, {d}l, {h}, {l}, &{d}h, &{d}l);'
        if op in ('asr','asl') and len(p)==1:
            return self.instruction(Instruction(i.pc,i.size,i.words,op+' #$1,'+p[0]+','+p[0]))
        if op in ('asr','asl') and len(p)==3 and p[1] in ('a','b') and p[2] in ('a','b'):
            if not p[0].startswith('#'):
                s,d=p[1:]
                step=(f'{d}l = (({d}l >> 1u) | ((uint32_t){d}h << 23u)) & 0xffffffu; {d}h = {d}h >> 1u;' if op=='asr' else f'{d}h = (int32_t)(((uint32_t){d}h << 1u) | ({d}l >> 23u)); {d}l = ({d}l << 1u) & 0xffffffu;')
                return f'th = {s}h; tl = {s}l; shift_count[0] = {self.word(p[0])} & 63u; {d}h = th; {d}l = tl; while (shift_count[0] != 0u) {{ '+step+' shift_count[0]--; }' 
            n=int(p[0].split('$')[1],16);s,d=p[1:]
            if not 0<=n<56: raise Unsupported('Shift domain')
            if n==0: return f'{d}h = {s}h; {d}l = {s}l;'
            if op=='asl':
                if n>=24:
                    return f'tl = {s}l; {d}h = (int32_t)(tl << {n-24}u); {d}l = 0u;'
                return f'th = {s}h; tl = {s}l; {d}h = (int32_t)(((uint32_t)th << {n}u) | (tl >> {24-n}u)); {d}l = (tl << {n}u) & 0xffffffu;'
            if n>=24:
                return f'th = {s}h; {d}l = (uint32_t)(th >> {n-24}u) & 0xffffffu; {d}h = th >> {min(n,31)}u;'
            return f'th = {s}h; tl = {s}l; {d}h = th >> {n}u; {d}l = ((tl >> {n}u) | ((uint32_t)th << {24-n}u)) & 0xffffffu;'
        if op=='and' and len(p)==2 and p[1] in ('a','b'):
            d=p[1]
            return f'{d}h = (int32_t)(((uint32_t){d}h & 0xff000000u) | (u24({d}h) & {self.word(p[0])}));'
        if op=='tst' and p[0] in ('a','b'):
            d=p[0]
            return f'zero = ({d}h == 0 && {d}l == 0u);'
        if op in ('neg','abs') and p[0] in ('a','b'):
            d=p[0];text=f'{d}h = (int32_t)(0u - (uint32_t){d}h - (uint32_t)({d}l != 0u)); {d}l = (0u - {d}l) & 0xffffffu;'
            return f'if ({d}h < 0) {{ {text} }}' if op=='abs' else text
        if op=='cmp' and len(p)==2 and p[1] in ('a','b'):
            h,l=self.pair(p[0]);d=p[1]
            return f'th = {h}; tl = {l}; zero = ({d}h == th && {d}l == tl); less = ({d}h < th || ({d}h == th && {d}l < tl)); greater = (!zero && !less); negative = less;'
        if op in ('tgt','tge','tlt','tle','teq','tne') and len(p)==2 and p[1] in ('a','b'):
            cond={'tgt':'greater','tge':'!less','tlt':'less','tle':'!greater','teq':'zero','tne':'!zero'}[op]
            return f'if ({cond}) {{ '+self.store(p[1],self.word(p[0]))+' }'
        if op=='tfr' and len(p)==2 and p[1] in ('a','b'):
            h,l=self.pair(p[0]);d=p[1]
            return f'{d}h = {h}; {d}l = {l};'
        if op=='max' and args=='a,b':
            return 'if (ah > bh || (ah == bh && al >= bl)) { bh = ah; bl = al; carry = 0u; } else { carry = 1u; }'
        if op in ('btst','brset','brclr'):
            bit=int(p[0].split('$')[1],16)
            value=self.word(p[1]);test=f'((({value}) >> {bit}u) & 1u)'
            if op=='btst':return f'carry = {test};'
            return f'if ({"!" if op=="brclr" else ""}{test}) goto L_{target(p[2]):x};'
        if op in ('lsl','lsr') and len(p)==2 and p[1] in ('a','b'):
            n=int(p[0].split('$')[1],16);d=p[1]
            if not 0<n<=24:raise Unsupported('Logical shift count')
            bit=24-n if op=='lsl' else n-1;shift='<<' if op=='lsl' else '>>'
            return f'carry = (u24({d}h) >> {bit}u) & 1u; {d}h = (int32_t)(((uint32_t){d}h & 0xff000000u) | ((u24({d}h) {shift} {n}u) & 0xffffffu)); zero = (u24({d}h) == 0u); negative = ((u24({d}h) >> 23) != 0u);'
        if op in ('bra','jmp','beq','bne','jeq','jne','blt','bgt','ble','bge','bmi','bpl','bcc','bcs'):
            condition={'bra':'','jmp':'','beq':'if (zero) ','jeq':'if (zero) ','bne':'if (!zero) ','jne':'if (!zero) ',
                       'blt':'if (less) ','bgt':'if (greater) ','ble':'if (!greater) ','bge':'if (!less) ',
                       'bmi':'if (negative) ','bpl':'if (!negative) ','bcc':'if (!carry) ','bcs':'if (carry) '}[op]
            return f'{condition}goto L_{target(args):x};'
        if op=='do' and len(p)==2:
            n=self.word(p[0]);end=target(p[1]);body=i.pc+i.size
            if end not in self.instructions:
                raise Unsupported('DO end is not a decoded instruction')
            # The reference disassembler prints LA+1 (the first instruction
            # after DO), not the inclusive hardware loop-address register.
            self.loops.setdefault(end,[]).append((i.pc,body))
            return f'loop_{i.pc:x} = {n}; if (loop_{i.pc:x} == 0u) goto L_{end:x};'
        if op=='lua' and len(p)==2 and re.fullmatch('r[0-7]',p[1]):
            match=re.fullmatch(r'\(r([0-7])([+-])\$([0-9a-f]+)\)',p[0])
            if match:
                return self.store(p[1],f'md_index(r{match[1]},m{match[1]},{"-" if match[2]=="-" else ""}0x{match[3]}u)')
            match=re.fullmatch(r'\(r([0-7])\)([+-])n([0-7])',p[0])
            if match:
                return self.store(p[1],f'md_index(r{match[1]}, m{match[1]}, {"" if match[2]=="+" else "-"}sx24((int32_t)n{match[3]}))')
            raise Unsupported('LUA addressing: '+args)
        if op=='rep':
            count=self.word(args);nexti=self.instructions[i.pc+i.size]
            if nexti.op in ('do','rep','dor','rts'):raise Unsupported('REP control instruction')
            return f'for (repeat = {count}; repeat > 0u; repeat--) {{ '+self.instruction(nexti)+f' }} goto L_{nexti.pc+nexti.size:x};'
        if op=='dor':
            match=re.fullmatch(r'>\*\+\$([0-9a-f]+)',p[1])
            if not match:raise Unsupported('DOR target')
            end=i.pc+int(match[1],16);self.loops.setdefault(end,[]).append((i.pc,i.pc+i.size))
            return f'loop_{i.pc:x} = {self.word(p[0])}; if (loop_{i.pc:x} == 0u) goto L_{end:x};'
        raise Unsupported(f'{i.pc:#x}: {i.text}')

    def long_move(self,src,dest,k):
        operand=src if src.startswith('l:') else dest
        match=re.fullmatch(r'l:[<>]?\$([0-9a-f]+)',operand)
        dynamic=False;prefix=''
        if match:
            address=int(match[1],16)
            if address>=256:raise Unsupported('Long scratch bound')
        else:
            match=re.fullmatch(r'l:\(r([0-7])\)([+-]?)(?:n([0-7]))?',operand)
            pre=False
            if not match:
                match=re.fullmatch(r'l:(-)\(r([0-7])\)',operand)
                if not match:raise Unsupported('Long memory form: '+operand)
                r,update,stride=match[2],'-',None;pre=True
            else:r,update,stride=match.groups()
            dynamic=True;address=f'long_address[{k}]'
            value=f'r{r}'
            if update:
                step=f'sx24((int32_t)n{stride})' if stride else '1'
                if update=='-':step='-('+step+')'
                value=f'md_agu(&r{r}, m{r}, {step}, {int(pre)}u)'
            prefix=f'{address} = {value}; '
        other=dest if src==operand else src
        if other not in ('a','b','a10','b10','x','y'):
            raise Unsupported('Long register pair: '+other)
        d=other[0]
        if src==operand:
            if dynamic:
                before=prefix+f'pm{k} = md_read_x(v, scratchx, {address}, sine); qm{k} = md_read_y(v, scratchy, {address}, sine);'
            else:before=f'pm{k} = u24(scratchx[{address}]); qm{k} = u24(scratchy[{address}]);'
            if d in ('a','b'):
                high=f'(int32_t)(((uint32_t){d}h & 0xff000000u) | (pm{k} & 0xffffffu))' if other.endswith('10') else f'sx24((int32_t)pm{k})'
                after=f'{d}h = {high}; {d}l = qm{k};'
            else:after=f'{d}1 = pm{k}; {d}0 = qm{k};'
        else:
            h,l=self.pair(d)
            if d in ('a','b') and other==d:
                h=f'u24(lim24({d}h))';l=f'({d}h > 0x7fffff ? 0xffffffu : {d}h < -0x800000 ? 0u : {d}l)'
            before=prefix+f'pm{k} = (uint32_t)({h}); qm{k} = {l};'
            if dynamic:after=f'md_write_x(v, scratchx, {address}, sine, pm{k}); md_write_y(v, scratchy, {address}, sine, qm{k});'
            else:after=f'scratchx[{address}] = sx24((int32_t)pm{k}); scratchy[{address}] = sx24((int32_t)qm{k});'
        return before,after

    def function(self, name, voice_type, entry):
        function_name = name
        code=reachable(self.instructions,entry)
        self.loops={}
        self.direct_addresses={}
        # When an AGU is confined to one 32-word scratch/output ring and its
        # address never escapes, only its low five bits are observable. Prove
        # those restrictions from all reachable instructions before lowering
        # accesses to direct array indexing. Other AGUs retain explicit modulo.
        for reg in range(8):
            r=f'r{reg}';bases=[];mods=[];escapes=False
            for i in code.values():
                if i.op=='move':
                    for part in re.split(r'\s{2,}',i.args):
                        p=part.split(',')
                        if len(p)!=2:continue
                        if p[0]==r:escapes=True
                        if p[1]==r:
                            if re.fullmatch(r'#[<>]?\$[0-9a-f]+',p[0]):bases.append(int(p[0].split('$')[1],16))
                            else:escapes=True
                        if p[1]==f'm{reg}':
                            if re.fullmatch(r'#[<>]?\$[0-9a-f]+',p[0]):mods.append(int(p[0].split('$')[1],16))
                            else:escapes=True
                elif re.search(r',\s*'+r+r'\b',i.args):escapes=True
            if reg==7 and not bases:bases=[0x100]
            if reg==7 and not mods:mods=[31]
            if not escapes and bases and set(mods)=={31} and len(set(b&~31 for b in bases))==1:
                base=bases[0]&~31
                if base<128 or reg==7:self.direct_addresses[str(reg)]=base
        for i in code.values():
            match=re.fullmatch(r'move\s+#[<>]?\$([0-9a-f]+),r[0-7]',i.text)
            if match and 0x100000<=int(match[1],16)<0x148000 and self.image:
                a=int(match[1],16)
                record=next((r for r in self.image.records if r.space=='P' and r.address<=a<r.end),None)
                if record is None:raise Unsupported(f'Unmapped immutable source {a:#x}')
                self.tables[record.address]=record.words
        # Prepass discovers loop boundaries even when the loop has an exit.
        emitted={pc:self.instruction(i) for pc,i in code.items()}
        lines=[f'void {name}({voice_type} *v, const int32_t *sine) {{',
               '    int32_t ah = 0, bh = 0, th = 0;',
               '    uint32_t al = 0, bl = 0, tl = 0;',
               '    uint32_t x0 = 0, x1 = 0, y0 = 0, y1 = 0;',
               '    uint32_t '+', '.join(f'{r}{k} = '+('0x800u' if r=='r' and k==6 else '0x100u' if r=='r' and k==7 else '0xffffffu' if r=='m' and k==7 else '0xffffffu' if r=='m' else '0u') for r in 'rmn' for k in range(8))+';',
               '    uint32_t zero = 0, less = 0, greater = 0, negative = 0, carry = 0;',
               '    uint32_t repeat = 0;',
               '    uint32_t long_address[3] = {0,0,0};',
               '    uint32_t pm0 = 0, pm1 = 0, pm2 = 0, qm0 = 0, qm1 = 0, qm2 = 0;',
               '    uint32_t shift_count[1];', f'    int32_t scratchx[{self.scratch_x_words}];', f'    int32_t scratchy[{self.scratch_y_words}];']
        # Give temporary storage defined C values. Persistent/read-before-write
        # dependencies still have to match the original state oracle.
        used_scratch=[space for space in ('x','y') if any('scratch'+space in body for body in emitted.values())]
        lines=[line for line in lines if not any(line.startswith('    int32_t scratch'+space+'[') and space not in used_scratch for space in ('x','y'))]
        if used_scratch: lines.append('    uint32_t clear_scratch;')
        for space in used_scratch:
            length=self.scratch_x_words if space=='x' else self.scratch_y_words
            if space=='y' and self.buffered_output:
                lines.append(f'    for (clear_scratch=256u;clear_scratch<{length}u;clear_scratch++) scratch{space}[clear_scratch]=0;')
        for pc,i in code.items():
            if i.op in ('do','dor'):
                lines.append(f'    uint32_t loop_{pc:x} = 0;')
        for pc,i in code.items():
            lines += [f'L_{pc:x}: /* {i.text} */', '    '+emitted[pc]]
            if i.op!='rts':
                lines += [f'    MD_TRANSLATION_TRACE(0x{pc:x}u,ah,al,bh,bl,x0,x1,y0,y1,r2,r4,r7);']
            if pc+i.size in self.loops:
                for loop,body in sorted(self.loops[pc+i.size],reverse=True):
                    if loop in code:
                        lines += [f'    if (--loop_{loop:x} != 0u) goto L_{body:x};']
        lines += ['}', '']
        text='\n'.join(lines)
        if self.buffered_output and name.endswith('_render_block_44k1'):
            text=text.replace('return;', 'v->scratch_y20_history = scratchy[32]; for (repeat=0u;repeat<32u;repeat++) v->blk44[repeat] = scratchy[256u+repeat]; return;')
            text=text.replace(f'L_{entry:x}:', f'scratchy[32] = v->scratch_y20_history;\nL_{entry:x}:')
        if self.private_memory:text=text.replace('r6 = 0x800u','r6 = (0x800u + 64u * v->virtual_voice)')
        # One serialized working bank, independent of owned per-voice delays.
        # Preserve the recovered original shared Y20 context across voices.
        if 'y' in used_scratch and self.scratch_y_words>32:
            text=text.replace('scratchy[32] = v->scratch_y20_history;', '')
            text=text.replace(f'L_{entry:x}:', f'scratchy[32] = sx24((int32_t)md_shared_y20);\nL_{entry:x}:')
            text=text.replace('return;', 'md_shared_y20 = u24(scratchy[32]); return;')
        text=re.sub(r'^    int32_t scratch[xy]\[\d+\];\n', '', text, flags=re.M)
        text=re.sub(r'\bscratchx\b','md_shared_scratch_x',text)
        text=re.sub(r'\bscratchy\b','md_shared_scratch_y',text)
        # Goto backedges must have memory-visible register values. The pinned
        # compiler propagates some scalar temporaries across these joins;
        # addressable banks avoid that transformation without changing any DSP
        # arithmetic or introducing a runtime instruction dispatcher.
        text=text.replace('int32_t ah = 0, bh = 0, th = 0;', 'int32_t accum[3] = {0,0,0};')
        text=text.replace('uint32_t al = 0, bl = 0, tl = 0;', 'uint32_t lows[3] = {0,0,0};')
        banks=[('words',('x0','x1','y0','y1')),
               ('addresses',tuple(f'{r}{k}' for r in 'rmn' for k in range(8))),
               ('flags',('zero','less','greater','negative','carry')),
               ('parallel_words',('pm0','pm1','pm2','qm0','qm1','qm2'))]
        for bank,names in banks:
            pattern=r'    uint32_t '+r', '.join(re.escape(n)+r' = ([^,;]+)' for n in names)+r';'
            match=re.search(pattern,text)
            if match is None: raise Unsupported('Internal register bank declaration')
            text=text[:match.start()]+f'    uint32_t {bank}[{len(names)}] = {{'+','.join(match.groups())+'};'+text[match.end():]
            for k,name in enumerate(names):
                text=re.sub(r'\b'+name+r'\b',f'{bank}[{k}]',text)
        for k,name in enumerate(('ah','bh','th')):
            text=re.sub(r'\b'+name+r'\b',f'accum[{k}]',text)
        for k,name in enumerate(('al','bl','tl')):
            text=re.sub(r'\b'+name+r'\b',f'lows[{k}]',text)
        from tools.machines.translation.outlining import outline
        if self.outline_native_groups:
            text=outline(text,code,function_name,voice_type)
        return text, code
