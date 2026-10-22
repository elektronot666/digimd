"""Source-bound original identities and native manifests, without guessed costs."""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PATH = ROOT / 'tools/machines/registry.json'


def load():
    registry = json.loads(PATH.read_text())
    inventory_bytes = (ROOT / registry['inventory']).read_bytes()
    if hashlib.sha256(inventory_bytes).hexdigest() != registry['inventory_sha256']:
        raise ValueError('Original inventory changed; reconcile the registry explicitly')
    original = {m['md_id']: m for m in json.loads(inventory_bytes)['machines']}
    machines = registry['machines']
    if len(machines) != 49 or {m['md_id'] for m in machines} != set(original):
        raise ValueError('Require all 49 original synthesis identities')
    if sorted(m['selector'] for m in machines) != list(range(5, 54)):
        raise ValueError('Selector collision or missing identity')
    for m in machines:
        source = original[m['md_id']]
        if (m['name'], m['family']) != (source['name'], source['family']):
            raise ValueError('Original identity mismatch')
        if [{k: p[k] for k in ('slot', 'label', 'default_raw')} for p in m['syn']] != source['syn']:
            raise ValueError('Original SYN controls changed')
        if m['original']['dsp_entries'] != source['dsp']['entries']:
            raise ValueError('Original DSP entries changed')
        n = m['native']
        if n['enabled'] and n['status'] != 'verified-software':
            raise ValueError('Unverified engine cannot enter audible selection')
        if n['enabled'] and not (n.get('verification') or {}).get('module'):
            raise ValueError('Enabled engine lacks original/native verification gates')
        if n['enabled'] and (not n['source'] or not n['state_bytes'] or not m['requirements']['native_complete_state_bound_proven']):
            raise ValueError('Enabled engine lacks proven source/state')
        if not n['enabled'] and any(n[k] is not None for k in ('code_bytes', 'data_bytes', 'state_bytes', 'benchmark_instructions', 'physical_cycles')):
            raise ValueError('Unimplemented native costs must remain UNKNOWN')
    return registry


def machines():
    return sorted(load()['machines'], key=lambda m: m['selector'])


def get(selector):
    return next((m for m in machines() if m['selector'] == selector), None)


def digest():
    return hashlib.sha256(PATH.read_bytes()).hexdigest()


if __name__ == '__main__':
    r = load()
    print(f"49 original identities; {sum(m['native']['enabled'] for m in r['machines'])} native engines; 16 concurrent native slots")
