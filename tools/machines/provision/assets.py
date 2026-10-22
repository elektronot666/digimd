"""Lossless sparse boot-word container prototype. No storage/device transport."""
from pathlib import Path
import hashlib,struct,zlib,json
ROOT=Path.cwd();OUT=ROOT/'out/machines/provision';MAGIC=b'DMDAS001';BASE=0x20000;WORDS=0x1e0000
HEADER=struct.Struct('<8s4I32s32s')
def encode(data):
 words=struct.unpack('<'+str(len(data)//4)+'I',data);payload=bytearray();i=0
 while i<len(words):
  if not words[i]:
   end=i+1
   while end<len(words) and not words[end]:end+=1
   payload+=struct.pack('<BI',0,end-i)
  else:
   end=i+1
   while end<len(words):
    if end+16<=len(words) and not any(words[end:end+16]):break
    end+=1
   payload+=struct.pack('<BI',1,end-i)+b''.join(v.to_bytes(3,'little') for v in words[i:end])
  i=end
 fw=hashlib.sha256((ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx').read_bytes()).digest()
 return HEADER.pack(MAGIC,BASE,len(words),len(payload),zlib.crc32(payload),fw,hashlib.sha256(data).digest())+payload

def decode(blob):
 if len(blob)<HEADER.size:raise ValueError('Short header')
 magic,base,n,length,crc,fw,digest=HEADER.unpack_from(blob);payload=blob[HEADER.size:]
 if magic!=MAGIC or base!=BASE or n!=WORDS or length!=len(payload) or zlib.crc32(payload)!=crc:raise ValueError('Container identity/length/checksum')
 if fw!=hashlib.sha256((ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx').read_bytes()).digest():raise ValueError('Firmware identity')
 words=[];i=0
 while i<len(payload):
  if len(payload)-i<5:raise ValueError('Short run')
  kind,count=struct.unpack_from('<BI',payload,i);i+=5
  if not count or count>n-len(words):raise ValueError('Run bound')
  if kind==0:words.extend([0]*count)
  elif kind==1:
   if len(payload)-i<3*count:raise ValueError('Short raw data')
   words.extend(int.from_bytes(payload[j:j+3],'little') for j in range(i,i+3*count,3));i+=3*count
  else:raise ValueError('Run kind')
 if len(words)!=n:raise ValueError('Missing words')
 result=struct.pack('<'+str(n)+'I',*words)
 if hashlib.sha256(result).digest()!=digest:raise ValueError('Decoded SHA256')
 return result

def run():
 OUT.mkdir(parents=True,exist_ok=True)
 data=(ROOT/'out/machines/translation/P.u32').read_bytes()[BASE*4:0x200000*4]
 blob=encode(data);assert decode(blob)==data
 (OUT/'md-original-1.63.assets').write_bytes(blob)
 # Require every documented E12 source/guard/directory word, not just audible bodies.
 spans=json.loads((ROOT/'data/datasets.json').read_text())['e12'];recovered=decode(blob);words=struct.unpack('<'+str(WORDS)+'I',recovered)
 original=struct.unpack('<2097152I',(ROOT/'out/machines/translation/P.u32').read_bytes())
 covered=[]
 for source in spans['sources']:
  a=int(source['pointer'],16);end=int(source['end_exclusive'],16)
  assert words[a-BASE:end-BASE]==original[a:end]
  covered.append(dict(index=source['index'],original_address=hex(a),words=end-a,guard_words=153))
 assert words[0x103d7b-BASE:0x103dba-BASE]==original[0x103d7b:0x103dba]
 rejected=[]
 for name,bad in [('truncated',blob[:-1]),('magic',b'X'+blob[1:]),('corrupt',blob[:-1]+bytes([blob[-1]^1])),('trailing',blob+b'X')]:
  try:decode(bad)
  except ValueError:rejected.append(name)
  else:raise AssertionError(name)
 report=dict(schema='digimd.asset-prototype.v1',status='PASS',packed_bytes=len(blob),native_bytes=len(data),base=hex(BASE),words=WORDS,payload_crc32=hex(zlib.crc32(blob[HEADER.size:])),container_sha256=hashlib.sha256(blob).hexdigest(),native_sha256=hashlib.sha256(data).hexdigest(),directory_words=63,sources=covered,rejected=rejected,limits=['Host format/word recovery only; native decoder, immutable memory ownership, real storage startup transfer and firmware integration need independent qualification.','Complete boot image deliberately preserves unexpected table/code reads as well as sample source words.','No device I/O or runtime overlays.'])
 (OUT/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print({k:v for k,v in report.items() if k not in ('sources',)})
if __name__=='__main__':run()
