#include <QDir>
#include <QMimeData>

#include "quazip/quazip.h"
#include "widgets.hpp"

/// @brief Constructor.
FilesListWidget::FilesListWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
    QStyle *style = QApplication::style();
    mLastDirectory = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
    mListWidget = new QListWidget(this);
    // setup toolbar and actions.
    mToolbar = new QToolBar(this);
    mChbRecurse = new QCheckBox(tr("Recursive search"), mToolbar);
    mChbRecurse->setToolTip(tr("Search for files recursively in directory."));
    mToolbar->addWidget(mChbRecurse);
    mToolbar->addSeparator();
    mActionAddFiles = mToolbar->addAction(style->standardIcon(QStyle::SP_FileIcon), tr("add Files"), this,
                                          &FilesListWidget::addFilesFromDialog);
    mActionAddFiles->setToolTip(tr("<p>Add selected files from dialog.</p>"));
    mActionAddDir = mToolbar->addAction(style->standardIcon(QStyle::SP_DirOpenIcon), tr("add Directory"), this,
                                        &FilesListWidget::addDirFromDialog);
    mActionAddDir->setToolTip(tr("<p>Add files of the selected directory from dialog.</p>"
                                 "<p>If <i>recursive search</i> is checked, add also files from sub-directories.</p>"));
    mToolbar->addSeparator();
    mActionClear = mToolbar->addAction(style->standardIcon(QStyle::SP_LineEditClearButton), tr("Clear list"), this,
                                       &FilesListWidget::clearFiles);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(mToolbar, 0, Qt::AlignTop);
    vLayout->addWidget(mListWidget);
    vLayout->setContentsMargins(0, 0, 0, 0);
}

/// @brief Destructor.
FilesListWidget::~FilesListWidget() {}

/// @brief List of current files.
QStringList FilesListWidget::listOfFiles() const {
    QStringList out;
    for (int i = 0; i < mListWidget->count(); i++)
        out << mListWidget->item(i)->text();
    return out;
}

/// @brief Get the number of files to compress.
int FilesListWidget::filesCount() const { return mListWidget->count(); }

/// @brief Performs action on drop.
void FilesListWidget::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    QList<QUrl> urls = mimeData->urls();
    QStringList currents = listOfFiles();
    QStringList news;
    for (QUrl url : urls) {
        QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            news << filesFromDirectory(path, mChbRecurse->isChecked());
        } else if (!currents.contains(path))
            news << path;
    }
    news.removeDuplicates();
    QMutableStringListIterator it(news);
    while (it.hasNext()) {
        if (currents.contains(it.next()))
            it.remove();
    }
    addFiles(news, false);
}

/// @brief Get the list of files (full)paths of the given directory.
QStringList FilesListWidget::filesFromDirectory(const QString &directory, bool recurse) {
    QDir dir(directory);
    QStringList out;
    out = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QMutableStringListIterator it(out);
    while (it.hasNext()) {
        QString &p = it.next();
        p = dir.absoluteFilePath(p);
    }
    if (recurse) {
        for (QString d : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
            out << filesFromDirectory(dir.absoluteFilePath(d), recurse);
    }
    return out;
}

/// @brief Add files selected from QFileDialog.
void FilesListWidget::addFilesFromDialog() {
    QStringList files =
        QFileDialog::getOpenFileNames(this, tr("Select one or more files to add"), QString(), "All files (*.*)");
    if (!files.isEmpty()) {
        addFiles(files);
    }
}

/// @brief Add files from a direcotry selected from QFileDialog.
void FilesListWidget::addDirFromDialog() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), mLastDirectory);
    if (!dir.isEmpty()) {
        mLastDirectory = dir;
        addFiles(filesFromDirectory(dir, mChbRecurse->isChecked()));
    }
}

/// @brief Clear all files from the list.
void FilesListWidget::clearFiles() {
    mListWidget->clear();
    emit listOfFilesChanged();
}

/// @brief Add list of file paths to the list widget
void FilesListWidget::addFiles(const QStringList &paths, bool checkDuplicates) {
    if (checkDuplicates) {
        QStringList currents = listOfFiles();
        for (const QString &p : paths) {
            if (QFileInfo(p).isFile() && !currents.contains(p))
                mListWidget->addItem(p);
        }
    } else {
        mListWidget->addItems(paths);
    }
    emit listOfFilesChanged();
}

/// @brief Performs action on drag move.
void FilesListWidget::dragMoveEvent(QDragMoveEvent *event) { event->accept(); }

/// @brief Check file type on drag enter.
void FilesListWidget::dragEnterEvent(QDragEnterEvent *event) { event->acceptProposedAction(); }

