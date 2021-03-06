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

#include <QtTest/QtTest>
#include <QtGui/qstringlistmodel.h>
#include <QtDeclarative/qsgview.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecomponent.h>
#include <QtDeclarative/qdeclarativecontext.h>
#include <QtDeclarative/qdeclarativeexpression.h>
#include <QtDeclarative/private/qsgitem_p.h>
#include <QtDeclarative/private/qlistmodelinterface_p.h>
#include <QtDeclarative/private/qsggridview_p.h>
#include <QtDeclarative/private/qsgtext_p.h>
#include <QtDeclarative/private/qdeclarativelistmodel_p.h>
#include "../../../shared/util.h"
#include <QtOpenGL/QGLShaderProgram>

#ifdef Q_OS_SYMBIAN
// In Symbian OS test data is located in applications private dir
#define SRCDIR "."
#endif

Q_DECLARE_METATYPE(Qt::LayoutDirection)
Q_DECLARE_METATYPE(QSGGridView::Flow)

class tst_QSGGridView : public QObject
{
    Q_OBJECT
public:
    tst_QSGGridView();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void items();
    void changed();
    void inserted();
    void removed();
    void clear();
    void moved();
    void moved_data();
    void swapWithFirstItem();
    void changeFlow();
    void currentIndex();
    void noCurrentIndex();
    void defaultValues();
    void properties();
    void propertyChanges();
    void componentChanges();
    void modelChanges();
    void positionViewAtIndex();
    void positionViewAtIndex_rightToLeft();
    void mirroring();
    void snapping();
    void resetModel();
    void enforceRange();
    void enforceRange_rightToLeft();
    void QTBUG_8456();
    void manualHighlight();
    void footer();
    void footer_data();
    void header();
    void header_data();
    void indexAt();
    void onAdd();
    void onAdd_data();
    void onRemove();
    void onRemove_data();
    void testQtQuick11Attributes();
    void testQtQuick11Attributes_data();

private:
    QSGView *createView();
    template<typename T>
    T *findItem(QSGItem *parent, const QString &id, int index=-1);
    template<typename T>
    QList<T*> findItems(QSGItem *parent, const QString &objectName);
    void dumpTree(QSGItem *parent, int depth = 0);
};

template<typename T>
void tst_qsggridview_move(int from, int to, int n, T *items)
{
    if (n == 1) {
        items->move(from, to);
    } else {
        T replaced;
        int i=0;
        typename T::ConstIterator it=items->begin(); it += from+n;
        for (; i<to-from; ++i,++it)
            replaced.append(*it);
        i=0;
        it=items->begin(); it += from;
        for (; i<n; ++i,++it)
            replaced.append(*it);
        typename T::ConstIterator f=replaced.begin();
        typename T::Iterator t=items->begin(); t += from;
        for (; f != replaced.end(); ++f, ++t)
            *t = *f;
    }
}

void tst_QSGGridView::initTestCase()
{
    QSGView canvas;
    if (!QGLShaderProgram::hasOpenGLShaderPrograms(canvas.context()))
        QSKIP("QSGGridView needs OpenGL 2.0", SkipAll);
}

void tst_QSGGridView::cleanupTestCase()
{

}
class TestModel : public QAbstractListModel
{
public:
    enum Roles { Name = Qt::UserRole+1, Number = Qt::UserRole+2 };

    TestModel(QObject *parent=0) : QAbstractListModel(parent) {
        QHash<int, QByteArray> roles;
        roles[Name] = "name";
        roles[Number] = "number";
        setRoleNames(roles);
    }

    int rowCount(const QModelIndex &parent=QModelIndex()) const { Q_UNUSED(parent); return list.count(); }
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const {
        QVariant rv;
        if (role == Name)
            rv = list.at(index.row()).first;
        else if (role == Number)
            rv = list.at(index.row()).second;

        return rv;
    }

    int count() const { return rowCount(); }
    QString name(int index) const { return list.at(index).first; }
    QString number(int index) const { return list.at(index).second; }

    void addItem(const QString &name, const QString &number) {
        emit beginInsertRows(QModelIndex(), list.count(), list.count());
        list.append(QPair<QString,QString>(name, number));
        emit endInsertRows();
    }

    void addItems(const QList<QPair<QString, QString> > &items) {
        emit beginInsertRows(QModelIndex(), list.count(), list.count()+items.count()-1);
        for (int i=0; i<items.count(); i++)
            list.append(QPair<QString,QString>(items[i].first, items[i].second));
        emit endInsertRows();
    }

    void insertItem(int index, const QString &name, const QString &number) {
        emit beginInsertRows(QModelIndex(), index, index);
        list.insert(index, QPair<QString,QString>(name, number));
        emit endInsertRows();
    }

    void removeItem(int index) {
        emit beginRemoveRows(QModelIndex(), index, index);
        list.removeAt(index);
        emit endRemoveRows();
    }

    void removeItems(int index, int count) {
        emit beginRemoveRows(QModelIndex(), index, index+count-1);
        while (count--)
            list.removeAt(index);
        emit endRemoveRows();
    }

    void moveItem(int from, int to) {
        emit beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
        list.move(from, to);
        emit endMoveRows();
    }

    void moveItems(int from, int to, int count) {
        emit beginMoveRows(QModelIndex(), from, from+count-1, QModelIndex(), to > from ? to+count : to);
        tst_qsggridview_move(from, to, count, &list);
        emit endMoveRows();
    }

    void modifyItem(int idx, const QString &name, const QString &number) {
        list[idx] = QPair<QString,QString>(name, number);
        emit dataChanged(index(idx,0), index(idx,0));
    }

    void clear() {
        int count = list.count();
        emit beginRemoveRows(QModelIndex(), 0, count-1);
        list.clear();
        emit endRemoveRows();
    }


private:
    QList<QPair<QString,QString> > list;
};

tst_QSGGridView::tst_QSGGridView()
{
}

void tst_QSGGridView::items()
{
    QSGView *canvas = createView();

    TestModel model;
    model.addItem("Fred", "12345");
    model.addItem("John", "2345");
    model.addItem("Bob", "54321");
    model.addItem("Billy", "22345");
    model.addItem("Sam", "2945");
    model.addItem("Ben", "04321");
    model.addItem("Jim", "0780");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QTRY_COMPARE(gridview->count(), model.count());
    QTRY_COMPARE(canvas->rootObject()->property("count").toInt(), model.count());
    QTRY_COMPARE(contentItem->childItems().count(), model.count()+1); // assumes all are visible, +1 for the (default) highlight item

    for (int i = 0; i < model.count(); ++i) {
        QSGText *name = findItem<QSGText>(contentItem, "textName", i);
        QTRY_VERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        QSGText *number = findItem<QSGText>(contentItem, "textNumber", i);
        QTRY_VERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));
    }

    // set an empty model and confirm that items are destroyed
    TestModel model2;
    ctxt->setContextProperty("testModel", &model2);

    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    QTRY_VERIFY(itemCount == 0);

    delete canvas;
}

void tst_QSGGridView::changed()
{
    QSGView *canvas = createView();

    TestModel model;
    model.addItem("Fred", "12345");
    model.addItem("John", "2345");
    model.addItem("Bob", "54321");
    model.addItem("Billy", "22345");
    model.addItem("Sam", "2945");
    model.addItem("Ben", "04321");
    model.addItem("Jim", "0780");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGFlickable *gridview = findItem<QSGFlickable>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    model.modifyItem(1, "Will", "9876");
    QSGText *name = findItem<QSGText>(contentItem, "textName", 1);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(1));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 1);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(1));

    delete canvas;
}

void tst_QSGGridView::inserted()
{
    QSGView *canvas = createView();
    canvas->show();

    TestModel model;
    model.addItem("Fred", "12345");
    model.addItem("John", "2345");
    model.addItem("Bob", "54321");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    model.insertItem(1, "Will", "9876");
    QCOMPARE(canvas->rootObject()->property("count").toInt(), model.count());

    QTRY_COMPARE(contentItem->childItems().count(), model.count()+1); // assumes all are visible, +1 for the (default) highlight item

    QSGText *name = findItem<QSGText>(contentItem, "textName", 1);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(1));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 1);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(1));

    // Checks that onAdd is called
    int added = canvas->rootObject()->property("added").toInt();
    QTRY_COMPARE(added, 1);

    // Confirm items positioned correctly
    for (int i = 0; i < model.count(); ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QTRY_COMPARE(item->x(), (i%3)*80.0);
        QTRY_COMPARE(item->y(), (i/3)*60.0);
    }

    model.insertItem(0, "Foo", "1111"); // zero index, and current item

    QTRY_COMPARE(contentItem->childItems().count(), model.count()+1); // assumes all are visible, +1 for the (default) highlight item

    name = findItem<QSGText>(contentItem, "textName", 0);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(0));
    number = findItem<QSGText>(contentItem, "textNumber", 0);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(0));

    QTRY_COMPARE(gridview->currentIndex(), 1);

    // Confirm items positioned correctly
    for (int i = 0; i < model.count(); ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QTRY_VERIFY(item->x() == (i%3)*80);
        QTRY_VERIFY(item->y() == (i/3)*60);
    }

    for (int i = model.count(); i < 30; ++i)
        model.insertItem(i, "Hello", QString::number(i));

    gridview->setContentY(120);

    // Insert item outside visible area
    model.insertItem(1, "Hello", "1324");

    QTRY_VERIFY(gridview->contentY() == 120);

    delete canvas;
}

