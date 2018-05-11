#ifndef JLCOMPRESS_OBJ_HPP
#define JLCOMPRESS_OBJ_HPP

/*
Copyright (C) 2010 Roberto Pompermaier
Copyright (C) 2005-2016 Sergey A. Tachenov

This file is part of QuaZIP.

QuaZIP is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

QuaZIP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QuaZIP.  If not, see <http://www.gnu.org/licenses/>.

See COPYING file for the full LGPL text.

Original ZIP package is copyrighted by Gilles Vollant and contributors,
see quazip/(un)zip.h files for details. Basically it's the zlib license.
*/

#include "quazip.h"
#include "quazipfile.h"
#include "quazipfileinfo.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

/// Utility class for typical operations.
/**
  This class contains a number of useful static functions to perform
  simple operations, such as mass ZIP packing or extraction.
  */

/**
 * @brief The JlCompressObj class
 * @details
 * This class is a QObject subclass that extends original (static) JLCompress class with the ability to report
 * progression of the compress/uncompress operations using signals.
 *
 * Reporting progression is done at two levels: globally and by file. For each level, the progress is expressed in terms
 * of percent of the number of bytes written ranging from 0 to 100.
 *
 * Additionally, it implements a counter that is updated and emitted each time a new file is processed. The name of the
 * latter is also emitted.
 *
 * Here is the list of the signals defined in the class (see their documentation for more details):
 *    - JlCompressObj::maxPerFileProgressChanged
 *    - JlCompressObj::maxOverallProgressChanged
 *    - JlCompressObj::maxFilesProgressChanged
 *    - JlCompressObj::overallProgressChanged
 *    - JlCompressObj::perFileProgressChanged
 *    - JlCompressObj::filesProgressChanged
 *    - JlCompressObj::fileChanged
 *
 * In addition to a few private methods used to compute size before process, the class defines three additional methods
 * that customize the reporting progression:
 *    - JlCompressObj::enableProgression. The method enables/disables the computations of progress. By default, the
 *      progress is not computed (see constructor).
 *    - JlCompressObj::setGlobalProgressReport. The method sets the rate of emission of
 *      JlCompressObj::overallProgressChanged, in term of percent of the overall progress.
 *    - JlCompressObj::setFileProgressReport. The method sets the rate of emission of
 *      JlCompressObj::perFileProgressChanged, in term of percent of the file progress.
 */
class QUAZIP_EXPORT JlCompressObj : public QObject {
    Q_OBJECT
  public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    JlCompressObj(QObject *parent = Q_NULLPTR) : QObject(parent), mReportProgress(false), mTPReport(1), mFPReport(5) {}

    /**
     * @brief Constructor
     * @param reportProgress @ti{true} to report progress, @ti{false} otherwise.
     * @param parent Parent object
     */
    JlCompressObj(bool reportProgress, QObject *parent = Q_NULLPTR)
        : QObject(parent), mReportProgress(reportProgress), mTPReport(1), mFPReport(5) {}

    /**
     * @brief Constructor
     * @param reportProgress @ti{true} to report progress, @ti{false} otherwise.
     * @param totalProgressReport Emission rate (in percent) of the progress over the total number of bytes to write.
     * @param fileProgressReport Emission rate (in percent) of the progress over the number of bytes to write for a
     * single file.
     * @param parent Parent object
     */
    JlCompressObj(bool reportProgress, int totalProgressReport, int fileProgressReport, QObject *parent = Q_NULLPTR)
        : QObject(parent), mReportProgress(reportProgress), mTPReport(qBound(1, totalProgressReport, 100)),
          mFPReport(qBound(1, fileProgressReport, 100)) {}

    virtual void setGlobalProgressReport(int percent);
    virtual void setFileProgressReport(int percent);
    virtual void enableProgression(bool enabled);

    /// Compress a single file.
    /**
      \param fileCompressed The name of the archive.
      \param file The file to compress.
      \return true if success, false otherwise.
      */
    bool compressFile(QString fileCompressed, QString file);
    /// Compress a list of files.
    /**
      \param fileCompressed The name of the archive.
      \param files The file list to compress.
      \return true if success, false otherwise.
      */
    bool compressFiles(QString fileCompressed, QStringList files);
    /// Compress a whole directory.
    /**
      Does not compress hidden files. See compressDir(QString, QString, bool, QDir::Filters).

      \param fileCompressed The name of the archive.
      \param dir The directory to compress.
      \param recursive Whether to pack the subdirectories as well, or
      just regular files.
      \return true if success, false otherwise.
      */
    bool compressDir(QString fileCompressed, QString dir = QString(), bool recursive = true);
    /**
     * @brief Compress a whole directory.
     *
     * Unless filters are specified explicitly, packs
     * only regular non-hidden files (and subdirs, if @c recursive is true).
     * If filters are specified, they are OR-combined with
     * <tt>%QDir::AllDirs|%QDir::NoDotAndDotDot</tt> when searching for dirs
     * and with <tt>QDir::Files</tt> when searching for files.
     *
     * @param fileCompressed path to the resulting archive
     * @param dir path to the directory being compressed
     * @param recursive if true, then the subdirectories are packed as well
     * @param filters what to pack, filters are applied both when searching
     * for subdirs (if packing recursively) and when looking for files to pack
     * @return true on success, false otherwise
     */
    bool compressDir(QString fileCompressed, QString dir, bool recursive, QDir::Filters filters);

