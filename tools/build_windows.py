from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
from urllib.request import urlopen,Request
from urllib.parse import quote
import hashlib,json,os,subprocess,re,shutil,sys
root=Path(__file__).resolve().parents[1];os.chdir(root)
repository=os.environ.get('GITHUB_REPOSITORY','Seondeng/Digimon');assert re.fullmatch(r'[A-Za-z0-9-]+/[A-Za-z0-9_.-]+',repository)
owner,repo_name=repository.split('/')
version=os.environ['RELEASE_VERSION'];assert re.fullmatch(r'\d{1,4}\.\d{1,4}\.\d{1,4}',version)
app=root/'app';native=root/'native';assets=app/'assets';assets.mkdir(exist_ok=True)
cache=root/'.cache';cache.mkdir(exist_ok=True)
# Seed original images from a GitHub release after the first successful build.
seed=subprocess.run(['gh','release','download','bootstrap-assets','--pattern','assets.zip','--dir',str(cache),'--clobber'],capture_output=True)
if seed.returncode==0:
 with ZipFile(cache/'assets.zip') as z:
  for name in z.namelist():
   if '/' in name or '\\' in name or name in ['.','..']:raise ValueError('Unexpected asset path')
   dst=assets/name
   if not dst.exists():dst.write_bytes(z.read(name))
manifest=json.loads((root/'tools/assets.json').read_text())
for row in manifest['files']:
 dst=assets/row['path']
 if dst.exists():continue # repository edits override the original image
 request=Request(manifest['origin']+quote(row['path']),headers={'User-Agent':'SeondaengGuideBuild/1.3'})
 with urlopen(request,timeout=60) as response:data=response.read(32*1024*1024)
 if hashlib.sha256(data).hexdigest()!=row['sha256']:raise ValueError('Asset digest mismatch: '+row['path'])
 dst.write_bytes(data)
# Reassemble the lossless map from API-sized repository parts.
subprocess.run([sys.executable,'tools/assemble_release_assets.py'],check=True)
# Microsoft WebView2 SDK from the official pinned NuGet package.
pkg=cache/'webview2.zip'
with urlopen('https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/1.0.3537.50/microsoft.web.webview2.1.0.3537.50.nupkg',timeout=60) as r:pkg.write_bytes(r.read(64*1024*1024))
with ZipFile(pkg) as z:
 for name in ['build/native/include/WebView2.h']:(native/Path(name).name).write_bytes(z.read(name))
 (app/'WebView2Loader.dll').write_bytes(z.read('build/native/x64/WebView2Loader.dll'))
(native/'update_config.h').write_text(f'#define APP_VERSION L"{version}"\n#define UPDATE_OWNER L"{owner}"\n#define UPDATE_REPOSITORY L"{repo_name}"\n')
s=(app/'index.html').read_text(encoding='utf-8');s=s.replace('v1.3',f'v{version}');(app/'index.html').write_text(s,encoding='utf-8')
data=json.loads((app/'distribution.json').read_text(encoding='utf-8'));assert data['edition']=='public';data['version']=version;(app/'distribution.json').write_text(json.dumps(data,ensure_ascii=False),encoding='utf-8')
for js in app.glob('*.js'):subprocess.run(['node','--check',str(js)],check=True)
with ZipFile(native/'payload.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for f in sorted(app.rglob('*')):
  if f.is_file():z.write(f,f.relative_to(app))
digest=hashlib.sha256((native/'payload.zip').read_bytes()).hexdigest()[:12]
(native/'release_id.h').write_text(f'#define RELEASE_ID L"{version}-{digest}"\n')
release=root/'release';release.mkdir(exist_ok=True);exe=release/'Seondaeng_Digimon.exe'
# Called from a VS developer command prompt, installed on windows-latest.
subprocess.run(['rc','/nologo','/fo','app.res','app.rc'],cwd=native,check=True)
subprocess.run(['cl','/nologo','/O2','/utf-8','/std:c11','/D_CRT_SECURE_NO_WARNINGS','launcher.c','app.res','/Fe:'+str(exe),'/link','/SUBSYSTEM:WINDOWS','/ENTRY:wWinMainCRTStartup','winhttp.lib','ole32.lib','shell32.lib','shlwapi.lib','uuid.lib','gdi32.lib','user32.lib'],cwd=native,check=True)
subprocess.run([sys.executable,'tools/prepare_release.py','--exe',str(exe),'--version',version,'--repository',repository,'--output','updates/public.json'],check=True)
with ZipFile(release/'assets.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for f in sorted(assets.iterdir()):
  if f.is_file():z.write(f,f.name)
print('Release built',exe)
