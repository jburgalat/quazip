/*
Copyright (C) 2010 Roberto Pompermaier
Copyright (C) 2005-2014 Sergey A. Tachenov

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

#include "jlcompress_obj.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>

/**
 * @brief Copy data from <i>inFile</i> to <i>outFile</i>
 * @param inFile Device to copy from.
 * @param outFile Device to copy to
 * @return @ti{true} on succes, @ti{false} otherwise.
 * @details
 * The method emits all progress signals if signals emission is enabled (see JlCompressObj::enableProgressSignals()):
 *   - JlCompressObj::maxValuePerFileProgressChanged is emitted at the beginning of the copy with the size of inFile
 *   - JlCompressObj::valuePerFileProgressChanged is emitted each JlCompressObj::mFPReport percent of the uncompressed
 * file size written.
 *   - JlCompressObj::valueProgressChanged is emitted each JlCompressObj::mFPReport percent of the total uncompressed
 * size written.
 *   - JlCompressObj::filesProgressChanged is emitted when the copy is done (regardless it has been successful).
 */
bool JlCompressObj::copyData(QIODevice &inFile, QIODevice &outFile) {
    qint64 sz = 0;
    qint64 fileBytes = 0;
    int fp, fpm1, op, opm1;
    if (mReportProgress) {
        QuaZipFile *zipFile = qobject_cast<QuaZipFile *>(&inFile);
        sz = zipFile ? zipFile->usize() : inFile.size();
        fp = 0;
        fpm1 = mFPReport;
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
    }
    if (mReportProgress) {
        emit perFileProgressChanged(100);
        emit overallProgressChanged(mCurBytes * 100 / mTotalBytes);
        emit filesProgressChanged(++mCurFiles);
    }
    return ret;
}

bool JlCompressObj::compressFile(QuaZip *zip, QString fileName, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // fileName: nome del file reale
    // fileDest: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip)
        return false;
    if (zip->getMode() != QuaZip::mdCreate && zip->getMode() != QuaZip::mdAppend && zip->getMode() != QuaZip::mdAdd)
        return false;

    // Apro il file originale
    QFile inFile;
    inFile.setFileName(fileName);
    if (!inFile.open(QIODevice::ReadOnly))
        return false;

    // Apro il file risulato
    QuaZipFile outFile(zip);
    if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest, inFile.fileName())))
        return false;

    // PATCH
    if (mReportProgress)
        emit fileChanged(fileName);
    // PATCH END

    // Copio i dati
    if (!copyData(inFile, outFile) || outFile.getZipError() != UNZ_OK) {
        return false;
    }

    // Chiudo i file
    outFile.close();
    if (outFile.getZipError() != UNZ_OK)
        return false;
    inFile.close();

    return true;
}

bool JlCompressObj::compressSubDir(QuaZip *zip, QString dir, QString origDir, bool recursive, QDir::Filters filters) {
    // zip: oggetto dove aggiungere il file
    // dir: cartella reale corrente
    // origDir: cartella reale originale
    // (path(dir)-path(origDir)) = path interno all'oggetto zip

    // Controllo l'apertura dello zip
    if (!zip)
        return false;
    if (zip->getMode() != QuaZip::mdCreate && zip->getMode() != QuaZip::mdAppend && zip->getMode() != QuaZip::mdAdd)
        return false;

    // Controllo la cartella
    QDir directory(dir);
    if (!directory.exists())
        return false;

    QDir origDirectory(origDir);
    if (dir != origDir) {
        QuaZipFile dirZipFile(zip);
        if (!dirZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(origDirectory.relativeFilePath(dir) + "/", dir), 0, 0,
                             0)) {
            return false;
        }
        dirZipFile.close();
    }

    // Se comprimo anche le sotto cartelle
    if (recursive) {
        // Per ogni sotto cartella
        QFileInfoList files = directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | filters);
        Q_FOREACH (QFileInfo file, files) {
            // Comprimo la sotto cartella
            if (!compressSubDir(zip, file.absoluteFilePath(), origDir, recursive, filters))
                return false;
        }
    }

    // Per ogni file nella cartella
    QFileInfoList files = directory.entryInfoList(QDir::Files | filters);
    Q_FOREACH (QFileInfo file, files) {
        // Se non e un file o e il file compresso che sto creando
        if (!file.isFile() || file.absoluteFilePath() == zip->getZipName())
            continue;

        // Creo il nome relativo da usare all'interno del file compresso
        QString filename = origDirectory.relativeFilePath(file.absoluteFilePath());

        // Comprimo il file
        if (!compressFile(zip, file.absoluteFilePath(), filename))
            return false;
    }

    return true;
}

