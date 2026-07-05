# RC-FTPd

A full-featured FTP server for AmigaOS with MUI graphical interface.

**[Download latest release](https://github.com/alfishe/rc-ftpd/releases/latest)**

## Features

- Complete FTP server implementation (RFC 959 compliant)
- MUI-based graphical user interface
- Real-time connection monitoring and logging
- Multi-user support with configurable access permissions
- Per-user and per-path access control
- Passive (PASV) and active mode support
- ARexx interface for scripting and automation
- Localization support (multiple languages)
- Asynchronous I/O for high performance

## Supported FTP Commands

Fully RFC 959 compliant with common extensions.

### Access Control

| Command | Description |
|---------|-------------|
| USER | Specify username for authentication |
| PASS | Specify password for authentication |
| ACCT | Specify account (accepted but not used) |
| REIN | Reinitialize connection |
| QUIT | Logout and close connection |

### Transfer Parameters

| Command | Description |
|---------|-------------|
| PORT | Specify data port for active mode |
| PASV | Enter passive mode |
| TYPE | Set transfer type (ASCII/Binary) |
| STRU | Set file structure (File only) |
| MODE | Set transfer mode (Stream only) |

### File Operations

| Command | Description |
|---------|-------------|
| RETR | Download (retrieve) a file |
| STOR | Upload (store) a file |
| APPE | Append data to existing file |
| ALLO | Allocate storage space |
| REST | Set restart point for resume |
| RNFR | Rename from (source path) |
| RNTO | Rename to (destination path) |
| ABOR | Abort current transfer |
| DELE | Delete a file |

### Directory Operations

| Command | Description |
|---------|-------------|
| CWD | Change working directory |
| CDUP | Change to parent directory |
| MKD | Create (make) directory |
| RMD | Remove directory |
| PWD | Print working directory |
| LIST | List directory contents (detailed) |
| NLST | List directory names only |

### Information

| Command | Description |
|---------|-------------|
| SYST | Return system type (AmigaOS) |
| STAT | Return server status |
| HELP | Show help for commands |
| SITE | Site-specific commands |
| NOOP | No operation (keep-alive) |
| FEAT | List supported features |
| CLNT | Identify client software |

### Extensions (RFC 3659)

| Command | Description |
|---------|-------------|
| MDTM | Get file modification time |
| SIZE | Get file size |

### X-Commands (Obsolete Aliases)

| Command | Alias For |
|---------|-----------|
| XCUP | CDUP |
| XCWD | CWD |
| XMKD | MKD |
| XPWD | PWD |
| XRMD | RMD |

## Requirements

### Runtime
- AmigaOS 3.0+
- MUI 3.8+
- TCP/IP stack (Roadshow, AmiTCP, Miami)
- MUI Custom Classes:
  - NList.mcc / NListview.mcc
  - Textinput.mcc
  - Lamp.mcc
  - BetterBalance.mcc

### Build
- bebbo's amiga-gcc cross-compiler toolchain
- GNU Make

## Building

### Prerequisites

1. Install bebbo's amiga-gcc toolchain from https://codeberg.org/bebbo/amiga-gcc
2. Ensure `/opt/amiga/bin` is in your PATH
3. Dependencies in `libs/` folder (included in repo)

### Make Targets

| Target | Description |
|--------|-------------|
| `make` | Build the ftpd binary to `build/ftpd` |
| `make clean` | Remove all build artifacts |
| `make dist` | Build and copy binary to `dist/RCFTPd/ftpd` |
| `make release` | Create Aminet release package in `release/` |

### Build Commands

```bash
# Simple build
export PATH=/opt/amiga/bin:$PATH
make

# Full release (creates rc-ftpd.lha for Aminet)
make release

# Clean and rebuild
make clean && make
```

### Build Output

| File | Description |
|------|-------------|
| `build/ftpd` | Compiled Amiga executable (~87KB) |
| `build/*.o` | Object files |
| `dist/RCFTPd/ftpd` | Binary ready for distribution |
| `release/rc-ftpd.lha` | Aminet release archive |
| `release/rc-ftpd.readme` | Aminet readme file |

### Compiler Flags

The default build uses size-optimized flags:

| Flag | Purpose |
|------|---------|
| `-Os` | Optimize for size |
| `-fomit-frame-pointer` | Free A6 register for smaller code |
| `-noixemul` | Use libnix (no ixemul.library dependency) |
| `-Wall` | Enable all warnings |
| `-s` | Strip symbols from final binary |

### Binary Size by Optimization Level

| Flags | Size |
|-------|------|
| `-O1` | 92KB |
| `-O2` | 101KB |
| `-O3` | 117KB |
| `-Os` | 87KB |
| `-Os -fomit-frame-pointer` | **87KB** (default) |

## Project Structure

```
rc-ftpd/
├── src/           # C source files
├── libs/          # MUI/MCC development headers
├── build/         # Compiled objects and binary
├── dist/          # Distribution files (installer, icons, docs)
├── release/       # Aminet release packages
├── doc/           # Documentation and RFCs
├── locale/        # Localization files (.cd, .ct)
├── contrib/       # ARexx scripts, examples
├── scripts/       # Key generation tools
├── Makefile       # Build configuration
└── README.md      # This file
```

## License

GNU General Public License version 2.0 (GPLv2)

## Authors

**Original Author:** Robin Cloutman

**Maintainer:** Ilia Sharin

## Links

- Repository: https://github.com/alfishe/rc-ftpd
- Original: https://sourceforge.net/p/rc-ftpd/