void tst_QSGGridView::removed()
{
    QSGView *canvas = createView();
    canvas->show();

    TestModel model;
    for (int i = 0; i < 40; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    model.removeItem(1);
    QCOMPARE(canvas->rootObject()->property("count").toInt(), model.count());

    QSGText *name = findItem<QSGText>(contentItem, "textName", 1);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(1));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 1);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(1));

    // Checks that onRemove is called
    QString removed = canvas->rootObject()->property("removed").toString();
    QTRY_COMPARE(removed, QString("Item1"));

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->x() == (i%3)*80);
        QTRY_VERIFY(item->y() == (i/3)*60);
    }

    // Remove first item (which is the current item);
    model.removeItem(0);

    name = findItem<QSGText>(contentItem, "textName", 0);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(0));
    number = findItem<QSGText>(contentItem, "textNumber", 0);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(0));

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->x() == (i%3)*80);
        QTRY_VERIFY(item->y() == (i/3)*60);
    }

    // Remove items not visible
    model.removeItem(25);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->x() == (i%3)*80);
        QTRY_VERIFY(item->y() == (i/3)*60);
    }

    // Remove items before visible
    gridview->setContentY(120);
    gridview->setCurrentIndex(10);

    // Setting currentIndex above shouldn't cause view to scroll
    QTRY_COMPARE(gridview->contentY(), 120.0);

    model.removeItem(1);

    // Confirm items positioned correctly
    for (int i = 6; i < 18; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->x() == (i%3)*80);
        QTRY_VERIFY(item->y() == (i/3)*60);
    }

    // Remove currentIndex
    QSGItem *oldCurrent = gridview->currentItem();
    model.removeItem(9);

    QTRY_COMPARE(gridview->currentIndex(), 9);
    QTRY_VERIFY(gridview->currentItem() != oldCurrent);

    gridview->setContentY(0);
    // let transitions settle.
    QTest::qWait(100);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->x() == (i%3)*80);
        QTRY_VERIFY(item->y() == (i/3)*60);
    }

    // remove item outside current view.
    gridview->setCurrentIndex(32);
    gridview->setContentY(240);

    model.removeItem(30);
    QTRY_VERIFY(gridview->currentIndex() == 31);

    // remove current item beyond visible items.
    gridview->setCurrentIndex(20);
    gridview->setContentY(0);
    model.removeItem(20);

    QTRY_COMPARE(gridview->currentIndex(), 20);
    QTRY_VERIFY(gridview->currentItem() != 0);

    // remove item before current, but visible
    gridview->setCurrentIndex(8);
    gridview->setContentY(240);
    oldCurrent = gridview->currentItem();
    model.removeItem(6);

    QTRY_COMPARE(gridview->currentIndex(), 7);
    QTRY_VERIFY(gridview->currentItem() == oldCurrent);

    delete canvas;
}

void tst_QSGGridView::clear()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QVERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QVERIFY(contentItem != 0);

    model.clear();

    QVERIFY(gridview->count() == 0);
    QVERIFY(gridview->currentItem() == 0);
    QVERIFY(gridview->contentY() == 0);
    QVERIFY(gridview->currentIndex() == -1);

    // confirm sanity when adding an item to cleared list
    model.addItem("New", "1");
    QVERIFY(gridview->count() == 1);
    QVERIFY(gridview->currentItem() != 0);
    QVERIFY(gridview->currentIndex() == 0);

    delete canvas;
}

void tst_QSGGridView::moved()
{
    QFETCH(qreal, contentY);
    QFETCH(int, from);
    QFETCH(int, to);
    QFETCH(int, count);
    QFETCH(qreal, itemsOffsetAfterMove);

    QSGText *name;
    QSGText *number;
    QSGView *canvas = createView();
    canvas->show();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGItem *currentItem = gridview->currentItem();
    QTRY_VERIFY(currentItem != 0);

    gridview->setContentY(contentY);
    model.moveItems(from, to, count);

    // Confirm items positioned correctly and indexes correct
    int firstVisibleIndex = qCeil(contentY / 60.0) * 3;
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();

    for (int i = firstVisibleIndex; i < model.count() && i < itemCount; ++i) {
        if (i >= firstVisibleIndex + 18)    // index has moved out of view
            continue;
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY2(item, QTest::toString(QString("Item %1 not found").arg(i)));

        QTRY_COMPARE(item->x(), (i%3)*80.0);
        QTRY_COMPARE(item->y(), (i/3)*60.0 + itemsOffsetAfterMove);

        name = findItem<QSGText>(contentItem, "textName", i);
        QVERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        number = findItem<QSGText>(contentItem, "textNumber", i);
        QVERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));

        // current index should have been updated
        if (item == currentItem)
            QTRY_COMPARE(gridview->currentIndex(), i);
    }

    delete canvas;
}

void tst_QSGGridView::moved_data()
{
    QTest::addColumn<qreal>("contentY");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");
    QTest::addColumn<int>("count");
    QTest::addColumn<qreal>("itemsOffsetAfterMove");

    // model starts with 30 items, each 80x60, in area 240x320
    // 18 items should be visible at a time

    QTest::newRow("move 1 forwards, within visible items")
            << 0.0
            << 1 << 8 << 1
            << 0.0;

    QTest::newRow("move 1 forwards, from non-visible -> visible")
            << 120.0     // show 6-23
            << 1 << 23 << 1
            << 0.0;

    QTest::newRow("move 1 forwards, from non-visible -> visible (move first item)")
            << 120.0     // // show 6-23
            << 0 << 6 << 1
            << 0.0;

    QTest::newRow("move 1 forwards, from visible -> non-visible")
            << 0.0
            << 1 << 20 << 1
            << 0.0;

    QTest::newRow("move 1 forwards, from visible -> non-visible (move first item)")
            << 0.0
            << 0 << 20 << 1
            << 0.0;


    QTest::newRow("move 1 backwards, within visible items")
            << 0.0
            << 10 << 5 << 1
            << 0.0;

    QTest::newRow("move 1 backwards, from non-visible -> visible")
            << 0.0
            << 28 << 8 << 1
            << 0.0;

    QTest::newRow("move 1 backwards, from non-visible -> visible (move last item)")
            << 0.0
            << 29 << 14 << 1
            << 0.0;

    QTest::newRow("move 1 backwards, from visible -> non-visible")
            << 120.0     // show 6-23
            << 7 << 1 << 1
            << 60.0 * 2;   // this results in a forward movement that removes 6 items (2 rows)

    QTest::newRow("move 1 backwards, from visible -> non-visible (move first item)")
            << 120.0     // show 6-23
            << 7 << 0 << 1
            << 60.0 * 2;     // first visible item moved, so all items shift 2 rows down with it


    QTest::newRow("move multiple forwards, within visible items")
            << 0.0
            << 0 << 5 << 3
            << 60.0;    // moved 3 items (i.e. 1 row) down

    QTest::newRow("move multiple forwards, from non-visible -> visible")
            << 120.0     // show 6-23
            << 1 << 6 << 3
            << 60.0;    // top row moved, all items should shift down by 1 row

    QTest::newRow("move multiple forwards, from non-visible -> visible (move first item)")
            << 120.0     // show 6-23
            << 0 << 6 << 3
            << 60.0;    // top row moved, all items should shift down by 1 row

    QTest::newRow("move multiple forwards, from visible -> non-visible")
            << 0.0
            << 1 << 16 << 3
            << 0.0;

    QTest::newRow("move multiple forwards, from visible -> non-visible (move first item)")
            << 0.0
            << 0 << 16 << 3
            << 60.0;


    QTest::newRow("move multiple backwards, within visible items")
            << 0.0
            << 4 << 1 << 3
            << 0.0;

    QTest::newRow("move multiple backwards, from non-visible -> visible")
            << 0.0
            << 20 << 4 << 3
            << 0.0;

    QTest::newRow("move multiple backwards, from non-visible -> visible (move last item)")
            << 0.0
            << 27 << 10 << 3
            << 0.0;

    QTest::newRow("move multiple backwards, from visible -> non-visible")
            << 120.0     // show 6-23
            << 16 << 1 << 3
            << 60.0 * 5;   // this results in a forward movement that removes 15 items (5 rows)

    QTest::newRow("move multiple backwards, from visible -> non-visible (move first item)")
            << 120.0     // show 6-23
            << 16 << 0 << 3
            << 60.0 * 5;    // this results in a forward movement that removes 16 items (5 rows)
}

void tst_QSGGridView::swapWithFirstItem()
{
    // QTBUG_9697
    QSGView *canvas = createView();
    canvas->show();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    // ensure content position is stable
    gridview->setContentY(0);
    model.moveItem(10, 0);
    QTRY_VERIFY(gridview->contentY() == 0);

    delete canvas;
}

