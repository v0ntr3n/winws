@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "ZAPRET_TEST_ROOT=%~dp0.."
set "ZAPRET_FILTER_OUT=%TEMP%\zapret-filter-%RANDOM%-%RANDOM%.txt"
"%ZAPRET_TEST_ROOT%\dist\winws.exe" --dry-run --wf-tls-clienthello-only --dpi-desync=multisplit --dpi-desync-split-pos=midsld --hostlist="%ZAPRET_TEST_ROOT%\dist\steam_vn.txt" --wf-save="%ZAPRET_FILTER_OUT%"
if errorlevel 1 exit /b 1
powershell.exe -NoProfile -Command "if ((Get-Content -Raw -LiteralPath $env:ZAPRET_FILTER_OUT).TrimEnd() -cne (Get-Content -Raw -LiteralPath ($env:ZAPRET_TEST_ROOT + '\dist\tls_only_filter.txt')).TrimEnd()) { exit 1 }"
if errorlevel 1 exit /b 1
del "%ZAPRET_FILTER_OUT%"
"%ZAPRET_TEST_ROOT%\dist\winws.exe" --dry-run --wf-tls-clienthello-only --dpi-desync=multisplit --dpi-desync-cutoff=n3
if not errorlevel 1 exit /b 1
"%ZAPRET_TEST_ROOT%\dist\winws.exe" --dry-run --wf-tls-clienthello-only --dpi-desync=fake
if not errorlevel 1 exit /b 1
"%ZAPRET_TEST_ROOT%\dist\winws.exe" --dry-run --wf-tls-clienthello-only --wf-tcp=443
if not errorlevel 1 exit /b 1
"%ZAPRET_TEST_ROOT%\dist\winws.exe" --dry-run --wf-tcp=443 --dpi-desync=fake --dpi-desync-cutoff=n3
if errorlevel 1 exit /b 1
echo Windows CLI checks passed.
exit /b 0