bool JlCompressObj::extractFile(QuaZip *zip, QString fileName, QString fileDest) {
    // zip: oggetto dove aggiungere il file
    // filename: nome del file reale
    // fileincompress: nome del file all'interno del file compresso

    // Controllo l'apertura dello zip
    if (!zip)
        return false;
    if (zip->getMode() != QuaZip::mdUnzip)
        return false;

    // Apro il file compresso
    if (!fileName.isEmpty())
        zip->setCurrentFile(fileName);
    QuaZipFile inFile(zip);
    if (!inFile.open(QIODevice::ReadOnly) || inFile.getZipError() != UNZ_OK)
        return false;

    // Controllo esistenza cartella file risultato
    QDir curDir;
    if (fileDest.endsWith('/')) {
        if (!curDir.mkpath(fileDest)) {
            return false;
        }
    } else {
        if (!curDir.mkpath(QFileInfo(fileDest).absolutePath())) {
            return false;
        }
    }

    QuaZipFileInfo64 info;
    if (!zip->getCurrentFileInfo(&info))
        return false;

    QFile::Permissions srcPerm = info.getPermissions();
    if (fileDest.endsWith('/') && QFileInfo(fileDest).isDir()) {
        if (srcPerm != 0) {
            QFile(fileDest).setPermissions(srcPerm);
        }
        return true;
    }

    // Apro il file risultato
    QFile outFile;
    outFile.setFileName(fileDest);
    if (!outFile.open(QIODevice::WriteOnly))
        return false;

    // PATCH
    if (mReportProgress)
        emit fileChanged(fileDest);
    // PATCH END
    // Copio i dati
    if (!copyData(inFile, outFile) || inFile.getZipError() != UNZ_OK) {
        outFile.close();
        removeFile(QStringList(fileDest));
        return false;
    }
    outFile.close();

    // Chiudo i file
    inFile.close();
    if (inFile.getZipError() != UNZ_OK) {
        removeFile(QStringList(fileDest));
        return false;
    }

    if (srcPerm != 0) {
        outFile.setPermissions(srcPerm);
    }
    return true;
}

bool JlCompressObj::removeFile(QStringList listFile) {
    bool ret = true;
    // Per ogni file
    for (int i = 0; i < listFile.count(); i++) {
        // Lo elimino
        ret = ret && QFile::remove(listFile.at(i));
    }
    return ret;
}

bool JlCompressObj::compressFile(QString fileCompressed, QString file) {
    // Creo lo zip
    QuaZip zip(fileCompressed);
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if (!zip.open(QuaZip::mdCreate)) {
        QFile::remove(fileCompressed);
        return false;
    }
    // reset counters
    if (mReportProgress) {
        mCurFiles = 0;
        mCurBytes = 0;
        mTotalBytes = countBytes(file, &mTotalFiles, false);
        emit maxOverallProgressChanged(100);
        emit maxFilesProgressChanged(mTotalFiles);
    }

    // Aggiungo il file
    if (!compressFile(&zip, file, QFileInfo(file).fileName())) {
        QFile::remove(fileCompressed);
        return false;
    }

    // Chiudo il file zip
    zip.close();
    if (zip.getZipError() != 0) {
        QFile::remove(fileCompressed);
        return false;
    }

    return true;
}

