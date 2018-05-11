#ifndef TESTDIALOG_HPP
#define TESTDIALOG_HPP

#include "widgets.hpp"
#include <QtWidgets>

class JlWorker;

class TestDialog : public QDialog {
    Q_OBJECT
  public:
    TestDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~TestDialog();

    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

  protected:
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

  private slots:
    void manageOperation();
    void onMaxValueProgressChanged(int maxValue);
    void onMaxFilesProgressChanged(int maxValue);
    void onMaxValuePerFileProgressChanged(int maxValue);
    void onValueProgressChanged(int value);
    void onValuePerFileProgressChanged(int value);
    void onFilesProgressChanged(int value);
    void onFileChanged(const QString &name);
    void onTabChanged(int index);
    void onProcessStarted();
    void onProcessFinished();
    void enableProgression(bool enabled);

  private:
    QTabWidget *mTabWidget;
    CompressParamsWidget *mCompressWidget;
    ExtractParamsWidget *mExtractWidget;
    QGroupBox *mGbxProgress;
    ProgressWidget *mProgressWidget;
    QPushButton *mBtnProcess;
    QThread *mWorkerThread;
    JlWorker *mWorker;
    QString mCurrentFile;
    int mFileCount;
    int mFileTotal;
    void setupWorker();
    void deleteWorker();
    void setupCompressionGroupBox();
    void setupExtractionGroupBox();
    void setupProgressGroupBox();
    void setFileProgressLabelText();
};

#endif // TESTDIALOG_HPP