/// @brief Constructor.
DirectoryWidget::DirectoryWidget(QWidget *parent) : QWidget(parent) {
    mLastDirectory = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
    setAcceptDrops(true);
    QStyle *style = QApplication::style();
    mModel = new QFileSystemModel(this);
    mTreeView = new QTreeView(this);
    mTreeView->setModel(mModel);
    mTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QLabel *mLblInputDirectory = new QLabel(tr("Input directory"), this);
    mLdtInputDirectory = new QLineEdit(this);
    mLdtInputDirectory->setReadOnly(true);
    mBtnInputDirectory = new QToolButton(this);
    mBtnInputDirectory->setIcon(style->standardIcon(QStyle::SP_DirOpenIcon));

    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->addWidget(mLblInputDirectory, 0, 0, Qt::AlignTop);
    gLayout->addWidget(mLdtInputDirectory, 0, 1, Qt::AlignTop);
    gLayout->addWidget(mBtnInputDirectory, 0, 2, Qt::AlignTop);
    gLayout->addWidget(mTreeView, 1, 0, 1, 3);
    gLayout->setRowStretch(1, 3);
    gLayout->setContentsMargins(0, 0, 0, 0);

    QObject::connect(mLdtInputDirectory, &QLineEdit::textChanged, this, &DirectoryWidget::setDirectory);
    QObject::connect(mBtnInputDirectory, &QToolButton::clicked, this, &DirectoryWidget::getDirFromDialog);
}

/// @brief Destructor.
DirectoryWidget::~DirectoryWidget() {}

/// @brief Get a directory from file dialog.
void DirectoryWidget::getDirFromDialog() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select output directory"), mLastDirectory);
    if (!dir.isEmpty()) {
        mLastDirectory = dir;
        mLdtInputDirectory->setText(dir);
    }
}

/// @brief Get the currently set directory.
QString DirectoryWidget::directory() const { return mLdtInputDirectory->text(); }

/// @brief Set new directory path.
void DirectoryWidget::setDirectory(const QString &dirPath) {
    mTreeView->setRootIndex(mModel->setRootPath(dirPath));
    mTreeView->expandAll();
}

/// @brief Performs action on drop.
void DirectoryWidget::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.size() != 1)
        return;
    QString path = urls.first().toLocalFile();
    if (path.isEmpty() || !QFileInfo(path).isDir())
        return;
    mLdtInputDirectory->setText(path);
    emit directoryChanged();
}

/// @brief Performs action on drag move.
void DirectoryWidget::dragMoveEvent(QDragMoveEvent *event) { event->accept(); }

/// @brief Performs action on drag enter.
void DirectoryWidget::dragEnterEvent(QDragEnterEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.size() != 1)
        return;
    QString path = urls.first().toLocalFile();
    if (path.isEmpty() || !QFileInfo(path).isDir())
        return;
    event->acceptProposedAction();
}

/// @brief Constructor
CompressParamsWidget::CompressParamsWidget(QWidget *parent) : QWidget(parent) {
    mLastDirectory = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
    QStyle *style = QApplication::style();
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    mBtnGroup = new QButtonGroup(this);
    mBtnDirectory = new QRadioButton(tr("Directory"), this);
    mBtnFile = new QRadioButton(tr("Files"), this);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(mBtnDirectory);
    hLayout->addWidget(mBtnFile);
    hLayout->addStretch(1);
    vLayout->addLayout(hLayout, 0);

    mStackedWidget = new QStackedWidget(this);
    mFilesListWidget = new FilesListWidget(this);
    mDirectoryWidget = new DirectoryWidget(this);

    mBtnGroup->addButton(mBtnDirectory, mStackedWidget->addWidget(mDirectoryWidget));
    mBtnGroup->addButton(mBtnFile, mStackedWidget->addWidget(mFilesListWidget));

    vLayout->addWidget(mStackedWidget);

    QLabel *mLblOutputFile = new QLabel(tr("Output archive file"));
    mLdtOutputFile = new QLineEdit(this);
    mLdtOutputFile->setReadOnly(true);
    mBtnOutputFile = new QToolButton(this);
    mBtnOutputFile->setIcon(style->standardIcon(QStyle::SP_FileIcon));

    hLayout = new QHBoxLayout;
    hLayout->addWidget(mLblOutputFile, 0, Qt::AlignLeft);
    hLayout->addWidget(mLdtOutputFile);
    hLayout->addWidget(mBtnOutputFile, 0, Qt::AlignRight);
    vLayout->addLayout(hLayout);

    QObject::connect(mBtnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), mStackedWidget,
                     &QStackedWidget::setCurrentIndex);
    QObject::connect(mBtnOutputFile, &QToolButton::clicked, this, &CompressParamsWidget::setOutputFileName);

    QObject::connect(mFilesListWidget, &FilesListWidget::listOfFilesChanged, this,
                     [=]() { emit parametersChanged(isValid()); });
    QObject::connect(mDirectoryWidget, &DirectoryWidget::directoryChanged, this,
                     [=]() { emit parametersChanged(isValid()); });
    mBtnDirectory->setChecked(true);
}

