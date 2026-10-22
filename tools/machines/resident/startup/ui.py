"""DigiMD UI corrections applied to c33's section 3 by startup.build.

c33's ColdFire runtime (tools/machines/control.py, built into the pinned c33
firmware) is copied from section 3 to RUN_VA at reset. These corrections
change its data in place and send a few stock call sites to code in ui.s,
which is linked into the resident cold-boot loader. Every replaced byte is
checked against c33 before it is written. Nothing here touches the SHARC
side, so the Full49 stream and every engine's PCM are unchanged.
"""
import json
import struct

from tools.machines.registry import ROOT, machines

MAIN = 0x40000400
RUN_VA, RUN_LOAD_VA = 0x47e00000, 0x4030c9c0
# c33 symbols (trxbd_cf.elf of the pinned c33 build)
C33_FULL49_MACHINE_LIST = 0x47e0bad8
C33_FULL49_MACHINES = list(range(5, 54))
# LFO DEST list builder 0x4003951e: `lea get_param_mod_mask,%a2`
DEST_MASK_SITE = 0x40039570
# Page view vtable +0x70 (target track's parameter for control all): `jsr page(type, n)`
CONTROL_ALL_PAGE_SITE = 0x40064ece
# Control all's clamp: `jsr limits_getter(id)`
CONTROL_ALL_LIMITS_SITE = 0x40066da0
# ParameterSet::setValue's clamp: `jsr limits_getter(id)`
SET_VALUE_LIMITS_SITE = 0x40037350
# ParameterSet::isValueValid's getter: `lea limits_getter,%a2`
VALID_LIMITS_SITE = 0x40036bce
# Machine setter 0x4004cc08: `jsr %pc@(0x4004c30c); movea.l (%a2),%a0`
MACHINE_RESET_SITE = 0x4004cca6
RECORDS = 0x401f83a4
# AMP {slot: (record, stock default)}; ui.s md_amp_stock / md_amp_md use them
AMP = {81: (62, 0x0800), 82: (63, 0x7f00), 83: (64, 0x2000), 91: (73, 0x0100)}


def menu_order():
    """MACHINE SEL order of the MD machines: TRX first (TRX-BD, TRX-SD, then
    the rest of the family), the other families in MD order, GND last."""
    rows = sorted(machines(), key=lambda m: m['md_id'])
    trx = [m for m in rows if m['family'] == 'TRX']
    gnd = [m for m in rows if m['family'] == 'GND']
    rest = [m for m in rows if m['family'] not in ('TRX', 'GND')]
    order = [m['selector'] for m in trx + rest + gnd]
    if order[:2] != [5, 6] or sorted(order) != C33_FULL49_MACHINES:
        raise SystemExit('unexpected MD machine registry')
    return order


def apply(main, symbols, loader, loader_base):
    """Patch c33 section 3 (bytearray) in place; return a report."""
    def at(address, n):
        return bytes(main[address - MAIN:address - MAIN + n])

    def put(address, old, new):
        if at(address, len(old)) != old:
            raise SystemExit(f'c33 differs at 0x{address:08x}: {at(address, len(old)).hex()}')
        main[address - MAIN:address - MAIN + len(new)] = new
        writes.append(dict(address=hex(address), before=old.hex(), after=new.hex()))

    writes = []
    # 1. MACHINE SEL order (the full list the menu uses once Full49 is ready).
    stock, mid = [0, 2, 1, 3], [4]
    old = struct.pack('>54I', *stock, *C33_FULL49_MACHINES, *mid)
    new = struct.pack('>54I', *stock, *menu_order(), *mid)
    put(C33_FULL49_MACHINE_LIST - RUN_VA + RUN_LOAD_VA, old, new)
    # 3. LFO DEST list: MD tracks offer their own SYN controls, not WaveTone's.
    put(DEST_MASK_SITE, bytes.fromhex('45f9400dc472'),
        b'\x45\xf9' + struct.pack('>I', symbols['md_ui_dest_mask']))
    # 2/7. Control all resolves each target track's page by its own type.
    put(CONTROL_ALL_PAGE_SITE, bytes.fromhex('4eb9400c2652'),
        b'\x4e\xb9' + struct.pack('>I', symbols['md_ui_page']))
    put(CONTROL_ALL_LIMITS_SITE, bytes.fromhex('4eb9400dc154'),
        b'\x4e\xb9' + struct.pack('>I', symbols['md_ui_target_limits']))
    put(SET_VALUE_LIMITS_SITE, bytes.fromhex('4eb9400dc154'),
        b'\x4e\xb9' + struct.pack('>I', symbols['md_ui_set_limits']))
    put(VALID_LIMITS_SITE, bytes.fromhex('45f9400dc154'),
        b'\x45\xf9' + struct.pack('>I', symbols['md_ui_valid_limits']))
    # 6. MD AMP envelope default on choosing an MD machine.
    fixture = json.loads((ROOT / 'data/control_defaults.json').read_text())
    slots = {row['slot']: row for row in fixture['machines'][0]['slots']}
    for slot, (record, default) in AMP.items():
        stored = struct.unpack('>I', at(RECORDS + 60 * record + 0x10, 4))[0]
        if slots[slot]['record'] != record or slots[slot]['default'] != default or stored != default:
            raise SystemExit(f'AMP slot {slot} is not record {record} with default {default:#x}')
    for name, values in (('md_amp_stock', [d for _, d in AMP.values()]), ('md_amp_md', [0, 0x7f00, 0x7f00, 0])):
        at_loader = symbols[name] - loader_base
        if loader[at_loader:at_loader + 16] != struct.pack('>8H', *(x for pair in zip(AMP, values) for x in pair)):
            raise SystemExit(f'ui.s {name} differs from ui.py AMP')
    put(MACHINE_RESET_SITE, bytes.fromhex('4ebaf6642052'),
        b'\x4e\xb9' + struct.pack('>I', symbols['md_ui_machine_reset']))
    return dict(menu_order=menu_order(), writes=writes)
