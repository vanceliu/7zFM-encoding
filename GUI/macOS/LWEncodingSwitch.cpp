#include "LWEncodingSwitch.h"

static QList<LWEncodingInfo> g_encodings = {
    { "UTF-8",        "UTF-8",                   65001 },
    { "Shift-JIS",    "Shift-JIS (Japanese)",    932   },
    { "Big5",         "Big5 (Traditional Chinese)", 950 },
    { "GBK",          "GBK (Simplified Chinese)", 936  },
    { "EUC-KR",       "EUC-KR (Korean)",         949   },
    { "ISO-8859-1",   "ISO-8859-1 (Latin-1)",    28591 },
    { "Windows-1252", "Windows-1252 (Western)",  1252  },
    { "EUC-JP",       "EUC-JP (Japanese)",       20932 },
    { "GB2312",       "GB2312 (Simplified Chinese)", 20936 },
    { "KOI8-R",       "KOI8-R (Russian)",        20866 },
    { "Windows-1251", "Windows-1251 (Cyrillic)", 1251  },
};

LWEncodingSwitch::LWEncodingSwitch()
    : m_currentIndex(0)
{
}

const QList<LWEncodingInfo> &LWEncodingSwitch::encodings()
{
    return g_encodings;
}

unsigned LWEncodingSwitch::currentCodePage() const
{
    if (m_currentIndex >= 0 && m_currentIndex < g_encodings.size())
        return g_encodings[m_currentIndex].codePage;
    return 65001;
}

void LWEncodingSwitch::setCurrentIndex(int index)
{
    if (index >= 0 && index < g_encodings.size())
        m_currentIndex = index;
}

QString LWEncodingSwitch::currentName() const
{
    if (m_currentIndex >= 0 && m_currentIndex < g_encodings.size())
        return g_encodings[m_currentIndex].name;
    return "UTF-8";
}