/// @brief Destructor.
CompressParamsWidget::~CompressParamsWidget() {}

/// @brief Set new compressed file to extract.
void CompressParamsWidget::setOutputFileName() {
    QString file = QFileDialog::getSaveFileName(this, tr("Select archive file"), QString(),
                                                "Zip files (*.zip) ;; All files (*.*)");
    if (!file.isEmpty()) {
        mLastDirectory = QFileInfo(file).dir().absolutePath();
        if (!file.endsWith(".zip"))
            file.append(".zip");
        mLdtOutputFile->setText(file);
    }
    emit parametersChanged(isValid());
}

/// @brief Check if directory mode is currently selected.
bool CompressParamsWidget::isDirectory() const { return mBtnDirectory->isChecked(); }

/// @brief Get the current selected directory.
QString CompressParamsWidget::inputDirectory() const { return mDirectoryWidget->directory(); }

/// @brief Get the current selected list of files.
QStringList CompressParamsWidget::inputFiles() const { return mFilesListWidget->listOfFiles(); }

/// @brief Ge tthe currently set output compressed file.
QString CompressParamsWidget::outputFile() const { return mLdtOutputFile->text(); }

/// @brief Chek if (current) parameters are valid.
bool CompressParamsWidget::isValid() const {
    return !mLdtOutputFile->text().isEmpty() &&
           ((mBtnDirectory->isChecked()) ? !mDirectoryWidget->directory().isEmpty() : mFilesListWidget->filesCount());
}

/// @brief Constructor.
ExtractParamsWidget::ExtractParamsWidget(QWidget *parent) : QWidget(parent) {
    QStyle *style = QApplication::style();
    setAcceptDrops(true);
    mModel = new QuazipTreeModel(this);
    mTreeView = new QTreeView(this);
    mTreeView->setModel(mModel);
    mTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QLabel *mLblInputFile = new QLabel(tr("Compressed file"), this);
    mLdtInputFile = new QLineEdit(this);
    mLdtInputFile->setReadOnly(true);
    mBtnExtractInputFile = new QToolButton(this);
    mBtnExtractInputFile->setIcon(style->standardIcon(QStyle::SP_FileIcon));

    QLabel *mLblOutputDir = new QLabel(tr("Destination directory"), this);
    mLdtOutputDir = new QLineEdit(this);
    mLdtOutputDir->setReadOnly(true);
    mBtnExtractOutputDir = new QToolButton(this);
    mBtnExtractOutputDir->setIcon(style->standardIcon(QStyle::SP_DirOpenIcon));

    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->addWidget(mLblInputFile, 0, 0, Qt::AlignTop);
    gLayout->addWidget(mLdtInputFile, 0, 1, Qt::AlignTop);
    gLayout->addWidget(mBtnExtractInputFile, 0, 2, Qt::AlignTop);

    gLayout->addWidget(mTreeView, 1, 0, 1, 3);

    gLayout->addWidget(mLblOutputDir, 2, 0, Qt::AlignBottom);
    gLayout->addWidget(mLdtOutputDir, 2, 1, Qt::AlignBottom);
    gLayout->addWidget(mBtnExtractOutputDir, 2, 2, Qt::AlignBottom);

    gLayout->setRowStretch(1, 3);

    QObject::connect(mBtnExtractInputFile, &QToolButton::clicked, this, &ExtractParamsWidget::setExtractInputFile);
    QObject::connect(mLdtInputFile, &QLineEdit::textChanged, this, &ExtractParamsWidget::onInputfileChanged);
    QObject::connect(mBtnExtractOutputDir, &QToolButton::clicked, this, &ExtractParamsWidget::setExtractOutputDir);
    QObject::connect(mLdtOutputDir, &QLineEdit::textChanged, this, &ExtractParamsWidget::onOutputDirectoryChanged);
    mTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(mTreeView, &QTreeView::customContextMenuRequested, this,
                     &ExtractParamsWidget::onCustomContextMenuRequested);
}

/// @brief Destructor.
ExtractParamsWidget::~ExtractParamsWidget() {}

