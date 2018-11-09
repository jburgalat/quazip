#include "dialog.hpp"
#include "quazip/jlworker.hpp"
#include "times.hpp"
#include <QStyle>

/**
 * @brief Constructor.
 */
TestDialog::TestDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f) {
    mWorkerThread = Q_NULLPTR;
    mWorker = Q_NULLPTR;
    mFileCount = 0;
    mTabWidget = new QTabWidget(this);
    mCompressWidget = new CompressParamsWidget(this);
    mExtractWidget = new ExtractParamsWidget(this);
    mTabWidget->addTab(mCompressWidget, "Compression");
    mTabWidget->addTab(mExtractWidget, "Extraction");
    mGbxProgress = new QGroupBox("Progression", this);
    mGbxProgress->setCheckable(true);
    mProgressWidget = new ProgressWidget(mGbxProgress);
    mBtnProcess = new QPushButton(this);
    mBtnProcess->setText(tr("Process"));
    QVBoxLayout *vLayout = new QVBoxLayout(mGbxProgress);
    vLayout->addWidget(mProgressWidget);
    vLayout = new QVBoxLayout(this);
    vLayout->addWidget(mTabWidget);
    vLayout->addWidget(mGbxProgress, 0, Qt::AlignBottom);
    vLayout->addWidget(mBtnProcess, 0, Qt::AlignBottom);

    setupWorker();

    QObject::connect(mTabWidget, &QTabWidget::currentChanged, this, &TestDialog::onTabChanged);
    QObject::connect(mExtractWidget, &ExtractParamsWidget::parametersChanged, mBtnProcess, &QPushButton::setEnabled);
    QObject::connect(mCompressWidget, &CompressParamsWidget::parametersChanged, mBtnProcess, &QPushButton::setEnabled);
    QObject::connect(mGbxProgress, &QGroupBox::toggled, this, &TestDialog::enableProgression);
    QObject::connect(mBtnProcess, &QPushButton::clicked, this, &TestDialog::manageOperation);
    mGbxProgress->setChecked(true);
}

/// @brief Destructor.
TestDialog::~TestDialog() {
    if (mWorkerThread && mWorkerThread->isRunning()) {
        mWorker->cancel();
        mWorkerThread->wait();
    }
    if (mWorker)
        mWorker->deleteLater();
    if (mWorkerThread)
        mWorkerThread->deleteLater();
}

// Dialog size hint
QSize TestDialog::sizeHint() const { return QSize(1920, 1080); }

/**
 * @brief Warn the user if an operation is still in progress.
 * @param event Associated close event.
 */
void TestDialog::closeEvent(QCloseEvent *event) {
    if (mWorkerThread->isRunning()) {
        QMessageBox::StandardButton ret = QMessageBox::information(this, tr("Zip operation in progress"),
                                                                   tr("<p>An operation is still in progress</br>"
                                                                      "Operation will be canceled !</p>"
                                                                      "<p>Do you really want to quit ?</p>"),
                                                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            mWorker->cancel();
            mWorkerThread->wait();
        } else {
            event->ignore();
            return;
        }
    }
    QDialog::closeEvent(event);
}

/**
 * @brief Setup the worker.
 */
void TestDialog::setupWorker() {
    if (!mWorkerThread)
        mWorkerThread = new QThread;
    if (!mWorker)
        mWorker = new JlWorker;
    mWorker->enableProgression(true);
    mWorker->setGlobalProgressReport(1);
    mWorker->setFileProgressReport(5);
    mWorker->setAbortPercentCheck(5);

    mWorker->moveToThread(mWorkerThread);
    QObject::connect(mWorkerThread, &QThread::started, mWorker, &JlWorker::process);
    QObject::connect(mWorker, &JlWorker::finished, mWorkerThread, &QThread::quit);
    QObject::connect(mWorkerThread, &QThread::started, this, &TestDialog::onProcessStarted);
    QObject::connect(mWorkerThread, &QThread::finished, this, &TestDialog::onProcessFinished);
}

/**
 * @brief Enable/disable progression.
 * @param enabled @ti{true} to enable, @ti{false} otherwise;
 */