    /// Extract a single file.
    /**
      \param fileCompressed The name of the archive.
      \param fileName The file to extract.
      \param fileDest The destination file, assumed to be identical to
      \a file if left empty.
      \return The list of the full paths of the files extracted, empty on failure.
      */
    QString extractFile(QString fileCompressed, QString fileName, QString fileDest = QString());
    /// Extract a list of files.
    /**
      \param fileCompressed The name of the archive.
      \param files The file list to extract.
      \param dir The directory to put the files to, the current
      directory if left empty.
      \return The list of the full paths of the files extracted, empty on failure.
      */
    QStringList extractFiles(QString fileCompressed, QStringList files, QString dir = QString());
    /// Extract a whole archive.
    /**
      \param fileCompressed The name of the archive.
      \param dir The directory to extract to, the current directory if
      left empty.
      \return The list of the full paths of the files extracted, empty on failure.
      */
    QStringList extractDir(QString fileCompressed, QString dir = QString());
    /// Get the file list.
    /**
      \return The list of the files in the archive, or, more precisely, the
      list of the entries, including both files and directories if they
      are present separately.
      */
    QStringList getFileList(QString fileCompressed);
    /// Extract a single file.
    /**
      \param ioDevice pointer to device with compressed data.
      \param fileName The file to extract.
      \param fileDest The destination file, assumed to be identical to
      \a file if left empty.
      \return The list of the full paths of the files extracted, empty on failure.
      */
    QString extractFile(QIODevice *ioDevice, QString fileName, QString fileDest = QString());
    /// Extract a list of files.
    /**
      \param ioDevice pointer to device with compressed data.
      \param files The file list to extract.
      \param dir The directory to put the files to, the current
      directory if left empty.
      \return The list of the full paths of the files extracted, empty on failure.
      */
    QStringList extractFiles(QIODevice *ioDevice, QStringList files, QString dir = QString());
    /// Extract a whole archive.
    /**
      \param ioDevice pointer to device with compressed data.
      \param dir The directory to extract to, the current directory if
      left empty.
      \return The list of the full paths of the files extracted, empty on failure.
      */
    QStringList extractDir(QIODevice *ioDevice, QString dir = QString());
    /// Get the file list.
    /**
      \return The list of the files in the archive, or, more precisely, the
      list of the entries, including both files and directories if they
      are present separately.
      */
    QStringList getFileList(QIODevice *ioDevice);

  protected:
    QStringList extractDir(QuaZip &zip, const QString &dir);
    QStringList getFileList(QuaZip *zip);
    QString extractFile(QuaZip &zip, QString fileName, QString fileDest);
    QStringList extractFiles(QuaZip &zip, const QStringList &files, const QString &dir);
    /// Compress a single file.
    /**
      \param zip Opened zip to compress the file to.
      \param fileName The full path to the source file.
      \param fileDest The full name of the file inside the archive.
      \return true if success, false otherwise.
      */
    bool compressFile(QuaZip *zip, QString fileName, QString fileDest);
    /// Compress a subdirectory.
    /**
      \param parentZip Opened zip containing the parent directory.
      \param dir The full path to the directory to pack.
      \param parentDir The full path to the directory corresponding to
      the root of the ZIP.
      \param recursive Whether to pack sub-directories as well or only
      files.
      \return true if success, false otherwise.
      */
    bool compressSubDir(QuaZip *parentZip, QString dir, QString parentDir, bool recursive, QDir::Filters filters);
    /// Extract a single file.
    /**
      \param zip The opened zip archive to extract from.
      \param fileName The full name of the file to extract.
      \param fileDest The full path to the destination file.
      \return true if success, false otherwise.
      */
    bool extractFile(QuaZip *zip, QString fileName, QString fileDest);
    /// Remove some files.
    /**
      \param listFile The list of files to remove.
      \return true if success, false otherwise.
      */
    bool removeFile(QStringList listFile);

    // added methods

    virtual bool copyData(QIODevice &inFile, QIODevice &outFile);
    void computeSizesInZip(QuaZip &zip, const QStringList paths = QStringList());
    qint64 countBytes(QStringList files, int *fileCount = Q_NULLPTR);
    qint64 countBytes(const QString &path, int *fileCount = Q_NULLPTR, bool recurse = true);

  protected:
    bool mReportProgress;
    qint64 mTotalBytes;
    qint64 mCurBytes;
    int mTotalFiles;
    int mCurFiles;
    int mTPReport;
    int mFPReport;

signals:

    /**
     * @brief Emitted whenever the maximum value of the "per file" progress changed.
     * @param maxValue Always 100.
     */
    void maxPerFileProgressChanged(int maxValue);
    /**
     * @brief Emitted whenever the maximum value of the "overall" progress changed.
     * @param maxValue Always 100.
     */
    void maxOverallProgressChanged(int maxValue);
    /**
     * @brief Emitted whenever the maximum number of files to process has changed.
     * @param maxValue New maximum number of files to process.
     */
    void maxFilesProgressChanged(int maxValue);
    /**
     * @brief Emitted whenever the overall progress has changed.
     * @param value Percent done of the overall operation.
     */
    void overallProgressChanged(int value);
    /**
     * @brief Emitted whenever the progress for the current file has changed.
     * @param value Percent done for the current file.
     */
    void perFileProgressChanged(int value);
    /**
     * @brief Emitted whenever the number of files written has changed.
     * @param value Current number of files written.
     */
    void filesProgressChanged(int value);
    /**
     * @brief Emitted whenever the current file has changed.
     * @param name Name of the current file being written.
     */
    void fileChanged(const QString &name);
};

#endif /* JLCOMPRESS_OBJ_HPP */
