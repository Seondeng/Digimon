$ErrorActionPreference = 'Stop'
$exe = (Resolve-Path 'release/Seondaeng_Digimon.exe').Path
# Do not disable protection or add exclusions. Scan only if Defender is running.
$defender = $null
try { $defender = Get-MpComputerStatus -ErrorAction Stop } catch { Write-Warning 'Defender status is unavailable on this runner; no antivirus verdict can be claimed.' }
if ($defender -and $defender.AntivirusEnabled -and $defender.AMServiceEnabled) {
    Write-Host ('Defender signatures: ' + $defender.AntivirusSignatureVersion)
    Start-MpScan -ScanType CustomScan -ScanPath $exe -ErrorAction Stop
    $detections = @(Get-MpThreatDetection -ErrorAction Stop | Where-Object { ($_.Resources -join ' ') -like ('*' + $exe + '*') })
    if ($detections.Count -or -not (Test-Path -LiteralPath $exe)) { throw 'Defender detected or removed the release file; publishing stopped.' }
    Write-Host 'Defender custom scan completed with no matching detection.'
} else { Write-Warning 'Defender is not active on this runner; antivirus scan not performed.' }
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class OverlayWindowProbe {
 public delegate bool Callback(IntPtr h, IntPtr p);
 [DllImport("user32.dll")] public static extern bool EnumWindows(Callback cb,IntPtr p);
 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h,out uint pid);
 [DllImport("user32.dll",EntryPoint="GetWindowLongW")] public static extern int GetWindowLong(IntPtr h,int index);
 [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
 [DllImport("user32.dll")] public static extern bool GetLayeredWindowAttributes(IntPtr h,out uint key,out byte alpha,out uint flags);
 [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h,uint msg,IntPtr w,IntPtr l);
 public static IntPtr Find(uint pid){IntPtr result=IntPtr.Zero;EnumWindows((h,p)=>{uint id;GetWindowThreadProcessId(h,out id);if(id==pid&&IsWindowVisible(h)){result=h;return false;}return true;},IntPtr.Zero);return result;}
}
'@
# Locked views must exit without opening a window, even from the command line.
$blocked = Start-Process -FilePath $exe -ArgumentList '--overlay','quiz' -PassThru
if (-not $blocked.WaitForExit(10000)) { $blocked.Kill(); throw 'Locked overlay did not exit.' }
if ($blocked.ExitCode -ne 1) { throw 'Locked overlay was accepted.' }
$child = $null
try {
    $child = Start-Process -FilePath $exe -ArgumentList '--overlay','maze-hard' -PassThru
    $handle = [IntPtr]::Zero
    $deadline = [DateTime]::UtcNow.AddSeconds(45)
    while ($handle -eq [IntPtr]::Zero -and [DateTime]::UtcNow -lt $deadline) {
        if ($child.HasExited) { throw 'Overlay exited before showing its window.' }
        $handle = [OverlayWindowProbe]::Find([uint32]$child.Id)
        Start-Sleep -Milliseconds 250
    }
    if ($handle -eq [IntPtr]::Zero) { throw 'Overlay window did not appear.' }
    $style = [OverlayWindowProbe]::GetWindowLong($handle,-20)
    if (($style -band 0x80000) -eq 0 -or ($style -band 8) -eq 0) { throw 'Layered/topmost window flags missing.' }
    [uint32]$key=0;[byte]$alpha=0;[uint32]$flags=0
    if (-not [OverlayWindowProbe]::GetLayeredWindowAttributes($handle,[ref]$key,[ref]$alpha,[ref]$flags)) { throw 'Cannot read overlay opacity.' }
    if ($alpha -ne 209 -or ($flags -band 2) -eq 0) { throw 'Overlay is not initially translucent.' }
    Write-Host 'Native overlay window appeared with topmost/layered flags and 82% whole-window alpha; locked view rejected.'
} finally {
    if ($child -and -not $child.HasExited) {
        $handle=[OverlayWindowProbe]::Find([uint32]$child.Id)
        if ($handle -ne [IntPtr]::Zero) { [void][OverlayWindowProbe]::PostMessage($handle,0x10,[IntPtr]::Zero,[IntPtr]::Zero) }
        if (-not $child.WaitForExit(15000)) { $child.Kill() }
    }
}
