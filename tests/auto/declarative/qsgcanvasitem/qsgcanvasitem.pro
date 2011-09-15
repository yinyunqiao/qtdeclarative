load(qttest_p4)
contains(QT_CONFIG,declarative): QT += declarative declarative-private gui v8-private
SOURCES += tst_qsgcanvasitem.cpp

macx:CONFIG -= app_bundle

CONFIG += parallel_test
QT += core-private gui-private declarative-private widgets