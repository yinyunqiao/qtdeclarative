load(qttest_p4)
contains(QT_CONFIG,declarative): QT += network declarative
macx:CONFIG -= app_bundle

SOURCES += tst_qdeclarativedebughelper.cpp
QT += core-private gui-private declarative-private
