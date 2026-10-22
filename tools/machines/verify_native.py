"""Whole-handler comparisons with frozen dual kernels and experimental L2.

Original DSP56300 oracles remain the PCM/state truth gate. This independent
integration matrix checks complete stock FX, lifecycle, ABI and overhead.
"""
import argparse
import hashlib
import json
import math
import struct
from concurrent.futures import ProcessPoolExecutor
from tools.machines.registry import ROOT
from tools.machines.build import POOL_BYTES
from tools.trxbd_sharc.integration import create_runner, configure
from tools.trxbd_sharc.benchmark_engines import stats
from tools.trxbd_sharc.integrity import trace_hook
from tools.trxbd_sharc.build_sharc import VOICE_RESUME_SW

STREAMS = {
    'frozen': ROOT/'out/resource-audit/experiments/l1/section7_trxbd.bin',
    'l1': ROOT/'out/machines/l1/section7_trxbd.bin',
    'l2-experimental': ROOT/'out/machines/l2-experimental/section7_trxbd.bin',
}
OFFSETS=(250,252,244,248,254,256,262,260)
_PRISTINE = {}


def runner_for(key, stream, keep_policy=False):
    """Reuse actual FXINIT results within a worker; reset all mapped memory.

    The expensive startup (including exact sine generation) executes once per
    binary per worker. No synthetic initializer or audio golden is substituted.
    """
    import ctypes as C
    import vm as V
    import vmrun
    digest=hashlib.sha256(stream.read_bytes()).hexdigest()
    if key not in _PRISTINE:
        vm,_=create_runner(stream.read_bytes())
        memory={name:bytes(buf) for name,(_,_,buf) in vm.mem.items()}
        _PRISTINE[key]=(digest,memory)
    saved,memory=_PRISTINE[key]
    assert saved==digest,'Binary changed during the matrix'
    # Match the pinned VM's dn2_after_init cache: fresh stock CPU/ABI context,
    # exact post-FXINIT memory. Universal register reads are not a CPU snapshot
    # API (notably DAG lengths and PC-stack registers have special semantics).
    vm=V.VM();vm.load_dn2()
    for name,(_,_,buf) in vm.mem.items(): C.memmove(C.addressof(buf),memory[name],len(memory[name]))
    if not keep_policy:disable_load_policy(vm)
    return vm,vmrun.Runner(vm,{'PATTERN_VOL':127})


ROUTE=0x8181d000
ROUTE_MAGIC=0x4d445254


def disable_load_policy(vm):
    """Original-parity runs measure synthesis, not real-time scheduling.

    Full49 renderers with voice routing carry a governor and a dormancy policy
    (MdRoute.policy, both bit-exact no-ops while not engaged). The interpreter's
    EMUCLK counts instructions, so its call intervals are not a block period;
    switch both policies off for parity matrices. Streams without the routing
    block are untouched."""
    if vm.rd32(ROUTE)==ROUTE_MAGIC:vm.wr32(ROUTE+8,0)


def state_address(t): return 0x300000 + 8 + 336*t + 12


def controls(frame, types, patch, b):
    for t,m in enumerate(types):
        if m in (5,6):
            syn=(64,64,0,0,0,0,0,0) if m==5 else (64,32,0,64,64,64,64,0)
            if patch=='max': syn=(127,)*8
            if patch=='noise': syn=(32,127,127,127,64,127,127,127) if m==5 else (32,127,0,64,127,0,64,127)
            for k,(off,val) in enumerate(zip(OFFSETS,syn)):
                raw=(b*139+t*271+k*197)%0x7f01 if patch=='locks' and b%3==0 else val<<8
                struct.pack_into('<H',frame,off+146*t,raw)


