/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <qtest.h>
#include <QtTest/QSignalSpy>
#include <QtDeclarative/qdeclarativecomponent.h>
#include <QtDeclarative/qdeclarativecontext.h>
#include <QtDeclarative/qsgview.h>
#include <QtDeclarative/qsgitem.h>
#include "../../../shared/util.h"

#ifdef Q_OS_SYMBIAN
// In Symbian OS test data is located in applications private dir
#define SRCDIR "."
#endif

class tst_QSGView : public QObject

{
    Q_OBJECT
public:
    tst_QSGView();

private slots:
    void resizemodeitem();
    void errors();
};


tst_QSGView::tst_QSGView()
{
}

void tst_QSGView::resizemodeitem()
{
    QWidget window;
    QSGView *canvas = new QSGView(&window);
    QVERIFY(canvas);
    canvas->setResizeMode(QSGView::SizeRootObjectToView);
    QCOMPARE(QSize(0,0), canvas->initialSize());
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/resizemodeitem.qml"));
    QSGItem* item = qobject_cast<QSGItem*>(canvas->rootObject());
    QVERIFY(item);
    window.show();

    // initial size from root object
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 200.0);
    QCOMPARE(canvas->size(), QSize(200, 200));
    QCOMPARE(canvas->size(), canvas->sizeHint());
    QCOMPARE(canvas->size(), canvas->initialSize());

    // size update from view
    canvas->resize(QSize(80,100));
    QCOMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QCOMPARE(canvas->size(), QSize(80, 100));
    QCOMPARE(canvas->size(), canvas->sizeHint());

    canvas->setResizeMode(QSGView::SizeViewToRootObject);

    // size update from view disabled
    canvas->resize(QSize(60,80));
    QCOMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QCOMPARE(canvas->size(), QSize(60, 80));

    // size update from root object
    item->setWidth(250);
    item->setHeight(350);
    QCOMPARE(item->width(), 250.0);
    QCOMPARE(item->height(), 350.0);
    QTRY_COMPARE(canvas->size(), QSize(250, 350));
    QCOMPARE(canvas->size(), QSize(250, 350));
    QCOMPARE(canvas->size(), canvas->sizeHint());

    // reset canvas
    window.hide();
    delete canvas;
    canvas = new QSGView(&window);
    QVERIFY(canvas);
    canvas->setResizeMode(QSGView::SizeViewToRootObject);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/resizemodeitem.qml"));
    item = qobject_cast<QSGItem*>(canvas->rootObject());
    QVERIFY(item);
    window.show();

    // initial size for root object
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 200.0);
    QCOMPARE(canvas->size(), canvas->sizeHint());
    QCOMPARE(canvas->size(), canvas->initialSize());

    // size update from root object
    item->setWidth(80);
    item->setHeight(100);
    QCOMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QTRY_COMPARE(canvas->size(), QSize(80, 100));
    QCOMPARE(canvas->size(), QSize(80, 100));
    QCOMPARE(canvas->size(), canvas->sizeHint());

    // size update from root object disabled
    canvas->setResizeMode(QSGView::SizeRootObjectToView);
    item->setWidth(60);
    item->setHeight(80);
    QCOMPARE(canvas->width(), 80);
    QCOMPARE(canvas->height(), 100);
    QCOMPARE(QSize(item->width(), item->height()), canvas->sizeHint());

    // size update from view
    canvas->resize(QSize(200,300));
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 300.0);
    QCOMPARE(canvas->size(), QSize(200, 300));
    QCOMPARE(canvas->size(), canvas->sizeHint());

    delete canvas;

    // if we set a specific size for the view then it should keep that size
    // for SizeRootObjectToView mode.
    canvas = new QSGView(&window);
    canvas->resize(300, 300);
    canvas->setResizeMode(QSGView::SizeRootObjectToView);
    QCOMPARE(QSize(0,0), canvas->initialSize());
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/resizemodeitem.qml"));
    item = qobject_cast<QSGItem*>(canvas->rootObject());
    QVERIFY(item);
    window.show();

    // initial size from root object
    QCOMPARE(item->width(), 300.0);
    QCOMPARE(item->height(), 300.0);
    QCOMPARE(canvas->size(), QSize(300, 300));
    QCOMPARE(canvas->size(), canvas->sizeHint());
    QCOMPARE(canvas->initialSize(), QSize(200, 200)); // initial object size

    delete canvas;
}

static void silentErrorsMsgHandler(QtMsgType, const char *)
{
}

void tst_QSGView::errors()
{
    QSGView *canvas = new QSGView;
    QVERIFY(canvas);
    QtMsgHandler old = qInstallMsgHandler(silentErrorsMsgHandler);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/error1.qml"));
    qInstallMsgHandler(old);
    QVERIFY(canvas->status() == QSGView::Error);
    QVERIFY(canvas->errors().count() == 1);
    delete canvas;
}


QTEST_MAIN(tst_QSGView)

#include "tst_qsgview.moc"
