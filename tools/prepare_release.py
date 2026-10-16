"""Generate the update manifest AFTER uploading the matching EXE to a GitHub Release."""
import argparse,hashlib,json,re
from pathlib import Path
from urllib.parse import quote
p=argparse.ArgumentParser();p.add_argument('--exe',type=Path,required=True);p.add_argument('--version',required=True);p.add_argument('--repository',required=True,help='owner/repository');p.add_argument('--edition',choices=['public','full'],default='public');p.add_argument('--output',type=Path,required=True);a=p.parse_args()
if not re.fullmatch(r'\d{1,4}\.\d{1,4}\.\d{1,4}',a.version):p.error('version must be x.y.z')
if not re.fullmatch(r'[A-Za-z0-9-]+/[A-Za-z0-9_.-]+',a.repository):p.error('repository must be owner/name')
data=a.exe.read_bytes()
if data[:2]!=b'MZ':p.error('not a Windows EXE')
if not 65536<=len(data)<=536870912:p.error('unexpected EXE size')
manifest={'schema':1,'edition':a.edition,'version':a.version,'url':f'https://github.com/{a.repository}/releases/download/v{a.version}/{quote(a.exe.name,safe="._-")}', 'size':len(data),'sha256':hashlib.sha256(data).hexdigest()}
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(a.output)
