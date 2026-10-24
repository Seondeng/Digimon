from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[1]
parts=root/'tools/release-assets'
m=json.loads((parts/'manifest.json').read_text(encoding='utf-8'))
assert Path(m['target']).name==m['target']
data=bytearray()
for part in m['parts']:
    assert Path(part['path']).name==part['path']
    chunk=(parts/part['path']).read_bytes()
    assert len(chunk)==part['size'],'Incomplete release asset part'
    data.extend(chunk)
assert len(data)==m['size'] and hashlib.sha256(data).hexdigest()==m['sha256'],'Release asset checksum mismatch'
target=root/'app/assets'/m['target']
target.parent.mkdir(exist_ok=True)
target.write_bytes(data)
print('Lossless release map assembled and verified:',len(data),'bytes')