bool JlCompressObj::compressFiles(QString fileCompressed, QStringList files) {
    // Creo lo zip
    QuaZip zip(fileCompressed);
    // PATCH
    if (mReportProgress) {
        mCurFiles = 0;
        mCurBytes = 0;
        mTotalBytes = countBytes(files, &mTotalFiles);
        emit maxOverallProgressChanged(100);
        emit maxFilesProgressChanged(mTotalFiles);
    }
    // PATCH END
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if (!zip.open(QuaZip::mdCreate)) {
        QFile::remove(fileCompressed);
        return false;
    }

    // Comprimo i file
    QFileInfo info;
    Q_FOREACH (QString file, files) {
        info.setFile(file);
        if (!info.exists() || !compressFile(&zip, file, info.fileName())) {
            QFile::remove(fileCompressed);
            return false;
        }
    }

    // Chiudo il file zip
    zip.close();
    if (zip.getZipError() != 0) {
        QFile::remove(fileCompressed);
        return false;
    }

    return true;
}

bool JlCompressObj::compressDir(QString fileCompressed, QString dir, bool recursive) {
    return compressDir(fileCompressed, dir, recursive, 0);
}

bool JlCompressObj::compressDir(QString fileCompressed, QString dir, bool recursive, QDir::Filters filters) {
    // PATCH
    if (mReportProgress) {
        mCurFiles = 0;
        mCurBytes = 0;
        mTotalBytes = countBytes(dir, &mTotalFiles, recursive);
        emit maxOverallProgressChanged(100);
        emit maxFilesProgressChanged(mTotalFiles);
    }
    // PATCH END
    // Creo lo zip
    QuaZip zip(fileCompressed);
    QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
    if (!zip.open(QuaZip::mdCreate)) {
        QFile::remove(fileCompressed);
        return false;
    }

    // Aggiungo i file e le sotto cartelle
    if (!compressSubDir(&zip, dir, dir, recursive, filters)) {
        QFile::remove(fileCompressed);
        return false;
    }

    // Chiudo il file zip
    zip.close();
    if (zip.getZipError() != 0) {
        QFile::remove(fileCompressed);
        return false;
    }

    return true;
}

QString JlCompressObj::extractFile(QString fileCompressed, QString fileName, QString fileDest) {
    // Apro lo zip
    QuaZip zip(fileCompressed);
    return extractFile(zip, fileName, fileDest);
}

QString JlCompressObj::extractFile(QuaZip &zip, QString fileName, QString fileDest) {
    if (!zip.open(QuaZip::mdUnzip)) {
        return QString();
    }

    // Estraggo il file
    if (fileDest.isEmpty())
        fileDest = fileName;

    // PATCH
    if (mReportProgress) {
        computeSizesInZip(zip, QStringList() << fileName);
        emit maxOverallProgressChanged(100);
        emit maxFilesProgressChanged(mTotalFiles);
    }
    // END PATCH

    if (!extractFile(&zip, fileName, fileDest)) {
        return QString();
    }

    // Chiudo il file zip
    zip.close();
    if (zip.getZipError() != 0) {
        removeFile(QStringList(fileDest));
        return QString();
    }
    return QFileInfo(fileDest).absoluteFilePath();
}

QStringList JlCompressObj::extractFiles(QString fileCompressed, QStringList files, QString dir) {
    // Creo lo zip
    QuaZip zip(fileCompressed);
    return extractFiles(zip, files, dir);
}

QStringList JlCompressObj::extractFiles(QuaZip &zip, const QStringList &files, const QString &dir) {
    if (!zip.open(QuaZip::mdUnzip)) {
        return QStringList();
    }

    // PATCH
    if (mReportProgress) {
        computeSizesInZip(zip, files);
        emit maxOverallProgressChanged(100);
        // emit maxBytesProgressChanged(mTotalBytes);
        emit maxFilesProgressChanged(mTotalFiles);
    }
    // END PATCH

    // Estraggo i file
    QStringList extracted;
    for (int i = 0; i < files.count(); i++) {
        QString absPath = QDir(dir).absoluteFilePath(files.at(i));
        if (!extractFile(&zip, files.at(i), absPath)) {
            removeFile(extracted);
            return QStringList();
        }
        extracted.append(absPath);
    }

    // Chiudo il file zip
    zip.close();
    if (zip.getZipError() != 0) {
        removeFile(extracted);
        return QStringList();
    }

    return extracted;
}

