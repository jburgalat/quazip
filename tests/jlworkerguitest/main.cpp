#include "dialog.hpp"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TestDialog *testDialog = new TestDialog;
    testDialog->show();
    int ret = app.exec();
    delete testDialog;
    return ret;
}
