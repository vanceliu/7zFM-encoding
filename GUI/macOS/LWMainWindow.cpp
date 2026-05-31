#include "LWMainWindow.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
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
{
    setupUI();
    setupMenuBar();
    setupToolBar();

    setWindowTitle(tr("LWZip File Manager"));
    setMinimumSize(900, 600);
    setAcceptDrops(true);

    m_statusLabel = new QLabel(tr("Ready"), this);
    statusBar()->addWidget(m_statusLabel);
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

    toolbar->addAction(tr("Add"), this, &LWMainWindow::onAdd);
    toolbar->addAction(tr("Extract"), this, &LWMainWindow::onExtract);
    toolbar->addAction(tr("Test"), this, &LWMainWindow::onTest);
    toolbar->addSeparator();
    toolbar->addAction(tr("Copy"), this, &LWMainWindow::onCopy);
    toolbar->addAction(tr("Move"), this, &LWMainWindow::onMove);
    toolbar->addAction(tr("Delete"), this, &LWMainWindow::onDelete);
    toolbar->addAction(tr("Info"), this, &LWMainWindow::onInfo);

    // Encoding switch - right side of toolbar
    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    auto *encLabel = new QLabel(tr("Encoding:"), this);
    toolbar->addWidget(encLabel);

    m_encodingCombo = new QComboBox(this);
    m_encodingCombo->setMinimumWidth(130);
    for (const auto &enc : LWEncodingSwitch::encodings())
        m_encodingCombo->addItem(enc.displayName);
    m_encodingCombo->setCurrentIndex(m_encoding.currentIndex());
    toolbar->addWidget(m_encodingCombo);

    connect(m_encodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LWMainWindow::onEncodingChanged);
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
    if (m_archivePath.isEmpty()) return;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Extract To"));
    if (dir.isEmpty()) return;

    // TODO: call 7zip core extract with current encoding
    m_statusLabel->setText(tr("Extracting..."));
}

void LWMainWindow::onAdd()
{
    // TODO: add files to archive
}

void LWMainWindow::onTest()
{
    if (m_archivePath.isEmpty()) return;
    // TODO: test archive integrity
    m_statusLabel->setText(tr("Testing..."));
}

void LWMainWindow::onCopy() {}
void LWMainWindow::onMove() {}
void LWMainWindow::onDelete() {}
void LWMainWindow::onInfo() {}

void LWMainWindow::onEncodingChanged(int index)
{
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
    // TODO: navigate into folder or open file
}

void LWMainWindow::loadArchive(const QString &path)
{
    m_archivePath = path;
    m_internalPath.clear();
    m_pathEdit->setText(path);
    setWindowTitle(tr("LWZip - %1").arg(QFileInfo(path).fileName()));
    m_statusLabel->setText(tr("Opened: %1").arg(path));
    refreshList();
}

void LWMainWindow::refreshList()
{
    m_fileModel->removeRows(0, m_fileModel->rowCount());
    m_folderModel->removeRows(0, m_folderModel->rowCount());

    // TODO: use 7zip core to list archive contents
    // Apply m_encoding.currentCodePage() for filename decoding
}

void LWMainWindow::navigateTo(const QString &internalPath)
{
    m_internalPath = internalPath;
    refreshList();
}