QStringList JlCompressObj::extractDir(QString fileCompressed, QString dir) {
    // Apro lo zip
    QuaZip zip(fileCompressed);
    return extractDir(zip, dir);
}

QStringList JlCompressObj::extractDir(QuaZip &zip, const QString &dir) {
    if (!zip.open(QuaZip::mdUnzip)) {
        return QStringList();
    }

    QDir directory(dir);
    QStringList extracted;

    // PATCH
    if (mReportProgress) {
        computeSizesInZip(zip);
        emit maxOverallProgressChanged(100);
        // emit maxBytesProgressChanged(mTotalBytes);
        emit maxFilesProgressChanged(mTotalFiles);
    }
    // END PATCH

    if (!zip.goToFirstFile()) {
        return QStringList();
    }
    do {
        QString name = zip.getCurrentFileName();
        QString absFilePath = directory.absoluteFilePath(name);
        if (!extractFile(&zip, "", absFilePath)) {
            removeFile(extracted);
            return QStringList();
        }
        extracted.append(absFilePath);
    } while (zip.goToNextFile());

    // Chiudo il file zip
    zip.close();
    if (zip.getZipError() != 0) {
        removeFile(extracted);
        return QStringList();
    }

    return extracted;
}

QStringList JlCompressObj::getFileList(QString fileCompressed) {
    // Apro lo zip
    QuaZip *zip = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
    return getFileList(zip);
}

QStringList JlCompressObj::getFileList(QuaZip *zip) {
    if (!zip->open(QuaZip::mdUnzip)) {
        delete zip;
        return QStringList();
    }

    // Estraggo i nomi dei file
    QStringList lst;
    QuaZipFileInfo64 info;
    for (bool more = zip->goToFirstFile(); more; more = zip->goToNextFile()) {
        if (!zip->getCurrentFileInfo(&info)) {
            delete zip;
            return QStringList();
        }
        lst << info.name;
        // info.name.toLocal8Bit().constData()
    }

    // Chiudo il file zip
    zip->close();
    if (zip->getZipError() != 0) {
        delete zip;
        return QStringList();
    }
    delete zip;
    return lst;
}

QStringList JlCompressObj::extractDir(QIODevice *ioDevice, QString dir) {
    QuaZip zip(ioDevice);
    return extractDir(zip, dir);
}

QStringList JlCompressObj::getFileList(QIODevice *ioDevice) {
    QuaZip *zip = new QuaZip(ioDevice);
    return getFileList(zip);
}

QString JlCompressObj::extractFile(QIODevice *ioDevice, QString fileName, QString fileDest) {
    QuaZip zip(ioDevice);
    return extractFile(zip, fileName, fileDest);
}

QStringList JlCompressObj::extractFiles(QIODevice *ioDevice, QStringList files, QString dir) {
    QuaZip zip(ioDevice);
    return extractFiles(zip, files, dir);
}

// PATCH

/**
 * @brief Enable/disable progress reporting signals.
 * @param enabled @ti{true} to enable, @ti{false} otherwise.
 * @details
 * The method always resets the internal counters.
 */
