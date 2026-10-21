@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "ZAPRET_LAUNCHER=%~f0"
set "ZAPRET_DIR=%~dp0"
powershell.exe -NoProfile -Command "if (([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { exit 0 } else { exit 1 }"
if errorlevel 1 (
    powershell.exe -NoProfile -Command "try { Start-Process -FilePath $env:ZAPRET_LAUNCHER -Verb RunAs -ErrorAction Stop } catch { Write-Error $_; exit 1 }"
    exit /b
)
cd /d "%~dp0"
echo [1] Portable mode
echo [2] Install and start automatic zapret service
echo [3] Stop and delete zapret and WinDivert services
choice /c 123 /n /m "Select [1-3]: "
if errorlevel 3 goto remove
if errorlevel 2 goto install
if errorlevel 1 goto portable
exit /b 1

:check
for %%F in (winws.exe cygwin1.dll WinDivert.dll WinDivert64.sys steam_vn.txt) do (
    if not exist "%~dp0%%F" (
        echo Missing file: %%F
        exit /b 1
    )
)
exit /b 0

:portable
call :check
if errorlevel 1 goto failed
sc.exe query zapret >nul 2>&1
if not errorlevel 1 (
    echo Remove the zapret service using option 3 before running portable mode.
    goto failed
)
"%~dp0winws.exe" --wf-tls-clienthello-only --dpi-desync=multisplit --dpi-desync-split-pos=midsld --hostlist="%~dp0steam_vn.txt"
if errorlevel 1 goto failed
exit /b 0

:install
call :check
if errorlevel 1 goto failed
sc.exe create zapret start= auto DisplayName= "Zapret Steam PUBG Vietnam" binPath= "\"%~dp0winws.exe\" --wf-tls-clienthello-only --dpi-desync=multisplit --dpi-desync-split-pos=midsld --hostlist=\"%~dp0steam_vn.txt\""
if errorlevel 1 goto failed
sc.exe description zapret "IPv4 TLS ClientHello SNI bypass for Steam and PUBG"
sc.exe start zapret
if errorlevel 1 goto failed
echo Service installed and started. Keep this folder at its current location.
pause
exit /b 0

:remove
sc.exe stop zapret
powershell.exe -NoProfile -Command "$s = Get-Service -Name zapret -ErrorAction SilentlyContinue; if ($s -and $s.Status -ne 'Stopped') { try { $s.WaitForStatus('Stopped', [TimeSpan]::FromSeconds(30)) } catch { Write-Error $_; exit 1 } }"
if errorlevel 1 goto failed
sc.exe query zapret >nul 2>&1
if not errorlevel 1 sc.exe delete zapret
rem WinDivert commonly registers its driver as WinDivert1.4 or WinDivert14.
for %%S in (WinDivert WinDivert1.4 WinDivert14) do (
    sc.exe query %%S >nul 2>&1
    if not errorlevel 1 (
        sc.exe stop %%S
        sc.exe delete %%S
    )
)
pause
exit /b 0

:failed
echo Operation failed. Review the error above.
pause
exit /b 1
