"""Small bounds-checked ELF32 reader; preserves SHARC SW versus byte units.

No reliance on symbol sizes: selache often emits zero ST_SIZE. Component spans
must be explicitly named in a manifest, or bytes remain unclassified.
"""
import hashlib
import struct
from pathlib import Path


def read_elf(path):
    data=Path(path).read_bytes()
    if len(data)<52 or data[:5]!=b'\x7fELF\x01' or data[5] not in (1,2):
        raise ValueError('Require ELF32 with known byte order')
    endian='<' if data[5]==1 else '>'
    def unpack(fmt, offset):
        size=struct.calcsize(endian+fmt)
        if offset<0 or offset+size>len(data):raise ValueError('Truncated ELF structure')
        return struct.unpack_from(endian+fmt,data,offset)
    hdr=unpack('HHIIIIIHHHHHH',16)
    typ,machine,_,entry,_,shoff,_,_,_,_,stride,count,strings=hdr
    if stride<40 or count==0 or strings>=count:raise ValueError('Invalid section table')
    headers=[unpack('IIIIIIIIII',shoff+i*stride) for i in range(count)]
    def content(sh):
        if sh[1]==8:return b''
        offset,size=sh[4:6]
        if offset+size>len(data):raise ValueError('Truncated ELF section')
        return data[offset:offset+size]
    def string(sh,offset):
        buf=content(sh)
        if offset>=len(buf):raise ValueError('Invalid ELF string offset')
        end=buf.find(b'\0',offset)
        if end<0:raise ValueError('Unterminated ELF string')
        return buf[offset:end].decode('utf-8')
    sections=[];symbols={}
    for index,sh in enumerate(headers):
        name=string(headers[strings],sh[0])
        _,kind,flags,addr,off,size,link,info,align,entsize=sh
        if kind in (1,8) and flags&2:
            sw=bool(machine==0x85 and flags&4 and entsize==1)
            if sw and (size%2 or addr<0):raise ValueError('Odd SW code size')
            byte_addr=(0x20000000+2*(addr-0xb80000) if 0xb80000<=addr<0xc00000 else 2*addr) if sw else addr
            payload=bytes(size) if kind==8 else content(sh)
            if sw:payload=b''.join(payload[i:i+2][::-1] for i in range(0,size,2))
            sections.append(dict(payload_sha256=hashlib.sha256(payload).hexdigest(),index=index,name=name,address=addr,byte_address=byte_addr,size=size,alignment=align,unit='SW16' if sw else 'byte',kind='bss' if kind==8 else 'code' if flags&4 else 'data'))
        elif kind==2:
            if link>=count or entsize not in (0,16) or size%16:raise ValueError('Invalid symbol table')
            for at in range(off,off+size,16):
                sn,val,ss,si,so,sec=unpack('IIIBBH',at)
                if sn:
                    symbols[string(headers[link],sn)]=dict(value=val,size=ss,section=sec,defined=sec!=0)
    return dict(type=typ,machine=machine,entry=entry,sections=sections,symbols=symbols)