void tst_QSGGridView::currentIndex()
{
    TestModel model;
    for (int i = 0; i < 60; i++)
        model.addItem("Item" + QString::number(i), QString::number(i));

    QSGView *canvas = new QSGView(0);
    canvas->setFixedSize(240,320);
    canvas->show();

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    QString filename(SRCDIR "/data/gridview-initCurrent.qml");
    canvas->setSource(QUrl::fromLocalFile(filename));

    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QVERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QVERIFY(contentItem != 0);

    // current item should be third item
    QCOMPARE(gridview->currentIndex(), 35);
    QCOMPARE(gridview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 35));
    QCOMPARE(gridview->currentItem()->y(), gridview->highlightItem()->y());
    QCOMPARE(gridview->contentY(), 400.0);

    gridview->moveCurrentIndexRight();
    QCOMPARE(gridview->currentIndex(), 36);
    gridview->moveCurrentIndexDown();
    QCOMPARE(gridview->currentIndex(), 39);
    gridview->moveCurrentIndexUp();
    QCOMPARE(gridview->currentIndex(), 36);
    gridview->moveCurrentIndexLeft();
    QCOMPARE(gridview->currentIndex(), 35);

    // no wrap
    gridview->setCurrentIndex(0);
    QCOMPARE(gridview->currentIndex(), 0);
    // confirm that the velocity is updated
    QTRY_VERIFY(gridview->verticalVelocity() != 0.0);

    gridview->moveCurrentIndexUp();
    QCOMPARE(gridview->currentIndex(), 0);

    gridview->moveCurrentIndexLeft();
    QCOMPARE(gridview->currentIndex(), 0);

    gridview->setCurrentIndex(model.count()-1);
    QCOMPARE(gridview->currentIndex(), model.count()-1);

    gridview->moveCurrentIndexRight();
    QCOMPARE(gridview->currentIndex(), model.count()-1);

    gridview->moveCurrentIndexDown();
    QCOMPARE(gridview->currentIndex(), model.count()-1);

    // with wrap
    gridview->setWrapEnabled(true);

    gridview->setCurrentIndex(0);
    QCOMPARE(gridview->currentIndex(), 0);

    gridview->moveCurrentIndexLeft();
    QCOMPARE(gridview->currentIndex(), model.count()-1);

    qApp->processEvents();
    QTRY_COMPARE(gridview->contentY(), 880.0);

    gridview->moveCurrentIndexRight();
    QCOMPARE(gridview->currentIndex(), 0);

    QTRY_COMPARE(gridview->contentY(), 0.0);


    // footer should become visible if it is out of view, and then current index moves to the first row
    canvas->rootObject()->setProperty("showFooter", true);
    QTRY_VERIFY(gridview->footerItem());
    gridview->setCurrentIndex(model.count()-3);
    QTRY_VERIFY(gridview->footerItem()->y() > gridview->contentY() + gridview->height());
    gridview->setCurrentIndex(model.count()-2);
    QTRY_COMPARE(gridview->contentY() + gridview->height(), (60.0 * model.count()/3) + gridview->footerItem()->height());
    canvas->rootObject()->setProperty("showFooter", false);

    // header should become visible if it is out of view, and then current index moves to the last row
    canvas->rootObject()->setProperty("showHeader", true);
    QTRY_VERIFY(gridview->headerItem());
    gridview->setCurrentIndex(3);
    QTRY_VERIFY(gridview->headerItem()->y() + gridview->headerItem()->height() < gridview->contentY());
    gridview->setCurrentIndex(1);
    QTRY_COMPARE(gridview->contentY(), -gridview->headerItem()->height());
    canvas->rootObject()->setProperty("showHeader", false);


    // Test keys
    qApp->setActiveWindow(canvas);
#ifdef Q_WS_X11
    // to be safe and avoid failing setFocus with window managers
    qt_x11_wait_for_window_manager(canvas);
#endif
    QTRY_VERIFY(canvas->hasFocus());
    qApp->processEvents();

    gridview->setCurrentIndex(0);

    QTest::keyClick(canvas, Qt::Key_Down);
    QCOMPARE(gridview->currentIndex(), 3);

    QTest::keyClick(canvas, Qt::Key_Up);
    QCOMPARE(gridview->currentIndex(), 0);

    // hold down Key_Down
    for (int i=0; i<(model.count() / 3) - 1; i++) {
        QTest::simulateEvent(canvas, true, Qt::Key_Down, Qt::NoModifier, "", true);
        QTRY_COMPARE(gridview->currentIndex(), i*3 + 3);
    }
    QTest::keyRelease(canvas, Qt::Key_Down);
    QTRY_COMPARE(gridview->currentIndex(), 57);
    QTRY_COMPARE(gridview->contentY(), 880.0);

    // hold down Key_Up
    for (int i=(model.count() / 3) - 1; i > 0; i--) {
        QTest::simulateEvent(canvas, true, Qt::Key_Up, Qt::NoModifier, "", true);
        QTRY_COMPARE(gridview->currentIndex(), i*3 - 3);
    }
    QTest::keyRelease(canvas, Qt::Key_Up);
    QTRY_COMPARE(gridview->currentIndex(), 0);
    QTRY_COMPARE(gridview->contentY(), 0.0);


    gridview->setFlow(QSGGridView::TopToBottom);

    qApp->setActiveWindow(canvas);
#ifdef Q_WS_X11
    // to be safe and avoid failing setFocus with window managers
    qt_x11_wait_for_window_manager(canvas);
#endif
    QTRY_VERIFY(canvas->hasFocus());
    qApp->processEvents();

    QTest::keyClick(canvas, Qt::Key_Right);
    QCOMPARE(gridview->currentIndex(), 5);

    QTest::keyClick(canvas, Qt::Key_Left);
    QCOMPARE(gridview->currentIndex(), 0);

    QTest::keyClick(canvas, Qt::Key_Down);
    QCOMPARE(gridview->currentIndex(), 1);

    QTest::keyClick(canvas, Qt::Key_Up);
    QCOMPARE(gridview->currentIndex(), 0);

    // hold down Key_Right
    for (int i=0; i<(model.count() / 5) - 1; i++) {
        QTest::simulateEvent(canvas, true, Qt::Key_Right, Qt::NoModifier, "", true);
        QTRY_COMPARE(gridview->currentIndex(), i*5 + 5);
    }
    QTest::keyRelease(canvas, Qt::Key_Right);
    QTRY_COMPARE(gridview->currentIndex(), 55);
    QTRY_COMPARE(gridview->contentX(), 720.0);

    // hold down Key_Left
    for (int i=(model.count() / 5) - 1; i > 0; i--) {
        QTest::simulateEvent(canvas, true, Qt::Key_Left, Qt::NoModifier, "", true);
        QTRY_COMPARE(gridview->currentIndex(), i*5 - 5);
    }
    QTest::keyRelease(canvas, Qt::Key_Left);
    QTRY_COMPARE(gridview->currentIndex(), 0);
    QTRY_COMPARE(gridview->contentX(), 0.0);


    // turn off auto highlight
    gridview->setHighlightFollowsCurrentItem(false);
    QVERIFY(gridview->highlightFollowsCurrentItem() == false);
    QVERIFY(gridview->highlightItem());
    qreal hlPosX = gridview->highlightItem()->x();
    qreal hlPosY = gridview->highlightItem()->y();

    gridview->setCurrentIndex(5);
    QTRY_COMPARE(gridview->highlightItem()->x(), hlPosX);
    QTRY_COMPARE(gridview->highlightItem()->y(), hlPosY);

    // insert item before currentIndex
    gridview->setCurrentIndex(28);
    model.insertItem(0, "Foo", "1111");
    QTRY_COMPARE(canvas->rootObject()->property("current").toInt(), 29);

    // check removing highlight by setting currentIndex to -1;
    gridview->setCurrentIndex(-1);

    QCOMPARE(gridview->currentIndex(), -1);
    QVERIFY(!gridview->highlightItem());
    QVERIFY(!gridview->currentItem());

    gridview->setHighlightFollowsCurrentItem(true);

    gridview->setFlow(QSGGridView::LeftToRight);
    gridview->setLayoutDirection(Qt::RightToLeft);

    qApp->setActiveWindow(canvas);
#ifdef Q_WS_X11
    // to be safe and avoid failing setFocus with window managers
    qt_x11_wait_for_window_manager(canvas);
#endif
    QTRY_VERIFY(canvas->hasFocus());
    qApp->processEvents();

    gridview->setCurrentIndex(35);

    QTest::keyClick(canvas, Qt::Key_Right);
    QCOMPARE(gridview->currentIndex(), 34);

    QTest::keyClick(canvas, Qt::Key_Down);
    QCOMPARE(gridview->currentIndex(), 37);

    QTest::keyClick(canvas, Qt::Key_Up);
    QCOMPARE(gridview->currentIndex(), 34);

    QTest::keyClick(canvas, Qt::Key_Left);
    QCOMPARE(gridview->currentIndex(), 35);


    // turn off auto highlight
    gridview->setHighlightFollowsCurrentItem(false);
    QVERIFY(gridview->highlightFollowsCurrentItem() == false);
    QVERIFY(gridview->highlightItem());
    hlPosX = gridview->highlightItem()->x();
    hlPosY = gridview->highlightItem()->y();

    gridview->setCurrentIndex(5);
    QTRY_COMPARE(gridview->highlightItem()->x(), hlPosX);
    QTRY_COMPARE(gridview->highlightItem()->y(), hlPosY);

    // insert item before currentIndex
    gridview->setCurrentIndex(28);
    model.insertItem(0, "Foo", "1111");
    QTRY_COMPARE(canvas->rootObject()->property("current").toInt(), 29);

    // check removing highlight by setting currentIndex to -1;
    gridview->setCurrentIndex(-1);

    QCOMPARE(gridview->currentIndex(), -1);
    QVERIFY(!gridview->highlightItem());
    QVERIFY(!gridview->currentItem());

    delete canvas;
}

