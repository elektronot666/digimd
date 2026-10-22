"""Execute original SPORT callbacks, allocated Audio Task, wait, handler and codec stores.

Callback completion events are explicit test inputs. Neither physical DMA timing
nor a mapping from emulator instructions to physical cycles is claimed.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

from tools.machines.registry import ROOT, machines
from tools.machines.resident.common import install_alias

HALF, TASK, GATE, HANDLE, SKIP = 0x268a38, 0x1c9fe7, 0x1c9ff1, 0x1c9ff4, 0x1c9ffb
# Routing renderers publish each skipped completion (missed deadline) here for
# the load governor. Older candidates and the original profile never store it.
MISSES = 0x8181d3f0
# The governor's stock-time hooks (audio_pacing.s): EMUCLK at the block start
# (render path of the gate) and at the handler's return (0x1C9FFB), R0 saved
# through SAVE. Older candidates have neither hook.
START, END, SAVE = 0x8181d3f4, 0x8181d3f8, 0x8181d3fc
STATE = [f'R{i}' for i in range(16)] + ['ASTATX', 'ASTATY', 'MODE1']


def state(vm):
    return [vm.reg(n) for n in STATE]
CALLBACK, CREATE, TASK_HANDLE, CURRENT = 0x1ca136, 0x1ca000, 0x268a84, 0x2d0700


def run_stream(path, backlog):
    install_alias()
    import fullblock
    from tools.trxbd_sharc import build_sharc as B
    from tools.trxbd_sharc.integration import create_runner, configure

    vm, runner = create_runner(Path(path).read_bytes())
    vm.call(0xb89c53, max_insns=1000000)
    assert vm.call(CREATE, max_insns=1000000) == 1
    task = vm.rd32(TASK_HANDLE)
    assert task and task == vm.rd32(CURRENT) and vm.rd32(task + 0x50) == 0
    settings = json.loads((ROOT / 'data/control_defaults.json').read_text())
    catalog = {m['selector']: m for m in machines()}
    rows = []
    start_stamps, end_stamps = [], []
    completed = 0
    pcm = hashlib.sha256()
    for block in range(32):
        if block < 4:
            types, trigger = [0] + [4] * 15, int(block == 0)
        elif block < 8:
            types, trigger = [5, 0] + [4] * 14, 2 if block == 4 else 0
        elif block < 16:
            types, trigger = [5, 0] + [4] * 14, 3 if block % 2 == 0 else 0
        elif block < 24:
            types, trigger = [5, 6, 0] + [4] * 13, 7 if block % 2 == 0 else 0
        else:
            types, trigger = [0] + [4] * 15, int(block == 24)
        configure(runner.d.frame, settings, types)
        for voice, selector in enumerate(types):
            if selector < 5:
                continue
            for offset, parameter in zip((250, 252, 244, 248, 254, 256, 262, 260), catalog[selector]['syn']):
                struct.pack_into('<H', runner.d.frame, offset + 146 * voice, parameter['default_raw'] << 8)
        for offset in (34, 38, 40):
            struct.pack_into('<H', runner.d.frame, offset, trigger)
        vm.write(fullblock.RX_BASE + 0x1000 * vm.rd32(fullblock.SPI_HALF), bytes(runner.d.frame))
        vm.write(fullblock.CODEC_IN, bytes(512))
        # The original callback creates each pending notification; no task
        # handle or pending field is supplied by the host test.
        count = backlog if block == 8 else 1
        for _ in range(count):
            vm.call(CALLBACK, args=(0, 0x100, 0), max_insns=100000)
        completed += count
        assert vm.rd32(task + 0x50) == count
        vm.pc = TASK
        waits = []
        skipped_stores = []
        miss_stores = []
        misses_before = vm.rd32(MISSES)
        for _ in range(count):
            assert vm.run(10000, stop=GATE) == B.V.ST_STOP
            pending = vm.reg('R0')
            before = vm.rd32(HALF)
            entry = state(vm)
            B.V.watch(vm, 0, 0xffffffff)
            for _ in range(20):
                vm.run(1, stop=0xffffffff)
                if vm.pc in (HANDLE, SKIP):
                    break
            stores = B.V.watch_log(vm); B.V.watch(vm, 0, 0)
            waits.append(pending)
            if vm.pc == HANDLE:
                # the render path may only stamp the block start; every
                # register and flag reaching the handler call is unchanged
                assert pending == 1 and all(a in (START, SAVE) for _, a, _ in stores), stores
                assert state(vm) == entry, 'render path changed registers or flags'
                if stores:
                    assert any(a == START and v for _, a, v in stores), stores
                    start_stamps.append(vm.rd32(START))
                break
            assert vm.pc == SKIP and pending > 1
            assert all(a in (HALF, MISSES) for _, a, _ in stores), stores
            skipped_stores.extend((pc, a, v) for pc, a, v in stores if a == HALF)
            miss_stores.extend((pc, a, v) for pc, a, v in stores if a == MISSES)
            vm.run(1, stop=0xffffffff)
        assert waits == list(range(count, 0, -1))
        if miss_stores:
            assert len(miss_stores) == count - 1 and vm.rd32(MISSES) == misses_before + count - 1, miss_stores
        assert vm.rd32(task + 0x50) == 0
        written_half = vm.rd32(HALF)
        B.V.watch(vm, fullblock.CODEC_OUT, fullblock.CODEC_OUT + 512)
        assert vm.run(100000000, stop=SKIP) == B.V.ST_STOP
        stores = B.V.watch_log(vm); B.V.watch(vm, 0, 0)
        assert len(stores) == 128
        assert {int((a - fullblock.CODEC_OUT) // 256) for _, a, _ in stores} == {written_half}
        # the end hook at SKIP (0x1C9FFB): only the end stamp, then the wait,
        # with every register and flag unchanged
        exit_state = state(vm)
        B.V.watch(vm, 0, 0xffffffff)
        for _ in range(12):
            vm.run(1, stop=0xffffffff)
            if vm.pc == TASK:
                break
        stores = B.V.watch_log(vm); B.V.watch(vm, 0, 0)
        assert vm.pc == TASK, hex(vm.pc)
        assert all(a in (END, SAVE) for _, a, _ in stores), stores
        assert state(vm) == exit_state, 'end hook changed registers or flags'
        if stores:
            end_stamps.append(vm.rd32(END))
        output = vm.read(fullblock.CODEC_OUT + written_half * 256, 256)
        pcm.update(output)
        rows.append(dict(block=block, callbacks=count, wait_results=waits,
                         written_half=written_half, half_after=vm.rd32(HALF),
                         expected_free_half=(completed - 1) & 1,
                         collision=written_half == (completed & 1),
                         skipped_half_stores=len(skipped_stores), miss_count_stores=len(miss_stores),
                         codec_peak=max(abs(x) for x in struct.unpack('<64i', output))))
    assert any(row['codec_peak'] for row in rows[:4]), 'stock baseline silent'
    assert any(row['codec_peak'] for row in rows[8:24]), 'mixed MD/stock silent'
    assert any(row['codec_peak'] for row in rows[26:]), 'stock recovery silent'
    # stamps: a block starts, then its handler ends, in EMUCLK order
    if start_stamps or end_stamps:
        assert len(start_stamps) == len(end_stamps) == len(rows), (len(start_stamps), len(end_stamps))
        assert all(s < e for s, e in zip(start_stamps, end_stamps))
        assert all(e <= s for e, s in zip(end_stamps, start_stamps[1:]))
    return dict(backlog=backlog, task_created_by_original_code=hex(task),
                stock_time_hooks=bool(start_stamps), handler_cycles=[e - s for s, e in zip(start_stamps, end_stamps)][:8],
                notification_count_seeded=False, blocks=rows, codec_pcm_sha256=pcm.hexdigest())


def run(candidate, baseline, output):
    cases = []
    for count in (1, 2, 3, 4, 17):
        old = run_stream(baseline, count)
        new = run_stream(candidate, count)
        assert all(not row['collision'] and row['written_half'] == row['expected_free_half'] for row in new['blocks'])
        assert all(row['collision'] == (count % 2 == 0 and row['block'] >= 8) for row in old['blocks'])
        assert old['codec_pcm_sha256'] == new['codec_pcm_sha256'], 'algorithm/codec PCM changed'
        cases.append(dict(pending=count, old=old, corrected=new))
        print('PASS original callbacks / codec ownership / exact PCM:', count, flush=True)
    report = dict(status='PASS-Full49-backlog-ownership-correction', hardware_accessed=False,
                  source_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  baseline_sha256=hashlib.sha256(Path(baseline).read_bytes()).hexdigest(),
                  candidate_sha256=hashlib.sha256(Path(candidate).read_bytes()).hexdigest(), cases=cases,
                  limits=['SPORT callback events are explicit input, not physical interrupts.',
                          'Expected active/free DMA halves follow the original alternating descriptor contract.',
                          'No physical deadline, CPU-cycle, or observed-hardware-failure claim.'])
    Path(output).parent.mkdir(parents=True, exist_ok=True)
    Path(output).write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    run(args.candidate, args.baseline, args.out)
