# DArc86

**DArc86** is the 32-bit Windows port of [DArc](https://github.com/YadeWira/DArc) — a command-line archiver derived from [FreeArc](http://freearc.org) — targeted at legacy **Windows 7 / 8 / 10 (x86)** systems.

Built via **Wine + GHC 8.6.5 i386**, the last GHC bindist that supports 32-bit Windows. The resulting binary is `arc86.exe` (PE32, Intel i386).

> The 64-bit variants (Linux x64 + Windows x64, built with MicroHs or GHC 9.4) live in the separate [DArc](https://github.com/YadeWira/DArc) repository.

---

## Archive format compatibility

The wire format is **100% compatible** between DArc86, DArc (64-bit), and FreeArc 0.67:

- Archives produced by DArc86 can be read by DArc, FreeArc 0.67 (32-bit legacy) and older.
- Archives produced by DArc / FreeArc 0.67 can be read by DArc86.
- For round-tripping with **FreeArc 0.67 32-bit legacy archives**, pass `--arc-32bit-legacy`.

---

## Building

Cross-compilation from Linux via Wine is the supported path.

### Requirements

- **Wine** (tested with wine 9.x)
- **GHC 8.6.5 Windows i386 bindist** installed under Wine at `~/.wine/drive_c/ghc865-i386/`
  - Download: <https://downloads.haskell.org/~ghc/8.6.5/ghc-8.6.5-i386-unknown-mingw32.tar.xz>
  - Extract with `tar -xf ghc-8.6.5-*.tar.xz -C ~/.wine/drive_c/ --strip-components=1` and rename to `ghc865-i386/`.
- **i686-w64-mingw32** toolchain on the host (`g++-posix`, `objdump`, `objcopy`)
  - Ubuntu/Debian: `sudo apt install mingw-w64 g++-mingw-w64-i686-posix`
- **Lua 5.1 static library** built for i686 at `/tmp/out/FreeArc-win86-ghc/liblua5.1.a`
  - The `compile-ghc-win86` script prints the exact build commands if this file is missing.

### Build

```bash
./compile-ghc-win86
```

The script runs two GHC stages around an `objcopy` strip pass (see **Notes** below) and produces:

```
Tests/arc86.exe     # PE32 Intel 80386, Windows console binary
```

### Native Windows build (legacy)

The `compile*.cmd` batch files are the original FreeArc native-Windows build scripts kept for reference. They assume an ancient GHC + Cygwin/MSYS environment and are not actively maintained — the Wine cross-build is the recommended path.

---

## Notes on the build process

GHC 8.6 i386 runs an incremental `ld -r` per Haskell module and bundles every `-optl` C object (together with its `.idata$*` sections) into the module that references it. The merged `.idata` then leaks into the final PE as a bogus 13th import descriptor, which causes Windows 7 to reject the binary with `STATUS_ENTRYPOINT_NOT_FOUND (0xC0000139)` at load time.

`compile-ghc-win86` works around this by splitting the build in two stages:

1. **Compile** (`ghc --make -c -no-link`) — produces all `.o` files in the temp dir but does not link.
2. **Strip** — for every `.o` whose `objdump -h` shows any `.idata` section, run `i686-w64-mingw32-objcopy --remove-section='.idata$2' --remove-section='.idata$4' --remove-section='.idata$5' --remove-section='.idata$6' --remove-section='.idata$7'`.
3. **Link** — re-invoke the same `ghc --make` line with `-o arc86.exe`. Since the sources are unchanged GHC skips compilation and proceeds straight to link, picking up the stripped `.o` files. The remaining `_imp__*` refs resolve cleanly from `libkernel32.a`, `libwininet.a`, etc., yielding a valid 12-descriptor PE import table.

---

## CLI Usage

```
arc86 <command> [options...] <archive> [files... @listfiles...]
```

- **`<command>`** — one of the commands listed below.
- **`[options...]`** — zero or more options (each prefixed with `-`).
- **`<archive>`** — path to the archive file. The default extension `.arc` is added automatically unless `--noarcext` is used.
- **`[files...]`** — files or directories to process. Wildcards are supported. If omitted, all files are processed (`*`).
- **`[@listfiles...]`** — text files containing lists of filenames to process, one per line.

Multiple commands can be chained with `;` as a separator:

```
arc86 "a archive -r ; t archive ; x archive"
```

---

## Commands

| Command   | Description |
|-----------|-------------|
| `a`       | Add files to archive |
| `c`       | Add comment to archive |
| `ch`      | Modify archive (recompress, encrypt, etc.) |
| `create`  | Create new archive |
| `cw`      | Write archive comment to file |
| `d`       | Delete files from archive |
| `e`       | Extract files from archive, ignoring pathnames |
| `f`       | Freshen archive (update files that are newer on disk) |
| `j`       | Join archives |
| `k`       | Lock archive |
| `l`       | List files in archive |
| `lb`      | Bare list of files in archive (filenames only) |
| `lt`      | Technical archive listing |
| `m`       | Move files and directories to archive |
| `mf`      | Move only files to archive |
| `r`       | Recover archive using recovery record |
| `rr`      | Add recovery record to archive |
| `s`       | Convert archive to SFX (self-extracting) |
| `t`       | Test archive integrity |
| `u`       | Update files in archive |
| `v`       | Verbosely list files in archive |
| `x`       | Extract files from archive (preserving paths) |

### Command Examples

```cmd
rem Add all files recursively
arc86 a archive.arc -r .

rem Extract all files
arc86 x archive.arc

rem Test archive integrity
arc86 t archive.arc

rem List archive contents
arc86 l archive.arc

rem Add a 5% recovery record
arc86 rr archive.arc -rr5%

rem Read a FreeArc 0.67 legacy 32-bit archive
arc86 x --arc-32bit-legacy old067.arc
```

---

## Options

Options use the short form `-<opt>` or long form `--<option>`.
Options that take a parameter use `-<opt><value>` or `--<option>=<value>`.

### General

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-y`  | `--yes`           | Answer Yes to all queries |
| `--`  |                   | Stop processing options |
| `-cfg FILE` | `--config=FILE` | Use config FILE (default: `arc.ini`) |
| `-env VAR`  |                 | Read default options from environment variable VAR (default: `FREEARC`) |
|       | `--arc-32bit-legacy` | Read FreeArc 0.67 / legacy 32-bit archives |

### File Selection

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-r`  | `--recursive`     | Recursively collect files from subdirectories |
| `-x FILESPECS` | `--exclude=FILESPECS` | Exclude matching files from operation |
| `-n FILESPECS` | `--include=FILESPECS` | Include only files matching FILESPECS |
| `-ep MODE` | `--ExcludePath=MODE` | Exclude/expand path (1, 2, or 3) |
| `-fn` | `--fullnames`     | Match filespecs against full file paths |
| `-sm SIZE` | `--SizeMore=SIZE` | Select files larger than SIZE |
| `-sl SIZE` | `--SizeLess=SIZE` | Select files smaller than SIZE |
| `-tb TIME` | `--TimeBefore=TIME` | Select files modified before TIME |
| `-ta TIME` | `--TimeAfter=TIME`  | Select files modified after TIME |
| `-tn PERIOD` | `--TimeNewer=PERIOD` | Select files newer than PERIOD |
| `-to PERIOD` | `--TimeOlder=PERIOD` | Select files older than PERIOD |

### Paths

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-ap DIR` | `--arcpath=DIR`  | Base directory inside archive |
| `-dp DIR` | `--diskpath=DIR` | Base directory on disk |
| `-ad`  | `--adddir`        | Add archive name to extraction path |
| `-w DIR` | `--workdir=DIR`  | Directory for temporary files |

### Compression

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-m METHOD` | `--method=METHOD` | Compression method (`-m0`–`-m9`, `-m1x`–`-m9x`, `-mbsc`, `-mzstd`, `-mdispack`, `-mlzma2`, `-m7z`) |
| `-dm METHOD` | `--dirmethod=METHOD` | Compression method for archive directory |
| `-md N` | `--dictionary=N` | Set compression dictionary to N MB |
| `-ms`  | `--StoreCompressed` | Store already-compressed files without recompression |
| `-mt N` | `--MultiThreaded=N` | Number of compression threads |
| `-mc`  |                   | Disable specific compression algorithms (e.g., `-mcd-`, `-mc-rep`) |
| `-mm MODE` | `--multimedia=MODE` | Multimedia compression mode |
| `-ma LEVEL` |               | File-type auto-detection level (0–9, `+`, `-`) |
| `-mx`  |                   | Maximum internal compression mode |
| `-max` |                   | Maximum compression (uses external tools: precomp, ecm, ppmonstr) |
| `-s GROUPING` | `--solid=GROUPING` | Solid compression grouping |
| `-ds ORDER` | `--sort=ORDER`   | Sort files in ORDER before compressing |
| `--groups=FILE` |            | Name of file-groups definition file |
| `-lc N` | `--LimitCompMem=N` | Limit memory for compression to N MB |
| `-ld N` | `--LimitDecompMem=N` | Limit memory for decompression to N MB |

> **Memory note**: because `arc86.exe` is a 32-bit process, total addressable memory is capped at ~2 GB (or ~4 GB with `/LARGEADDRESSAWARE` on Win64 hosts). Very large dictionaries (`-md 1g+`) or deep solid blocks may hit this ceiling; prefer DArc x64 for that workload.

#### Compression Levels

| Option | Description |
|--------|-------------|
| `-m0`  | No compression (store only) |
| `-m1`–`-m9` | Compression levels 1–9 (increasing compression/time) |
| `-m1x`–`-m9x` | Extra-mode compression at levels 1–9 |
| `-mx` or `-max` | Maximum compression |

#### Solid Grouping Values (`-s`)

| Value | Description |
|-------|-------------|
| _(empty)_ | All files in one solid block |
| `-` | No solid compression |
| `e` | Group by file extension |
| `s<size>` | Group by block size |

### Encryption

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-p PASSWORD` | `--password=PASSWORD` | Encrypt/decrypt data with PASSWORD |
| `-hp PASSWORD` | `--HeadersPassword=PASSWORD` | Encrypt/decrypt archive headers and data |
| `-ae ALGO` | `--encryption=ALGO` | Encryption algorithm: `aes` (default), `blowfish`, `serpent`, `twofish` |
| `-kf FILE` | `--keyfile=FILE`  | Encrypt/decrypt using KEYFILE |
| `-op PASSWORD` | `--OldPassword=PASSWORD` | Old password used only for decryption |
| `-okf FILE` | `--OldKeyfile=FILE` | Old keyfile used only for decryption |

### Archive Management

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-f`  | `--freshen`       | Update only files that are newer on disk |
| `-u`  | `--update`        | Update only files not present or newer on disk |
| `--sync` |                | Synchronize archive and disk contents |
| `-o MODE` | `--overwrite=MODE` | Overwrite mode: `+` (always), `-` (never), `p` (prompt) |
| `-k`  | `--lock`          | Lock archive to prevent modifications |
| `-rr SIZE` | `--recovery=SIZE` | Add recovery information of SIZE to archive |
| `-sfx MODULE` |            | Add SFX module (`freearc.sfx` by default) |
| `--noarcext` |              | Do not add the default `.arc` extension to archive name |
| `-ag FMT` | `--autogenerate=FMT` | Autogenerate archive name using a time format string |
| `--recompress` |            | Force recompression of all files |
| `--append` |                | Add new files to the end of archive only |
| `-z FILE` | `--arccmt=FILE` | Read archive comment from FILE or stdin |
| `--archive-comment=TEXT` |  | Specify archive comment directly on the command line |
| `-t`  | `--test`          | Test archive integrity after archiving |
| `-tp MODE` | `--pretest=MODE` | Test archive before operation (0=none, 1=recovery only, 2=recovery or full, 3=full) |
| `-d`  | `--delete`        | Delete files and directories after successful archiving |
| `-df` | `--delfiles`      | Delete only files after successful archiving |
| `-kb` | `--keepbroken`    | Keep broken extracted files |
| `-ba MODE` | `--BrokenArchive=MODE` | Handle badly broken archives (`-`, `0`, or `1`) |
| `-tk` | `--keeptime`      | Keep original archive modification time |
| `-tl` | `--timetolast`    | Set archive time to the latest file's modification time |
| `--dirs` |                  | Add empty directories to archive |
| `-ed` | `--nodirs`        | Do not add empty directories to archive |
| `--nodates` |              | Do not store file modification dates in archive |

### Windows-Only Options

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-ac` | `--ClearArchiveBit` | Clear Archive attribute on successfully (de)archived files |
| `-ao` | `--SelectArchiveBit` | Select only files with Archive attribute set |

### Display and Logging

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-i TYPE` | `--indicator=TYPE` | Progress indicator type: `0` (none), `1` (default), `2` (per-file) |
| `-di AMOUNT` | `--display=AMOUNT` | Control amount of information displayed |
| `--logfile=FILE` |          | Duplicate all output to FILE |
| `--print-config` |          | Display built-in compression method definitions |

### Network/URL Options (WinInet)

| Long              | Description |
|-------------------|-------------|
| `--proxy=PROXY`   | Set proxy server(s) for URL access |
| `--bypass=LIST`   | Set proxy bypass list for URL access |
| `--original=URL`  | Re-download broken archive parts from URL |
| `--save-bad-ranges=FILE` | Save list of broken archive parts to FILE |
| `--cache=N`       | Use N MB for read-ahead cache |

### Charset

| Short | Long              | Description |
|-------|-------------------|-------------|
| `-sc CHARSETS` | `--charset=CHARSETS` | Character sets for list files and comment files |
| `--language=FILE` |          | Load localization strings from FILE |

---

## Configuration File (`arc.ini`)

By default, DArc86 reads options from `arc.ini` in standard search paths. You can override the config file with `-cfg <file>` or disable it with `-cfg-`.

**Default options** can be set per-command in the `[Default options]` section:

```ini
[Default options]
a = --display
ch = -m4x -ms
```

**Compression methods** can be defined in the `[Compression methods]` section.

The `FREEARC` environment variable is also read for default options (override with `-env <VAR>` or disable with `-env-`).

---

## List Files

You can pass a file containing a list of filenames (one per line) to any command by prefixing the filename with `@`:

```cmd
arc86 a archive.arc @myfiles.txt
```

---

## Examples

```cmd
rem Create archive with maximum compression
arc86 a -mx myarchive.arc documents\

rem Create encrypted archive
arc86 a -p"my secret" secure.arc private\

rem Extract archive to a specific directory
arc86 x archive.arc -dp C:\Users\me\extracted\

rem Add recovery record (10% of archive size)
arc86 ch myarchive.arc -rr10%

rem Update archive with changed files
arc86 u myarchive.arc documents\

rem Compress with BSC
arc86 a -mbsc myarchive.arc text\

rem Compress with zstd level 9
arc86 a -mzstd:9 myarchive.arc data\

rem List contents of a FreeArc 0.67 legacy archive
arc86 l --arc-32bit-legacy old067.arc
```

---

## Known limitations

- **2 GB process memory cap** (32-bit). For large dictionaries or big solid blocks, use the 64-bit DArc build.
- No GUI variant — `arc86.exe` is console-only. The FreeArc GUI front-end is not ported.
- The `max` compression mode relies on external tools (`precomp`, `ecm`, `ppmonstr`) that must be present in `PATH`.

---

## License

See [LICENSE](LICENSE) for details.
