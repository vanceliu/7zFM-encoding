// EncodingSwitch.h
// LWZip: Encoding switch for archive filename display

#ifndef ZIP7_INC_ENCODING_SWITCH_H
#define ZIP7_INC_ENCODING_SWITCH_H

#include "../../../Common/MyWindows.h"
#include "../../../Common/MyString.h"

struct CEncodingInfo
{
  const wchar_t *Name;
  UInt32 CodePage;
};

class CEncodingSwitch
{
public:
  CEncodingSwitch(): _currentIndex(0) {}

  static const CEncodingInfo *GetEncodings();
  static unsigned GetNumEncodings();

  UInt32 GetCurrentCodePage() const;
  unsigned GetCurrentIndex() const { return _currentIndex; }
  void SetCurrentIndex(unsigned index);

  static UString DecodeFileName(const AString &rawName, UInt32 codePage);

private:
  unsigned _currentIndex;
};

extern CEncodingSwitch g_EncodingSwitch;

#endif
