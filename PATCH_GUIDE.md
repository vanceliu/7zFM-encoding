# 7zFM-encoding Patch Guide

當 7-Zip 上游 (ip7z/7zip) 發布新版本時，依照此文件將 encoding switch 功能套用到新版本。

## 修改清單

共修改/新增 **12 個檔案**，分為 3 層：

### Layer 1: 全域 Codepage 狀態 (新增)

| 檔案 | 動作 | 說明 |
|------|------|------|
| `CPP/7zip/LWZipCodePage.h` | 新增 | 宣告 `g_LWZip_CodePage` 和 `g_LWZip_ForceCodePage` |
| `CPP/7zip/LWZipCodePage.cpp` | 新增 | 定義全域變數 |

### Layer 2: Archive Handler 修改 (最小化)

| 檔案 | 動作 | 修改位置 | 說明 |
|------|------|----------|------|
| `CPP/7zip/Archive/Zip/ZipHandler.cpp` | 修改 | `GetProperty(kpidPath)` case | 加入 `g_LWZip_ForceCodePage` 檢查，用全域 codepage 解碼檔名 |
| `CPP/7zip/UI/Agent/AgentProxy.cpp` | 修改 | `CProxyArc::Load()` 的 GetRawProps 區塊 | 當 override 啟用時跳過快速路徑 |

### Layer 3: FileManager UI 修改

| 檔案 | 動作 | 說明 |
|------|------|------|
| `CPP/7zip/UI/FileManager/EncodingSwitch.h` | 新增 | 編碼切換類別宣告 |
| `CPP/7zip/UI/FileManager/EncodingSwitch.cpp` | 新增 | 編碼列表、設定全域 codepage |
| `CPP/7zip/UI/FileManager/Encoding.bmp` | 新增 | Toolbar 大圖 icon (48x36) |
| `CPP/7zip/UI/FileManager/Encoding2.bmp` | 新增 | Toolbar 小圖 icon (24x24) |
| `CPP/7zip/UI/FileManager/resource.h` | 修改 | 加入 `IDC_ENCODING_COMBO`, `IDM_ENCODING_CHANGED`, `IDB_ENCODING`, `IDB_ENCODING2` |
| `CPP/7zip/UI/FileManager/resource.rc` | 修改 | 加入 `IDB_ENCODING` 和 `IDB_ENCODING2` BITMAP 資源 |
| `CPP/7zip/UI/FileManager/App.h` | 修改 | 加入 `_encodingButton`, `CreateEncodingButton()`, `ShowEncodingMenu()`, `OnEncodingSelected()` |
| `CPP/7zip/UI/FileManager/App.cpp` | 修改 | 實作 toolbar button + popup menu + CloseOpenFolders 重新開啟 |
| `CPP/7zip/UI/FileManager/FM.cpp` | 修改 | 處理 `TBN_DROPDOWN` 和 `WM_COMMAND` 編碼選單事件 |
| `CPP/7zip/UI/FileManager/FM.mak` | 修改 | 加入 `EncodingSwitch.obj` |
| `CPP/7zip/Bundles/Fm/makefile` | 修改 | 加入 `LWZipCodePage.obj` 和建構規則 |

---

## 升級步驟

### Step 1: 取得新版 7-Zip 原始碼

```bash
git checkout original-7zip
# 清除舊檔案 (保留 .git)
find . -maxdepth 1 ! -name '.git' ! -name '.' -exec rm -rf {} \;
# 複製新版原始碼
cp -r /path/to/new-7zip-source/* .
git add -A && git commit -m "Update to 7-Zip XX.XX"
```

### Step 2: 合併到 main

```bash
git checkout main
git merge original-7zip
```

### Step 3: 解決衝突

可能衝突的檔案（依優先順序處理）：

1. **`CPP/7zip/Archive/Zip/ZipHandler.cpp`**
   - 找到 `case kpidPath:` 區塊
   - 確認 `item.GetUnicodeString(res, item.Name, false, _forceCodePage, _specifiedCodePage)` 這行
   - 替換為帶 override 的版本

