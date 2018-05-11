#ifndef JLWORKER_HPP
#define JLWORKER_HPP

#include <QMutex>
#include <QTime>
#include "jlcompress_obj.hpp"

/*!
  @def ABORT_CHECK
  Defines the interval (expressed in file progress percent) for Cancel flag checking operations in
  the file copy loop.

  For instance, if ABORT_CHECK is set to 5, JlWorker::copyData will check for the cancel flag each 5% progress
  during the copy of the input file to the output file.
*/
#define ABORT_CHECK 5

class QUAZIP_EXPORT JlWorker : public JlCompressObj {
    Q_OBJECT
  public:
    JlWorker(QObject *parent = Q_NULLPTR);

    JlWorker(bool reportProgress, QObject *parent = Q_NULLPTR) : JlCompressObj(reportProgress, parent), mCPReport(5) {}

    JlWorker(bool reportProgress, int totalProgressReport, int fileProgressReport, int CancelCheck,
             QObject *parent = Q_NULLPTR)
        : JlCompressObj(reportProgress, totalProgressReport, fileProgressReport, parent),
          mCPReport(qBound(1, CancelCheck, 100)) {}

    bool canceled() const;
    bool failed() const;
    void setupExtraction(const QString &compressedFile, const QString &source = QString(),
                         const QString &dest = QString());
    void setupExtraction(const QString &compressedFile, const QStringList &files, const QString &dest = QString());
    void setupCompression(const QString &compressedFile, const QString &filedir = QString(), bool recursive = true,
                          QDir::Filters filters = 0);
    void setupCompression(const QString &compressedFile, const QStringList &files);
    QStringList extractedFiles() const;
    virtual void enableProgression(bool enabled) Q_DECL_OVERRIDE;
    virtual void setGlobalProgressReport(int percent) Q_DECL_OVERRIDE;
    virtual void setFileProgressReport(int percent) Q_DECL_OVERRIDE;
    virtual void setAbortPercentCheck(int percent);
    qint64 elapsedTime() const;
signals:
    void finished();

  public slots:
    void process();
    void cancel();

  private:
    // make some public methods of JlCompressObj private for the sake of simplicity when using the worker.
    using JlCompressObj::compressFile;
    using JlCompressObj::compressFiles;
    using JlCompressObj::compressDir;
    using JlCompressObj::extractFile;
    using JlCompressObj::extractFiles;
    using JlCompressObj::extractDir;

    // before we make intensive use of mutex let see if it works without it... it should
    virtual bool copyData(QIODevice &inFile, QIODevice &outFile) Q_DECL_OVERRIDE;
    enum Operation {
        None,
        SingleFile,
        MultiFiles,
        SingleDirectory,
    };
    int mCPReport;
    Operation mOperation;
    bool mExtractMode;
    QStringList mExtracted;
    QString mCompressedFile;
    QString mIDir;
    QDir::Filters mFilters;
    bool mRecurse;
    QStringList mIFiles;
    QString mDestination;
    bool mSuccess;
    bool mCancel;
    mutable QMutex mCancelMutex;
    mutable QMutex mDataMutex;
    qint64 mElapsed;
};

#endif // JLWORKER_HPP
