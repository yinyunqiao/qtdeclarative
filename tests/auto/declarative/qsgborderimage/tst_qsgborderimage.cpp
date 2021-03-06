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
#include <QTextDocument>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDir>
#include <QGraphicsScene>
#include <QPainter>

#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecomponent.h>
#include <private/qsgborderimage_p.h>
#include <private/qsgimagebase_p.h>
#include <private/qsgscalegrid_p_p.h>
#include <private/qsgloader_p.h>
#include <QtDeclarative/qsgview.h>
#include <QtDeclarative/qdeclarativecontext.h>

#include "../shared/testhttpserver.h"
#include "../../../shared/util.h"

#ifdef Q_OS_SYMBIAN
// In Symbian OS test data is located in applications private dir
#define SRCDIR "."
#endif

#define SERVER_PORT 14446
#define SERVER_ADDR "http://127.0.0.1:14446"

class tst_qsgborderimage : public QObject

{
    Q_OBJECT
public:
    tst_qsgborderimage();

private slots:
    void noSource();
    void imageSource();
    void imageSource_data();
    void clearSource();
    void resized();
    void smooth();
    void mirror();
    void tileModes();
    void sciSource();
    void sciSource_data();
    void invalidSciFile();
    void pendingRemoteRequest();
    void pendingRemoteRequest_data();

private:
    QDeclarativeEngine engine;
};

tst_qsgborderimage::tst_qsgborderimage()
{
}

void tst_qsgborderimage::noSource()
{
    QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"\" }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);
    QCOMPARE(obj->source(), QUrl());
    QCOMPARE(obj->width(), 0.);
    QCOMPARE(obj->height(), 0.);
    QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Stretch);
    QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Stretch);

    delete obj;
}

void tst_qsgborderimage::imageSource_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<bool>("remote");
    QTest::addColumn<QString>("error");

    QTest::newRow("local") << QUrl::fromLocalFile(SRCDIR "/data/colors.png").toString() << false << "";
    QTest::newRow("local not found") << QUrl::fromLocalFile(SRCDIR "/data/no-such-file.png").toString() << false
        << "file::2:1: QML BorderImage: Cannot open: " + QUrl::fromLocalFile(SRCDIR "/data/no-such-file.png").toString();
    QTest::newRow("remote") << SERVER_ADDR "/colors.png" << true << "";
    QTest::newRow("remote not found") << SERVER_ADDR "/no-such-file.png" << true
        << "file::2:1: QML BorderImage: Error downloading " SERVER_ADDR "/no-such-file.png - server replied: Not found";
}

void tst_qsgborderimage::imageSource()
{
    QFETCH(QString, source);
    QFETCH(bool, remote);
    QFETCH(QString, error);

    TestHTTPServer *server = 0;
    if (remote) {
        server = new TestHTTPServer(SERVER_PORT);
        QVERIFY(server->isValid());
        server->serveDirectory(SRCDIR "/data");
    }

    if (!error.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, error.toUtf8());

    QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" + source + "\" }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);

    if (remote)
        QTRY_VERIFY(obj->status() == QSGBorderImage::Loading);

    QCOMPARE(obj->source(), remote ? source : QUrl(source));

    if (error.isEmpty()) {
        QTRY_VERIFY(obj->status() == QSGBorderImage::Ready);
        QCOMPARE(obj->width(), 120.);
        QCOMPARE(obj->height(), 120.);
        QCOMPARE(obj->sourceSize().width(), 120);
        QCOMPARE(obj->sourceSize().height(), 120);
        QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Stretch);
        QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Stretch);
    } else {
        QTRY_VERIFY(obj->status() == QSGBorderImage::Error);
    }

    delete obj;
    delete server;
}

void tst_qsgborderimage::clearSource()
{
    QString componentStr = "import QtQuick 2.0\nBorderImage { source: srcImage }";
    QDeclarativeContext *ctxt = engine.rootContext();
    ctxt->setContextProperty("srcImage", QUrl::fromLocalFile(SRCDIR "/data/colors.png"));
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);
    QVERIFY(obj->status() == QSGBorderImage::Ready);
    QCOMPARE(obj->width(), 120.);
    QCOMPARE(obj->height(), 120.);

    ctxt->setContextProperty("srcImage", "");
    QVERIFY(obj->source().isEmpty());
    QVERIFY(obj->status() == QSGBorderImage::Null);
    QCOMPARE(obj->width(), 0.);
    QCOMPARE(obj->height(), 0.);
}

void tst_qsgborderimage::resized()
{
    QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" + QUrl::fromLocalFile(SRCDIR "/data/colors.png").toString() + "\"; width: 300; height: 300 }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);
    QCOMPARE(obj->width(), 300.);
    QCOMPARE(obj->height(), 300.);
    QCOMPARE(obj->sourceSize().width(), 120);
    QCOMPARE(obj->sourceSize().height(), 120);
    QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Stretch);
    QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Stretch);

    delete obj;
}

void tst_qsgborderimage::smooth()
{
    QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" SRCDIR "/data/colors.png\"; smooth: true; width: 300; height: 300 }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);
    QCOMPARE(obj->width(), 300.);
    QCOMPARE(obj->height(), 300.);
    QCOMPARE(obj->smooth(), true);
    QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Stretch);
    QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Stretch);

    delete obj;
}

