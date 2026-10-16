# Windows PowerShell 5.1. No account password or GitHub token is distributed.
$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$target = [IO.Path]::GetFullPath($env:SEONDAENG_UPDATE_TARGET)
$current = [version]$env:SEONDAENG_UPDATE_VERSION
$edition = $env:SEONDAENG_UPDATE_EDITION
$owner = $env:SEONDAENG_UPDATE_OWNER
$repository = $env:SEONDAENG_UPDATE_REPO
$appProcessId = [int]$env:SEONDAENG_UPDATE_PID
$manual = $env:SEONDAENG_UPDATE_MANUAL -eq '1'
$form = $null; $mutex = $null; $locked = $false; $staged = $null; $backup = $null; $closedApp = $false; $cancelled = $false
$logDirectory = Join-Path $env:LOCALAPPDATA 'SeondaengDigimonGuide'
function Note([string]$text) {
    try { [IO.Directory]::CreateDirectory($logDirectory) | Out-Null
        $log = Join-Path $logDirectory 'update.log'
        if ((Test-Path -LiteralPath $log) -and (Get-Item -LiteralPath $log).Length -gt 1048576) { Remove-Item -LiteralPath $log }
        Add-Content -LiteralPath $log -Value ((Get-Date -Format s) + ' ' + $edition + ' ' + $text) -Encoding UTF8
    } catch {}
}
function Notify([string]$text) {
    Add-Type -AssemblyName System.Windows.Forms
    [System.Windows.Forms.MessageBox]::Show($text, '디지몬 RPG 업데이트') | Out-Null
}
function SafeUri([string]$value) {
    $uri = [uri]$value
    if ($uri.Scheme -ne 'https' -or $uri.UserInfo -or $uri.Port -ne 443) { throw 'Invalid update HTTPS address.' }
    if ($uri.Host -notin @('raw.githubusercontent.com','github.com','release-assets.githubusercontent.com','objects.githubusercontent.com')) { throw 'Unapproved update host.' }
    return $uri
}
function Fetch([string]$url, [string]$destination, [long]$limit) {
    $response = $null; $stream = $null; $output = $null
    try {
        $uri = SafeUri $url
        for ($hop = 0; $hop -lt 6; $hop++) {
            $request = [Net.HttpWebRequest]::Create($uri)
            $request.AllowAutoRedirect = $false
            $request.Timeout = 10000; $request.ReadWriteTimeout = 20000
            $request.UserAgent = 'SeondaengDigimonGuide/1.3'
            $request.Headers['Cache-Control'] = 'no-cache'
            $response = $request.GetResponse()
            if ([int]$response.StatusCode -in @(301,302,303,307,308)) {
                $next = [uri]::new($uri, $response.Headers['Location'])
                $response.Close(); $response = $null
                $uri = SafeUri $next.AbsoluteUri
                continue
            }
            break
        }
        if ($null -eq $response -or [int]$response.StatusCode -ne 200) { throw 'Update request failed.' }
        if ($response.ContentLength -gt $limit) { throw 'Update response is too large.' }
        $stream = $response.GetResponseStream()
        if ($destination) { $output = [IO.File]::Open($destination, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write, [IO.FileShare]::None) }
        else { $output = [IO.MemoryStream]::new() }
        $buffer = New-Object byte[] 65536; [long]$total = 0; $watch = [Diagnostics.Stopwatch]::StartNew()
        while (($read = $stream.Read($buffer, 0, $buffer.Length)) -gt 0) {
            $total += $read
            if ($total -gt $limit -or $watch.Elapsed.TotalMinutes -gt 10) { throw 'Update download limit exceeded.' }
            $output.Write($buffer, 0, $read)
            if ($null -ne $form) {
                $script:statusLabel.Text = '새 버전 다운로드 중… ' + [Math]::Round($total / 1048576, 1) + ' MB'
                [System.Windows.Forms.Application]::DoEvents()
                if ($script:cancelled) { throw 'Update cancelled.' }
            }
        }
        if ($response.ContentLength -ge 0 -and $total -ne $response.ContentLength) { throw 'Incomplete update download.' }
        if (-not $destination) { return [Text.Encoding]::UTF8.GetString($output.ToArray()) }
    } finally {
        if ($output) { $output.Dispose() }; if ($stream) { $stream.Dispose() }; if ($response) { $response.Close() }
    }
}
try {
    if ($edition -notin @('public','full') -or $owner -notmatch '^[a-zA-Z0-9-]+$' -or $repository -notmatch '^[a-zA-Z0-9_.-]+$') { throw 'Invalid updater configuration.' }
    $hash = [Security.Cryptography.SHA256]::Create()
    $lockKey = [BitConverter]::ToString($hash.ComputeHash([Text.Encoding]::UTF8.GetBytes($target.ToLowerInvariant()))).Replace('-','')
    $hash.Dispose()
    $mutex = [Threading.Mutex]::new($false, ('Local\SeondaengGuideUpdate-' + $lockKey))
    $locked = $mutex.WaitOne(0)
    if (-not $locked) { return }
    # Only this fixed repository can declare the next release.
    $manifestUrl = 'https://raw.githubusercontent.com/' + $owner + '/' + $repository + '/main/updates/' + $edition + '.json'
    $manifest = (Fetch $manifestUrl '' 65536) | ConvertFrom-Json
    if ($manifest.schema -ne 1 -or $manifest.edition -cne $edition -or $manifest.version -notmatch '^\d{1,4}\.\d{1,4}\.\d{1,4}$') { throw 'Invalid release manifest.' }
    if ([version]$manifest.version -le $current) { if ($manual) { Notify '현재 최신 버전입니다.' }; return }
    if ($manifest.sha256 -notmatch '^[a-fA-F0-9]{64}$' -or $manifest.size -lt 65536 -or $manifest.size -gt 536870912) { throw 'Invalid release integrity metadata.' }
    $asset = SafeUri ([string]$manifest.url)
    $releasePrefix = '/' + $owner + '/' + $repository + '/releases/download/'
    if ($asset.Host -ne 'github.com' -or -not $asset.AbsolutePath.StartsWith($releasePrefix, [StringComparison]::Ordinal) -or -not $asset.AbsolutePath.EndsWith('.exe') -or $asset.Query -or $asset.Fragment) { throw 'Release asset is not in the configured repository.' }
    # Stage next to the application: a permission failure leaves the current app running.
    $staged = Join-Path ([IO.Path]::GetDirectoryName($target)) ('.seondaeng-update-' + [guid]::NewGuid().ToString('N') + '.exe')
    Add-Type -AssemblyName System.Windows.Forms
    $form = New-Object System.Windows.Forms.Form
    $form.Text = '디지몬 RPG 업데이트'; $form.Width = 430; $form.Height = 130
    $form.StartPosition = 'CenterScreen'; $form.FormBorderStyle = 'FixedDialog'; $form.MaximizeBox = $false; $form.TopMost = $true
    $statusLabel = New-Object System.Windows.Forms.Label
    $statusLabel.AutoSize = $true; $statusLabel.Left = 20; $statusLabel.Top = 24
    $statusLabel.Text = '버전 ' + $manifest.version + ' 업데이트를 준비합니다…'
    $form.Controls.Add($statusLabel)
    $form.add_FormClosing({ $script:cancelled = $true })
    $form.Show(); [System.Windows.Forms.Application]::DoEvents()
    Fetch $asset.AbsoluteUri $staged ([long]$manifest.size)
    if ((Get-Item -LiteralPath $staged).Length -ne [long]$manifest.size) { throw 'Update file size mismatch.' }
    if ((Get-FileHash -LiteralPath $staged -Algorithm SHA256).Hash -ine $manifest.sha256) { throw 'Update SHA256 mismatch.' }
    $pe = [IO.File]::OpenRead($staged)
    try { if ($pe.ReadByte() -ne 77 -or $pe.ReadByte() -ne 90) { throw 'Not a Windows executable.' } } finally { $pe.Dispose() }
    if ($cancelled) { throw 'Update cancelled.' }
    $app = Get-Process -Id $appProcessId -ErrorAction SilentlyContinue
    if ($app -and [IO.Path]::GetFullPath($app.Path) -ine $target) { throw 'Application process changed.' }
    $statusLabel.Text = '파일 확인 완료. 프로그램을 다시 시작합니다…'
    [System.Windows.Forms.Application]::DoEvents()
    if ($app) {
        if (-not $app.CloseMainWindow()) { throw 'Cannot close application for update.' }
        if (-not $app.WaitForExit(20000)) { throw 'Application did not exit; update postponed.' }
    }
    $closedApp = $true
    $backup = $target + '.previous'
    # File.Replace is atomic and keeps the old executable for rollback.
    [IO.File]::Replace($staged, $target, $backup, $true)
    $staged = $null
    try { Start-Process -FilePath $target -WorkingDirectory ([IO.Path]::GetDirectoryName($target)) -ErrorAction Stop | Out-Null }
    catch { [IO.File]::Copy($backup, $target, $true); Start-Process -FilePath $target; throw }
    $closedApp = $false
    Note ('Updated to ' + $manifest.version)
} catch {
    Note $_.Exception.Message
    if ($closedApp) { try { Start-Process -FilePath $target } catch {} }
    if ($manual -or $null -ne $form) { Notify '업데이트하지 못했습니다. 기존 버전을 계속 사용할 수 있습니다.' }
} finally {
    if ($form) { $form.Dispose() }
    if ($staged -and (Test-Path -LiteralPath $staged)) { Remove-Item -LiteralPath $staged -Force -ErrorAction SilentlyContinue }
    if ($locked) { $mutex.ReleaseMutex() }; if ($mutex) { $mutex.Dispose() }
}