void tst_QSGGridView::noCurrentIndex()
{
    TestModel model;
    for (int i = 0; i < 60; i++)
        model.addItem("Item" + QString::number(i), QString::number(i));

    QSGView *canvas = new QSGView(0);
    canvas->setFixedSize(240,320);

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    QString filename(SRCDIR "/data/gridview-noCurrent.qml");
    canvas->setSource(QUrl::fromLocalFile(filename));

    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QVERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QVERIFY(contentItem != 0);

    // current index should be -1
    QCOMPARE(gridview->currentIndex(), -1);
    QVERIFY(!gridview->currentItem());
    QVERIFY(!gridview->highlightItem());
    QCOMPARE(gridview->contentY(), 0.0);

    gridview->setCurrentIndex(5);
    QCOMPARE(gridview->currentIndex(), 5);
    QVERIFY(gridview->currentItem());
    QVERIFY(gridview->highlightItem());

    delete canvas;
}

void tst_QSGGridView::changeFlow()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), QString::number(i));

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly and indexes correct
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal((i%3)*80));
        QTRY_COMPARE(item->y(), qreal((i/3)*60));
        QSGText *name = findItem<QSGText>(contentItem, "textName", i);
        QTRY_VERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        QSGText *number = findItem<QSGText>(contentItem, "textNumber", i);
        QTRY_VERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));
    }

    ctxt->setContextProperty("testTopToBottom", QVariant(true));

    // Confirm items positioned correctly and indexes correct
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal((i/5)*80));
        QTRY_COMPARE(item->y(), qreal((i%5)*60));
        QSGText *name = findItem<QSGText>(contentItem, "textName", i);
        QTRY_VERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        QSGText *number = findItem<QSGText>(contentItem, "textNumber", i);
        QTRY_VERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));
    }

    ctxt->setContextProperty("testRightToLeft", QVariant(true));

    // Confirm items positioned correctly and indexes correct
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal(-(i/5)*80 - item->width()));
        QTRY_COMPARE(item->y(), qreal((i%5)*60));
        QSGText *name = findItem<QSGText>(contentItem, "textName", i);
        QTRY_VERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        QSGText *number = findItem<QSGText>(contentItem, "textNumber", i);
        QTRY_VERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));
    }
    gridview->setContentX(100);
    QTRY_COMPARE(gridview->contentX(), 100.);
    ctxt->setContextProperty("testTopToBottom", QVariant(false));
    QTRY_COMPARE(gridview->contentX(), 0.);

    // Confirm items positioned correctly and indexes correct
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal(240 - (i%3+1)*80));
        QTRY_COMPARE(item->y(), qreal((i/3)*60));
        QSGText *name = findItem<QSGText>(contentItem, "textName", i);
        QTRY_VERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        QSGText *number = findItem<QSGText>(contentItem, "textNumber", i);
        QTRY_VERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));
    }

    delete canvas;
}

void tst_QSGGridView::defaultValues()
{
    QDeclarativeEngine engine;
    QDeclarativeComponent c(&engine, QUrl::fromLocalFile(SRCDIR "/data/gridview3.qml"));
    QSGGridView *obj = qobject_cast<QSGGridView*>(c.create());

    QTRY_VERIFY(obj != 0);
    QTRY_VERIFY(obj->model() == QVariant());
    QTRY_VERIFY(obj->delegate() == 0);
    QTRY_COMPARE(obj->currentIndex(), -1);
    QTRY_VERIFY(obj->currentItem() == 0);
    QTRY_COMPARE(obj->count(), 0);
    QTRY_VERIFY(obj->highlight() == 0);
    QTRY_VERIFY(obj->highlightItem() == 0);
    QTRY_COMPARE(obj->highlightFollowsCurrentItem(), true);
    QTRY_VERIFY(obj->flow() == 0);
    QTRY_COMPARE(obj->isWrapEnabled(), false);
    QTRY_COMPARE(obj->cacheBuffer(), 0);
    QTRY_COMPARE(obj->cellWidth(), 100); //### Should 100 be the default?
    QTRY_COMPARE(obj->cellHeight(), 100);
    delete obj;
}

void tst_QSGGridView::properties()
{
    QDeclarativeEngine engine;
    QDeclarativeComponent c(&engine, QUrl::fromLocalFile(SRCDIR "/data/gridview2.qml"));
    QSGGridView *obj = qobject_cast<QSGGridView*>(c.create());

    QTRY_VERIFY(obj != 0);
    QTRY_VERIFY(obj->model() != QVariant());
    QTRY_VERIFY(obj->delegate() != 0);
    QTRY_COMPARE(obj->currentIndex(), 0);
    QTRY_VERIFY(obj->currentItem() != 0);
    QTRY_COMPARE(obj->count(), 4);
    QTRY_VERIFY(obj->highlight() != 0);
    QTRY_VERIFY(obj->highlightItem() != 0);
    QTRY_COMPARE(obj->highlightFollowsCurrentItem(), false);
    QTRY_VERIFY(obj->flow() == 0);
    QTRY_COMPARE(obj->isWrapEnabled(), true);
    QTRY_COMPARE(obj->cacheBuffer(), 200);
    QTRY_COMPARE(obj->cellWidth(), 100);
    QTRY_COMPARE(obj->cellHeight(), 100);
    delete obj;
}

void tst_QSGGridView::propertyChanges()
{
    QSGView *canvas = createView();
    QTRY_VERIFY(canvas);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/propertychangestest.qml"));

    QSGGridView *gridView = canvas->rootObject()->findChild<QSGGridView*>("gridView");
    QTRY_VERIFY(gridView);

    QSignalSpy keyNavigationWrapsSpy(gridView, SIGNAL(keyNavigationWrapsChanged()));
    QSignalSpy cacheBufferSpy(gridView, SIGNAL(cacheBufferChanged()));
    QSignalSpy layoutSpy(gridView, SIGNAL(layoutDirectionChanged()));
    QSignalSpy flowSpy(gridView, SIGNAL(flowChanged()));

    QTRY_COMPARE(gridView->isWrapEnabled(), true);
    QTRY_COMPARE(gridView->cacheBuffer(), 10);
    QTRY_COMPARE(gridView->flow(), QSGGridView::LeftToRight);

    gridView->setWrapEnabled(false);
    gridView->setCacheBuffer(3);
    gridView->setFlow(QSGGridView::TopToBottom);

    QTRY_COMPARE(gridView->isWrapEnabled(), false);
    QTRY_COMPARE(gridView->cacheBuffer(), 3);
    QTRY_COMPARE(gridView->flow(), QSGGridView::TopToBottom);

    QTRY_COMPARE(keyNavigationWrapsSpy.count(),1);
    QTRY_COMPARE(cacheBufferSpy.count(),1);
    QTRY_COMPARE(flowSpy.count(),1);

    gridView->setWrapEnabled(false);
    gridView->setCacheBuffer(3);
    gridView->setFlow(QSGGridView::TopToBottom);

    QTRY_COMPARE(keyNavigationWrapsSpy.count(),1);
    QTRY_COMPARE(cacheBufferSpy.count(),1);
    QTRY_COMPARE(flowSpy.count(),1);

    gridView->setFlow(QSGGridView::LeftToRight);
    QTRY_COMPARE(gridView->flow(), QSGGridView::LeftToRight);

    gridView->setWrapEnabled(true);
    gridView->setCacheBuffer(5);
    gridView->setLayoutDirection(Qt::RightToLeft);

    QTRY_COMPARE(gridView->isWrapEnabled(), true);
    QTRY_COMPARE(gridView->cacheBuffer(), 5);
    QTRY_COMPARE(gridView->layoutDirection(), Qt::RightToLeft);

    QTRY_COMPARE(keyNavigationWrapsSpy.count(),2);
    QTRY_COMPARE(cacheBufferSpy.count(),2);
    QTRY_COMPARE(layoutSpy.count(),1);
    QTRY_COMPARE(flowSpy.count(),2);

    gridView->setWrapEnabled(true);
    gridView->setCacheBuffer(5);
    gridView->setLayoutDirection(Qt::RightToLeft);

    QTRY_COMPARE(keyNavigationWrapsSpy.count(),2);
    QTRY_COMPARE(cacheBufferSpy.count(),2);
    QTRY_COMPARE(layoutSpy.count(),1);
    QTRY_COMPARE(flowSpy.count(),2);

    gridView->setFlow(QSGGridView::TopToBottom);
    QTRY_COMPARE(gridView->flow(), QSGGridView::TopToBottom);
    QTRY_COMPARE(flowSpy.count(),3);

    gridView->setFlow(QSGGridView::TopToBottom);
    QTRY_COMPARE(flowSpy.count(),3);

    delete canvas;
}

