
#include <QMutexLocker>
#include <QElapsedTimer>
#include "jlworker.hpp"

/// @brief Default Constructor.
JlWorker::JlWorker(QObject *parent) : JlCompressObj(parent) {
    mOperation = None;
    mSuccess = true;
    mCancel = false;
    mExtractMode = false;
    mFilters = 0;
    mRecurse = false;
    mElapsed = 0;
}

/// @brief Cancel the job.
void JlWorker::cancel() {
    QMutexLocker locker(&mCancelMutex);
    mCancel = true;
}

/// @brief Check whether or not the job was canceled.
bool JlWorker::canceled() const {
    QMutexLocker locker(&mCancelMutex);
    return mCancel;
}

/// @brief Check whether or not the last job has failed.
bool JlWorker::failed() const { return !mSuccess; }

/**
 * @brief Setup the worker for extraction job.
 * @param compressedFile Path of the compressed file.
 * @param source Optional source file from the archive to extract.
 * @param dest Optional destination path on filesystem.
 * @details
 * If <i>source</i> is empty, the whole archive will be extracted during process.
 *
 * If <i>dest</i> is empty, the extraction will be performed in the current working directory, during process execution.
 */
void JlWorker::setupExtraction(const QString &compressedFile, const QString &source, const QString &dest) {
    QMutexLocker locker(&mDataMutex);
    mCompressedFile = compressedFile;
    mOperation = None;
    mExtractMode = true;
    mIDir = QString();
    mIFiles = QStringList();
    if (source.isEmpty()) {
        mOperation = SingleDirectory;
    } else {
        mIFiles << source;
        mOperation = SingleFile;
    }
    mDestination = dest;
}

/**
 * @brief Setup the worker for extraction job.
 * @param compressedFile Path of the compressed file.
 * @param files List of files to extract.
 * @param dest Optional destination path on filesystem.
 * @overload void JlWorker::setupExtraction(const QString &compressedFile, const QString &source, const QString &dest)
 *
 * If the list of <i>files</i> is empty, the whole archive will be extracted during process.
 */
void JlWorker::setupExtraction(const QString &compressedFile, const QStringList &files, const QString &dest) {
    QMutexLocker locker(&mDataMutex);
    mExtractMode = true;
    mCompressedFile = compressedFile;
    mIFiles = files;
    mIDir = QString();
    if (mIFiles.isEmpty()) {
        mOperation = SingleDirectory;
    } else
        mOperation = MultiFiles;
    mDestination = dest;
}

/**
 * @brief Setup the worker for compression job.
 * @param compressedFile Path of the compressed file.
 * @param filedir Either a directory or file path to compress.
 * @param recursive For directory, @ti{true} to compress the directory contents recursively.
 * @param filters For directory, what to pack, filters are applied both when searching
 * for subdirs (if packing recursively) and when looking for files to pack.
 *
 * If <i>filedir</i> is empty, the worker will compress the current working directory (at the time
 * the process starts).
 * Unless filters are specified explicitly, packs only regular non-hidden files (and subdirs, if @c recursive is true).
 * If filters are specified, they are OR-combined with <tt>%QDir::AllDirs|%QDir::NoDotAndDotDot</tt> when searching for
 * dirs and with <tt>QDir::Files</tt> when searching for files.
 */
void JlWorker::setupCompression(const QString &compressedFile, const QString &filedir, bool recursive,
                                QDir::Filters filters) {
    QMutexLocker locker(&mDataMutex);
    // reset previous internal data
    mCompressedFile = compressedFile;
    mOperation = SingleDirectory;
    mIDir = QString();
    mFilters = filters;
    mRecurse = recursive;
    mIFiles = QStringList();
    mDestination = QString();
    if (QFileInfo(filedir).isFile()) {
        mOperation = SingleFile;
        mIFiles << filedir;
    } else if (filedir.isEmpty() || QFileInfo(filedir).isDir()) {
        mIDir = filedir;
    } else {
        mOperation = None;
    }
}

/**
 * @brief Setup the worker for compression job.
 * @param compressedFile Path of the compressed file.
 * @param files List of files to compress.
 * @overload void JlWorker::setupCompression(const QString &compressedFile, QString filedir = QString(), bool recursive
 * = true, QDir::Filters filters = 0)
 */
void JlWorker::setupCompression(const QString &compressedFile, const QStringList &files) {
    QMutexLocker locker(&mDataMutex);
    mCompressedFile = compressedFile;
    mOperation = MultiFiles;
    mIDir = QString();
    mFilters = 0;
    mRecurse = false;
    mIFiles = QStringList();
    mDestination = QString();
    mIFiles = files;
    if (mIFiles.isEmpty())
        mOperation = None;
}

/**
 * @brief Get the list of the extracted files.
 */
QStringList JlWorker::extractedFiles() const {
    QMutexLocker locker(&mDataMutex);
    return mExtracted;
}

/**
 * @brief Enable/disable progress reporting signals.
 * @param enabled @ti{true} to enable, @ti{false} otherwise.
 * @details
 * The method always resets the internal counters.
 */
void JlWorker::enableProgression(bool enabled) {
    QMutexLocker locker(&mDataMutex);
    mReportProgress = enabled;
    mCurFiles = 0;
    mTotalFiles = 0;
    mCurBytes = 0;
    mTotalBytes = 0;
}

/**
 * @brief Set overvall progress report rate.
 * @param percent Percent value (between 1 and 100).
 * @details
 * Set the emission rate of JlCompressObj::valueProgressChanged signal in term of percent of the number of
 * (uncompressed) bytes written of the list of files to process.
 * @note
 * Defaults to 1%.
 */
void JlWorker::setGlobalProgressReport(int percent) {
    QMutexLocker locker(&mDataMutex);
    mTPReport = qBound(1, percent, 100);
}

