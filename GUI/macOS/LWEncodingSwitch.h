// EncodingSwitch.h (shared between Windows and macOS)
// LWZip: Encoding switch for archive filename display

#ifndef LWZIP_ENCODING_SWITCH_SHARED_H
#define LWZIP_ENCODING_SWITCH_SHARED_H

#include <QString>
#include <QList>

struct LWEncodingInfo
{
    QString name;
    QString displayName;
    unsigned codePage;
};

class LWEncodingSwitch
{
public:
    LWEncodingSwitch();

    static const QList<LWEncodingInfo> &encodings();
    unsigned currentCodePage() const;
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);
    QString currentName() const;

private:
    int m_currentIndex;
};

#endif
