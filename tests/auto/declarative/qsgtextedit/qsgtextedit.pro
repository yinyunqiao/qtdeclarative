load(qttest_p4)
contains(QT_CONFIG,declarative): QT += declarative gui network
macx:CONFIG -= app_bundle

SOURCES += tst_qsgtextedit.cpp ../shared/testhttpserver.cpp
HEADERS += ../shared/testhttpserver.h

symbian: {
    importFiles.files = data
    importFiles.path = .
    DEPLOYMENT += importFiles
} else {
    DEFINES += SRCDIR=\\\"$$PWD\\\"
}
QT += core-private gui-private declarative-private
QT += opengl-private