/**
 * @brief Set current file progress report rate.
 * @param percent Percent value (between 1 and 100).
 * @details
 * Set the emission rate of JlCompressObj::valuePerFileProgressChanged signal in term of percent of the number of
 * (uncompressed) bytes written of the currently processed file.
 * @note
 * Defaults to 5%.
 */
void JlWorker::setFileProgressReport(int percent) {
    QMutexLocker locker(&mDataMutex);
    mFPReport = qBound(1, percent, 100);
}

/**
 * @brief Set the abort operation checking rate.
 * @param percent Percent value (between 1 and 100).
 * @details
 * Defines how frequently the cancellation process should be checked. It is expressed in terms of percent of the number
 * of (uncompressed) bytes written of the currently processed file.
 * @note
 * Defaults to 5%.
 * @note
 * Cancellation procdure is implemented during file copy.
 */
void JlWorker::setAbortPercentCheck(int percent) {
    QMutexLocker locker(&mDataMutex);
    mCPReport = qBound(1, percent, 100);
}

/// @brief Get the total time of the last operation.
qint64 JlWorker::elapsedTime() const { return mElapsed; }

/**
 * @brief Process the files for the stored operation.
 */
void JlWorker::process() {
    mCancelMutex.lock();
    mCancel = false;
    mCancelMutex.unlock();

    QMutexLocker locker(&mDataMutex);
    mElapsed = 0;
    mSuccess = true;
    mExtracted = QStringList();
    if (mCompressedFile.isEmpty() || mOperation == None) {
        emit finished();
        return;
    }
    QElapsedTimer timer;
    timer.start();

    if (mExtractMode) {

        switch (mOperation) {
        case SingleFile:
            mExtracted << extractFile(mCompressedFile, mIFiles.first(), mDestination);
            mSuccess = mExtracted.size();
            break;
        case MultiFiles:
            mExtracted << extractFiles(mCompressedFile, mIFiles, mDestination);
            mSuccess = mExtracted.size();
            break;
        case SingleDirectory:
            mExtracted << extractDir(mCompressedFile, mDestination);
            mSuccess = mExtracted.size();
            break;
        default:
            break;
        }
    } else {
        switch (mOperation) {
        case SingleFile:
            mSuccess = compressFile(mCompressedFile, mIFiles.first());
            break;
        case MultiFiles:
            mSuccess = compressFiles(mCompressedFile, mIFiles);
            break;
        case SingleDirectory:
            mSuccess = compressDir(mCompressedFile, mIDir, mRecurse, mFilters);
            break;
        default:
            break;
        }
    }
    mSuccess = mSuccess && !mCancel;
    mElapsed = timer.elapsed();
    emit finished();
}

/**
 * @brief Copy data from <i>inFile</i> to <i>outFile</i>
 * @param inFile Device to copy from.
 * @param outFile Device to copy to
 * @return @ti{true} on succes, @ti{false} otherwise.
 * @details
 * The method emits all progress signals if signals emission is enabled (see JlCompressObj::enableProgressSignals()):
 *   - JlWorker::maxValuePerFileProgressChanged is emitted at the beginning of the copy with the size of inFile
 *   - JlWorker::valuePerFileProgressChanged is emitted each JlWorker::mFPReport percent of the uncompressed file size
 *     written.
 *   - JlWorker::valueProgressChanged is emitted each JlWorker::mFPReport percent of the total uncompressed size
 *     written.
 *   - JlWorker::filesProgressChanged is emitted when the copy is done (regardless it has been successful).
 *
 * The method checks if the process should be aborted each JlWorker::mCPReport percent of the uncompressed file size
 * written.
 */
bool JlWorker::copyData(QIODevice &inFile, QIODevice &outFile) {
    qint64 sz = 0;
    qint64 fileBytes = 0;
    int fp, fpm1, op, opm1, cp, cpm1;
    if (mReportProgress) {
        QuaZipFile *zipFile = qobject_cast<QuaZipFile *>(&inFile);
        sz = zipFile ? zipFile->usize() : inFile.size();
        fp = cp = 0;
        fpm1 = mFPReport;
        cpm1 = mCPReport;
        op = mCurBytes * 100 / mTotalBytes;
        opm1 = op + mTPReport;
        emit maxPerFileProgressChanged(100);
    }
    bool ret = true;
    while (!inFile.atEnd()) {
        char buf[4096];
        qint64 readLen = inFile.read(buf, 4096);
        if (readLen <= 0) {
            ret = false;
            break;
        }
        if (outFile.write(buf, readLen) != readLen) {
            ret = false;
            break;
        }
        fileBytes += readLen;
        mCurBytes += readLen;
        cp = fileBytes * 100 / sz;

        if (mReportProgress) {
            fp = fileBytes * 100 / sz;
            op = mCurBytes * 100 / mTotalBytes;
            if (fp >= fpm1) {
                emit perFileProgressChanged(fp);
                fpm1 = fp + mFPReport;
            }
            if (op >= opm1) {
                emit overallProgressChanged(op);
                opm1 = op + mTPReport;
            }
        }
        if (cp >= cpm1) {
            // do we really need a mutex here (for threaded stuff)??
            mCancelMutex.lock();
            bool cancel = mCancel;
            mCancelMutex.unlock();
            if (cancel) {
                ret = false;
                break;
            }
            cpm1 = cp + mCPReport;
        }
    }
    if (mReportProgress) {
        emit perFileProgressChanged(100);
        emit overallProgressChanged(mCurBytes * 100 / mTotalBytes);
        ++mCurFiles;
        emit filesProgressChanged(qMin(mCurFiles, mTotalFiles));
    }
    return ret;
}
