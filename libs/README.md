# Dependencies

This folder contains development headers for MUI and MUI Custom Classes required to build rc-ftpd.

## Required Packages

| Package | Version | URL | Description |
|---------|---------|-----|-------------|
| MUI 3.8 Developer | 3.8 | http://aminet.net/dev/mui/mui38dev.lha | MUI development headers |
| MCC_NList | 0.128 | http://aminet.net/dev/mui/MCC_NList-0.128.lha | NList/NListview custom classes |
| MCC_TextInput | 29.5 | http://aminet.net/dev/mui/MCC_TextInput.lha | Textinput custom class |
| MCC_Lamp | - | http://aminet.net/dev/mui/MCC_Lamp.lha | Lamp indicator custom class |
| MCC_BetterBalance | 11.2 | http://aminet.net/dev/mui/MCC_BBalance.lha | BetterBalance custom class |

## bsdsocket Headers

The bsdsocket/networking headers (`bsdsocket.h`, `inetd.h`, etc.) are included in the NDK 3.2 which is part of bebbo's amiga-gcc toolchain. No separate download is required.

## Directory Structure

```
libs/
├── mui38dev/
│   └── MUI/
│       └── Developer/
│           └── C/
│               └── Include/
│                   ├── clib/
│                   ├── libraries/
│                   │   └── mui.h
│                   ├── pragma/
│                   └── proto/
├── MCC_NList/
│   └── Developer/
│       └── C/
│           └── Include/
│               └── mui/
│                   ├── NList_mcc.h
│                   └── NListview_mcc.h
├── MCC_Textinput/
│   └── Developer/
│       └── C/
│           └── Include/
│               └── mui/
│                   └── Textinput_mcc.h
├── MCC_Lamp/
│   └── Developer/
│       └── C/
│           └── Include/
│               └── mui/
│                   └── Lamp_mcc.h
└── MCC_BetterBalance/
    └── MCC_BetterBalance/
        └── Developer/
            └── C/
                └── Include/
                    └── mui/
                        └── BetterBalance_mcc.h
```

## Setup

To download and extract all dependencies:

```bash
cd libs
# MUI 3.8 dev
wget http://aminet.net/dev/mui/mui38dev.lha
lha x mui38dev.lha && mv MUI* mui38dev/

# NList
wget http://aminet.net/dev/mui/MCC_NList-0.128.lha
lha x MCC_NList-0.128.lha

# Textinput  
wget http://aminet.net/dev/mui/MCC_TextInput.lha
lha x MCC_TextInput.lha

# Lamp
wget http://aminet.net/dev/mui/MCC_Lamp.lha
lha x MCC_Lamp.lha

# BetterBalance
wget http://aminet.net/dev/mui/MCC_BBalance.lha
lha x MCC_BBalance.lha
# Note: extracts as nested .lzh, needs second extraction
mkdir MCC_BetterBalance && cd MCC_BetterBalance && lha x ../MCC_BetterBalance.lzh
```
