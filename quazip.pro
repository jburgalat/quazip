TEMPLATE=subdirs
CONFIG += ordered

SUBDIRS= quazip JlWorkerGUItest qztest

JlWorkerGUItest.depends = quazip
qztest.depends = quazip