void JlCompressObj::enableProgression(bool enabled) {
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
 */
void JlCompressObj::setGlobalProgressReport(int percent) { mTPReport = qBound(1, percent, 100); }

/**
 * @brief Set current file progress report rate.
 * @param percent Percent value (between 1 and 100).
 * @details
 * Set the emission rate of JlCompressObj::valuePerFileProgressChanged signal in term of percent of the number of
 * (uncompressed) bytes written of the currently processed file.
 */
void JlCompressObj::setFileProgressReport(int percent) { mFPReport = qBound(1, percent, 100); }

/**
 * @brief Count the total number of bytes of <i>path</i>.
 * @param Path path of a single file or a directory.
 * @param fileCount Optional pointer to a int with the number of files.
 * @param recurse Optional boolean flag with @ti{true} (default) to search for files recursively in <i>path</i> if it
 * points a directory.
 * @return Total bytes size.
 * @note
 * If <i>path</i> is neither a directory nor a file, the method returns 0.
 */
qint64 JlCompressObj::countBytes(const QString &path, int *fileCount, bool recurse) {
    qint64 out = 0;
    QFileInfo infos;
    infos.setFile(path);
    if (infos.isFile()) {
        if (fileCount)
            *fileCount = 1;
        return infos.size();
    } else if (infos.isDir()) {
        int files = 0;
        QDirIterator::IteratorFlag flag = recurse ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
        QDirIterator it(path, flag);
        while (it.hasNext()) {
            it.next();
            infos.setFile(it.filePath());
            if (infos.isFile()) {
                files++;
                out += infos.size();
            }
        }
        if (fileCount)
            *fileCount = files;
        return out;
    }
    if (fileCount)
        *fileCount = 0;
    return 0;
}

/**
 * @brief Count the total number of bytes that is represented by the list of files.
 * @param files List of files to process.
 * @param fileCount Optional pointer to a qint64 with the number of files.
 * @return Total bytes size.
 * @overload qint64 JlCompressObj::countBytes(const QString &path,qint64 *fileCount, bool recurse)
 * @details
 * Files in <i>files</i> that cannot be fetched (see QFileInfo::size) are quietly ignored.
 *
 * <i>fileCount</i> is set here for convinience, it only returns the size of <i>files</i> if set to a valid pointers.
 * It does not reflect the actual files accounted for the sum.
 */
qint64 JlCompressObj::countBytes(QStringList files, int *fileCount) {
    int out = 0;
    QFileInfo infos;
    for (QString f : files) {
        infos.setFile(f);
        out += infos.size();
    }
    if (fileCount)
        *fileCount = files.size();
    return out;
}

/**
 * @brief Update internal counter for uncompression interfaces.
 * @param zip Reference to QuaZip instance.
 * @param paths List of paths to process.
 * @details
 * <i>zip</i> must be in QuaZip::mdUnzip mode otherwise, the method returns immediately !
 *
 * The method is only intended to compute the internal counter for progress in the "uncompress" interfaces. It does not
 * emit any signals.
 *
 * If no <i>paths</i> are given, the whole archive is taken into account.
 *
 * If a single path is given, the method attempts to compute internal counters only for this file. On failure, the whole
 * archive is taken into account in order to satisfy
 * bool JlCompressObj::extractFile(QuaZip *zip, QString fileName, QString fileDest).
 *
 * In other cases, the method computes internal counters as function of the list of paths given.
 */
void JlCompressObj::computeSizesInZip(QuaZip &zip, const QStringList paths) {
    if (zip.getMode() != QuaZip::mdUnzip)
        return;
    // reset counters.
    mCurFiles = 0;
    mTotalFiles = 0;
    mCurBytes = 0;
    mTotalBytes = 0;
    QList<QuaZipFileInfo64> infos = zip.getFileInfoList64();
    if (paths.isEmpty() || (paths.size() == 1 && !zip.setCurrentFile(paths.first()))) {
        // -> no paths of single paths that IS NOT in the archive...
        // whole archive.
        for (const QuaZipFileInfo64 &info : infos) {
            mTotalBytes += info.uncompressedSize;
            // directory has 0 bytes size, so skip 0 bytes files !
            if (info.uncompressedSize)
                mTotalFiles++;
        }
        zip.goToFirstFile(); // useful ?
    } else if (paths.size() == 1) {
        // -> single path in archive.
        QuaZipFileInfo64 *info = new QuaZipFileInfo64();
        zip.getCurrentFileInfo(info);
        mTotalBytes = info->uncompressedSize;
        mTotalFiles = 1;
        delete info;
        zip.goToFirstFile(); // useful ?
    } else {
        // specific paths (maybe in archive)
        for (const QuaZipFileInfo64 &info : infos) {
            if (paths.contains(info.name)) {
                mTotalBytes += info.uncompressedSize;
                // directory has 0 bytes size, so skip 0 bytes files !
                if (info.uncompressedSize)
                    mTotalFiles++;
            }
        }
    }
}
// PATCH END
