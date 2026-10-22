"""Experimental source-equal E12 kernel sharing, no algorithm approximation."""
import hashlib,json,re
from pathlib import Path
from tools.machines.registry import ROOT

def normalized(m,text):
    p=m['prefix']
    externs=set(re.findall(r'\b[A-Za-z_]\w*\b','\n'.join(re.findall(r'\bextern\b[^;]+;',text))))
    bindings=set(re.findall(r'\b[A-Za-z_]\w*[\s*]+([A-Za-z_]\w*)\s*(?=[\[=;])',text))
    def identifier(match):
        token=match[0]
        if token[0] in ('"',"'") or token in externs or token in bindings:return token
        for a,b in ((p.upper(),'E12FAMILY'),(p.title(),'E12Family'),(p,'e12family')):
            if token.startswith(a):return b+token[len(a):]
        return token
    # A hexadecimal data word must never be mistaken for a machine prefix.
    return re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|\b[A-Za-z_]\w*\b',identifier,text)


class E12Sharing:
    def __init__(self,entries,sources=None):
        self.entries=[m for m in entries if m['name'].startswith('E12-')]
        self.originals={m['name']:(sources[m['name']] if sources is not None else (ROOT/m['source']).read_text()) for m in self.entries}
        self.word_counts={m['name']:int(re.search(r'int32_t y\[(\d+)\]',self.voice_definition(m))[1]) for m in self.entries}
        entries=self.entries;originals=self.originals
        segments={};groups={}
        for m in entries:
            text=originals[m['name']];p=m['prefix']
            markers=[text.index('#ifndef '+p.upper()+'_'+op+'_EXEC_DEFINED') for op in ('DSP_INIT','DSP_TRIGGER','RENDER_BLOCK_44K1')]
            markers.append(re.search(r'\n(?:void|uint32_t) '+p+r'_compute_coeffs\(',text).start())
            preamble=text[:markers[0]]
            for i,op in enumerate(('dsp_init','dsp_trigger','render_block_44k1')):
                segment=text[markers[i]:markers[i+1]]
                canonical=normalized(m,preamble+segment)
                key=(op,hashlib.sha256(canonical.encode()).hexdigest())
                groups.setdefault(key,[]).append(m)
                segments[(m['name'],op)]=segment

        groups={key:members for key,members in groups.items() if len(members)>1}
        aliases={};voice_groups={};records=[]
        for (op,digest),members in groups.items():
            anchor=members[0]
            canonical_voice='MdE12Voice'+str(self.word_counts[anchor['name']])
            for m in members:voice_groups[m['name']]=canonical_voice
            for m in members[1:]:aliases[(m['name'],op)]=anchor
            records.append(dict(operation=op,normalized_preamble_and_kernel_sha256=digest,anchor=anchor['name'],members=[m['name'] for m in members],original_slot_words=self.word_counts[anchor['name']]))
        self.segments=segments;self.groups=groups;self.aliases=aliases;self.voice_groups=voice_groups;self.records=records

    def voice_definition(self,m):
        text=self.originals[m['name']]
        pattern=r'typedef struct \{.*?\} '+re.escape(m['voice_type'])+r';'
        found=re.search(pattern,text,re.S)
        assert found,m['name']
        return found.group()

    def prepare(self,folder):
        definitions={}
        for m in self.entries:
            if m['name'] not in self.voice_groups:continue
            name=self.voice_groups[m['name']]
            definition=self.voice_definition(m).replace(m['voice_type'],name)
            if name in definitions:assert definitions[name]==definition,m['name']
            definitions[name]=definition
        header='#ifndef MD_E12_SHARED_VOICE_DEFINED\n#define MD_E12_SHARED_VOICE_DEFINED\n'+'\n'.join(definitions.values())+'\n#endif\n'
        (folder/'e12_family_voice.h').write_text(header)
        (folder/'family-sharing.json').write_text(json.dumps(dict(status='EXPERIMENTAL-source-equivalence-only',groups=self.records,eliminated_bodies=len(self.aliases),probe_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()),indent=2)+'\n')

    def fold_source(self,m,text):
        if m['name'] not in self.voice_groups:return text
        canonical=self.voice_groups[m['name']]
        definition=self.voice_definition(m)
        assert text.count(definition)==1
        text=text.replace(definition,'#include "e12_family_voice.h"\ntypedef '+canonical+' '+m['voice_type']+';')
        for op in ('dsp_init','dsp_trigger','render_block_44k1'):
            anchor=self.aliases.get((m['name'],op))
            if not anchor:continue
            segment=self.segments[(m['name'],op)]
            assert text.count(segment)==1
            target=anchor['prefix']+'_'+op
            wrapper='extern void '+target+'('+canonical+' *, const int32_t *);\n'
            wrapper+='void '+m['prefix']+'_'+op+'('+m['voice_type']+' *v, const int32_t *sine) {\n    '+target+'(v, sine);\n}\n'
            text=text.replace(segment,wrapper)
        return text
