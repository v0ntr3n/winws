# winws Steam and PUBG Vietnam TLS build

A focused source modification of [bol-van/zapret](https://github.com/bol-van/zapret), based on upstream commit `c4d0b1990129dce2b28095277173ee0d7702d304`.

Adds an explicit stateless IPv4 TLS ClientHello filter, optimized compiler flags, release logging removal, and Steam/PUBG deployment files. Ordinary capture filters retain the existing stateful modes.

- [Build and deployment instructions](dist/README.md)
- [Windows launcher](dist/zapret_pubg.bat)
- [Patch against upstream](dist/winws-tls-only.patch)
- [Upstream MIT license](docs/LICENSE.txt)

Run portable checks with `sh tests/run.sh`. Windows CLI checks are in `tests/winws_cli.bat`.

No Windows executable is included. Windows compilation, driver capture and service operation have not been validated here; latency improvements have not been benchmarked.
