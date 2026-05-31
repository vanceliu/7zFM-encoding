#include "LWArchiveHandler.h"
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>
#include <QStringDecoder>
#include <QCoreApplication>

LWArchiveHandler::LWArchiveHandler(QObject *parent)
    : QObject(parent)
{
}

bool LWArchiveHandler::open(const QString &archivePath)
{
    close();

    if (!QFileInfo::exists(archivePath))
        return false;

    m_archivePath = archivePath;

    // Test if archive can be opened
    QString output = run7z({"l", "-slt", archivePath});
    if (output.isEmpty())
        return false;

    m_isOpen = true;
    m_isEncrypted = output.contains("Encrypted = +");
    m_lastCodePage = 0; // force re-parse on next listItems()

    return true;
}

void LWArchiveHandler::close()
{
    m_isOpen = false;
    m_isEncrypted = false;
    m_archivePath.clear();
    m_items.clear();
    m_lastCodePage = 0;
}

QList<LWArchiveItem> LWArchiveHandler::listItems(unsigned codePage) const
{
    if (!m_isOpen)
        return {};

    if (m_lastCodePage == codePage && !m_items.isEmpty())
        return m_items;

    // Use 7z l -slt for detailed listing
    QStringList args = {"l", "-slt"};
    if (codePage != 65001 && codePage != 0)
        args << QString("-mcp=%1").arg(codePage);
    args << m_archivePath;

    QString output = run7z(args);
    if (output.isEmpty())
        return m_items;

    parseListOutput(output, codePage);
    m_lastCodePage = codePage;
    return m_items;
}

void LWArchiveHandler::parseListOutput(const QString &output, unsigned codePage) const
{
    Q_UNUSED(codePage)
    m_items.clear();

    // Parse 7z -slt output format:
    // ----------
    // Path = filename
    // Size = 1234
    // Packed Size = 567
    // Modified = 2024-01-01 12:00:00
    // CRC = ABCD1234
    // Method = LZMA2
    // Encrypted = -
    // Folder = -

    LWArchiveItem currentItem;
    bool inItem = false;

    const auto lines = output.split('\n');
    for (const QString &line : lines)
    {
        if (line.startsWith("----------"))
        {
            inItem = true;
            continue;
        }

        if (!inItem)
            continue;

        if (line.trimmed().isEmpty())
        {
            if (!currentItem.name.isEmpty())
            {
                m_items.append(currentItem);
                currentItem = LWArchiveItem();
            }
            continue;
        }

        int eqPos = line.indexOf(" = ");
        if (eqPos < 0)
            continue;

        QString key = line.left(eqPos).trimmed();
        QString value = line.mid(eqPos + 3).trimmed();

        if (key == "Path")
            currentItem.name = value;
        else if (key == "Size")
            currentItem.size = value.toULongLong();
        else if (key == "Packed Size")
            currentItem.packedSize = value.toULongLong();
        else if (key == "Modified")
            currentItem.modified = QDateTime::fromString(value, "yyyy-MM-dd HH:mm:ss");
        else if (key == "CRC")
            currentItem.crc = value;
        else if (key == "Method")
            currentItem.method = value;
        else if (key == "Encrypted")
            currentItem.isEncrypted = (value == "+");
        else if (key == "Folder")
            currentItem.isDirectory = (value == "+");
    }

    // Don't forget the last item
    if (!currentItem.name.isEmpty())
        m_items.append(currentItem);
}

bool LWArchiveHandler::extract(const QString &outputDir, const QString &password)
{
    if (!m_isOpen)
        return false;

    QStringList args = {"x", "-y"};
    args << QString("-o%1").arg(outputDir);
    if (!password.isEmpty())
        args << QString("-p%1").arg(password);
    args << m_archivePath;

    QString output = run7z(args);
    bool success = output.contains("Everything is Ok");
    emit operationFinished(success, success ? tr("Extraction complete.") : tr("Extraction failed."));
    return success;
}

bool LWArchiveHandler::extractFiles(const QStringList &files, const QString &outputDir, const QString &password)
{
    if (!m_isOpen)
        return false;

    QStringList args = {"x", "-y"};
    args << QString("-o%1").arg(outputDir);
    if (!password.isEmpty())
        args << QString("-p%1").arg(password);
    args << m_archivePath;
    args << files;

    QString output = run7z(args);
    bool success = output.contains("Everything is Ok");
    emit operationFinished(success, success ? tr("Extraction complete.") : tr("Extraction failed."));
    return success;
}

QString LWArchiveHandler::run7z(const QStringList &args) const
{
    QProcess process;

    // Look for 7z binary: bundled first, then system
    QString binary = QCoreApplication::applicationDirPath() + "/7z";
    if (!QFileInfo::exists(binary))
        binary = "/usr/local/bin/7z";
    if (!QFileInfo::exists(binary))
        binary = "/opt/homebrew/bin/7z";
    if (!QFileInfo::exists(binary))
        binary = "7z"; // hope it's in PATH

    process.start(binary, args);
    process.waitForFinished(30000);

    if (process.exitCode() != 0 && process.exitCode() != 1)
        return QString();

    return QString::fromUtf8(process.readAllStandardOutput());
}
