// EncodingSwitch.cpp
// LWZip: Encoding switch for archive filename display

#include "StdAfx.h"
#include "EncodingSwitch.h"

#include "../../../Common/StringConvert.h"

CEncodingSwitch g_EncodingSwitch;

static const CEncodingInfo g_Encodings[] =
{
  { L"UTF-8",        65001 },
  { L"Shift-JIS",    932   },
  { L"Big5",         950   },
  { L"GBK",          936   },
  { L"EUC-KR",       949   },
  { L"ISO-8859-1",   28591 },
  { L"Windows-1252", 1252  },
  { L"EUC-JP",       20932 },
  { L"GB2312",       20936 },
  { L"KOI8-R",       20866 },
  { L"Windows-1251", 1251  },
  { L"System (ACP)", 0     },  // CP_ACP
  { L"OEM",          1     },  // CP_OEMCP
};

const CEncodingInfo *CEncodingSwitch::GetEncodings()
{
  return g_Encodings;
}

unsigned CEncodingSwitch::GetNumEncodings()
{
  return sizeof(g_Encodings) / sizeof(g_Encodings[0]);
}

UInt32 CEncodingSwitch::GetCurrentCodePage() const
{
  if (_currentIndex < GetNumEncodings())
    return g_Encodings[_currentIndex].CodePage;
  return 65001; // default UTF-8
}

void CEncodingSwitch::SetCurrentIndex(unsigned index)
{
  if (index < GetNumEncodings())
    _currentIndex = index;
}

UString CEncodingSwitch::DecodeFileName(const AString &rawName, UInt32 codePage)
{
  if (codePage == 65001)
  {
    UString result;
    ConvertUTF8ToUnicode(rawName, result);
    return result;
  }
  return MultiByteToUnicodeString(rawName, codePage);
}
