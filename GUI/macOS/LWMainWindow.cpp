#include "LWMainWindow.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>

LWMainWindow::LWMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_archiveHandler(new LWArchiveHandler(this))
{
    setupUI();
    setupMenuBar();
    setupToolBar();

    setWindowTitle(tr("LWZip File Manager"));
    setMinimumSize(900, 600);
    setAcceptDrops(true);

    m_statusLabel = new QLabel(tr("Ready"), this);
    statusBar()->addWidget(m_statusLabel);

    connect(m_archiveHandler, &LWArchiveHandler::operationFinished,
            [this](bool success, const QString &msg) {
        m_statusLabel->setText(msg);
        if (!success)
            QMessageBox::critical(this, tr("Error"), msg);
    });
}

void LWMainWindow::setupUI()
{
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(2);

    // Path bar
    auto *pathBar = new QHBoxLayout();
    auto *upBtn = new QPushButton(tr(".."), this);
    upBtn->setFixedWidth(30);
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText(tr("Open an archive..."));
    pathBar->addWidget(upBtn);
    pathBar->addWidget(m_pathEdit);
    mainLayout->addLayout(pathBar);

    connect(upBtn, &QPushButton::clicked, this, &LWMainWindow::onNavigateUp);
    connect(m_pathEdit, &QLineEdit::returnPressed, this, &LWMainWindow::onPathEntered);

    // Splitter: folder tree + file table
    m_splitter = new QSplitter(Qt::Horizontal, this);

    m_folderTree = new QTreeView(this);
    m_folderModel = new QStandardItemModel(this);
    m_folderModel->setHorizontalHeaderLabels({tr("Folders")});
    m_folderTree->setModel(m_folderModel);
    m_folderTree->setMaximumWidth(220);
    m_splitter->addWidget(m_folderTree);

    m_fileTable = new QTableView(this);
    m_fileModel = new QStandardItemModel(this);
    m_fileModel->setHorizontalHeaderLabels({
        tr("Name"), tr("Size"), tr("Packed Size"), tr("Modified"), tr("CRC")
    });
    m_fileTable->setModel(m_fileModel);
    m_fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fileTable->horizontalHeader()->setStretchLastSection(true);
    m_fileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_fileTable->setSortingEnabled(true);
    m_splitter->addWidget(m_fileTable);

    m_splitter->setSizes({200, 700});
    mainLayout->addWidget(m_splitter);

    connect(m_fileTable, &QTableView::doubleClicked,
            this, &LWMainWindow::onItemDoubleClicked);

    setCentralWidget(central);
}

void LWMainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open Archive..."), QKeySequence::Open, this, &LWMainWindow::onOpenArchive);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), QKeySequence::Quit, this, &QWidget::close);

    auto *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, [this]() {
        m_fileTable->selectAll();
    });

    auto *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(tr("&Refresh"), QKeySequence::Refresh, this, &LWMainWindow::refreshList);

    auto *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), [this]() {
        QMessageBox::about(this, tr("About LWZip"),
            tr("LWZip File Manager\n"
               "Based on 7-Zip by Igor Pavlov\n"
               "Version 0.1.0"));
    });
}

void LWMainWindow::setupToolBar()
{
    auto *toolbar = addToolBar(tr("Archive"));
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setIconSize(QSize(32, 32));

    toolbar->addAction(tr("Add"), this, &LWMainWindow::onAdd);
    toolbar->addAction(tr("Extract"), this, &LWMainWindow::onExtract);
    toolbar->addAction(tr("Test"), this, &LWMainWindow::onTest);
    toolbar->addAction(tr("Copy"), this, &LWMainWindow::onCopy);
    toolbar->addAction(tr("Move"), this, &LWMainWindow::onMove);
    toolbar->addAction(tr("Delete"), this, &LWMainWindow::onDelete);
    toolbar->addAction(tr("Info"), this, &LWMainWindow::onInfo);

    // Encoding button with popup menu (like Bandizip's "字碼頁")
    m_encodingMenu = new QMenu(this);
    m_encodingGroup = new QActionGroup(this);
    m_encodingGroup->setExclusive(true);

    const auto &encodings = LWEncodingSwitch::encodings();
    for (int i = 0; i < encodings.size(); i++) {
        QAction *action = m_encodingMenu->addAction(encodings[i].name);
        action->setCheckable(true);
        action->setData(i);
        m_encodingGroup->addAction(action);
        if (i == m_encoding.currentIndex())
            action->setChecked(true);
    }

    m_encodingButton = new QToolButton(this);
    m_encodingButton->setText(tr("Encoding"));
    m_encodingButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_encodingButton->setPopupMode(QToolButton::InstantPopup);
    m_encodingButton->setMenu(m_encodingMenu);
    toolbar->addWidget(m_encodingButton);

    connect(m_encodingGroup, &QActionGroup::triggered,
            this, &LWMainWindow::onEncodingSelected);
}

void LWMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void LWMainWindow::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString path = urls.first().toLocalFile();
        if (!path.isEmpty())
            loadArchive(path);
    }
}