void tst_qsgborderimage::mirror()
{
    QSGView *canvas = new QSGView;
    canvas->show();

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/mirror.qml"));
    QSGBorderImage *image = qobject_cast<QSGBorderImage*>(canvas->rootObject());
    QVERIFY(image != 0);
    canvas->show();

    QImage screenshot = canvas->grabFrameBuffer();

    QImage srcPixmap(screenshot);
    QTransform transform;
    transform.translate(image->width(), 0).scale(-1, 1.0);
    srcPixmap = srcPixmap.transformed(transform);

    image->setProperty("mirror", true);
    screenshot = canvas->grabFrameBuffer();
    QCOMPARE(screenshot, srcPixmap);

    delete canvas;
}

void tst_qsgborderimage::tileModes()
{
    {
        QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" SRCDIR "/data/colors.png\"; width: 100; height: 300; horizontalTileMode: BorderImage.Repeat; verticalTileMode: BorderImage.Repeat }";
        QDeclarativeComponent component(&engine);
        component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
        QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
        QVERIFY(obj != 0);
        QCOMPARE(obj->width(), 100.);
        QCOMPARE(obj->height(), 300.);
        QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Repeat);
        QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Repeat);

        delete obj;
    }
    {
        QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" SRCDIR "/data/colors.png\"; width: 300; height: 150; horizontalTileMode: BorderImage.Round; verticalTileMode: BorderImage.Round }";
        QDeclarativeComponent component(&engine);
        component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
        QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
        QVERIFY(obj != 0);
        QCOMPARE(obj->width(), 300.);
        QCOMPARE(obj->height(), 150.);
        QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Round);
        QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Round);

        delete obj;
    }
}

void tst_qsgborderimage::sciSource()
{
    QFETCH(QString, source);
    QFETCH(bool, valid);

    bool remote = source.startsWith("http");
    TestHTTPServer *server = 0;
    if (remote) {
        server = new TestHTTPServer(SERVER_PORT);
        QVERIFY(server->isValid());
        server->serveDirectory(SRCDIR "/data");
    }

    QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" + source + "\"; width: 300; height: 300 }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);

    if (remote)
        QTRY_VERIFY(obj->status() == QSGBorderImage::Loading);

    QCOMPARE(obj->source(), remote ? source : QUrl(source));
    QCOMPARE(obj->width(), 300.);
    QCOMPARE(obj->height(), 300.);

    if (valid) {
        QTRY_VERIFY(obj->status() == QSGBorderImage::Ready);
        QCOMPARE(obj->border()->left(), 10);
        QCOMPARE(obj->border()->top(), 20);
        QCOMPARE(obj->border()->right(), 30);
        QCOMPARE(obj->border()->bottom(), 40);
        QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Round);
        QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Repeat);
    } else {
        QTRY_VERIFY(obj->status() == QSGBorderImage::Error);
    }

    delete obj;
    delete server;
}

void tst_qsgborderimage::sciSource_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<bool>("valid");

    QTest::newRow("local") << QUrl::fromLocalFile(SRCDIR "/data/colors-round.sci").toString() << true;
    QTest::newRow("local not found") << QUrl::fromLocalFile(SRCDIR "/data/no-such-file.sci").toString() << false;
    QTest::newRow("remote") << SERVER_ADDR "/colors-round.sci" << true;
    QTest::newRow("remote image") << SERVER_ADDR "/colors-round-remote.sci" << true;
    QTest::newRow("remote not found") << SERVER_ADDR "/no-such-file.sci" << false;
}

void tst_qsgborderimage::invalidSciFile()
{
    QTest::ignoreMessage(QtWarningMsg, "QSGGridScaledImage: Invalid tile rule specified. Using Stretch."); // for "Roun"
    QTest::ignoreMessage(QtWarningMsg, "QSGGridScaledImage: Invalid tile rule specified. Using Stretch."); // for "Repea"

    QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" + QUrl::fromLocalFile(SRCDIR "/data/invalid.sci").toString() +"\"; width: 300; height: 300 }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);
    QCOMPARE(obj->width(), 300.);
    QCOMPARE(obj->height(), 300.);
    QCOMPARE(obj->status(), QSGImageBase::Error);
    QCOMPARE(obj->horizontalTileMode(), QSGBorderImage::Stretch);
    QCOMPARE(obj->verticalTileMode(), QSGBorderImage::Stretch);

    delete obj;
}

void tst_qsgborderimage::pendingRemoteRequest()
{
    QFETCH(QString, source);

    QString componentStr = "import QtQuick 2.0\nBorderImage { source: \"" + source + "\" }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QSGBorderImage *obj = qobject_cast<QSGBorderImage*>(component.create());
    QVERIFY(obj != 0);
    QCOMPARE(obj->status(), QSGBorderImage::Loading);

    // verify no crash
    // This will cause a delayed "QThread: Destroyed while thread is still running" warning
    delete obj;
    QTest::qWait(50);
}

void tst_qsgborderimage::pendingRemoteRequest_data()
{
    QTest::addColumn<QString>("source");

    QTest::newRow("png file") << "http://localhost/none.png";
    QTest::newRow("sci file") << "http://localhost/none.sci";
}

QTEST_MAIN(tst_qsgborderimage)

#include "tst_qsgborderimage.moc"
