# LWZip — Cross-Platform Archive Manager

## Project Overview

基於 [ip7z/7zip](https://github.com/ip7z/7zip) 原始碼修改的跨平台壓縮/解壓縮 GUI 程式。
支援 macOS 與 Windows，具備編碼切換、密碼加解密、多格式支援等功能。

## Architecture

### Tech Stack

- **Language:** C++ (與 7zip 原始碼一致)
- **Windows GUI:** 原生 Win32 API (保持 7zip 原有 UI，僅加入編碼切換)
- **macOS GUI:** Qt 6 (仿 7zip FileManager 外觀，加入編碼切換)
- **Core Engine:** 7zip C/C++ core (直接整合)
- **Build System:** Windows 用 nmake (原生 makefile)，macOS 用 CMake + Qt6
- **Distribution:** Portable executables (macOS / Windows)

### Directory Structure

```
lwzip/
├── Asm/           — 7zip 組合語言優化 (CRC, SHA, AES, LZMA)
├── C/             — 7zip C 核心 (LZMA SDK)
├── CPP/           — 7zip C++ 層 (Archive, Compress, Crypto, UI)
│   └── 7zip/UI/FileManager/
│       ├── EncodingSwitch.h/cpp  — [新增] 編碼切換模組 (Windows)
│       ├── App.h/cpp             — [修改] 加入編碼 ComboBox
│       ├── FM.cpp                — [修改] 處理編碼切換事件
│       ├── FM.mak               — [修改] 加入 EncodingSwitch.obj
│       └── resource.h           — [修改] 加入 IDC_ENCODING_COMBO
├── DOC/           — 7zip 原始文件
├── GUI/
│   └── macOS/     — macOS Qt GUI (仿 7zip FileManager)
│       ├── LWMainWindow.h/cpp   — 主視窗 (含編碼切換)
│       ├── LWEncodingSwitch.h/cpp — 編碼切換模組
│       ├── main.cpp
│       └── CMakeLists.txt
├── CMakeLists.txt — 頂層 CMake (macOS 建構用)
└── CLAUDE.md
```

## Features

### 1. 編碼切換 (Encoding Switching) — 核心新增功能

- Toolbar 右側內建編碼下拉選單，可即時切換顯示編碼
- 支援編碼：UTF-8, Shift-JIS, Big5, GBK, EUC-KR, ISO-8859-1, Windows-1252, EUC-JP, GB2312, KOI8-R, Windows-1251
- 切換後壓縮檔內的檔名顯示即時更新
- 整合 7zip 現有的 codepage 機制

### 2. 壓縮格式支援 (與 7zip 完全一致)

**讀寫 (壓縮 + 解壓):**
- 7z, ZIP (預設), XZ, GZIP, BZIP2, TAR, WIM

**唯讀 (僅解壓):**
- RAR (含 RAR5), CAB, ISO, NSIS, UDF, ARJ, CPIO, RPM, ZSTD, CHM, VHD, XAR, SquashFS, QCOW, APM, HFS

### 3. 密碼加密/解密 (7zip 原有功能)

- 壓縮時可設定密碼 (支援 AES-256 加密)
- 解壓時彈出密碼輸入對話框
- 支援加密格式：7z AES, ZIP AES, ZipCrypto

### 4. 跨平台

- **Windows:** 原生 7zip Win32 UI + 編碼切換 ComboBox
- **macOS:** Qt6 GUI 仿 7zip FileManager 外觀 + 編碼切換
- Portable executable 形式發布

## Build Instructions

### macOS

```bash
# Prerequisites: Qt 6, CMake 3.20+, C++17 compiler
brew install cmake qt@6

mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
cmake --build . --parallel
```

### Windows

```cmd
# Prerequisites: Visual Studio, Windows SDK
cd CPP\7zip\Bundles\Fm
nmake
```

## Design Principles

- **最小化修改 7zip core** — 方便未來同步上游更新
- Windows 版只在原生 UI 上加入編碼切換 ComboBox，其餘保持不變
- macOS 版用 Qt 重現 7zip FileManager 的基本外觀和操作邏輯
- 編碼切換邏輯集中在 `EncodingSwitch` 模組

## Development Guidelines

- 遵循 7zip 原有的 coding style (C++ 層)
- macOS Qt GUI 層使用 Qt 官方推薦的命名慣例 (LW 前綴)
- 新增功能需同時考慮 macOS 和 Windows 的行為差異
- `original-7zip` branch 保存未修改的原始碼，方便 diff 比對
