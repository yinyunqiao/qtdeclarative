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

#include <QtDeclarative/QSGView>
#include "private/qsgcanvasitem_p.h"
#include "private/qsgcontext2d_p.h"
#include "../../../shared/util.h"
#include <QDebug>

class tst_qsgcanvasitem : public QObject
{
    Q_OBJECT
public:
    tst_qsgcanvasitem();

private slots:
    void initTestCase();
    void cleanupTestCase();
    //canvas tests
    void renderInThread();
    void renderTarget();
    void tiles();
    void toImage();
    void loadImage();
    void save();

    //context2d tests
    void context2d_basics();
    void context2d_states();
    void context2d_transforms();
    void context2d_compositing();
    void context2d_colors();
    void context2d_gradients();
    void context2d_patterns();
    void context2d_lineStyles();
    void context2d_shadows();
    void context2d_rects();
    void context2d_texts();
    void context2d_paths();
    void context2d_images();
    void context2d_imageData();
private:
    QSGView* createView(const QString& file);
};

tst_qsgcanvasitem::tst_qsgcanvasitem()
{
}

QSGView* tst_qsgcanvasitem::createView(const QString &file)
{
    QSGView *view = new QSGView;
    view->setSource(QUrl::fromLocalFile(file));
    view->show();
    view->requestActivateWindow();
    QTest::qWait(50);
    return view;
}

void tst_qsgcanvasitem::initTestCase()
{
}

void tst_qsgcanvasitem::cleanupTestCase()
{
}
void tst_qsgcanvasitem::renderInThread()
{

}
void tst_qsgcanvasitem::renderTarget()
{

}
void tst_qsgcanvasitem::tiles()
{

}
void tst_qsgcanvasitem::toImage()
{

}
void tst_qsgcanvasitem::loadImage()
{

}
void tst_qsgcanvasitem::save()
{

}

void tst_qsgcanvasitem::context2d_basics()
{
}

void tst_qsgcanvasitem::context2d_states()
{
}

void tst_qsgcanvasitem::context2d_transforms()
{
}

void tst_qsgcanvasitem::context2d_compositing()
{
}

void tst_qsgcanvasitem::context2d_colors()
{
    QSGView *view = createView(QStringLiteral("data/color.qml"));
    QTRY_COMPARE(QGuiApplication::activeWindow(), static_cast<QWindow *>(view));

    QSGCanvasItem *root = static_cast<QSGCanvasItem*>(view->rootObject());

    QVERIFY(root != 0);
    QSGContext2D* ctx = root->context();
    QVERIFY(ctx != 0);

    const char* styles[] = {"fillStyle" , "strokeStyle"};
    for(int i=0; i < 2; i++) {
        const char* style = styles[i];
        QBrush* brush;

        if (i == 0)
            brush = &(ctx->state.fillStyle);
        else
            brush = &(ctx->state.strokeStyle);

        root->setProperty(style, "rgba(15, 100, 255, 1.0)");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(15,100,255,255)));
        root->setProperty(style, "rgb(15, 100, 255)");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(15,100,255,255)));

        root->setProperty(style, "rgba(15, 100, 255, 0.5)");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(15, 100, 255, 127)));
        root->setProperty(style, "rgba(-15, 300, 255, 3)");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(0, 255, 255, 255)));
        root->setProperty(style, "rgba(-15, 300, 255, -1)");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(0, 255, 255, 0)));

        root->setProperty(style, "rgb(55, 155, 255)");
        qDebug() << brush->color();
        QVERIFY(brush->color() == QColor::fromRgb(qRgb(55, 155, 255)));

        root->setProperty(style, "rgba(0%, 100%, 50%, 1.0)");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(0, 255, 127, 255)));
        root->setProperty(style, "rgb(0%, 100%, 50%)");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(0, 255, 127, 255)));

        root->setProperty(style, "#FFFFFF");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(255, 255, 255, 255)));
        root->setProperty(style, "#000");
        QVERIFY(brush->color() == QColor::fromRgba(qRgba(0, 0, 0, 255)));

    }
    delete view;
}

void tst_qsgcanvasitem::context2d_gradients()
{
}
void tst_qsgcanvasitem::context2d_patterns()
{
}
void tst_qsgcanvasitem::context2d_lineStyles()
{
}
void tst_qsgcanvasitem::context2d_shadows()
{
}
void tst_qsgcanvasitem::context2d_rects()
{
    QSGView *view = createView(QStringLiteral("data/fillrect.qml"));
    QTRY_COMPARE(QGuiApplication::activeWindow(), static_cast<QWindow *>(view));
    QObject *obj = view->rootObject();

    QVERIFY(obj != 0);
    QVERIFY(obj->property("ok").toBool());
}
void tst_qsgcanvasitem::context2d_texts()
{
}
void tst_qsgcanvasitem::context2d_paths()
{
}
void tst_qsgcanvasitem::context2d_images()
{
}
void tst_qsgcanvasitem::context2d_imageData()
{
}

QTEST_MAIN(tst_qsgcanvasitem)

#include "tst_qsgcanvasitem.moc"
