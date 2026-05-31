#ifndef LWZIP_MAIN_WINDOW_H
#define LWZIP_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QTableView>
#include <QSplitter>
#include <QToolBar>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QStatusBar>
#include <QStandardItemModel>
#include <QProcess>

#include "LWEncodingSwitch.h"

class LWMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LWMainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onOpenArchive();
    void onExtract();
    void onAdd();
    void onTest();
    void onCopy();
    void onMove();
    void onDelete();
    void onInfo();
    void onEncodingChanged(int index);
    void onNavigateUp();
    void onPathEntered();
    void onItemDoubleClicked(const QModelIndex &index);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void loadArchive(const QString &path);
    void refreshList();
    void navigateTo(const QString &internalPath);

    // UI
    QSplitter *m_splitter = nullptr;
    QTreeView *m_folderTree = nullptr;
    QTableView *m_fileTable = nullptr;
    QLineEdit *m_pathEdit = nullptr;
    QComboBox *m_encodingCombo = nullptr;
    QLabel *m_statusLabel = nullptr;

    // Models
    QStandardItemModel *m_folderModel = nullptr;
    QStandardItemModel *m_fileModel = nullptr;

    // State
    LWEncodingSwitch m_encoding;
    QString m_archivePath;
    QString m_internalPath;
    QProcess *m_7zProcess = nullptr;
};

#endif
