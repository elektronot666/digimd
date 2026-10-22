"""Static lowering of original ColdFire coefficient routines to native C.

Address-register provenance resolves parameter/slot/table accesses at generation
time. No 68k instructions or virtual CPU are shipped in the audio callback.
"""
import re
from dataclasses import dataclass
from tools.machines.translation.dsp import Unsupported


@dataclass(frozen=True)
class Pointer:
    kind: str
    offset: int = 0
    index: str = '0u'


def split(text):
    parts, start, depth = [], 0, 0
    for i,c in enumerate(text):
        depth += (c=='(')-(c==')')
        if c==',' and not depth:
            parts.append(text[start:i].strip()); start=i+1
    parts.append(text[start:].strip())
    return parts


class Control:
    def __init__(self, cpu, entry, table_bounds):
        import capstone
        self.cpu=cpu
        self.entry=entry
        self.decoder=capstone.Cs(capstone.CS_ARCH_M68K,capstone.CS_MODE_M68K_040)
        self.pointers={}
        self.depth=0
        self.tables={}
        self.table_bounds=table_bounds

    def address(self, operand):
        m=re.fullmatch(r'\$([0-9a-f]+)\.l',operand)
        if m: return Pointer('rom',int(m[1],16)), '0u'
        m=re.fullmatch(r'(?:([-]?)\$([0-9a-f]+))?\((a[0-7])(?:, ([ad][0-7])\.l(?: \* ([1248]))?)?\)',operand)
        if not m: raise Unsupported('Control address: '+operand)
        offset=int(m[2] or '0',16)*(-1 if m[1]=='-' else 1)
        if m[3]=='a7':
            n=offset-self.depth
            if n==4: return Pointer('argument_out'), '0u'
            if n==8: return Pointer('argument_params'), '0u'
            raise Unsupported('Unresolved stack argument: '+operand)
        ptr=self.pointers.get(m[3])
        if ptr is None: raise Unsupported('Unknown pointer: '+m[3])
        index=f'{m[4]} * {m[5] or "1"}u + ({offset})' if m[4] else f'{offset}u' if offset>=0 else f'({offset})'
        if ptr.index!='0u': index=f'({index}) + {ptr.index}'
        return ptr,index

    def read(self,operand,size):
        if re.fullmatch('d[0-7]',operand):
            return operand if size==4 else f'({operand} & 0x{(1<<(8*size))-1:x}u)'
        if re.fullmatch('a[0-6]',operand):
            ptr=self.pointers.get(operand)
            if ptr and ptr.kind=='numeric': return operand
            raise Unsupported('Pointer converted to value: '+operand)
        if operand.startswith('#$'): return '0x'+operand[2:]+'u'
        ptr,index=self.address(operand)
        if ptr.kind=='rom' and ptr.offset==0x100150c and index=='0u' and size==4:
            return 'md_control_tempo_denominator'
        if ptr.kind=='params':
            if size==1:
                return f'((*(p + ({index}) / 2u) >> ((({index}) & 1u) ? 0u : 8u)) & 0xffu)'
            if size==4:
                return f'((*(p + ({index}) / 2u) << 16) | *(p + ({index}) / 2u + 1u))'
            if size!=2: raise Unsupported('Parameter width')
            return f'*(p + ({index}) / 2u)'
        if ptr.kind=='out':
            if size!=4: raise Unsupported('Output read width')
            return f'(uint32_t)*(y + ({index}) / 4u)'
        if ptr.kind=='rom':
            if size!=4: raise Unsupported('Table read width')
            segment=next(((base,n) for base,n in self.table_bounds.items() if base<=ptr.offset<base+n*4),None)
            if segment is None:
                raise Unsupported(f'Table bound not recovered: {ptr.offset:#x}')
            base,words=segment
            self.tables[base]=words
            return f'*(control_{base:x} + (({ptr.offset-base}u + ({index})) / 4u))'
        raise Unsupported('Pointer value used as data')

    def store(self,dest,size,value):
        if re.fullmatch('d[0-7]',dest):
            if size==4: return f'{dest} = {value};'
            mask=(1<<(size*8))-1
            return f'{dest} = ({dest} & 0x{0xffffffff^mask:x}u) | (({value}) & 0x{mask:x}u);'
        ptr,index=self.address(dest)
        if ptr.kind!='out' or size!=4: raise Unsupported('Control write: '+dest)
        return f'*(y + ({index}) / 4u) = (int32_t)({value});'

    def function(self,name):
        instructions={}
        pending=[self.entry]
        while pending:
            pc=pending.pop()
            if pc in instructions: continue
            i=next(self.decoder.disasm(self.cpu[pc-0x200000:pc-0x200000+22],pc,count=1),None)
            if i is None: raise Unsupported(f'Invalid control instruction {pc:#x}')
            instructions[pc]=i
            op=i.mnemonic.split('.')[0]
            if op=='rts': continue
            if op.startswith('b'):
                if op not in ('bra','bhi','bls','beq','bne','bcs','bcc','bge','bgt','ble','blt'):
                    raise Unsupported('Control flow: '+i.mnemonic)
                pending.append(int(i.op_str.lstrip('$'),16))
                if op=='bra': continue
            pending.append(pc+i.size)
        # Pointer assignments in a routine must have unambiguous static
        # provenance. Branches may change numeric data, never pointer bindings.
        lines=[f'uint32_t {name}(const uint32_t *p, int32_t *y) {{',
               # Keep the translated register file addressable: selcc's
               # scalar propagation ignores volatile at some goto joins.
               '    uint32_t d[8] = {0,0,0,0,0,0,0,0};',
               '    uint32_t '+', '.join(f'a{k} = 0' for k in range(7))+';',
               '    uint32_t lhs = 0, rhs = 0;',
               '    uint32_t cf_carry[1] = {0};', '    uint32_t cf_temp[1] = {0};']
        occupied=set()
        for pc,i in sorted(instructions.items()):
            if any(a in occupied for a in range(pc,pc+i.size)):
                raise Unsupported('Overlapping control instructions')
            occupied.update(range(pc,pc+i.size))
            mnemonic=i.mnemonic;op=mnemonic.split('.')[0]
            size={'b':1,'w':2,'l':4}.get(mnemonic.split('.')[-1],4)
            args=split(i.op_str);text=None
            if op=='rts': text='return d0;'
            elif op in ('move','movea'):
                src,dest=args
                if dest=='-(a7)':
                    self.depth+=size;text='/* local register save */'
                elif src=='(a7)+':
                    self.depth-=size;text='/* local register restore */'
                elif dest.startswith('a'):
                    if re.fullmatch('d[0-7]',src) or src.startswith('#$') or re.fullmatch('a[0-6]',src):
                        if re.fullmatch('a[0-6]',src) and self.pointers.get(src,Pointer('unknown')).kind!='numeric':
                            self.pointers[dest]=self.pointers[src];text='/* resolved pointer copy */'
                        else:
                            self.pointers[dest]=Pointer('numeric',index=dest);text=f'{dest} = {self.read(src,4)};'
                    else:
                        ptr,index=self.address(src)
                        if ptr.kind.startswith('argument_'):
                            ptr=Pointer(ptr.kind[9:]);text='/* resolved argument pointer */'
                        else:
                            # MOVEA loads the addressed word, rather than taking
                            # its address (LEA). It can be a numeric coefficient.
                            text=f'{dest} = {self.read(src,4)};';ptr=Pointer('numeric',index=dest)
                        self.pointers[dest]=ptr
                else: text=self.store(dest,size,self.read(src,size))
            elif op=='lea':
                if args[1]=='a7':
                    match=re.fullmatch(r'(-?)\$([0-9a-f]+)\(a7\)',args[0])
                    if not match: raise Unsupported('Stack LEA')
                    self.depth += int(match[2],16) * (1 if match[1] else -1)
                    text='/* local register save area */'
                else:
                    ptr,index=self.address(args[0]);dest=args[1]
                    if index!='0u':
                        text=f'{dest} = {index};';ptr=Pointer(ptr.kind,ptr.offset,dest)
                    else:text='/* resolved table pointer */'
                    self.pointers[dest]=ptr
            elif op=='movem':
                text='/* native locals replace original callee saves */'
            elif op=='adda':
                dest=args[1];ptr=self.pointers.get(dest)
                if not args[0].startswith('#$'):
                    if ptr is None:raise Unsupported('Dynamic ADDA pointer')
                    value=self.read(args[0],4)
                    text=f'{dest} = {ptr.index} + {value};'
                    self.pointers[dest]=Pointer(ptr.kind,ptr.offset,dest)
                    lines += [f'C_{pc:x}: /* {i.mnemonic} {i.op_str} */','    '+text]
                    continue
                offset=int(args[0][2:],16)
                if ptr is None: raise Unsupported('ADDA pointer')
                self.pointers[dest]=Pointer('rom' if ptr.kind=='numeric' else ptr.kind,ptr.offset+offset,ptr.index)
                text='/* resolved pointer base */'
            elif op in ('moveq','clr'):
                value='0u'
                if op=='moveq':
                    immediate=int(args[0].lstrip('#$'),16)&255
                    if immediate>=128: immediate-=256
                    value=f'0x{immediate&0xffffffff:08x}u'
                text=self.store(args[-1],size,value)
            elif op in ('lsr','asr','lsl','asl'):
                if size!=4: raise Unsupported('Non-long shift')
                n=self.read(args[0],4);d=args[1]
                if op=='lsr': expr=f'((({n}) & 63u) < 32u ? {d} >> (({n}) & 63u) : 0u)'
                elif op=='asr': expr=f'(uint32_t)((int32_t){d} >> ((({n}) & 63u) < 32u ? (({n}) & 63u) : 31u))'
                else: expr=f'((({n}) & 63u) < 32u ? {d} << (({n}) & 63u) : 0u)'
                text=self.store(d,4,expr)
            elif op in ('mulu','muls'):
                s=self.read(args[0],size);d=args[1]
                if size==2:
                    expr=f'({d} & 0xffffu) * ({s})' if op=='mulu' else f'(uint32_t)((int32_t)(int16_t){d} * (int32_t)(int16_t)({s}))'
                else: expr=f'{d} * ({s})'
                text=self.store(d,4,expr)
            elif op in ('divs','divu'):
                if size!=4 or ':' in args[-1]:raise Unsupported('Wide divide')
                fn='md_divs32' if op=='divs' else 'md_divu32'
                text=self.store(args[-1],4,f'{fn}({self.read(args[-1],4)}, {self.read(args[0],4)})')
            elif op in ('neg','not'):
                if size!=4:raise Unsupported('Non-long unary control operation')
                text=self.store(args[0],4,('0u - ' if op=='neg' else '~')+self.read(args[0],4))
            elif op in ('extb','ext'):
                if size!=4:raise Unsupported('Non-long sign extension')
                text=self.store(args[0],4,f'(uint32_t)(int32_t)({"int8_t" if op=="extb" else "int16_t"}){self.read(args[0],4)}')
            elif op in ('seq','sne'):
                text=self.store(args[0],1,f'(lhs {"==" if op=="seq" else "!="} rhs ? 0xffu : 0u)')
            elif op=='subx':
                if size!=4:raise Unsupported('Non-long SUBX')
                text=f'lhs = {self.read(args[1],4)}; rhs = {self.read(args[0],4)}; cf_temp[0] = lhs - rhs - cf_carry[0]; cf_carry[0] = (lhs < rhs || (cf_carry[0] && lhs == rhs)); '
                text+=self.store(args[1],4,'cf_temp[0]')
            elif op=='swap':
                d=args[0];text=f'{d} = ({d} << 16) | ({d} >> 16);'
            elif op in ('addq','subq'):
                d=args[1];symbol='+' if op=='addq' else '-'
                text=self.store(d,size,f'{self.read(d,size)} {symbol} {self.read(args[0],size)}')
            elif op in ('add','addi','sub','subi','and','andi','or','ori','cmp','cmpi','cmpa','tst'):
                if size!=4: raise Unsupported('Non-long arithmetic')
                d=args[-1];s='0u' if op=='tst' else self.read(args[0],size)
                text=f'lhs = {self.read(d,size)}; rhs = {s};'
                if op not in ('cmp','cmpi','cmpa','tst'):
                    symbol='+' if op.startswith('add') else '-' if op.startswith('sub') else '&' if op.startswith('and') else '|'
                    if op.startswith('add'):text+=' cf_carry[0] = (lhs > 0xffffffffu - rhs);'
                    elif op.startswith('sub'):text+=' cf_carry[0] = (lhs < rhs);'
                    text+=' '+self.store(d,size,f'lhs {symbol} rhs')
            elif op.startswith('b'):
                cond={'bra':'','bhi':'lhs > rhs','bls':'lhs <= rhs','beq':'lhs == rhs','bne':'lhs != rhs','bcc':'lhs >= rhs','bcs':'lhs < rhs',
                      'bgt':'(int32_t)lhs > (int32_t)rhs','bge':'(int32_t)lhs >= (int32_t)rhs','blt':'(int32_t)lhs < (int32_t)rhs','ble':'(int32_t)lhs <= (int32_t)rhs'}.get(op)
                if cond is not None:
                    text=(f'if ({cond}) ' if cond else '')+f'goto C_{int(args[0].lstrip("$"),16):x};'
            if text is None: raise Unsupported(f'Control {pc:#x}: {i.mnemonic} {i.op_str}')
            lines += [f'C_{pc:x}: /* {i.mnemonic} {i.op_str} */', '    '+text]
        lines += ['}', '']
        text='\n'.join(lines)
        text=re.sub(r'\bd([0-7])\b',lambda m:f'd[{m[1]}]',text)
        return text, instructions

    def data(self):
        return ('extern uint32_t md_control_tempo_denominator;\nextern uint32_t md_divs32(uint32_t, uint32_t);\nextern uint32_t md_divu32(uint32_t, uint32_t);\n'+
                ''.join(f'extern const uint32_t control_{address:x}[{n}];\n' for address,n in self.tables.items()))