2. **`CPP/7zip/UI/Agent/AgentProxy.cpp`**
   - 找到 `if (arc.GetRawProps)` 那行
   - 加入 `&& !g_LWZip_ForceCodePage` 條件

3. **`CPP/7zip/UI/FileManager/App.h` / `App.cpp`**
   - 確認 `CApp` class 的成員變數和方法還在
   - `ReloadToolbars()` 裡的 Encoding button 程式碼

4. **`CPP/7zip/UI/FileManager/FM.cpp`**
   - `WM_NOTIFY` 的 `TBN_DROPDOWN` 處理
   - `WM_COMMAND` 的 encoding menu 處理

5. **`CPP/7zip/UI/FileManager/resource.h`**
   - 確認 ID 數字不與新版衝突

6. **`CPP/7zip/UI/FileManager/FM.mak`**
   - 確認 `EncodingSwitch.obj` 在 `FM_OBJS` 裡

7. **`CPP/7zip/Bundles/Fm/makefile`**
   - 確認 `LWZipCodePage.obj` 和建構規則

### Step 4: 驗證建構

```cmd
cd CPP\7zip\Bundles\Fm
nmake NEW_COMPILER=1 MY_STATIC_LINK=1
```

### Step 5: 測試

1. 開啟含日文檔名的 ZIP (Shift-JIS 編碼)
2. 確認顯示亂碼
3. 點擊 Encoding → Shift-JIS
4. 確認檔名正確顯示
5. 切回 UTF-8，確認恢復亂碼
6. 測試其他編碼 (Big5, GBK)

---

## 各修改點的 Diff 模板

### ZipHandler.cpp — GetProperty(kpidPath)

```cpp
// 在檔案開頭加入 include
#include "../../LWZipCodePage.h"

// 找到 case kpidPath: 區塊，替換 GetUnicodeString 呼叫
case kpidPath:
{
  UString res;
  bool forceCP = _forceCodePage || g_LWZip_ForceCodePage;
  UInt32 cp = g_LWZip_ForceCodePage ? g_LWZip_CodePage : _specifiedCodePage;
  item.GetUnicodeString(res, item.Name, false, forceCP, cp);
  // ... 後面不變
```

### AgentProxy.cpp — CProxyArc::Load()

```cpp
// 在 include 區加入
#include "../../LWZipCodePage.h"

// 找到 GetRawProps 快速路徑，加入條件
#if defined(MY_CPU_LE) && defined(_WIN32)
    if (arc.GetRawProps && !g_LWZip_ForceCodePage)  // <-- 加入 !g_LWZip_ForceCodePage
    {
```

### App.h — CApp class

```cpp
// 加入 include
#include "EncodingSwitch.h"

// 在 _toolBar 後加入
HWND _encodingButton;

// 在 ReloadToolbars() 後加入
void CreateEncodingButton();
void ShowEncodingMenu();
void OnEncodingSelected(unsigned index);

// 初始化列表加入 (注意順序必須與宣告順序一致)
_encodingButton(NULL),
```

### App.cpp — ReloadToolbars() 加入 Encoding button

```cpp
// 在 _toolBar.AutoSize() 之前加入
{
  TBBUTTON but;
  but.iBitmap = 0;
  but.idCommand = IDC_ENCODING_COMBO;
  but.fsState = TBSTATE_ENABLED;
  but.fsStyle = TBSTYLE_BUTTON | BTNS_WHOLEDROPDOWN;
  but.dwData = 0;
  UString s(L"Encoding");
  but.iString = 0;
  if (ShowButtonsLables)
    but.iString = (INT_PTR)(LPCWSTR)s;
  but.iBitmap = _buttonsImageList.GetImageCount();
  HBITMAP b = ::LoadBitmap(g_hInstance,
      LargeButtons ? MAKEINTRESOURCE(IDB_ENCODING) : MAKEINTRESOURCE(IDB_ENCODING2));
  if (b) {
    _buttonsImageList.AddMasked(b, RGB(255, 0, 255));
    ::DeleteObject(b);
  }
  #ifdef _UNICODE
  _toolBar.AddButton(1, &but);
  #else
  _toolBar.AddButtonW(1, &but);
  #endif
}
```

### App.cpp — OnEncodingSelected()