void TestDialog::enableProgression(bool enabled) {
    QObject::disconnect(mWorker, &JlWorker::maxOverallProgressChanged, this, &TestDialog::onMaxValueProgressChanged);
    QObject::disconnect(mWorker, &JlWorker::maxFilesProgressChanged, this, &TestDialog::onMaxFilesProgressChanged);
    QObject::disconnect(mWorker, &JlWorker::maxPerFileProgressChanged, this,
                        &TestDialog::onMaxValuePerFileProgressChanged);
    QObject::disconnect(mWorker, &JlWorker::overallProgressChanged, this, &TestDialog::onValueProgressChanged);
    QObject::disconnect(mWorker, &JlWorker::perFileProgressChanged, this,
                        &TestDialog::onValuePerFileProgressChanged);
    QObject::disconnect(mWorker, &JlWorker::filesProgressChanged, this, &TestDialog::onFilesProgressChanged);
    QObject::disconnect(mWorker, &JlWorker::fileChanged, this, &TestDialog::onFileChanged);
    if (enabled) {
        QObject::connect(mWorker, &JlWorker::maxOverallProgressChanged, this, &TestDialog::onMaxValueProgressChanged);
        QObject::connect(mWorker, &JlWorker::maxFilesProgressChanged, this, &TestDialog::onMaxFilesProgressChanged);
        QObject::connect(mWorker, &JlWorker::maxPerFileProgressChanged, this,
                         &TestDialog::onMaxValuePerFileProgressChanged);
        QObject::connect(mWorker, &JlWorker::overallProgressChanged, this, &TestDialog::onValueProgressChanged);
        QObject::connect(mWorker, &JlWorker::perFileProgressChanged, this,
                         &TestDialog::onValuePerFileProgressChanged);
        QObject::connect(mWorker, &JlWorker::filesProgressChanged, this, &TestDialog::onFilesProgressChanged);
        QObject::connect(mWorker, &JlWorker::fileChanged, this, &TestDialog::onFileChanged);
    }
}

/**
 * @brief Performs actions after the process has started.
 * @details
 * The method disables widgets related to user-interactions with exception of the progress widget.
 * the "process" button's text is updated and can be used to cancel the operation.
 */
void TestDialog::onProcessStarted() {
    mExtractWidget->setEnabled(false);
    mCompressWidget->setEnabled(false);
}

/**
 * @brief Performs actions after the process has finished.
 */
void TestDialog::onProcessFinished() {
    mExtractWidget->setEnabled(true);
    mCompressWidget->setEnabled(true);
    mBtnProcess->setText(tr("Process"));
    qint64 totalTime = mWorker->elapsedTime();
    qDebug() << "time elapsed..." << totalTime;
    QMessageBox::information(this, tr("Job done"),
                             tr("Job tooks %1").arg(msecsToTimeFormat(totalTime, "%m minutes and %s seconds")),
                             QMessageBox::Ok);
}

/// @brief Update process button enable state on tab widget changes.
void TestDialog::onTabChanged(int index) {
    mBtnProcess->setEnabled(index == 0 ? mCompressWidget->isValid() : mExtractWidget->isValid());
}

/// @brief Manage operation to perform before processing file(s).
void TestDialog::manageOperation() {
    // check state of the worker
    if (mWorkerThread->isRunning()) {
        // we should cancel the operation.
        mWorker->cancel();
    } else {
        if (mTabWidget->currentIndex() == 0) {
            // compress
            if (mCompressWidget->isDirectory()) {
                mWorker->setupCompression(mCompressWidget->outputFile(), mCompressWidget->inputDirectory());
            } else {
                mWorker->setupCompression(mCompressWidget->outputFile(), mCompressWidget->inputFiles());
            }
        } else {
            // extract
            mWorker->setupExtraction(mExtractWidget->inputFile(), mExtractWidget->filesToExtract(),
                                     mExtractWidget->outputDirectory());
        }
        mBtnProcess->setText(tr("Cancel"));
        mWorkerThread->start();
    }
}

/// @brief Update Overall progress bar range on max bytes changed.
void TestDialog::onMaxValueProgressChanged(int maxValue) {
    mProgressWidget->overallProgressBar()->setRange(0, maxValue);
}

/// @brief Update internal file counters on max files changed.
void TestDialog::onMaxFilesProgressChanged(int maxValue) {
    mFileCount = 0;
    mFileTotal = maxValue;
}

/// @brief Update "by file" progress bar range on max bytes changed.
void TestDialog::onMaxValuePerFileProgressChanged(int maxValue) {
    mProgressWidget->byFileProgressBar()->setRange(0, maxValue);
}

/// @brief Update overall progress bar value on current bytes written changed.
void TestDialog::onValueProgressChanged(int value) { mProgressWidget->overallProgressBar()->setValue(value); }

/// @brief Update "by file" progress bar value on current bytes written changed.
void TestDialog::onValuePerFileProgressChanged(int value) { mProgressWidget->byFileProgressBar()->setValue(value); }

/// @brief Update the  progress bar value on current bytes written changed.
void TestDialog::onFilesProgressChanged(int value) {
    mFileCount = value;
    setFileProgressLabelText();
}

/// @brief Update overall progress bar value on current bytes written changed.
void TestDialog::onFileChanged(const QString &name) {
    mCurrentFile = name;
    setFileProgressLabelText();
}

/// @brief Set new text for the file progress label.
void TestDialog::setFileProgressLabelText() {
    if (mCurrentFile.isEmpty())
        return;
    mProgressWidget->fileLabel()->setText(QString("%1 (%2/%3)").arg(mCurrentFile).arg(mFileCount).arg(mFileTotal));
}