void tst_QSGGridView::componentChanges()
{
    QSGView *canvas = createView();
    QTRY_VERIFY(canvas);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/propertychangestest.qml"));

    QSGGridView *gridView = canvas->rootObject()->findChild<QSGGridView*>("gridView");
    QTRY_VERIFY(gridView);

    QDeclarativeComponent component(canvas->engine());
    component.setData("import QtQuick 1.0; Rectangle { color: \"blue\"; }", QUrl::fromLocalFile(""));

    QDeclarativeComponent delegateComponent(canvas->engine());
    delegateComponent.setData("import QtQuick 1.0; Text { text: '<b>Name:</b> ' + name }", QUrl::fromLocalFile(""));

    QSignalSpy highlightSpy(gridView, SIGNAL(highlightChanged()));
    QSignalSpy delegateSpy(gridView, SIGNAL(delegateChanged()));
    QSignalSpy headerSpy(gridView, SIGNAL(headerChanged()));
    QSignalSpy footerSpy(gridView, SIGNAL(footerChanged()));

    gridView->setHighlight(&component);
    gridView->setDelegate(&delegateComponent);
    gridView->setHeader(&component);
    gridView->setFooter(&component);

    QTRY_COMPARE(gridView->highlight(), &component);
    QTRY_COMPARE(gridView->delegate(), &delegateComponent);
    QTRY_COMPARE(gridView->header(), &component);
    QTRY_COMPARE(gridView->footer(), &component);

    QTRY_COMPARE(highlightSpy.count(),1);
    QTRY_COMPARE(delegateSpy.count(),1);
    QTRY_COMPARE(headerSpy.count(),1);
    QTRY_COMPARE(footerSpy.count(),1);

    gridView->setHighlight(&component);
    gridView->setDelegate(&delegateComponent);
    gridView->setHeader(&component);
    gridView->setFooter(&component);

    QTRY_COMPARE(highlightSpy.count(),1);
    QTRY_COMPARE(delegateSpy.count(),1);
    QTRY_COMPARE(headerSpy.count(),1);
    QTRY_COMPARE(footerSpy.count(),1);

    delete canvas;
}

void tst_QSGGridView::modelChanges()
{
    QSGView *canvas = createView();
    QTRY_VERIFY(canvas);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/propertychangestest.qml"));

    QSGGridView *gridView = canvas->rootObject()->findChild<QSGGridView*>("gridView");
    QTRY_VERIFY(gridView);

    QDeclarativeListModel *alternateModel = canvas->rootObject()->findChild<QDeclarativeListModel*>("alternateModel");
    QTRY_VERIFY(alternateModel);
    QVariant modelVariant = QVariant::fromValue<QObject *>(alternateModel);
    QSignalSpy modelSpy(gridView, SIGNAL(modelChanged()));

    gridView->setModel(modelVariant);
    QTRY_COMPARE(gridView->model(), modelVariant);
    QTRY_COMPARE(modelSpy.count(),1);

    gridView->setModel(modelVariant);
    QTRY_COMPARE(modelSpy.count(),1);

    gridView->setModel(QVariant());
    QTRY_COMPARE(modelSpy.count(),2);
    delete canvas;
}

void tst_QSGGridView::positionViewAtIndex()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 40; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), (i%3)*80.);
        QTRY_COMPARE(item->y(), (i/3)*60.);
    }

    // Position on a currently visible item
    gridview->positionViewAtIndex(4, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->indexAt(120, 90), 4);
    QTRY_COMPARE(gridview->contentY(), 60.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 3; i < model.count() && i < itemCount-3-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), (i%3)*80.);
        QTRY_COMPARE(item->y(), (i/3)*60.);
    }

    // Position on an item beyond the visible items
    gridview->positionViewAtIndex(21, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->indexAt(40, 450), 21);
    QTRY_COMPARE(gridview->contentY(), 420.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 22; i < model.count() && i < itemCount-22-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), (i%3)*80.);
        QTRY_COMPARE(item->y(), (i/3)*60.);
    }

    // Position on an item that would leave empty space if positioned at the top
    gridview->positionViewAtIndex(31, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->indexAt(120, 630), 31);
    QTRY_COMPARE(gridview->contentY(), 520.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 24; i < model.count() && i < itemCount-24-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), (i%3)*80.);
        QTRY_COMPARE(item->y(), (i/3)*60.);
    }

    // Position at the beginning again
    gridview->positionViewAtIndex(0, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->indexAt(0, 0), 0);
    QTRY_COMPARE(gridview->indexAt(40, 30), 0);
    QTRY_COMPARE(gridview->indexAt(80, 60), 4);
    QTRY_COMPARE(gridview->contentY(), 0.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), (i%3)*80.);
        QTRY_COMPARE(item->y(), (i/3)*60.);
    }

    // Position at End
    gridview->positionViewAtIndex(30, QSGGridView::End);
    QTRY_COMPARE(gridview->contentY(), 340.);

    // Position in Center
    gridview->positionViewAtIndex(15, QSGGridView::Center);
    QTRY_COMPARE(gridview->contentY(), 170.);

    // Ensure at least partially visible
    gridview->positionViewAtIndex(15, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentY(), 170.);

    gridview->setContentY(302);
    gridview->positionViewAtIndex(15, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentY(), 302.);

    gridview->setContentY(360);
    gridview->positionViewAtIndex(15, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentY(), 300.);

    gridview->setContentY(60);
    gridview->positionViewAtIndex(20, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentY(), 60.);

    gridview->setContentY(20);
    gridview->positionViewAtIndex(20, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentY(), 100.);

    // Ensure completely visible
    gridview->setContentY(120);
    gridview->positionViewAtIndex(20, QSGGridView::Contain);
    QTRY_COMPARE(gridview->contentY(), 120.);

    gridview->setContentY(302);
    gridview->positionViewAtIndex(15, QSGGridView::Contain);
    QTRY_COMPARE(gridview->contentY(), 300.);

    gridview->setContentY(60);
    gridview->positionViewAtIndex(20, QSGGridView::Contain);
    QTRY_COMPARE(gridview->contentY(), 100.);

    // Test for Top To Bottom layout
    ctxt->setContextProperty("testTopToBottom", QVariant(true));

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), (i/5)*80.);
        QTRY_COMPARE(item->y(), (i%5)*60.);
    }

    // Position at End
    gridview->positionViewAtIndex(30, QSGGridView::End);
    QTRY_COMPARE(gridview->contentX(), 320.);
    QTRY_COMPARE(gridview->contentY(), 0.);

    // Position in Center
    gridview->positionViewAtIndex(15, QSGGridView::Center);
    QTRY_COMPARE(gridview->contentX(), 160.);

    // Ensure at least partially visible
    gridview->positionViewAtIndex(15, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), 160.);

    gridview->setContentX(170);
    gridview->positionViewAtIndex(25, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), 170.);

    gridview->positionViewAtIndex(30, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), 320.);

    gridview->setContentX(170);
    gridview->positionViewAtIndex(25, QSGGridView::Contain);
    QTRY_COMPARE(gridview->contentX(), 240.);

    // positionViewAtBeginning
    gridview->positionViewAtBeginning();
    QTRY_COMPARE(gridview->contentX(), 0.);

    gridview->setContentX(80);
    canvas->rootObject()->setProperty("showHeader", true);
    gridview->positionViewAtBeginning();
    QTRY_COMPARE(gridview->contentX(), -30.);

    // positionViewAtEnd
    gridview->positionViewAtEnd();
    QTRY_COMPARE(gridview->contentX(), 400.);   // 8*80 - 240   (8 columns)

    gridview->setContentX(80);
    canvas->rootObject()->setProperty("showFooter", true);
    gridview->positionViewAtEnd();
    QTRY_COMPARE(gridview->contentX(), 430.);

    // set current item to outside visible view, position at beginning
    // and ensure highlight moves to current item
    gridview->setCurrentIndex(6);
    gridview->positionViewAtBeginning();
    QTRY_COMPARE(gridview->contentX(), -30.);
    QVERIFY(gridview->highlightItem());
    QCOMPARE(gridview->highlightItem()->x(), 80.);

    delete canvas;
}

void tst_QSGGridView::snapping()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 40; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testTopToBottom", QVariant(false));
    ctxt->setContextProperty("testRightToLeft", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    gridview->setHeight(220);
    QCOMPARE(gridview->height(), 220.);

    gridview->positionViewAtIndex(12, QSGGridView::Visible);
    QCOMPARE(gridview->contentY(), 80.);

    gridview->setContentY(0);
    QCOMPARE(gridview->contentY(), 0.);

    gridview->setSnapMode(QSGGridView::SnapToRow);
    QCOMPARE(gridview->snapMode(), QSGGridView::SnapToRow);

    gridview->positionViewAtIndex(12, QSGGridView::Visible);
    QCOMPARE(gridview->contentY(), 60.);

    gridview->positionViewAtIndex(15, QSGGridView::End);
    QCOMPARE(gridview->contentY(), 120.);

    delete canvas;

}