//// @brief Get the input compressed file to unzip.
QString ExtractParamsWidget::inputFile() const { return mLdtInputFile->text(); }

/// @brief Ge tthe list of files to extract.
QStringList ExtractParamsWidget::filesToExtract() const { return mModel->checkedPaths(); }

/// @brief Get the output directory or extraction (fallback to current directory).
QString ExtractParamsWidget::outputDirectory() const {
    return mLdtOutputDir->text().isEmpty() ? QDir().absolutePath() : mLdtOutputDir->text();
}

/// @brief Check whether or not displayed values are valid parameters.
bool ExtractParamsWidget::isValid() const { return !mLdtInputFile->text().isEmpty(); }

/// @brief Performs action on drop.
void ExtractParamsWidget::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    QList<QUrl> urls = mimeData->urls();
    QString path;
    if (urls.isEmpty() || (path = urls.first().toLocalFile()).isEmpty())
        return;
    mLdtInputFile->setText(path);
}

/// @brief Performs action on drag move.
void ExtractParamsWidget::dragMoveEvent(QDragMoveEvent *event) { event->accept(); }

/// @brief Check file type on drag enter.
void ExtractParamsWidget::dragEnterEvent(QDragEnterEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    QString path;
    if (urls.isEmpty() || (path = urls.first().toLocalFile()).isEmpty())
        return;
    QMimeDatabase db;
    if (db.mimeTypeForUrl(path).inherits("application/zip"))
        event->acceptProposedAction();
}

/// @brief Build and pop up context menu.
void ExtractParamsWidget::onCustomContextMenuRequested(const QPoint &point) {
    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction("Check All", mModel, &QuazipTreeModel::checkAllItems);
    menu->addAction("Uncheck All", mModel, &QuazipTreeModel::uncheckAllItems);
    menu->addSeparator();
    menu->addAction("Expand All", mTreeView, &QTreeView::expandAll);
    menu->addAction("Collapse All", mTreeView, &QTreeView::collapseAll);
    menu->addSeparator();
    menu->addAction("Display checked", this, &ExtractParamsWidget::displayCheckedList);
    menu->exec(mTreeView->mapToGlobal(point));
}

/// @brief Display list of checked paths.
void ExtractParamsWidget::displayCheckedList() {
    QStringList list = mModel->checkedPaths();
    for (QString p : list)
        qDebug() << p;
    qDebug() << "+++++++++++++++++++++++++++++++++++++";
}

/// @brief Set new compressed file to extract.
void ExtractParamsWidget::setExtractInputFile() {
    QString file = QFileDialog::getOpenFileName(this, tr("Select archive file"), QString(),
                                                "Zip files (*.zip) ;; All files (*.*)");
    if (!file.isEmpty())
        mLdtInputFile->setText(file);
    else
        emit parametersChanged(isValid());
}

/// @brief Update model on input file changed.
void ExtractParamsWidget::onInputfileChanged(const QString &value) {
    QuaZip zip(value);
    if (zip.open(QuaZip::mdUnzip)) {
        mModel->setZipInfos(zip.getFileInfoList64());
        zip.close();
    } else
        mModel->clear();
    QFileInfo vInfos(value);
    QString path;
    if (!(value.isEmpty() || (path = vInfos.dir().absolutePath()).isEmpty())) {
        mLdtOutputDir->setText(QString("%1/%2").arg(path).arg(vInfos.completeBaseName()));
    } else
        emit parametersChanged(isValid());
}

/// @brief Set new extraction directory destination.
void ExtractParamsWidget::setExtractOutputDir() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select output directory"));
    if (!dir.isEmpty())
        mLdtOutputDir->setText(dir);
    else
        emit parametersChanged(isValid());
}

/// @brief Update model on input file changed.
void ExtractParamsWidget::onOutputDirectoryChanged(const QString &value) {
    Q_UNUSED(value)
    emit parametersChanged(isValid());
}

/// @brief Constructor.
ProgressWidget::ProgressWidget(QWidget *parent) : QWidget(parent) {
    mLblCurrentFile = new QLabel(this);
    mPrbOverAll = new QProgressBar(this);
    mPrbByFile = new QProgressBar(this);
    mPrbOverAll->setAlignment(Qt::AlignHCenter);
    mPrbByFile->setAlignment(Qt::AlignHCenter);
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->addRow(new QLabel(tr("Current file:")), mLblCurrentFile);
    formLayout->addRow(new QLabel(tr("File progress:")), mPrbByFile);
    formLayout->addRow(new QLabel(tr("Overall progress:")), mPrbOverAll);
}

/// @brief Destructor.
ProgressWidget::~ProgressWidget() {}
