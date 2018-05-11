#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include <QtEvents>
#include <QtWidgets>
#include "quaziptreemodel.hpp"

/// @brief Widget wrapper for user-defined files to compress selection.
class FilesListWidget : public QWidget {
    Q_OBJECT
  public:
    FilesListWidget(QWidget *parent = Q_NULLPTR);
    virtual ~FilesListWidget();
    QStringList listOfFiles() const;
    int filesCount() const;

signals:
    void listOfFilesChanged();

  protected:
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

  private slots:
    void addFilesFromDialog();
    void addDirFromDialog();
    void clearFiles();

  private:
    void addFiles(const QStringList &paths, bool checkDuplicates = true);
    QStringList filesFromDirectory(const QString &directory, bool recurse = true);
    QListWidget *mListWidget;
    QToolBar *mToolbar;
    QAction *mActionAddFiles;
    QAction *mActionAddDir;
    QCheckBox *mChbRecurse;
    QAction *mActionClear;
    QString mLastDirectory;
};

/// @brief Widget wrapper for user-defined directory to compress selection.
class DirectoryWidget : public QWidget {
    Q_OBJECT
  public:
    DirectoryWidget(QWidget *parent = Q_NULLPTR);
    virtual ~DirectoryWidget();
    QString directory() const;

signals:
    void directoryChanged();

  protected:
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

  private slots:
    void setDirectory(const QString &dirPath);
    void getDirFromDialog();

  private:
    QString mLastDirectory;
    QLineEdit *mLdtInputDirectory;
    QToolButton *mBtnInputDirectory;
    QTreeView *mTreeView;
    QFileSystemModel *mModel;
};

class CompressParamsWidget : public QWidget {
    Q_OBJECT
  public:
    CompressParamsWidget(QWidget *parent = Q_NULLPTR);
    virtual ~CompressParamsWidget();
    bool isValid() const;
    bool isDirectory() const;
    QString inputDirectory() const;
    QStringList inputFiles() const;
    QString outputFile() const;
signals:
    void parametersChanged(bool valid);

  private slots:
    void setOutputFileName();

  private:
    QString mLastDirectory;
    QButtonGroup *mBtnGroup;
    QRadioButton *mBtnDirectory;
    QRadioButton *mBtnFile;

    QStackedWidget *mStackedWidget;
    FilesListWidget *mFilesListWidget;
    DirectoryWidget *mDirectoryWidget;
    QLineEdit *mLdtOutputFile;
    QToolButton *mBtnOutputFile;
};

class ExtractParamsWidget : public QWidget {
    Q_OBJECT
  public:
    ExtractParamsWidget(QWidget *parent = Q_NULLPTR);
    virtual ~ExtractParamsWidget();
    QString inputFile() const;
    QStringList filesToExtract() const;
    QString outputDirectory() const;
    bool isValid() const;
signals:
    void parametersChanged(bool valid);

  protected:
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

  private slots:
    void setExtractOutputDir();
    void setExtractInputFile();
    void displayCheckedList();
    void onCustomContextMenuRequested(const QPoint &point);

    void onInputfileChanged(const QString &value);
    void onOutputDirectoryChanged(const QString &value);

  private:
    QLineEdit *mLdtInputFile;
    QToolButton *mBtnExtractInputFile;
    QLineEdit *mLdtOutputDir;
    QToolButton *mBtnExtractOutputDir;
    QTreeView *mTreeView;
    QuazipTreeModel *mModel;
};

class ProgressWidget : public QWidget {
  public:
    ProgressWidget(QWidget *parent = Q_NULLPTR);
    virtual ~ProgressWidget();

    QProgressBar *overallProgressBar() const { return mPrbOverAll; }
    QProgressBar *byFileProgressBar() const { return mPrbByFile; }
    QLabel *fileLabel() const { return mLblCurrentFile; }

  private:
    QLabel *mLblCurrentFile;
    QProgressBar *mPrbOverAll;
    QProgressBar *mPrbByFile;
};

#endif // WIDGETS_HPP
