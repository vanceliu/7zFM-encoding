# LWZip — Windows Archive Manager with Encoding Switch

## Project Overview

基於 [ip7z/7zip](https://github.com/ip7z/7zip) 原始碼修改的 Windows 壓縮/解壓縮 GUI 程式。
核心新增功能為**即時編碼切換**，解決壓縮檔內檔名亂碼問題。

## Architecture

### Tech Stack

- **Language:** C++ (與 7zip 原始碼一致)
- **Windows GUI:** 原生 Win32 API (保持 7zip 原有 UI，加入 Encoding toolbar button)
- **Core Engine:** 7zip C/C++ core (直接整合，最小化修改)
- **Build System:** nmake (原生 makefile)
- **CI/CD:** GitHub Actions (Windows build + artifact upload)
- **Distribution:** Portable executable (Windows .exe)

### Directory Structure

```
lwzip/
├── Asm/                — 7zip 組合語言優化 (CRC, SHA, AES, LZMA)
├── C/                  — 7zip C 核心 (LZMA SDK, 跨平台)
├── CPP/                — 7zip C++ 層
│   ├── Common/         — 通用工具 (String, UTF, Wildcard)
│   ├── Windows/        — Windows 抽象層 (File, Registry, Control)
│   ├── 7zip/
│   │   ├── Archive/    — 壓縮格式 handlers (Zip, 7z, Rar, Tar...)
│   │   │   └── Zip/ZipHandler.cpp  — [修改] kpidPath 加入 codepage override
│   │   ├── Compress/   — 壓縮演算法 (LZMA, Deflate, PPMd...)
│   │   ├── Crypto/     — 加密 (AES, ZipCrypto, Rar AES)
│   │   ├── LWZipCodePage.h/cpp     — [新增] 全域 codepage override 狀態
│   │   ├── UI/
│   │   │   ├── Agent/
│   │   │   │   └── AgentProxy.cpp  — [修改] 跳過 GetRawProps 快速路徑
│   │   │   ├── FileManager/
│   │   │   │   ├── EncodingSwitch.h/cpp — [新增] 編碼切換模組
│   │   │   │   ├── App.h/cpp       — [修改] Encoding toolbar button + popup menu
│   │   │   │   ├── FM.cpp          — [修改] TBN_DROPDOWN + WM_COMMAND 處理
│   │   │   │   ├── FM.mak          — [修改] 加入 EncodingSwitch.obj
│   │   │   │   ├── resource.h      — [修改] IDC_ENCODING_COMBO, IDB_ENCODING
│   │   │   │   ├── resource.rc     — [修改] 加入 Encoding bitmap 資源
│   │   │   │   ├── Encoding.bmp    — [新增] 大圖 icon (48x36, bold "A")
│   │   │   │   └── Encoding2.bmp   — [新增] 小圖 icon (24x24, bold "A")
│   │   │   └── Common/             — UI 共用 (OpenArchive, Extract, Update)
│   │   └── Bundles/
│   │       └── Fm/makefile          — [修改] 加入 LWZipCodePage.obj
│   └── Build.mak                    — 7zip 共用建構規則
├── DOC/                — 7zip 原始文件
├── .github/workflows/
│   └── build-windows.yml           — GitHub Actions Windows build
└── CLAUDE.md
```

### Branches

- `main` — 含編碼切換功能的修改版
- `original-7zip` — 未修改的原始 7zip source (方便 diff 比對)

## Encoding Switch — 核心功能實作

### 運作原理 (Windows)

```
User clicks Encoding → selects Shift-JIS
    ↓
App::OnEncodingSelected(1)
    ↓
EncodingSwitch::SetCurrentIndex(1)
  → g_LWZip_ForceCodePage = true
  → g_LWZip_CodePage = 932
    ↓
panel.CloseOpenFolders()     ← 關閉已開啟的壓縮檔
    ↓
panel.BindToPathAndRefresh(path)
    ↓
_parentFolders is empty → takes filesystem path
    ↓
OpenAsArc() → new CAgent → CProxyArc::Load()
    ↓
arc.Archive->GetProperty(i, kpidPath, &prop)
    ↓
ZipHandler::GetProperty checks g_LWZip_ForceCodePage
  → item.GetUnicodeString(res, item.Name, false, true, 932)
    ↓
Filename decoded with Shift-JIS → 正確顯示日文
```

### 關鍵修改點

1. **`LWZipCodePage.h/cpp`** — 全域 codepage 狀態 (`g_LWZip_ForceCodePage`, `g_LWZip_CodePage`)
2. **`ZipHandler.cpp` GetProperty(kpidPath)** — 檢查全域 override，用指定 codepage 解碼檔名
3. **`AgentProxy.cpp` CProxyArc::Load()** — 當 override 啟用時跳過 GetRawProps 快速路徑
4. **`App.cpp` OnEncodingSelected()** — CloseOpenFolders + BindToPathAndRefresh 強制重新開啟
5. **`EncodingSwitch.h/cpp`** — 編碼列表管理、設定全域狀態

### 支援的編碼

| 名稱 | Code Page | 用途 |
|------|-----------|------|
| UTF-8 | 65001 | 預設 |
| Shift-JIS | 932 | 日文 |
| Big5 | 950 | 繁體中文 |
| GBK | 936 | 簡體中文 |
| EUC-KR | 949 | 韓文 |
| ISO-8859-1 | 28591 | 西歐 |
| Windows-1252 | 1252 | 西歐 (Windows) |
| EUC-JP | 20932 | 日文 (Unix) |
| GB2312 | 20936 | 簡體中文 (舊) |
| KOI8-R | 20866 | 俄文 |
| Windows-1251 | 1251 | 俄文 (Windows) |
| System (ACP) | 0 | 系統預設 |
| OEM | 1 | OEM code page |

## Build Instructions

### Windows (本機)

```cmd
# 需要 Visual Studio 2022 + Windows SDK
# 開啟 Developer Command Prompt

# Build 7z.dll (格式庫)
cd CPP\7zip\Bundles\Format7zF
nmake NEW_COMPILER=1 MY_STATIC_LINK=1

# Build 7zFM.exe (File Manager)
cd ..\..\UI\FileManager
nmake NEW_COMPILER=1 MY_STATIC_LINK=1

# 產出:
#   CPP\7zip\Bundles\Format7zF\x64\7z.dll
#   CPP\7zip\UI\FileManager\x64\7zFM.exe
# 使用時將 7zFM.exe + 7z.dll 放在同一目錄
```

### Windows (GitHub Actions)

Push 到 main branch 後自動觸發，或手動 dispatch。
下載: Actions → 最新 run → Artifacts → `7zFM-encoding-windows-x64`

## Design Principles

- **最小化修改 7zip core** — 只改必要的地方，方便未來同步上游更新
- **不改變原有行為** — 預設不啟用 codepage override，只有使用者主動切換才生效
- **保持原生 UI** — 只加入一個 toolbar button，其餘完全不變

## Development Guidelines

- 遵循 7zip 原有的 coding style (C++ 層)
- MSVC 編譯使用 `-Wall -WX`，所有 warning 都是 error
- 不使用 `..` 相對路徑 include（觸發 C4464）
- 成員初始化順序必須與宣告順序一致（C5038）
- `original-7zip` branch 保存未修改的原始碼