void tst_QSGGridView::mirroring()
{
    QSGView *canvasA = createView();
    canvasA->setSource(QUrl::fromLocalFile(SRCDIR "/data/mirroring.qml"));
    QSGGridView *gridviewA = findItem<QSGGridView>(canvasA->rootObject(), "view");
    QTRY_VERIFY(gridviewA != 0);

    QSGView *canvasB = createView();
    canvasB->setSource(QUrl::fromLocalFile(SRCDIR "/data/mirroring.qml"));
    QSGGridView *gridviewB = findItem<QSGGridView>(canvasB->rootObject(), "view");
    QTRY_VERIFY(gridviewA != 0);
    qApp->processEvents();

    QList<QString> objectNames;
    objectNames << "item1" << "item2"; // << "item3"

    gridviewA->setProperty("layoutDirection", Qt::LeftToRight);
    gridviewB->setProperty("layoutDirection", Qt::RightToLeft);
    QCOMPARE(gridviewA->layoutDirection(), gridviewA->effectiveLayoutDirection());

    // LTR != RTL
    foreach(const QString objectName, objectNames)
        QVERIFY(findItem<QSGItem>(gridviewA, objectName)->x() != findItem<QSGItem>(gridviewB, objectName)->x());

    gridviewA->setProperty("layoutDirection", Qt::LeftToRight);
    gridviewB->setProperty("layoutDirection", Qt::LeftToRight);

    // LTR == LTR
    foreach(const QString objectName, objectNames)
        QCOMPARE(findItem<QSGItem>(gridviewA, objectName)->x(), findItem<QSGItem>(gridviewB, objectName)->x());

    QVERIFY(gridviewB->layoutDirection() == gridviewB->effectiveLayoutDirection());
    QSGItemPrivate::get(gridviewB)->setLayoutMirror(true);
    QVERIFY(gridviewB->layoutDirection() != gridviewB->effectiveLayoutDirection());

    // LTR != LTR+mirror
    foreach(const QString objectName, objectNames)
        QVERIFY(findItem<QSGItem>(gridviewA, objectName)->x() != findItem<QSGItem>(gridviewB, objectName)->x());

    gridviewA->setProperty("layoutDirection", Qt::RightToLeft);

    // RTL == LTR+mirror
    foreach(const QString objectName, objectNames)
        QCOMPARE(findItem<QSGItem>(gridviewA, objectName)->x(), findItem<QSGItem>(gridviewB, objectName)->x());

    gridviewB->setProperty("layoutDirection", Qt::RightToLeft);

    // RTL != RTL+mirror
    foreach(const QString objectName, objectNames)
        QVERIFY(findItem<QSGItem>(gridviewA, objectName)->x() != findItem<QSGItem>(gridviewB, objectName)->x());

    gridviewA->setProperty("layoutDirection", Qt::LeftToRight);

    // LTR == RTL+mirror
    foreach(const QString objectName, objectNames)
        QCOMPARE(findItem<QSGItem>(gridviewA, objectName)->x(), findItem<QSGItem>(gridviewB, objectName)->x());

    delete canvasA;
    delete canvasB;
}

void tst_QSGGridView::positionViewAtIndex_rightToLeft()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 40; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testTopToBottom", QVariant(true));
    ctxt->setContextProperty("testRightToLeft", QVariant(true));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal(-(i/5)*80-item->width()));
        QTRY_COMPARE(item->y(), qreal((i%5)*60));
    }

    // Position on a currently visible item
    gridview->positionViewAtIndex(6, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->contentX(), -320.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 3; i < model.count() && i < itemCount-3-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal(-(i/5)*80-item->width()));
        QTRY_COMPARE(item->y(), qreal((i%5)*60));
    }

    // Position on an item beyond the visible items
    gridview->positionViewAtIndex(21, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->contentX(), -560.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 22; i < model.count() && i < itemCount-22-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal(-(i/5)*80-item->width()));
        QTRY_COMPARE(item->y(), qreal((i%5)*60));
    }

    // Position on an item that would leave empty space if positioned at the top
    gridview->positionViewAtIndex(31, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->contentX(), -640.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 24; i < model.count() && i < itemCount-24-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal(-(i/5)*80-item->width()));
        QTRY_COMPARE(item->y(), qreal((i%5)*60));
    }

    // Position at the beginning again
    gridview->positionViewAtIndex(0, QSGGridView::Beginning);
    QTRY_COMPARE(gridview->contentX(), -240.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), qreal(-(i/5)*80-item->width()));
        QTRY_COMPARE(item->y(), qreal((i%5)*60));
    }

    // Position at End
    gridview->positionViewAtIndex(30, QSGGridView::End);
    QTRY_COMPARE(gridview->contentX(), -560.);

    // Position in Center
    gridview->positionViewAtIndex(15, QSGGridView::Center);
    QTRY_COMPARE(gridview->contentX(), -400.);

    // Ensure at least partially visible
    gridview->positionViewAtIndex(15, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), -400.);

    gridview->setContentX(-555.);
    gridview->positionViewAtIndex(15, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), -555.);

    gridview->setContentX(-239);
    gridview->positionViewAtIndex(15, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), -320.);

    gridview->setContentX(-239);
    gridview->positionViewAtIndex(20, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), -400.);

    gridview->setContentX(-640);
    gridview->positionViewAtIndex(20, QSGGridView::Visible);
    QTRY_COMPARE(gridview->contentX(), -560.);

    // Ensure completely visible
    gridview->setContentX(-400);
    gridview->positionViewAtIndex(20, QSGGridView::Contain);
    QTRY_COMPARE(gridview->contentX(), -400.);

    gridview->setContentX(-315);
    gridview->positionViewAtIndex(15, QSGGridView::Contain);
    QTRY_COMPARE(gridview->contentX(), -320.);

    gridview->setContentX(-640);
    gridview->positionViewAtIndex(20, QSGGridView::Contain);
    QTRY_COMPARE(gridview->contentX(), -560.);

    delete canvas;
}

void tst_QSGGridView::resetModel()
{
    QSGView *canvas = createView();

    QStringList strings;
    strings << "one" << "two" << "three";
    QStringListModel model(strings);

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/displaygrid.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QTRY_COMPARE(gridview->count(), model.rowCount());

    for (int i = 0; i < model.rowCount(); ++i) {
        QSGText *display = findItem<QSGText>(contentItem, "displayText", i);
        QTRY_VERIFY(display != 0);
        QTRY_COMPARE(display->text(), strings.at(i));
    }

    strings.clear();
    strings << "four" << "five" << "six" << "seven";
    model.setStringList(strings);

    QTRY_COMPARE(gridview->count(), model.rowCount());

    for (int i = 0; i < model.rowCount(); ++i) {
        QSGText *display = findItem<QSGText>(contentItem, "displayText", i);
        QTRY_VERIFY(display != 0);
        QTRY_COMPARE(display->text(), strings.at(i));
    }

    delete canvas;
}

void tst_QSGGridView::enforceRange()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview-enforcerange.qml"));
    qApp->processEvents();
    QVERIFY(canvas->rootObject() != 0);

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QTRY_COMPARE(gridview->preferredHighlightBegin(), 100.0);
    QTRY_COMPARE(gridview->preferredHighlightEnd(), 100.0);
    QTRY_COMPARE(gridview->highlightRangeMode(), QSGGridView::StrictlyEnforceRange);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // view should be positioned at the top of the range.
    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QTRY_VERIFY(item);
    QTRY_COMPARE(gridview->contentY(), -100.0);

    QSGText *name = findItem<QSGText>(contentItem, "textName", 0);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(0));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 0);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(0));

    // Check currentIndex is updated when contentItem moves
    gridview->setContentY(0);
    QTRY_COMPARE(gridview->currentIndex(), 2);

    gridview->setCurrentIndex(5);
    QTRY_COMPARE(gridview->contentY(), 100.);

    TestModel model2;
    for (int i = 0; i < 5; i++)
        model2.addItem("Item" + QString::number(i), "");

    ctxt->setContextProperty("testModel", &model2);
    QCOMPARE(gridview->count(), 5);

    delete canvas;
}

void tst_QSGGridView::enforceRange_rightToLeft()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(true));
    ctxt->setContextProperty("testTopToBottom", QVariant(true));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview-enforcerange.qml"));
    qApp->processEvents();
    QVERIFY(canvas->rootObject() != 0);

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QTRY_COMPARE(gridview->preferredHighlightBegin(), 100.0);
    QTRY_COMPARE(gridview->preferredHighlightEnd(), 100.0);
    QTRY_COMPARE(gridview->highlightRangeMode(), QSGGridView::StrictlyEnforceRange);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // view should be positioned at the top of the range.
    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QTRY_VERIFY(item);
    QTRY_COMPARE(gridview->contentX(), -100.);
    QTRY_COMPARE(gridview->contentY(), 0.0);

    QSGText *name = findItem<QSGText>(contentItem, "textName", 0);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(0));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 0);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(0));

    // Check currentIndex is updated when contentItem moves
    gridview->setContentX(-200);
    QTRY_COMPARE(gridview->currentIndex(), 3);

    gridview->setCurrentIndex(7);
    QTRY_COMPARE(gridview->contentX(), -300.);
    QTRY_COMPARE(gridview->contentY(), 0.0);

    TestModel model2;
    for (int i = 0; i < 5; i++)
        model2.addItem("Item" + QString::number(i), "");

    ctxt->setContextProperty("testModel", &model2);
    QCOMPARE(gridview->count(), 5);

    delete canvas;
}

void tst_QSGGridView::QTBUG_8456()
{
    QSGView *canvas = createView();

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/setindex.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QTRY_COMPARE(gridview->currentIndex(), 0);

    delete canvas;
}

void tst_QSGGridView::manualHighlight()
{
    QSGView *canvas = createView();

    QString filename(SRCDIR "/data/manual-highlight.qml");
    canvas->setSource(QUrl::fromLocalFile(filename));

    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QTRY_COMPARE(gridview->currentIndex(), 0);
    QTRY_COMPARE(gridview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 0));
    QTRY_COMPARE(gridview->highlightItem()->y() - 5, gridview->currentItem()->y());
    QTRY_COMPARE(gridview->highlightItem()->x() - 5, gridview->currentItem()->x());

    gridview->setCurrentIndex(2);

    QTRY_COMPARE(gridview->currentIndex(), 2);
    QTRY_COMPARE(gridview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 2));
    QTRY_COMPARE(gridview->highlightItem()->y() - 5, gridview->currentItem()->y());
    QTRY_COMPARE(gridview->highlightItem()->x() - 5, gridview->currentItem()->x());

    gridview->positionViewAtIndex(8, QSGGridView::Contain);

    QTRY_COMPARE(gridview->currentIndex(), 2);
    QTRY_COMPARE(gridview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 2));
    QTRY_COMPARE(gridview->highlightItem()->y() - 5, gridview->currentItem()->y());
    QTRY_COMPARE(gridview->highlightItem()->x() - 5, gridview->currentItem()->x());

    gridview->setFlow(QSGGridView::TopToBottom);
    QTRY_COMPARE(gridview->flow(), QSGGridView::TopToBottom);

    gridview->setCurrentIndex(0);
    QTRY_COMPARE(gridview->currentIndex(), 0);
    QTRY_COMPARE(gridview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 0));
    QTRY_COMPARE(gridview->highlightItem()->y() - 5, gridview->currentItem()->y());
    QTRY_COMPARE(gridview->highlightItem()->x() - 5, gridview->currentItem()->x());

    delete canvas;
}