void LWMainWindow::onOpenArchive()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Archive"), QString(),
        tr("Archives (*.zip *.7z *.tar *.gz *.bz2 *.xz *.rar *.cab *.iso *.wim *.zst);;All Files (*)"));
    if (!path.isEmpty())
        loadArchive(path);
}

void LWMainWindow::onExtract()
{
    if (!m_archiveHandler->isOpen()) return;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Extract To"));
    if (dir.isEmpty()) return;

    QString password;
    if (m_archiveHandler->isEncrypted()) {
        bool ok;
        password = QInputDialog::getText(this, tr("Password"),
            tr("Enter password:"), QLineEdit::Password, QString(), &ok);
        if (!ok) return;
    }

    m_statusLabel->setText(tr("Extracting..."));
    m_archiveHandler->extract(dir, password);
}

void LWMainWindow::onAdd()
{
    // TODO: add files to archive
}

void LWMainWindow::onTest()
{
    if (!m_archiveHandler->isOpen()) return;
    m_statusLabel->setText(tr("Testing..."));
    // TODO: run 7z t command
}

void LWMainWindow::onCopy() {}
void LWMainWindow::onMove() {}
void LWMainWindow::onDelete() {}
void LWMainWindow::onInfo() {}

void LWMainWindow::onEncodingSelected(QAction *action)
{
    int index = action->data().toInt();
    m_encoding.setCurrentIndex(index);
    refreshList();
    m_statusLabel->setText(tr("Encoding: %1").arg(m_encoding.currentName()));
}

void LWMainWindow::onNavigateUp()
{
    if (m_internalPath.isEmpty()) return;
    int lastSlash = m_internalPath.lastIndexOf('/');
    m_internalPath = (lastSlash > 0) ? m_internalPath.left(lastSlash) : QString();
    refreshList();
}

void LWMainWindow::onPathEntered()
{
    QString path = m_pathEdit->text();
    if (QFileInfo::exists(path))
        loadArchive(path);
}

void LWMainWindow::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    QString name = m_fileModel->item(index.row(), 0)->text();

    // Check if it's a folder by looking in the folder model
    for (int r = 0; r < m_folderModel->rowCount(); r++) {
        if (m_folderModel->item(r, 0)->text() == name) {
            navigateTo(m_internalPath.isEmpty() ? name : m_internalPath + "/" + name);
            return;
        }
    }
}

void LWMainWindow::loadArchive(const QString &path)
{
    m_archivePath = path;
    m_internalPath.clear();
    m_pathEdit->setText(path);

    if (m_archiveHandler->open(path)) {
        setWindowTitle(tr("LWZip - %1").arg(QFileInfo(path).fileName()));
        m_statusLabel->setText(tr("Opened: %1").arg(path));
        refreshList();
    } else {
        m_statusLabel->setText(tr("Failed to open: %1").arg(path));
        QMessageBox::warning(this, tr("Error"),
            tr("Cannot open archive: %1").arg(path));
    }
}

void LWMainWindow::refreshList()
{
    m_fileModel->removeRows(0, m_fileModel->rowCount());
    m_folderModel->removeRows(0, m_folderModel->rowCount());

    if (!m_archiveHandler->isOpen())
        return;

    unsigned codePage = m_encoding.currentCodePage();
    QList<LWArchiveItem> items = m_archiveHandler->listItems(codePage);

    for (const auto &item : items) {
        // Filter by current internal path
        if (!m_internalPath.isEmpty()) {
            if (!item.name.startsWith(m_internalPath + "/"))
                continue;
        }

        // Get relative name
        QString displayName = item.name;
        if (!m_internalPath.isEmpty())
            displayName = item.name.mid(m_internalPath.length() + 1);

        // Skip sub-directory items (only show direct children)
        if (displayName.contains('/')) {
            // Add folder to tree if it's a direct child folder
            QString folderName = displayName.left(displayName.indexOf('/'));
            bool found = false;
            for (int r = 0; r < m_folderModel->rowCount(); r++) {
                if (m_folderModel->item(r, 0)->text() == folderName) {
                    found = true;
                    break;
                }
            }
            if (!found)
                m_folderModel->appendRow(new QStandardItem(folderName));
            continue;
        }

        QList<QStandardItem *> row;
        row << new QStandardItem(displayName);
        row << new QStandardItem(item.isDirectory ? QString() : QString::number(item.size));
        row << new QStandardItem(item.packedSize > 0 ? QString::number(item.packedSize) : QString());
        row << new QStandardItem(item.modified.isValid() ? item.modified.toString("yyyy-MM-dd HH:mm:ss") : QString());
        row << new QStandardItem(item.crc);
        m_fileModel->appendRow(row);

        if (item.isDirectory)
            m_folderModel->appendRow(new QStandardItem(displayName));
    }

    m_statusLabel->setText(tr("%1 items").arg(m_fileModel->rowCount()));
}

void LWMainWindow::navigateTo(const QString &internalPath)
{
    m_internalPath = internalPath;
    refreshList();
}
