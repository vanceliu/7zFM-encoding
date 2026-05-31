#ifndef LWZIP_ARCHIVE_HANDLER_H
#define LWZIP_ARCHIVE_HANDLER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QProcess>

struct LWArchiveItem
{
    QString name;
    quint64 size = 0;
    quint64 packedSize = 0;
    QDateTime modified;
    QString crc;
    QString method;
    bool isDirectory = false;
    bool isEncrypted = false;
    QByteArray rawName;
};

class LWArchiveHandler : public QObject
{
    Q_OBJECT

public:
    explicit LWArchiveHandler(QObject *parent = nullptr);

    bool open(const QString &archivePath);
    void close();
    bool isOpen() const { return m_isOpen; }
    bool isEncrypted() const { return m_isEncrypted; }
    QString archivePath() const { return m_archivePath; }

    QList<LWArchiveItem> listItems(unsigned codePage = 65001) const;

    bool extract(const QString &outputDir, const QString &password = QString());
    bool extractFiles(const QStringList &files, const QString &outputDir, const QString &password = QString());

signals:
    void progressChanged(int percent, const QString &currentFile);
    void operationFinished(bool success, const QString &message);
    void passwordRequired();

private:
    void parseListOutput(const QString &output, unsigned codePage) const;
    QString run7z(const QStringList &args) const;

    bool m_isOpen = false;
    bool m_isEncrypted = false;
    QString m_archivePath;
    mutable QList<LWArchiveItem> m_items;
    mutable unsigned m_lastCodePage = 0;
};

#endif
