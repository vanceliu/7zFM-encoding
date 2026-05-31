#ifndef LWZIP_MAIN_WINDOW_H
#define LWZIP_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QTableView>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QStatusBar>
#include <QStandardItemModel>
#include <QProcess>
#include <QActionGroup>

#include "LWEncodingSwitch.h"
#include "LWArchiveHandler.h"

class LWMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LWMainWindow(QWidget *parent = nullptr);
    void openFile(const QString &path) { loadArchive(path); }

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
    void onEncodingSelected(QAction *action);
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
    QToolButton *m_encodingButton = nullptr;
    QMenu *m_encodingMenu = nullptr;
    QActionGroup *m_encodingGroup = nullptr;
    QLabel *m_statusLabel = nullptr;

    // Models
    QStandardItemModel *m_folderModel = nullptr;
    QStandardItemModel *m_fileModel = nullptr;

    // State
    LWEncodingSwitch m_encoding;
    LWArchiveHandler *m_archiveHandler = nullptr;
    QString m_archivePath;
    QString m_internalPath;
};

#endif
