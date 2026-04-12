# DArc86

Legacy Windows 7/8/10 x86 (32-bit) archiver based on FreeArc. A standalone
build targeting `i386` Windows, compiled with GHC 8.6.5 (the last GHC with
an official Windows i386 bindist) linked against the msvcrt C runtime.

Output binary: `Tests/arc86.exe` (PE32 Intel 80386).

Archive format is compatible with other FreeArc-derived archivers.

---

## Building (cross-compile from Linux via Wine)

### Requirements

- `wine` (tested on Wine 11.x)
- GHC 8.6.5 Windows i386 bindist installed at `~/.wine/drive_c/ghc865-i386/`  
  Download: https://downloads.haskell.org/~ghc/8.6.5/ghc-8.6.5-i386-unknown-mingw32.tar.xz  
  Extract with `tar --strip-components=1 -xf` into `~/.wine/drive_c/ghc865-i386/`.
- Lua 5.1 static library built for i686 msvcrt, placed at  
  `/tmp/out/FreeArc-win86-ghc/liblua5.1.a`.  
  Build it by running the bundled gcc under Wine:
  ```sh
  cd /path/to/lua-5.1.5/src
  for s in *.c; do
    [[ "$s" =~ ^(lua|luac|print)\.c$ ]] && continue
    wine ~/.wine/drive_c/ghc865-i386/mingw/bin/gcc.exe -c -O2 "$s" -o "${s%.c}.o"
  done
  wine ~/.wine/drive_c/ghc865-i386/mingw/bin/ar.exe rcs liblua5.1.a \
    lapi.o lauxlib.o lbaselib.o lcode.o ldblib.o ldebug.o ldo.o ldump.o \
    lfunc.o lgc.o linit.o liolib.o llex.o lmathlib.o lmem.o loadlib.o \
    lobject.o lopcodes.o loslib.o lparser.o lstate.o lstring.o lstrlib.o \
    ltable.o ltablib.o ltm.o lundump.o lvm.o lzio.o
  mkdir -p /tmp/out/FreeArc-win86-ghc
  cp liblua5.1.a /tmp/out/FreeArc-win86-ghc/
  ```

### Compile

```sh
./compile-ghc-win86
```

This runs `./compile-win86-c` first (C/C++ objects via GHC 8.6's bundled
mingw gcc 7.2.0 under Wine), builds `Win32Files.o` and `ntrljmp.o`, then
invokes GHC 8.6 under Wine to compile the Haskell side and link
`Tests/arc86.exe`.

### Runtime

`arc86.exe` depends on these DLLs (copy them next to the exe for
distribution):

- `libstdc++-6.dll`
- `libwinpthread-1.dll`
- `libgcc_s_dw2-1.dll`

All three are in `~/.wine/drive_c/ghc865-i386/mingw/bin/`.

The archiver also reads `arc.ini` and `arc.groups` at startup
(`Installer/bin/arc.ini` and `Installer/bin/arc.groups` in this repo).

---

## Why GHC 8.6.5?

GHC 8.6.5 is the last official GHC release with a Windows i386 (32-bit)
bindist. Later GHC versions dropped the i386 target and moved to UCRT.
This project targets msvcrt specifically so the resulting binary runs on
stock Windows 7/8/10 without the Universal C Runtime.