void tst_QSGGridView::footer()
{
    QFETCH(QSGGridView::Flow, flow);
    QFETCH(Qt::LayoutDirection, layoutDirection);
    QFETCH(QPointF, initialFooterPos);
    QFETCH(QPointF, changedFooterPos);
    QFETCH(QPointF, initialContentPos);
    QFETCH(QPointF, changedContentPos);
    QFETCH(QPointF, firstDelegatePos);

    QSGView *canvas = createView();
    canvas->show();

    TestModel model;
    for (int i = 0; i < 7; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/footer.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);
    gridview->setFlow(flow);
    gridview->setLayoutDirection(layoutDirection);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGText *footer = findItem<QSGText>(contentItem, "footer");
    QVERIFY(footer);

    QVERIFY(footer == gridview->footerItem());

    QCOMPARE(footer->pos(), initialFooterPos);
    QCOMPARE(footer->width(), 100.);
    QCOMPARE(footer->height(), 30.);
    QCOMPARE(QPointF(gridview->contentX(), gridview->contentY()), initialContentPos);

    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->pos(), firstDelegatePos);

    if (flow == QSGGridView::LeftToRight) {
        // shrink by one row
        model.removeItem(2);
        QTRY_COMPARE(footer->y(), initialFooterPos.y() - gridview->cellHeight());
    } else {
        // shrink by one column
        model.removeItem(2);
        model.removeItem(3);
        if (layoutDirection == Qt::LeftToRight)
            QTRY_COMPARE(footer->x(), initialFooterPos.x() - gridview->cellWidth());
        else
            QTRY_COMPARE(footer->x(), initialFooterPos.x() + gridview->cellWidth());
    }

    // remove all items
    model.clear();

    QPointF posWhenNoItems(0, 0);
    if (layoutDirection == Qt::RightToLeft)
        posWhenNoItems.setX(flow == QSGGridView::LeftToRight ? gridview->width() - footer->width() : -footer->width());
    QTRY_COMPARE(footer->pos(), posWhenNoItems);

    // if header is present, it's at a negative pos, so the footer should not move
    canvas->rootObject()->setProperty("showHeader", true);
    QVERIFY(findItem<QSGItem>(contentItem, "header") != 0);
    QTRY_COMPARE(footer->pos(), posWhenNoItems);
    canvas->rootObject()->setProperty("showHeader", false);

    // add 30 items
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QSignalSpy footerItemSpy(gridview, SIGNAL(footerItemChanged()));
    QMetaObject::invokeMethod(canvas->rootObject(), "changeFooter");

    QCOMPARE(footerItemSpy.count(), 1);

    footer = findItem<QSGText>(contentItem, "footer");
    QVERIFY(!footer);
    footer = findItem<QSGText>(contentItem, "footer2");
    QVERIFY(footer);

    QVERIFY(footer == gridview->footerItem());

    QCOMPARE(footer->pos(), changedFooterPos);
    QCOMPARE(footer->width(), 50.);
    QCOMPARE(footer->height(), 20.);
    QTRY_COMPARE(QPointF(gridview->contentX(), gridview->contentY()), changedContentPos);

    item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->pos(), firstDelegatePos);

    delete canvas;
}

void tst_QSGGridView::footer_data()
{
    QTest::addColumn<QSGGridView::Flow>("flow");
    QTest::addColumn<Qt::LayoutDirection>("layoutDirection");
    QTest::addColumn<QPointF>("initialFooterPos");
    QTest::addColumn<QPointF>("changedFooterPos");
    QTest::addColumn<QPointF>("initialContentPos");
    QTest::addColumn<QPointF>("changedContentPos");
    QTest::addColumn<QPointF>("firstDelegatePos");

    // footer1 = 100 x 30
    // footer2 = 100 x 20
    // cells = 80 * 60
    // view width = 240

    // footer below items, bottom left
    QTest::newRow("flow left to right") << QSGGridView::LeftToRight << Qt::LeftToRight
        << QPointF(0, 3 * 60)  // 180 = height of 3 rows (cell height is 60)
        << QPointF(0, 10 * 60)  // 30 items = 10 rows
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(0, 0);

    // footer below items, bottom right
    QTest::newRow("flow left to right, layout right to left") << QSGGridView::LeftToRight << Qt::RightToLeft
        << QPointF(240 - 100, 3 * 60)
        << QPointF((240 - 100) + 50, 10 * 60)     // 50 = width diff between old and new footers
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(240 - 80, 0);

    // footer to right of items
    QTest::newRow("flow top to bottom, layout left to right") << QSGGridView::TopToBottom << Qt::LeftToRight
        << QPointF(2 * 80, 0)      // 2 columns, cell width 80
        << QPointF(6 * 80, 0)      // 30 items = 6 columns
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(0, 0);

    // footer to left of items
    QTest::newRow("flow top to bottom, layout right to left") << QSGGridView::TopToBottom << Qt::RightToLeft
        << QPointF(-(2 * 80) - 100, 0)
        << QPointF(-(6 * 80) - 50, 0)     // 50 = new footer width
        << QPointF(-240, 0)
        << QPointF(-240, 0)    // unchanged, footer change doesn't change content pos
        << QPointF(-80, 0);
}

void tst_QSGGridView::header()
{
    QFETCH(QSGGridView::Flow, flow);
    QFETCH(Qt::LayoutDirection, layoutDirection);
    QFETCH(QPointF, initialHeaderPos);
    QFETCH(QPointF, changedHeaderPos);
    QFETCH(QPointF, initialContentPos);
    QFETCH(QPointF, changedContentPos);
    QFETCH(QPointF, firstDelegatePos);

    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/header.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);
    gridview->setFlow(flow);
    gridview->setLayoutDirection(layoutDirection);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGText *header = findItem<QSGText>(contentItem, "header");
    QVERIFY(header);

    QVERIFY(header == gridview->headerItem());

    QCOMPARE(header->pos(), initialHeaderPos);
    QCOMPARE(header->width(), 100.);
    QCOMPARE(header->height(), 30.);
    QCOMPARE(QPointF(gridview->contentX(), gridview->contentY()), initialContentPos);

    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->pos(), firstDelegatePos);

    model.clear();
    QCOMPARE(header->pos(), initialHeaderPos); // header should stay where it is

    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QSignalSpy headerItemSpy(gridview, SIGNAL(headerItemChanged()));
    QMetaObject::invokeMethod(canvas->rootObject(), "changeHeader");

    QCOMPARE(headerItemSpy.count(), 1);

    header = findItem<QSGText>(contentItem, "header");
    QVERIFY(!header);
    header = findItem<QSGText>(contentItem, "header2");
    QVERIFY(header);

    QVERIFY(header == gridview->headerItem());

    QCOMPARE(header->pos(), changedHeaderPos);
    QCOMPARE(header->width(), 50.);
    QCOMPARE(header->height(), 20.);
    QTRY_COMPARE(QPointF(gridview->contentX(), gridview->contentY()), changedContentPos);

    item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->pos(), firstDelegatePos);

    delete canvas;
}

void tst_QSGGridView::header_data()
{
    QTest::addColumn<QSGGridView::Flow>("flow");
    QTest::addColumn<Qt::LayoutDirection>("layoutDirection");
    QTest::addColumn<QPointF>("initialHeaderPos");
    QTest::addColumn<QPointF>("changedHeaderPos");
    QTest::addColumn<QPointF>("initialContentPos");
    QTest::addColumn<QPointF>("changedContentPos");
    QTest::addColumn<QPointF>("firstDelegatePos");

    // header1 = 100 x 30
    // header2 = 100 x 20
    // cells = 80 x 60
    // view width = 240

    // header above items, top left
    QTest::newRow("flow left to right") << QSGGridView::LeftToRight << Qt::LeftToRight
        << QPointF(0, -30)
        << QPointF(0, -20)
        << QPointF(0, -30)
        << QPointF(0, -20)
        << QPointF(0, 0);

    // header above items, top right
    QTest::newRow("flow left to right, layout right to left") << QSGGridView::LeftToRight << Qt::RightToLeft
        << QPointF(240 - 100, -30)
        << QPointF((240 - 100) + 50, -20)     // 50 = width diff between old and new headers
        << QPointF(0, -30)
        << QPointF(0, -20)
        << QPointF(160, 0);

    // header to left of items
    QTest::newRow("flow top to bottom, layout left to right") << QSGGridView::TopToBottom << Qt::LeftToRight
        << QPointF(-100, 0)
        << QPointF(-50, 0)
        << QPointF(-100, 0)
        << QPointF(-50, 0)
        << QPointF(0, 0);

    // header to right of items
    QTest::newRow("flow top to bottom, layout right to left") << QSGGridView::TopToBottom << Qt::RightToLeft
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(-(240 - 100), 0)
        << QPointF(-(240 - 50), 0)
        << QPointF(-80, 0);
}

