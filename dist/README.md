Source changes and deployment files are prepared; **no Windows executable was built here**.

The initially empty workspace was populated with the upstream `nfq/` source snapshot from
`bol-van/zapret` commit `c4d0b1990129dce2b28095277173ee0d7702d304` through the connected
GitHub tool because shell DNS failed. This is a source snapshot, not a Git checkout.
The accompanying patch can also be applied to a checkout of that commit.
Upstream licensing is preserved in `../docs/LICENSE.txt`.

Build on 64-bit Windows with this source tree at `C:\zapret` and the Cygwin installer
`setup-x86_64.exe` in the current directory (run from an elevated Command Prompt):

```bat
setup-x86_64.exe -q -R C:\cygwin64 -s https://mirrors.kernel.org/sourceware/cygwin/ -P gcc-core,make,zlib-devel && C:\cygwin64\bin\bash.exe -lc "cd /cygdrive/c/zapret && make -C nfq winws"
```

Copy `nfq\winws.exe` and `C:\cygwin64\bin\cygwin1.dll` into this folder. Add the matching
64-bit `WinDivert.dll` and signed `WinDivert64.sys` from your existing zapret/WinDivert
distribution. The upstream source snapshot does not contain those runtime binaries.
Run `tests\winws_cli.bat` from Windows to check option validation and the saved filter,
then run `zapret_pubg.bat` and choose portable mode, automatic service installation,
or service removal. Installation uses absolute paths; keep this folder in place.
Option 3 also removes WinDivert driver services, which may be shared by other tools.

`--wf-tls-clienthello-only` locks capture to the exact expression in
`tls_only_filter.txt`, disables conntrack, bypasses UDP and plaintext HTTP inspection,
and retains TLS SNI extraction, hostlists, multisplit and multidisorder. It rejects
fake/cutoff, auto hostlists, autottl, hostname caching and other state-dependent options.
Use ordinary filters such as `--wf-tcp=443` for fake/cutoff configurations.
Raw filters remain supported but do not implicitly enable the stateless mode.

With `-DNDEBUG`, verbose `DLOG` and hex dumps, including argument evaluation, disappear.
Error logging (`DLOG_ERR`/`DLOG_PERROR`) and startup/status logging (`DLOG_CONDUP`)
remain available. No SSID filter means no WLAN API calls and no polling clock read.

This IPv4-only filter requires a TLS record and ClientHello to begin at the packet's
payload start. Retransmitted ClientHellos can also match. It cannot reassemble a
ClientHello whose SNI spans packets, handle IPv6/QUIC, or recover an encrypted SNI.
CPU cost and latency are not zero and have not been benchmarked on a Vietnam ISP.
The requested `-march=native` targets the build machine's CPU; build on the intended
machine or override `OPTIMIZE` for a different CPU.

Local verification: `sh tests/run.sh` passes release logging argument-elision,
TLS/SNI/midsld/hostlist checks, and packet-path multisplit/multidisorder tests with
conntrack disabled. Packet tests replace only the OS send boundary and check that
the emitted segments reconstruct the original TLS payload. `make -n -C nfq winws` confirms the Windows build
command includes the requested optimization, strip and LTO flags.
Common C code was syntax checked using host GCC; Windows-specific branches,
driver capture, batch execution and service operation require Windows validation.
Native Linux full checks lack libcap/Netfilter/libmnl development headers.
Existing precedence, signedness and platform-dependent unused-variable warnings remain.
