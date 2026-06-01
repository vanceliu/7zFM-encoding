# 7zFM-encoding

Windows archive manager based on [7-Zip](https://github.com/ip7z/7zip) with **real-time encoding switching** to fix garbled filenames in archives.

## Features

- **Encoding Switch** — Toolbar button to instantly switch filename decoding (UTF-8, Shift-JIS, Big5, GBK, EUC-KR, and more)
- **Full 7-Zip format support** — All formats supported by 7-Zip (ZIP, 7z, RAR, TAR, GZ, XZ, ISO, CAB, etc.)
- **Password encryption/decryption** — AES-256, ZipCrypto (inherited from 7-Zip)
- **Native Win32 UI** — Single toolbar button addition, everything else unchanged from 7-Zip
- **Portable** — No installation required

## Screenshots

Toolbar with Encoding dropdown menu (click to switch codepage):

```
Add | Extract | Test | Copy | Move | Delete | Info | [A▾] Encoding
                                                     ┌──────────────┐
                                                     │ ✓ UTF-8      │
                                                     │   Shift-JIS  │
                                                     │   Big5       │
                                                     │   GBK        │
                                                     │   ...        │
                                                     └──────────────┘
```

## Build

Requires Visual Studio 2022 with C++ Desktop development tools.

```cmd
cd CPP\7zip\Bundles\Fm
nmake NEW_COMPILER=1 MY_STATIC_LINK=1
```

Output: `CPP\7zip\Bundles\Fm\x64\7zFM.exe`

### GitHub Actions

Pre-built Windows binaries are available from [Actions](../../actions) — download the `lwzip-windows-x64` artifact from the latest successful run.

## How It Works

When you open a ZIP archive with non-UTF-8 filenames (e.g., Japanese Shift-JIS, Chinese Big5), the filenames appear as garbled text. 7zFM-encoding adds an Encoding button to the 7-Zip toolbar that lets you switch the codepage on the fly — the archive is re-opened with the selected encoding and filenames display correctly.

## Supported Encodings

| Encoding | Code Page | Region |
|----------|-----------|--------|
| UTF-8 | 65001 | Universal |
| Shift-JIS | 932 | Japanese |
| Big5 | 950 | Traditional Chinese |
| GBK | 936 | Simplified Chinese |
| EUC-KR | 949 | Korean |
| ISO-8859-1 | 28591 | Western European |
| Windows-1252 | 1252 | Western European |
| EUC-JP | 20932 | Japanese (Unix) |
| GB2312 | 20936 | Simplified Chinese |
| KOI8-R | 20866 | Russian |
| Windows-1251 | 1251 | Russian |

## License

Based on 7-Zip by Igor Pavlov. See [DOC/License.txt](DOC/License.txt) for the 7-Zip license.