void tst_QSGGridView::indexAt()
{
    QSGView *canvas = createView();

    TestModel model;
    model.addItem("Fred", "12345");
    model.addItem("John", "2345");
    model.addItem("Bob", "54321");
    model.addItem("Billy", "22345");
    model.addItem("Sam", "2945");
    model.addItem("Ben", "04321");
    model.addItem("Jim", "0780");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testRightToLeft", QVariant(false));
    ctxt->setContextProperty("testTopToBottom", QVariant(false));

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/gridview1.qml"));
    qApp->processEvents();

    QSGGridView *gridview = findItem<QSGGridView>(canvas->rootObject(), "grid");
    QTRY_VERIFY(gridview != 0);

    QSGItem *contentItem = gridview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QTRY_COMPARE(gridview->count(), model.count());

    QCOMPARE(gridview->indexAt(0, 0), 0);
    QCOMPARE(gridview->indexAt(79, 59), 0);
    QCOMPARE(gridview->indexAt(80, 0), 1);
    QCOMPARE(gridview->indexAt(0, 60), 3);
    QCOMPARE(gridview->indexAt(240, 0), -1);

    delete canvas;
}

void tst_QSGGridView::onAdd()
{
    QFETCH(int, initialItemCount);
    QFETCH(int, itemsToAdd);

    const int delegateWidth = 50;
    const int delegateHeight = 100;
    TestModel model;
    QSGView *canvas = createView();
    canvas->setFixedSize(5 * delegateWidth, 5 * delegateHeight); // just ensure all items fit

    // these initial items should not trigger GridView.onAdd
    for (int i=0; i<initialItemCount; i++)
        model.addItem("dummy value", "dummy value");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("delegateWidth", delegateWidth);
    ctxt->setContextProperty("delegateHeight", delegateHeight);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/attachedSignals.qml"));

    QObject *object = canvas->rootObject();
    object->setProperty("width", canvas->width());
    object->setProperty("height", canvas->height());
    qApp->processEvents();

    QList<QPair<QString, QString> > items;
    for (int i=0; i<itemsToAdd; i++)
        items << qMakePair(QString("value %1").arg(i), QString::number(i));
    model.addItems(items);

    qApp->processEvents();

    QVariantList result = object->property("addedDelegates").toList();
    QCOMPARE(result.count(), items.count());
    for (int i=0; i<items.count(); i++)
        QCOMPARE(result[i].toString(), items[i].first);

    delete canvas;
}

void tst_QSGGridView::onAdd_data()
{
    QTest::addColumn<int>("initialItemCount");
    QTest::addColumn<int>("itemsToAdd");

    QTest::newRow("0, add 1") << 0 << 1;
    QTest::newRow("0, add 2") << 0 << 2;
    QTest::newRow("0, add 10") << 0 << 10;

    QTest::newRow("1, add 1") << 1 << 1;
    QTest::newRow("1, add 2") << 1 << 2;
    QTest::newRow("1, add 10") << 1 << 10;

    QTest::newRow("5, add 1") << 5 << 1;
    QTest::newRow("5, add 2") << 5 << 2;
    QTest::newRow("5, add 10") << 5 << 10;
}

void tst_QSGGridView::onRemove()
{
    QFETCH(int, initialItemCount);
    QFETCH(int, indexToRemove);
    QFETCH(int, removeCount);

    const int delegateWidth = 50;
    const int delegateHeight = 100;
    TestModel model;
    for (int i=0; i<initialItemCount; i++)
        model.addItem(QString("value %1").arg(i), "dummy value");

    QSGView *canvas = createView();
    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("delegateWidth", delegateWidth);
    ctxt->setContextProperty("delegateHeight", delegateHeight);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/attachedSignals.qml"));
    QObject *object = canvas->rootObject();

    qApp->processEvents();

    model.removeItems(indexToRemove, removeCount);
    qApp->processEvents();
    QCOMPARE(object->property("removedDelegateCount"), QVariant(removeCount));

    delete canvas;
}

void tst_QSGGridView::onRemove_data()
{
    QTest::addColumn<int>("initialItemCount");
    QTest::addColumn<int>("indexToRemove");
    QTest::addColumn<int>("removeCount");

    QTest::newRow("remove first") << 1 << 0 << 1;
    QTest::newRow("two items, remove first") << 2 << 0 << 1;
    QTest::newRow("two items, remove last") << 2 << 1 << 1;
    QTest::newRow("two items, remove all") << 2 << 0 << 2;

    QTest::newRow("four items, remove first") << 4 << 0 << 1;
    QTest::newRow("four items, remove 0-2") << 4 << 0 << 2;
    QTest::newRow("four items, remove 1-3") << 4 << 1 << 2;
    QTest::newRow("four items, remove 2-4") << 4 << 2 << 2;
    QTest::newRow("four items, remove last") << 4 << 3 << 1;
    QTest::newRow("four items, remove all") << 4 << 0 << 4;

    QTest::newRow("ten items, remove 1-8") << 10 << 0 << 8;
    QTest::newRow("ten items, remove 2-7") << 10 << 2 << 5;
    QTest::newRow("ten items, remove 4-10") << 10 << 4 << 6;
}

void tst_QSGGridView::testQtQuick11Attributes()
{
    QFETCH(QString, code);
    QFETCH(QString, warning);
    QFETCH(QString, error);

    QDeclarativeEngine engine;
    QObject *obj;

    QDeclarativeComponent valid(&engine);
    valid.setData("import QtQuick 1.1; GridView { " + code.toUtf8() + " }", QUrl(""));
    obj = valid.create();
    QVERIFY(obj);
    QVERIFY(valid.errorString().isEmpty());
    delete obj;

    QDeclarativeComponent invalid(&engine);
    invalid.setData("import QtQuick 1.0; GridView { " + code.toUtf8() + " }", QUrl(""));
    QTest::ignoreMessage(QtWarningMsg, warning.toUtf8());
    obj = invalid.create();
    QCOMPARE(invalid.errorString(), error);
    delete obj;
}

void tst_QSGGridView::testQtQuick11Attributes_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<QString>("warning");
    QTest::addColumn<QString>("error");

    QTest::newRow("positionViewAtBeginning") << "Component.onCompleted: positionViewAtBeginning()"
        << "<Unknown File>:1: ReferenceError: Can't find variable: positionViewAtBeginning"
        << "";

    QTest::newRow("positionViewAtEnd") << "Component.onCompleted: positionViewAtEnd()"
        << "<Unknown File>:1: ReferenceError: Can't find variable: positionViewAtEnd"
        << "";
}

QSGView *tst_QSGGridView::createView()
{
    QSGView *canvas = new QSGView(0);
    canvas->setFixedSize(240,320);

    return canvas;
}

/*
   Find an item with the specified objectName.  If index is supplied then the
   item must also evaluate the {index} expression equal to index
*/
template<typename T>
T *tst_QSGGridView::findItem(QSGItem *parent, const QString &objectName, int index)
{
    const QMetaObject &mo = T::staticMetaObject;
    //qDebug() << parent->childItems().count() << "children";
    for (int i = 0; i < parent->childItems().count(); ++i) {
        QSGItem *item = qobject_cast<QSGItem*>(parent->childItems().at(i));
        if(!item)
            continue;
        //qDebug() << "try" << item;
        if (mo.cast(item) && (objectName.isEmpty() || item->objectName() == objectName)) {
            if (index != -1) {
                QDeclarativeContext *context = QDeclarativeEngine::contextForObject(item);
                if (context) {
                    if (context->contextProperty("index").toInt() == index) {
                        return static_cast<T*>(item);
                    }
                }
            } else {
                return static_cast<T*>(item);
            }
        }
        item = findItem<T>(item, objectName, index);
        if (item)
            return static_cast<T*>(item);
    }

    return 0;
}

template<typename T>
QList<T*> tst_QSGGridView::findItems(QSGItem *parent, const QString &objectName)
{
    QList<T*> items;
    const QMetaObject &mo = T::staticMetaObject;
    //qDebug() << parent->childItems().count() << "children";
    for (int i = 0; i < parent->childItems().count(); ++i) {
        QSGItem *item = qobject_cast<QSGItem*>(parent->childItems().at(i));
        if(!item)
            continue;
        //qDebug() << "try" << item;
        if (mo.cast(item) && (objectName.isEmpty() || item->objectName() == objectName)) {
            items.append(static_cast<T*>(item));
            //qDebug() << " found:" << item;
        }
        items += findItems<T>(item, objectName);
    }

    return items;
}

void tst_QSGGridView::dumpTree(QSGItem *parent, int depth)
{
    static QString padding("                       ");
    for (int i = 0; i < parent->childItems().count(); ++i) {
        QSGItem *item = qobject_cast<QSGItem*>(parent->childItems().at(i));
        if(!item)
            continue;
        QDeclarativeContext *context = QDeclarativeEngine::contextForObject(item);
        qDebug() << padding.left(depth*2) << item << (context ? context->contextProperty("index").toInt() : -1);
        dumpTree(item, depth+1);
    }
}


QTEST_MAIN(tst_QSGGridView)

#include "tst_qsggridview.moc"