```cpp
void CApp::OnEncodingSelected(unsigned index)
{
  if (index < CEncodingSwitch::GetNumEncodings())
  {
    g_EncodingSwitch.SetCurrentIndex(index);
    for (unsigned i = 0; i < NumPanels; i++)
    {
      unsigned panelIndex = (NumPanels == 1) ? LastFocusedPanel : i;
      CPanel &panel = Panels[panelIndex];
      if (panel.PanelCreated)
      {
        UString path = panel._currentFolderPrefix;
        panel.CloseOpenFolders();
        panel.BindToPathAndRefresh(path);
      }
    }
  }
}
```

### FM.cpp — WM_NOTIFY 加入 TBN_DROPDOWN

```cpp
case WM_NOTIFY:
{
  LPNMHDR pnmh = (LPNMHDR)lParam;
  if (pnmh->code == TBN_DROPDOWN)
  {
    LPNMTOOLBAR pnmtb = (LPNMTOOLBAR)lParam;
    if (pnmtb->iItem == IDC_ENCODING_COMBO)
    {
      g_App.ShowEncodingMenu();
      return TBDDRET_DEFAULT;
    }
  }
  g_App.OnNotify((int)wParam, pnmh);
  break;
}
```

### FM.cpp — WM_COMMAND 加入 encoding menu

```cpp
// 在 WM_COMMAND case 開頭加入
if (wmId >= IDM_ENCODING_CHANGED && wmId < IDM_ENCODING_CHANGED + CEncodingSwitch::GetNumEncodings())
{
  g_App.OnEncodingSelected(wmId - IDM_ENCODING_CHANGED);
  return 0;
}
```

### resource.h — 加入 ID

```cpp
// 在 IDM_ABOUT 後加入
#define IDC_ENCODING_COMBO       970
#define IDM_ENCODING_CHANGED     971

// 在 IDB_INFO 後加入
#define IDB_ENCODING  107
#define IDB_ENCODING2 157
```

### resource.rc — 加入 BITMAP

```rc
IDB_ENCODING   BITMAP  "../../UI/FileManager/Encoding.bmp"
IDB_ENCODING2  BITMAP  "../../UI/FileManager/Encoding2.bmp"
```

### FM.mak — 加入 EncodingSwitch.obj

```makefile
FM_OBJS = \
  $O\App.obj \
  ...
  $O\EncodingSwitch.obj \   // <-- 加入這行
  $O\EnumFormatEtc.obj \
  ...
```

### Bundles/Fm/makefile — 加入 LWZipCodePage.obj

```makefile
// 在 !include "../../7zip.mak" 之前加入
LWZIP_OBJS = \
  $O\LWZipCodePage.obj \

CURRENT_OBJS = $(LWZIP_OBJS)

!include "../../7zip.mak"

$O\LWZipCodePage.obj: ../../LWZipCodePage.cpp
	$(COMPL_O2)
```

---

## 注意事項

1. **MSVC 編譯限制**
   - `-Wall -WX` 所有 warning 都是 error
   - 不可使用 `..` 相對路徑 include (C4464)
   - 成員初始化順序必須與宣告順序一致 (C5038)
   - `LWZipCodePage.h` 用 `typedef unsigned int UInt32` 避免 include 7zTypes.h

2. **GetRawProps 快速路徑**
   - ZIP handler 沒有實作 `IArchiveGetRawProps`，所以 ZIP 格式不受影響
   - 但其他格式 (WIM, NTFS) 有實作，需要跳過快速路徑才能讓 override 生效

3. **Proxy Cache**
   - `CProxyArc::Load()` 只在開啟壓縮檔時呼叫一次
   - 切換編碼必須 `CloseOpenFolders()` + `BindToPathAndRefresh()` 強制重新開啟
   - 單純 `RefreshAllPanels()` 不會重建 proxy

4. **ID 衝突**
   - `IDC_ENCODING_COMBO = 970`, `IDM_ENCODING_CHANGED = 971`
   - 如果新版 7-Zip 使用了這些 ID，需要調整數字
   - 編碼選單的 command ID 範圍是 `971` ~ `971 + 編碼數量`