def types_at(kind,n,b):
    if kind=='stock-switch': return [(5,6,0,1,2,3)[(b//3+t)%6] for t in range(n)]
    if kind=='stock-mixed': return [5+(t%2) if t<n//2 else (t-n//2)%4 for t in range(n)]
    if kind=='stock': return [t%4 for t in range(n)]
    return [(5 if kind=='BD' else 6 if kind=='SD' else 5+(t%2) if kind=='mixed' else 5+((b//3+t)%2)) for t in range(n)]


def compare(work, *, streams=None, max_regression=1.01):
    # Diagnostic callers may supply a separately identified stream and explicit
    # instrumentation budget. The production matrix retains its 1% screen.
    streams = STREAMS if streams is None else streams
    kind,n,patch,blocks,fx=work
    settings=json.loads((ROOT/'data/control_defaults.json').read_text())
    runners=[]
    counts={k:[] for k in streams}; traces=[]; samples=bytearray(); peak=0.0
    for key,stream in streams.items():
        vm,r=runner_for(key,stream);runners.append((key,vm,r))
        r.set(CHR_DPTH=64,CHR_SPD=48,CHR_HPF=0,CHR_WDTH=127,CHR_VOL=64 if fx else 0,
              DEL_TIME=1,DEL_X=1,DEL_WID=127,DEL_FDBK=32,DEL_HPF=0,DEL_LPF=127,DEL_REV=32,DEL_VOL=64 if fx else 0,
              REV_PRE=0,REV_DEC=40,REV_FREQ=64,REV_GAIN=64,REV_HPF=0,REV_LPF=127,REV_VOL=64 if fx else 0)
        configure(r.d.frame,settings,types_at(kind,n,0))
    for b in range(blocks+32):
        types=types_at(kind,n,b)+[4]*(16-n)
        outputs=[]; buffers=[]; states=[]; effects=[]
        for key,vm,r in runners:
            if kind in ('switch','stock-switch') or b==0:
                configure(r.d.frame,settings,types)
            controls(r.d.frame,types,patch,b)
            for t,m in enumerate(types):
                for off,val in ((340,40),(342,48),(344,56)):
                    struct.pack_into('<H',r.d.frame,off+146*t,val<<8 if fx else 0)
            mask=(1<<n)-1 if b%19==0 or (kind in ('switch','stock-switch') and b%3==0) else 0
            for off in (34,38,40): struct.pack_into('<H',r.d.frame,off,mask)
            dry=[]; private=[]
            def capture():
                for t,m in enumerate(types):
                    dry.append(vm.read(vm.rd32(0x254a60+4*t),128))
                    if m in (5,6):
                        size=320 if m==5 else 296
                        addr=(0x300008+320*t if m==5 else 0x304008+296*t) if key=='frozen' else state_address(t)
                        private.append(vm.read(addr,size))
                    if key!='frozen':
                        base=0x300008+336*t
                        track=struct.unpack_from('<H',r.d.frame,0xa5a+2*t)[0]
                        assert vm.rd32(base)==((track<<8)|m if m in (5,6) else 0xffffffff),(key,b,t,'owner')
                        assert vm.rd32(base+8)==0x4d444752 and vm.rd32(base+332)==0x4d444752
                        assert vm.rd32(base+4)==0
            before = vm.icount
            if key!='frozen' and fx and b in (0,1,32,33,blocks+31):
                trace=trace_hook(vm,r,[t for t,m in enumerate(types) if m>=5],capture=capture,state_region=(0x300000,0x300000+POOL_BYTES),verify_mr=True)
                traces.append(dict(layout=key,block=b,**trace))
            else:
                import fullblock
                vm.write(fullblock.RX_BASE+0x1000*vm.rd32(fullblock.SPI_HALF),bytes(r.d.frame))
                vm.write(fullblock.CODEC_IN+0x100*vm.rd32(fullblock.DMA_HALF),bytes(256))
                r._call_probed([(VOICE_RESUME_SW,capture)])
            import fullblock
            stereo=vm.read(fullblock.OUT_FLOATS,256)
            assert all(math.isfinite(v) for v in struct.unpack('<64f',stereo))
            peak=max(peak,*map(abs,struct.unpack('<64f',stereo)))
            outputs.append(stereo);buffers.append(dry);states.append(private)
            effects.append([vm.read(a,256) for a in (0x25b668,0x25b068,0x25b368)])
            if b>=32: counts[key].append(vm.icount-before)
        if outputs[0]!=outputs[1] or outputs[0]!=outputs[2]:
            debug=dict(kind=kind,voices=n,patch=patch,block=b,
                       stereo=[x.hex() for x in outputs],
                       dry_equal=[buffers[0]==buffers[i] for i in (1,2)],
                       states_equal=[states[0]==states[i] for i in (1,2)],
                       effects_equal=[effects[0]==effects[i] for i in (1,2)])
            (ROOT/'out/machines/stock-mismatch.json').write_text(json.dumps(debug,indent=2))
            raise AssertionError((kind,n,patch,b,'stock stereo',debug))
        assert buffers[0]==buffers[1]==buffers[2],(kind,n,patch,b,'prefilter PCM')
        assert states[0]==states[1]==states[2],(kind,n,patch,b,'full synthesis/stream/control state')
        assert effects[0]==effects[1]==effects[2],(kind,n,patch,b,'stock effects')
        samples.extend(outputs[0])
    assert peak>0
    metrics={k:stats(v) for k,v in counts.items()}
    # A fixed up-front lookup guard is paid even for stock voices. Investigate
    # >1% whole-handler regressions; no kernel instruction budget is relaxed.
    for key in ('l1','l2-experimental'):
        for metric in ('median','p99','max'):
            assert metrics[key][metric] <= metrics['frozen'][metric]*max_regression,(work,key,metric,metrics)
    result=dict(kind=kind,voices=n,patch=patch,fx=fx,whole_handler=metrics,per_block=counts,pcm_sha256=hashlib.sha256(samples).hexdigest(),traces=traces)
    print(kind,n,patch,metrics,flush=True)
    return result


def run(blocks=96,jobs=4):
    assert hashlib.sha256(STREAMS['frozen'].read_bytes()).hexdigest()=='58cb60f11aa3a3fd67cc468e70ef46d058d0d864fcc93e2c26a7192de5984cc0','Frozen dual implementation changed'
    work=[(kind,n,patch,blocks,False) for kind in ('BD','SD','mixed','switch') for n in (1,4,8,16) for patch in ('default','max','noise','locks')]
    work += [(kind,n,'locks',blocks,True) for kind in ('stock-mixed','stock') for n in (1,4,8,16)]
    work += [('stock-switch',n,patch,blocks,True) for n in (1,4,8,16) for patch in ('default','max','noise','locks')]
    with ProcessPoolExecutor(jobs) as pool: rows=list(pool.map(compare,work))
    report=dict(schema='digimd.machines.native-matrix.v1',status='PASS',blocks=blocks,warmup=32,cases=rows,
                streams={k:hashlib.sha256(p.read_bytes()).hexdigest() for k,p in STREAMS.items()},
                measurement='Executed whole-handler emulator instructions; physical cycles and headroom UNKNOWN')
    (ROOT/'out/machines/native-matrix.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--blocks',type=int,default=96);p.add_argument('--jobs',type=int,default=4);a=p.parse_args();run(a.blocks,a.jobs)
