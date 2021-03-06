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
#include <QtGui/QStringListModel>
#include <QtDeclarative/qsgview.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecontext.h>
#include <QtDeclarative/qdeclarativeexpression.h>
#include <QtDeclarative/private/qsgitem_p.h>
#include <QtDeclarative/private/qsglistview_p.h>
#include <QtDeclarative/private/qsgtext_p.h>
#include <QtDeclarative/private/qsgvisualitemmodel_p.h>
#include <QtDeclarative/private/qdeclarativelistmodel_p.h>
#include <QtDeclarative/private/qlistmodelinterface_p.h>
#include "../../../shared/util.h"
#include "incrementalmodel.h"
#include <QtOpenGL/QGLShaderProgram>

#ifdef Q_OS_SYMBIAN
// In Symbian OS test data is located in applications private dir
#define SRCDIR "."
#endif

Q_DECLARE_METATYPE(Qt::LayoutDirection)
Q_DECLARE_METATYPE(QSGListView::Orientation)

class tst_QSGListView : public QObject
{
    Q_OBJECT
public:
    tst_QSGListView();

private slots:
    void initTestCase();
    void cleanupTestCase();
    // Test both QListModelInterface and QAbstractItemModel model types
    void qListModelInterface_items();
    void qAbstractItemModel_items();

    void qListModelInterface_changed();
    void qAbstractItemModel_changed();

    void qListModelInterface_inserted();
    void qAbstractItemModel_inserted();

    void qListModelInterface_removed();
    void qAbstractItemModel_removed();

    void qListModelInterface_moved();
    void qListModelInterface_moved_data();
    void qAbstractItemModel_moved();
    void qAbstractItemModel_moved_data();

    void qListModelInterface_clear();
    void qAbstractItemModel_clear();

    void swapWithFirstItem();
    void itemList();
    void currentIndex();
    void noCurrentIndex();
    void enforceRange();
    void enforceRange_withoutHighlight();
    void spacing();
    void sections();
    void sectionsDelegate();
    void cacheBuffer();
    void positionViewAtIndex();
    void resetModel();
    void propertyChanges();
    void componentChanges();
    void modelChanges();
    void QTBUG_9791();
    void manualHighlight();
    void QTBUG_11105();
    void header();
    void header_data();
    void header_delayItemCreation();
    void footer();
    void footer_data();
    void headerFooter();
    void resizeView();
    void sizeLessThan1();
    void QTBUG_14821();
    void resizeDelegate();
    void resizeFirstDelegate();
    void QTBUG_16037();
    void indexAt();
    void incrementalModel();
    void onAdd();
    void onAdd_data();
    void onRemove();
    void onRemove_data();
    void rightToLeft();
    void test_mirroring();

private:
    template <class T> void items();
    template <class T> void changed();
    template <class T> void inserted();
    template <class T> void removed(bool animated);
    template <class T> void moved();
    template <class T> void clear();
    QSGView *createView();
    template<typename T>
    T *findItem(QSGItem *parent, const QString &id, int index=-1);
    template<typename T>
    QList<T*> findItems(QSGItem *parent, const QString &objectName);
    void dumpTree(QSGItem *parent, int depth = 0);

    void moved_data();
};

void tst_QSGListView::initTestCase()
{
    QSGView canvas;
    if (!QGLShaderProgram::hasOpenGLShaderPrograms(canvas.context()))
        QSKIP("QSGListView needs OpenGL 2.0", SkipAll);
}

void tst_QSGListView::cleanupTestCase()
{

}
class TestObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool error READ error WRITE setError NOTIFY changedError)
    Q_PROPERTY(bool animate READ animate NOTIFY changedAnim)
    Q_PROPERTY(bool invalidHighlight READ invalidHighlight NOTIFY changedHl)
    Q_PROPERTY(int cacheBuffer READ cacheBuffer NOTIFY changedCacheBuffer)

public:
    TestObject(QObject *parent = 0)
        : QObject(parent), mError(true), mAnimate(false), mInvalidHighlight(false)
        , mCacheBuffer(0) {}

    bool error() const { return mError; }
    void setError(bool err) { mError = err; emit changedError(); }

    bool animate() const { return mAnimate; }
    void setAnimate(bool anim) { mAnimate = anim; emit changedAnim(); }

    bool invalidHighlight() const { return mInvalidHighlight; }
    void setInvalidHighlight(bool invalid) { mInvalidHighlight = invalid; emit changedHl(); }

    int cacheBuffer() const { return mCacheBuffer; }
    void setCacheBuffer(int buffer) { mCacheBuffer = buffer; emit changedCacheBuffer(); }

signals:
    void changedError();
    void changedAnim();
    void changedHl();
    void changedCacheBuffer();

public:
    bool mError;
    bool mAnimate;
    bool mInvalidHighlight;
    int mCacheBuffer;
};

template<typename T>
void tst_qsglistview_move(int from, int to, int n, T *items)
{
    if (from > to) {
        // Only move forwards - flip if backwards moving
        int tfrom = from;
        int tto = to;
        from = tto;
        to = tto+n;
        n = tfrom-tto;
    }
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

class TestModel : public QListModelInterface
{
    Q_OBJECT
public:
    TestModel(QObject *parent = 0) : QListModelInterface(parent) {}
    ~TestModel() {}

    enum Roles { Name, Number };

    QString name(int index) const { return list.at(index).first; }
    QString number(int index) const { return list.at(index).second; }

    int count() const { return list.count(); }

    QList<int> roles() const { return QList<int>() << Name << Number; }
    QString toString(int role) const {
        switch(role) {
        case Name:
            return "name";
        case Number:
            return "number";
        default:
            return "";
        }
    }

    QVariant data(int index, int role) const
    {
        if (role==0)
            return list.at(index).first;
        if (role==1)
            return list.at(index).second;
        return QVariant();
    }
    QHash<int, QVariant> data(int index, const QList<int> &roles) const {
        QHash<int,QVariant> returnHash;

        for (int i = 0; i < roles.size(); ++i) {
            int role = roles.at(i);
            QVariant info;
            switch(role) {
            case Name:
                info = list.at(index).first;
                break;
            case Number:
                info = list.at(index).second;
                break;
            default:
                break;
            }
            returnHash.insert(role, info);
        }
        return returnHash;
    }

    void addItem(const QString &name, const QString &number) {
        list.append(QPair<QString,QString>(name, number));
        emit itemsInserted(list.count()-1, 1);
    }

    void insertItem(int index, const QString &name, const QString &number) {
        list.insert(index, QPair<QString,QString>(name, number));
        emit itemsInserted(index, 1);
    }

    void removeItem(int index) {
        list.removeAt(index);
        emit itemsRemoved(index, 1);
    }

    void removeItems(int index, int count) {
        int c = count;
        while (c--)
            list.removeAt(index);
        emit itemsRemoved(index, count);
    }

    void moveItem(int from, int to) {
        list.move(from, to);
        emit itemsMoved(from, to, 1);
    }

    void moveItems(int from, int to, int count) {
        tst_qsglistview_move(from, to, count, &list);
        emit itemsMoved(from, to, count);
    }

    void modifyItem(int index, const QString &name, const QString &number) {
        list[index] = QPair<QString,QString>(name, number);
        emit itemsChanged(index, 1, roles());
    }

    void clear() {
        int count = list.count();
        list.clear();
        emit itemsRemoved(0, count);
    }

private:
    QList<QPair<QString,QString> > list;
};


class TestModel2 : public QAbstractListModel
{
public:
    enum Roles { Name = Qt::UserRole+1, Number = Qt::UserRole+2 };

    TestModel2(QObject *parent=0) : QAbstractListModel(parent) {
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
        tst_qsglistview_move(from, to, count, &list);
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

tst_QSGListView::tst_QSGListView()
{
}

template <class T>
void tst_QSGListView::items()
{
    QSGView *canvas = createView();

    T model;
    model.addItem("Fred", "12345");
    model.addItem("John", "2345");
    model.addItem("Bob", "54321");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QMetaObject::invokeMethod(canvas->rootObject(), "checkProperties");
    QTRY_VERIFY(testObject->error() == false);

    QTRY_VERIFY(listview->highlightItem() != 0);
    QTRY_COMPARE(listview->count(), model.count());
    QTRY_COMPARE(canvas->rootObject()->property("count").toInt(), model.count());
    QTRY_COMPARE(contentItem->childItems().count(), model.count()+1); // assumes all are visible, +1 for the (default) highlight item

    // current item should be first item
    QTRY_COMPARE(listview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 0));

    for (int i = 0; i < model.count(); ++i) {
        QSGText *name = findItem<QSGText>(contentItem, "textName", i);
        QTRY_VERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        QSGText *number = findItem<QSGText>(contentItem, "textNumber", i);
        QTRY_VERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));
    }

    // switch to other delegate
    testObject->setAnimate(true);
    QMetaObject::invokeMethod(canvas->rootObject(), "checkProperties");
    QTRY_VERIFY(testObject->error() == false);
    QTRY_VERIFY(listview->currentItem());

    // set invalid highlight
    testObject->setInvalidHighlight(true);
    QMetaObject::invokeMethod(canvas->rootObject(), "checkProperties");
    QTRY_VERIFY(testObject->error() == false);
    QTRY_VERIFY(listview->currentItem());
    QTRY_VERIFY(listview->highlightItem() == 0);

    // back to normal highlight
    testObject->setInvalidHighlight(false);
    QMetaObject::invokeMethod(canvas->rootObject(), "checkProperties");
    QTRY_VERIFY(testObject->error() == false);
    QTRY_VERIFY(listview->currentItem());
    QTRY_VERIFY(listview->highlightItem() != 0);

    // set an empty model and confirm that items are destroyed
    T model2;
    ctxt->setContextProperty("testModel", &model2);

    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    QTRY_VERIFY(itemCount == 0);

    QTRY_COMPARE(listview->highlightResizeSpeed(), 1000.0);
    QTRY_COMPARE(listview->highlightMoveSpeed(), 1000.0);

    delete canvas;
    delete testObject;
}


template <class T>
void tst_QSGListView::changed()
{
    QSGView *canvas = createView();

    T model;
    model.addItem("Fred", "12345");
    model.addItem("John", "2345");
    model.addItem("Bob", "54321");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGFlickable *listview = findItem<QSGFlickable>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    model.modifyItem(1, "Will", "9876");
    QSGText *name = findItem<QSGText>(contentItem, "textName", 1);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(1));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 1);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(1));

    delete canvas;
    delete testObject;
}

template <class T>
void tst_QSGListView::inserted()
{
    QSGView *canvas = createView();

    T model;
    model.addItem("Fred", "12345");
    model.addItem("John", "2345");
    model.addItem("Bob", "54321");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    model.insertItem(1, "Will", "9876");

    QTRY_COMPARE(contentItem->childItems().count(), model.count()+1); // assumes all are visible, +1 for the (default) highlight item

    QSGText *name = findItem<QSGText>(contentItem, "textName", 1);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(1));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 1);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(1));

    // Confirm items positioned correctly
    for (int i = 0; i < model.count(); ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QTRY_COMPARE(item->y(), i*20.0);
    }

    model.insertItem(0, "Foo", "1111"); // zero index, and current item

    QCOMPARE(canvas->rootObject()->property("count").toInt(), model.count());
    QTRY_COMPARE(contentItem->childItems().count(), model.count()+1); // assumes all are visible, +1 for the (default) highlight item

    name = findItem<QSGText>(contentItem, "textName", 0);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(0));
    number = findItem<QSGText>(contentItem, "textNumber", 0);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(0));

    QTRY_COMPARE(listview->currentIndex(), 1);

    // Confirm items positioned correctly
    for (int i = 0; i < model.count(); ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QTRY_COMPARE(item->y(), i*20.0);
    }

    for (int i = model.count(); i < 30; ++i)
        model.insertItem(i, "Hello", QString::number(i));

    listview->setContentY(80);

    // Insert item outside visible area
    model.insertItem(1, "Hello", "1324");

    QTRY_VERIFY(listview->contentY() == 80);

    // Confirm items positioned correctly
    for (int i = 5; i < 5+15; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.0 - 20.0);
    }

//    QTRY_COMPARE(listview->contentItemHeight(), model.count() * 20.0);

    // QTBUG-19675
    model.clear();
    model.insertItem(0, "Hello", "1234");
    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->y(), 0.);
    QVERIFY(listview->contentY() == 0);

    delete canvas;
    delete testObject;
}

template <class T>
void tst_QSGListView::removed(bool animated)
{
    QSGView *canvas = createView();

    T model;
    for (int i = 0; i < 50; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    canvas->show();
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    model.removeItem(1);
    QCOMPARE(canvas->rootObject()->property("count").toInt(), model.count());

    QSGText *name = findItem<QSGText>(contentItem, "textName", 1);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(1));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 1);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(1));

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->y() == i*20);
    }

    // Remove first item (which is the current item);
    model.removeItem(0);  // post: top item starts at 20

    QTest::qWait(300);

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
        QTRY_COMPARE(item->y(),i*20.0 + 20.0);
    }

    // Remove items not visible
    model.removeItem(18);
    qApp->processEvents();

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(),i*20.0+20.0);
    }

    // Remove items before visible
    listview->setContentY(80);
    listview->setCurrentIndex(10);

    model.removeItem(1); // post: top item will be at 40
    qApp->processEvents();

    // Confirm items positioned correctly
    for (int i = 2; i < 18; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(),40+i*20.0);
    }

    // Remove current index
    QTRY_VERIFY(listview->currentIndex() == 9);
    QSGItem *oldCurrent = listview->currentItem();
    model.removeItem(9);

    QTRY_COMPARE(listview->currentIndex(), 9);
    QTRY_VERIFY(listview->currentItem() != oldCurrent);

    listview->setContentY(40); // That's the top now
    // let transitions settle.
    QTest::qWait(300);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(),40+i*20.0);
    }

    // remove current item beyond visible items.
    listview->setCurrentIndex(20);
    listview->setContentY(40);
    model.removeItem(20);

    QTRY_COMPARE(listview->currentIndex(), 20);
    QTRY_VERIFY(listview->currentItem() != 0);

    // remove item before current, but visible
    listview->setCurrentIndex(8);
    oldCurrent = listview->currentItem();
    model.removeItem(6);

    QTRY_COMPARE(listview->currentIndex(), 7);
    QTRY_VERIFY(listview->currentItem() == oldCurrent);

    listview->setContentY(80);
    QTest::qWait(300);

    // remove all visible items
    model.removeItems(1, 18);
    QTest::qWait(300);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i+2);
        if (!item) qWarning() << "Item" << i+2 << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(),80+i*20.0);
    }

    model.removeItems(1, 17);
//    QTest::qWait(300);

    model.removeItems(2, 1);
    model.addItem("New", "1");

    QTRY_VERIFY(name = findItem<QSGText>(contentItem, "textName", model.count()-1));
    QCOMPARE(name->text(), QString("New"));

    // Add some more items so that we don't run out
    model.clear();
    for (int i = 0; i < 50; i++)
        model.addItem("Item" + QString::number(i), "");

    // QTBUG-QTBUG-20575
    listview->setCurrentIndex(0);
    listview->setContentY(30);
    model.removeItem(0);
    QTRY_VERIFY(name = findItem<QSGText>(contentItem, "textName", 0));

    // QTBUG-19198 move to end and remove all visible items one at a time.
    listview->positionViewAtEnd();
    for (int i = 0; i < 18; ++i)
        model.removeItems(model.count() - 1, 1);
    QTRY_VERIFY(findItems<QSGItem>(contentItem, "wrapper").count() > 16);

    delete canvas;
    delete testObject;
}

template <class T>
void tst_QSGListView::clear()
{
    QSGView *canvas = createView();

    T model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    model.clear();

    QTRY_VERIFY(listview->count() == 0);
    QTRY_VERIFY(listview->currentItem() == 0);
    QTRY_VERIFY(listview->contentY() == 0);
    QVERIFY(listview->currentIndex() == -1);

    // confirm sanity when adding an item to cleared list
    model.addItem("New", "1");
    QTRY_VERIFY(listview->count() == 1);
    QVERIFY(listview->currentItem() != 0);
    QVERIFY(listview->currentIndex() == 0);

    delete canvas;
    delete testObject;
}

template <class T>
void tst_QSGListView::moved()
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

    T model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGItem *currentItem = listview->currentItem();
    QTRY_VERIFY(currentItem != 0);

    listview->setContentY(contentY);
    model.moveItems(from, to, count);
    qApp->processEvents();

    // Confirm items positioned correctly and indexes correct
    int firstVisibleIndex = qCeil(contentY / 20.0);
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();

    for (int i = firstVisibleIndex; i < model.count() && i < itemCount; ++i) {
        if (i >= firstVisibleIndex + 16)    // index has moved out of view
            continue;
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY2(item, QTest::toString(QString("Item %1 not found").arg(i)));
        QTRY_COMPARE(item->y(), i*20.0 + itemsOffsetAfterMove);
        name = findItem<QSGText>(contentItem, "textName", i);
        QVERIFY(name != 0);
        QTRY_COMPARE(name->text(), model.name(i));
        number = findItem<QSGText>(contentItem, "textNumber", i);
        QVERIFY(number != 0);
        QTRY_COMPARE(number->text(), model.number(i));

        // current index should have been updated
        if (item == currentItem)
            QTRY_COMPARE(listview->currentIndex(), i);
    }

    delete canvas;
    delete testObject;
}

void tst_QSGListView::moved_data()
{
    QTest::addColumn<qreal>("contentY");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");
    QTest::addColumn<int>("count");
    QTest::addColumn<qreal>("itemsOffsetAfterMove");

    // model starts with 30 items, each 20px high, in area 320px high
    // 16 items should be visible at a time
    // itemsOffsetAfterMove should be > 0 whenever items above the visible pos have moved

    QTest::newRow("move 1 forwards, within visible items")
            << 0.0
            << 1 << 4 << 1
            << 0.0;

    QTest::newRow("move 1 forwards, from non-visible -> visible")
            << 80.0     // show 4-19
            << 1 << 18 << 1
            << 20.0;

    QTest::newRow("move 1 forwards, from non-visible -> visible (move first item)")
            << 80.0     // show 4-19
            << 0 << 4 << 1
            << 20.0;

    QTest::newRow("move 1 forwards, from visible -> non-visible")
            << 0.0
            << 1 << 16 << 1
            << 0.0;

    QTest::newRow("move 1 forwards, from visible -> non-visible (move first item)")
            << 0.0
            << 0 << 16 << 1
            << 20.0;


    QTest::newRow("move 1 backwards, within visible items")
            << 0.0
            << 4 << 1 << 1
            << 0.0;

    QTest::newRow("move 1 backwards, from non-visible -> visible")
            << 0.0
            << 20 << 4 << 1
            << 0.0;

    QTest::newRow("move 1 backwards, from non-visible -> visible (move last item)")
            << 0.0
            << 29 << 15 << 1
            << 0.0;

    QTest::newRow("move 1 backwards, from visible -> non-visible")
            << 80.0     // show 4-19
            << 16 << 1 << 1
            << 20.0 * 15;   // this results in a forward movement that removes 15 items

    QTest::newRow("move 1 backwards, from visible -> non-visible (move first item)")
            << 80.0     // show 4-19
            << 16 << 0 << 1
            << 20.0 * 16;   // everything should move to after item 16


    QTest::newRow("move multiple forwards, within visible items")
            << 0.0
            << 0 << 5 << 3
            << 20.0 * 3;

    QTest::newRow("move multiple forwards, from non-visible -> visible")
            << 80.0     // show 4-19
            << 1 << 5 << 3
            << 20.0 * 3;    // moving 3 from above the content y should adjust y positions accordingly

    QTest::newRow("move multiple forwards, from non-visible -> visible (move first item)")
            << 80.0     // show 4-19
            << 0 << 5 << 3
            << 20.0 * 3;        // moving 3 from above the content y should adjust y positions accordingly

    QTest::newRow("move multiple forwards, from visible -> non-visible")
            << 0.0
            << 1 << 16 << 3
            << 0.0;

    QTest::newRow("move multiple forwards, from visible -> non-visible (move first item)")
            << 0.0
            << 0 << 16 << 3
            << 20.0 * 3;


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
            << 80.0     // show 4-19
            << 16 << 1 << 3
            << 20.0 * 15;   // this results in a forward movement that removes 15 items

    QTest::newRow("move multiple backwards, from visible -> non-visible (move first item)")
            << 80.0     // show 4-19
            << 16 << 0 << 3
            << 20.0 * 16;
}

void tst_QSGListView::swapWithFirstItem()
{
    QSGView *canvas = createView();
    canvas->show();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    // ensure content position is stable
    listview->setContentY(0);
    model.moveItem(1, 0);
    QTRY_VERIFY(listview->contentY() == 0);

    delete testObject;
    delete canvas;
}

void tst_QSGListView::enforceRange()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listview-enforcerange.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QTRY_COMPARE(listview->preferredHighlightBegin(), 100.0);
    QTRY_COMPARE(listview->preferredHighlightEnd(), 100.0);
    QTRY_COMPARE(listview->highlightRangeMode(), QSGListView::StrictlyEnforceRange);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // view should be positioned at the top of the range.
    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QTRY_VERIFY(item);
    QTRY_COMPARE(listview->contentY(), -100.0);

    QSGText *name = findItem<QSGText>(contentItem, "textName", 0);
    QTRY_VERIFY(name != 0);
    QTRY_COMPARE(name->text(), model.name(0));
    QSGText *number = findItem<QSGText>(contentItem, "textNumber", 0);
    QTRY_VERIFY(number != 0);
    QTRY_COMPARE(number->text(), model.number(0));

    // Check currentIndex is updated when contentItem moves
    listview->setContentY(20);

    QTRY_COMPARE(listview->currentIndex(), 6);

    // change model
    TestModel model2;
    for (int i = 0; i < 5; i++)
        model2.addItem("Item" + QString::number(i), "");

    ctxt->setContextProperty("testModel", &model2);
    QCOMPARE(listview->count(), 5);

    delete canvas;
}

void tst_QSGListView::enforceRange_withoutHighlight()
{
    // QTBUG-20287
    // If no highlight is set but StrictlyEnforceRange is used, the content should still move
    // to the correct position (i.e. to the next/previous item, not next/previous section)
    // when moving up/down via incrementCurrentIndex() and decrementCurrentIndex()

    QSGView *canvas = createView();
    canvas->show();

    TestModel model;
    model.addItem("Item 0", "a");
    model.addItem("Item 1", "b");
    model.addItem("Item 2", "b");
    model.addItem("Item 3", "c");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listview-enforcerange-nohighlight.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    qreal expectedPos = -100.0;

    expectedPos += 10.0;    // scroll past 1st section's delegate (10px height)
    QTRY_COMPARE(listview->contentY(), expectedPos);

    expectedPos += 20 + 10;     // scroll past 1st section and section delegate of 2nd section
    QTest::keyClick(canvas, Qt::Key_Down);
    QTRY_COMPARE(listview->contentY(), expectedPos);

    expectedPos += 20;     // scroll past 1st item of 2nd section
    QTest::keyClick(canvas, Qt::Key_Down);
    QTRY_COMPARE(listview->contentY(), expectedPos);

    expectedPos += 20 + 10;     // scroll past 2nd item of 2nd section and section delegate of 3rd section
    QTest::keyClick(canvas, Qt::Key_Down);
    QTRY_COMPARE(listview->contentY(), expectedPos);

    delete canvas;
}

void tst_QSGListView::spacing()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->y() == i*20);
    }

    listview->setSpacing(10);
    QTRY_VERIFY(listview->spacing() == 10);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->y() == i*30);
    }

    listview->setSpacing(0);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.0);
    }

    delete canvas;
    delete testObject;
}

void tst_QSGListView::sections()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), QString::number(i/5));

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listview-sections.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), qreal(i*20 + ((i+4)/5) * 20));
        QSGText *next = findItem<QSGText>(item, "nextSection");
        QCOMPARE(next->text().toInt(), (i+1)/5);
    }

    QSignalSpy currentSectionChangedSpy(listview, SIGNAL(currentSectionChanged()));

    // Remove section boundary
    model.removeItem(5);

    // New section header created
    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 5);
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->height(), 40.0);

    model.insertItem(3, "New Item", "0");

    // Section header moved
    item = findItem<QSGItem>(contentItem, "wrapper", 5);
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->height(), 20.0);

    item = findItem<QSGItem>(contentItem, "wrapper", 6);
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->height(), 40.0);

    // insert item which will become a section header
    model.insertItem(6, "Replace header", "1");

    item = findItem<QSGItem>(contentItem, "wrapper", 6);
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->height(), 40.0);

    item = findItem<QSGItem>(contentItem, "wrapper", 7);
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->height(), 20.0);

    QTRY_COMPARE(listview->currentSection(), QString("0"));

    listview->setContentY(140);
    QTRY_COMPARE(listview->currentSection(), QString("1"));

    QTRY_COMPARE(currentSectionChangedSpy.count(), 1);

    listview->setContentY(20);
    QTRY_COMPARE(listview->currentSection(), QString("0"));

    QTRY_COMPARE(currentSectionChangedSpy.count(), 2);

    item = findItem<QSGItem>(contentItem, "wrapper", 1);
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->height(), 20.0);

    // check that headers change when item changes
    listview->setContentY(0);
    model.modifyItem(0, "changed", "2");

    item = findItem<QSGItem>(contentItem, "wrapper", 1);
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->height(), 40.0);

    delete canvas;
}

void tst_QSGListView::sectionsDelegate()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), QString::number(i/5));

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listview-sections_delegate.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), qreal(i*20 + ((i+5)/5) * 20));
        QSGText *next = findItem<QSGText>(item, "nextSection");
        QCOMPARE(next->text().toInt(), (i+1)/5);
    }

    for (int i = 0; i < 3; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "sect_" + QString::number(i));
        QVERIFY(item);
        QTRY_COMPARE(item->y(), qreal(i*20*6));
    }

    model.modifyItem(0, "One", "aaa");
    model.modifyItem(1, "Two", "aaa");
    model.modifyItem(2, "Three", "aaa");
    model.modifyItem(3, "Four", "aaa");
    model.modifyItem(4, "Five", "aaa");

    for (int i = 0; i < 3; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem,
                "sect_" + (i == 0 ? QString("aaa") : QString::number(i)));
        QVERIFY(item);
        QTRY_COMPARE(item->y(), qreal(i*20*6));
    }

    // remove section boundary
    model.removeItem(5);
    qApp->processEvents();
    for (int i = 0; i < 3; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem,
                "sect_" + (i == 0 ? QString("aaa") : QString::number(i)));
        QVERIFY(item);
    }

    // QTBUG-17606
    QList<QSGItem*> items = findItems<QSGItem>(contentItem, "sect_1");
    QCOMPARE(items.count(), 1);

    // QTBUG-17759
    model.modifyItem(0, "One", "aaa");
    model.modifyItem(1, "One", "aaa");
    model.modifyItem(2, "One", "aaa");
    model.modifyItem(3, "Four", "aaa");
    model.modifyItem(4, "Four", "aaa");
    model.modifyItem(5, "Four", "aaa");
    model.modifyItem(6, "Five", "aaa");
    model.modifyItem(7, "Five", "aaa");
    model.modifyItem(8, "Five", "aaa");
    model.modifyItem(9, "Two", "aaa");
    model.modifyItem(10, "Two", "aaa");
    model.modifyItem(11, "Two", "aaa");
    QTRY_COMPARE(findItems<QSGItem>(contentItem, "sect_aaa").count(), 1);
    canvas->rootObject()->setProperty("sectionProperty", "name");
    // ensure view has settled.
    QTRY_COMPARE(findItems<QSGItem>(contentItem, "sect_Four").count(), 1);
    for (int i = 0; i < 4; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem,
                "sect_" + model.name(i*3));
        QVERIFY(item);
        QTRY_COMPARE(item->y(), qreal(i*20*4));
    }

    // QTBUG-17769
    model.removeItems(10, 20);
    // ensure view has settled.
    QTRY_COMPARE(findItems<QSGItem>(contentItem, "wrapper").count(), 10);
    // Drag view up beyond bounds
    QTest::mousePress(canvas, Qt::LeftButton, 0, QPoint(20,20));
    {
        QMouseEvent mv(QEvent::MouseMove, QPoint(20,0), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(canvas, &mv);
    }
    {
        QMouseEvent mv(QEvent::MouseMove, QPoint(20,-50), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(canvas, &mv);
    }
    {
        QMouseEvent mv(QEvent::MouseMove, QPoint(20,-200), Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(canvas, &mv);
    }
    QTest::mouseRelease(canvas, Qt::LeftButton, 0, QPoint(20,-200));
    // view should settle back at 0
    QTRY_COMPARE(listview->contentY(), 0.0);

    delete canvas;
}

void tst_QSGListView::currentIndex()
{
    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), QString::number(i));

    QSGView *canvas = new QSGView(0);
    canvas->setFixedSize(240,320);

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("testWrap", QVariant(false));

    QString filename(SRCDIR "/data/listview-initCurrent.qml");
    canvas->setSource(QUrl::fromLocalFile(filename));

    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // current item should be 20th item at startup
    // and current item should be in view
    QCOMPARE(listview->currentIndex(), 20);
    QCOMPARE(listview->contentY(), 100.0);
    QCOMPARE(listview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 20));
    QCOMPARE(listview->highlightItem()->y(), listview->currentItem()->y());

    // no wrap
    listview->setCurrentIndex(0);
    QCOMPARE(listview->currentIndex(), 0);
    // confirm that the velocity is updated
    QTRY_VERIFY(listview->verticalVelocity() != 0.0);

    listview->incrementCurrentIndex();
    QCOMPARE(listview->currentIndex(), 1);
    listview->decrementCurrentIndex();
    QCOMPARE(listview->currentIndex(), 0);

    listview->decrementCurrentIndex();
    QCOMPARE(listview->currentIndex(), 0);

    // with wrap
    ctxt->setContextProperty("testWrap", QVariant(true));
    QVERIFY(listview->isWrapEnabled());

    listview->decrementCurrentIndex();
    QCOMPARE(listview->currentIndex(), model.count()-1);

    QTRY_COMPARE(listview->contentY(), 280.0);

    listview->incrementCurrentIndex();
    QCOMPARE(listview->currentIndex(), 0);

    QTRY_COMPARE(listview->contentY(), 0.0);


    // footer should become visible if it is out of view, and then current index is set to count-1
    canvas->rootObject()->setProperty("showFooter", true);
    QTRY_VERIFY(listview->footerItem());
    listview->setCurrentIndex(model.count()-2);
    QTRY_VERIFY(listview->footerItem()->y() > listview->contentY() + listview->height());
    listview->setCurrentIndex(model.count()-1);
    QTRY_COMPARE(listview->contentY() + listview->height(), (20.0 * model.count()) + listview->footerItem()->height());
    canvas->rootObject()->setProperty("showFooter", false);

    // header should become visible if it is out of view, and then current index is set to 0
    canvas->rootObject()->setProperty("showHeader", true);
    QTRY_VERIFY(listview->headerItem());
    listview->setCurrentIndex(1);
    QTRY_VERIFY(listview->headerItem()->y() + listview->headerItem()->height() < listview->contentY());
    listview->setCurrentIndex(0);
    QTRY_COMPARE(listview->contentY(), -listview->headerItem()->height());
    canvas->rootObject()->setProperty("showHeader", false);


    // Test keys
    canvas->show();
    qApp->setActiveWindow(canvas);
#ifdef Q_WS_X11
    // to be safe and avoid failing setFocus with window managers
    qt_x11_wait_for_window_manager(canvas);
#endif
    QTRY_VERIFY(canvas->hasFocus());
    qApp->processEvents();

    listview->setCurrentIndex(0);

    QTest::keyClick(canvas, Qt::Key_Down);
    QCOMPARE(listview->currentIndex(), 1);

    QTest::keyClick(canvas, Qt::Key_Up);
    QCOMPARE(listview->currentIndex(), 0);

    // hold down Key_Down
    for (int i=0; i<model.count()-1; i++) {
        QTest::simulateEvent(canvas, true, Qt::Key_Down, Qt::NoModifier, "", true);
        QTRY_COMPARE(listview->currentIndex(), i+1);
    }
    QTest::keyRelease(canvas, Qt::Key_Down);
    QTRY_COMPARE(listview->currentIndex(), model.count()-1);
    QTRY_COMPARE(listview->contentY(), 280.0);

    // hold down Key_Up
    for (int i=model.count()-1; i > 0; i--) {
        QTest::simulateEvent(canvas, true, Qt::Key_Up, Qt::NoModifier, "", true);
        QTRY_COMPARE(listview->currentIndex(), i-1);
    }
    QTest::keyRelease(canvas, Qt::Key_Up);
    QTRY_COMPARE(listview->currentIndex(), 0);
    QTRY_COMPARE(listview->contentY(), 0.0);


    // turn off auto highlight
    listview->setHighlightFollowsCurrentItem(false);
    QVERIFY(listview->highlightFollowsCurrentItem() == false);

    QVERIFY(listview->highlightItem());
    qreal hlPos = listview->highlightItem()->y();

    listview->setCurrentIndex(4);
    QTRY_COMPARE(listview->highlightItem()->y(), hlPos);

    // insert item before currentIndex
    listview->setCurrentIndex(28);
    model.insertItem(0, "Foo", "1111");
    QTRY_COMPARE(canvas->rootObject()->property("current").toInt(), 29);

    // check removing highlight by setting currentIndex to -1;
    listview->setCurrentIndex(-1);

    QCOMPARE(listview->currentIndex(), -1);
    QVERIFY(!listview->highlightItem());
    QVERIFY(!listview->currentItem());

    delete canvas;
}

void tst_QSGListView::noCurrentIndex()
{
    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), QString::number(i));

    QSGView *canvas = new QSGView(0);
    canvas->setFixedSize(240,320);

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    QString filename(SRCDIR "/data/listview-noCurrent.qml");
    canvas->setSource(QUrl::fromLocalFile(filename));

    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // current index should be -1 at startup
    // and we should not have a currentItem or highlightItem
    QCOMPARE(listview->currentIndex(), -1);
    QCOMPARE(listview->contentY(), 0.0);
    QVERIFY(!listview->highlightItem());
    QVERIFY(!listview->currentItem());

    listview->setCurrentIndex(2);
    QCOMPARE(listview->currentIndex(), 2);
    QVERIFY(listview->highlightItem());
    QVERIFY(listview->currentItem());

    delete canvas;
}

void tst_QSGListView::itemList()
{
    QSGView *canvas = createView();

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/itemlist.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "view");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGVisualItemModel *model = canvas->rootObject()->findChild<QSGVisualItemModel*>("itemModel");
    QTRY_VERIFY(model != 0);

    QTRY_VERIFY(model->count() == 3);
    QTRY_COMPARE(listview->currentIndex(), 0);

    QSGItem *item = findItem<QSGItem>(contentItem, "item1");
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->x(), 0.0);
    QCOMPARE(item->height(), listview->height());

    QSGText *text = findItem<QSGText>(contentItem, "text1");
    QTRY_VERIFY(text);
    QTRY_COMPARE(text->text(), QLatin1String("index: 0"));

    listview->setCurrentIndex(2);

    item = findItem<QSGItem>(contentItem, "item3");
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->x(), 480.0);

    text = findItem<QSGText>(contentItem, "text3");
    QTRY_VERIFY(text);
    QTRY_COMPARE(text->text(), QLatin1String("index: 2"));

    delete canvas;
}

void tst_QSGListView::cacheBuffer()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);
    QTRY_VERIFY(listview->delegate() != 0);
    QTRY_VERIFY(listview->model() != 0);
    QTRY_VERIFY(listview->highlight() != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->y() == i*20);
    }

    testObject->setCacheBuffer(400);
    QTRY_VERIFY(listview->cacheBuffer() == 400);

    int newItemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    QTRY_VERIFY(newItemCount > itemCount);

    // Confirm items positioned correctly
    for (int i = 0; i < model.count() && i < newItemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->y() == i*20);
    }

    delete canvas;
    delete testObject;
}

void tst_QSGListView::positionViewAtIndex()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 40; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.);
    }

    // Position on a currently visible item
    listview->positionViewAtIndex(3, QSGListView::Beginning);
    QTRY_COMPARE(listview->contentY(), 60.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 3; i < model.count() && i < itemCount-3-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.);
    }

    // Position on an item beyond the visible items
    listview->positionViewAtIndex(22, QSGListView::Beginning);
    QTRY_COMPARE(listview->contentY(), 440.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 22; i < model.count() && i < itemCount-22-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.);
    }

    // Position on an item that would leave empty space if positioned at the top
    listview->positionViewAtIndex(28, QSGListView::Beginning);
    QTRY_COMPARE(listview->contentY(), 480.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 24; i < model.count() && i < itemCount-24-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.);
    }

    // Position at the beginning again
    listview->positionViewAtIndex(0, QSGListView::Beginning);
    QTRY_COMPARE(listview->contentY(), 0.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount-1; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.);
    }

    // Position at End using last index
    listview->positionViewAtIndex(model.count()-1, QSGListView::End);
    QTRY_COMPARE(listview->contentY(), 480.);

    // Confirm items positioned correctly
    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 24; i < model.count(); ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.);
    }

    // Position at End
    listview->positionViewAtIndex(20, QSGListView::End);
    QTRY_COMPARE(listview->contentY(), 100.);

    // Position in Center
    listview->positionViewAtIndex(15, QSGListView::Center);
    QTRY_COMPARE(listview->contentY(), 150.);

    // Ensure at least partially visible
    listview->positionViewAtIndex(15, QSGListView::Visible);
    QTRY_COMPARE(listview->contentY(), 150.);

    listview->setContentY(302);
    listview->positionViewAtIndex(15, QSGListView::Visible);
    QTRY_COMPARE(listview->contentY(), 302.);

    listview->setContentY(320);
    listview->positionViewAtIndex(15, QSGListView::Visible);
    QTRY_COMPARE(listview->contentY(), 300.);

    listview->setContentY(85);
    listview->positionViewAtIndex(20, QSGListView::Visible);
    QTRY_COMPARE(listview->contentY(), 85.);

    listview->setContentY(75);
    listview->positionViewAtIndex(20, QSGListView::Visible);
    QTRY_COMPARE(listview->contentY(), 100.);

    // Ensure completely visible
    listview->setContentY(120);
    listview->positionViewAtIndex(20, QSGListView::Contain);
    QTRY_COMPARE(listview->contentY(), 120.);

    listview->setContentY(302);
    listview->positionViewAtIndex(15, QSGListView::Contain);
    QTRY_COMPARE(listview->contentY(), 300.);

    listview->setContentY(85);
    listview->positionViewAtIndex(20, QSGListView::Contain);
    QTRY_COMPARE(listview->contentY(), 100.);

    // positionAtBeginnging
    listview->positionViewAtBeginning();
    QTRY_COMPARE(listview->contentY(), 0.);

    listview->setContentY(80);
    canvas->rootObject()->setProperty("showHeader", true);
    listview->positionViewAtBeginning();
    QTRY_COMPARE(listview->contentY(), -30.);

    // positionAtEnd
    listview->positionViewAtEnd();
    QTRY_COMPARE(listview->contentY(), 480.); // 40*20 - 320

    listview->setContentY(80);
    canvas->rootObject()->setProperty("showFooter", true);
    listview->positionViewAtEnd();
    QTRY_COMPARE(listview->contentY(), 510.);

    // set current item to outside visible view, position at beginning
    // and ensure highlight moves to current item
    listview->setCurrentIndex(1);
    listview->positionViewAtBeginning();
    QTRY_COMPARE(listview->contentY(), -30.);
    QVERIFY(listview->highlightItem());
    QCOMPARE(listview->highlightItem()->y(), 20.);

    delete canvas;
    delete testObject;
}

void tst_QSGListView::resetModel()
{
    QSGView *canvas = createView();

    QStringList strings;
    strings << "one" << "two" << "three";
    QStringListModel model(strings);

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/displaylist.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QTRY_COMPARE(listview->count(), model.rowCount());

    for (int i = 0; i < model.rowCount(); ++i) {
        QSGText *display = findItem<QSGText>(contentItem, "displayText", i);
        QTRY_VERIFY(display != 0);
        QTRY_COMPARE(display->text(), strings.at(i));
    }

    strings.clear();
    strings << "four" << "five" << "six" << "seven";
    model.setStringList(strings);

    QTRY_COMPARE(listview->count(), model.rowCount());

    for (int i = 0; i < model.rowCount(); ++i) {
        QSGText *display = findItem<QSGText>(contentItem, "displayText", i);
        QTRY_VERIFY(display != 0);
        QTRY_COMPARE(display->text(), strings.at(i));
    }

    delete canvas;
}

void tst_QSGListView::propertyChanges()
{
    QSGView *canvas = createView();
    QTRY_VERIFY(canvas);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/propertychangestest.qml"));

    QSGListView *listView = canvas->rootObject()->findChild<QSGListView*>("listView");
    QTRY_VERIFY(listView);

    QSignalSpy highlightFollowsCurrentItemSpy(listView, SIGNAL(highlightFollowsCurrentItemChanged()));
    QSignalSpy preferredHighlightBeginSpy(listView, SIGNAL(preferredHighlightBeginChanged()));
    QSignalSpy preferredHighlightEndSpy(listView, SIGNAL(preferredHighlightEndChanged()));
    QSignalSpy highlightRangeModeSpy(listView, SIGNAL(highlightRangeModeChanged()));
    QSignalSpy keyNavigationWrapsSpy(listView, SIGNAL(keyNavigationWrapsChanged()));
    QSignalSpy cacheBufferSpy(listView, SIGNAL(cacheBufferChanged()));
    QSignalSpy snapModeSpy(listView, SIGNAL(snapModeChanged()));

    QTRY_COMPARE(listView->highlightFollowsCurrentItem(), true);
    QTRY_COMPARE(listView->preferredHighlightBegin(), 0.0);
    QTRY_COMPARE(listView->preferredHighlightEnd(), 0.0);
    QTRY_COMPARE(listView->highlightRangeMode(), QSGListView::ApplyRange);
    QTRY_COMPARE(listView->isWrapEnabled(), true);
    QTRY_COMPARE(listView->cacheBuffer(), 10);
    QTRY_COMPARE(listView->snapMode(), QSGListView::SnapToItem);

    listView->setHighlightFollowsCurrentItem(false);
    listView->setPreferredHighlightBegin(1.0);
    listView->setPreferredHighlightEnd(1.0);
    listView->setHighlightRangeMode(QSGListView::StrictlyEnforceRange);
    listView->setWrapEnabled(false);
    listView->setCacheBuffer(3);
    listView->setSnapMode(QSGListView::SnapOneItem);

    QTRY_COMPARE(listView->highlightFollowsCurrentItem(), false);
    QTRY_COMPARE(listView->preferredHighlightBegin(), 1.0);
    QTRY_COMPARE(listView->preferredHighlightEnd(), 1.0);
    QTRY_COMPARE(listView->highlightRangeMode(), QSGListView::StrictlyEnforceRange);
    QTRY_COMPARE(listView->isWrapEnabled(), false);
    QTRY_COMPARE(listView->cacheBuffer(), 3);
    QTRY_COMPARE(listView->snapMode(), QSGListView::SnapOneItem);

    QTRY_COMPARE(highlightFollowsCurrentItemSpy.count(),1);
    QTRY_COMPARE(preferredHighlightBeginSpy.count(),1);
    QTRY_COMPARE(preferredHighlightEndSpy.count(),1);
    QTRY_COMPARE(highlightRangeModeSpy.count(),1);
    QTRY_COMPARE(keyNavigationWrapsSpy.count(),1);
    QTRY_COMPARE(cacheBufferSpy.count(),1);
    QTRY_COMPARE(snapModeSpy.count(),1);

    listView->setHighlightFollowsCurrentItem(false);
    listView->setPreferredHighlightBegin(1.0);
    listView->setPreferredHighlightEnd(1.0);
    listView->setHighlightRangeMode(QSGListView::StrictlyEnforceRange);
    listView->setWrapEnabled(false);
    listView->setCacheBuffer(3);
    listView->setSnapMode(QSGListView::SnapOneItem);

    QTRY_COMPARE(highlightFollowsCurrentItemSpy.count(),1);
    QTRY_COMPARE(preferredHighlightBeginSpy.count(),1);
    QTRY_COMPARE(preferredHighlightEndSpy.count(),1);
    QTRY_COMPARE(highlightRangeModeSpy.count(),1);
    QTRY_COMPARE(keyNavigationWrapsSpy.count(),1);
    QTRY_COMPARE(cacheBufferSpy.count(),1);
    QTRY_COMPARE(snapModeSpy.count(),1);

    delete canvas;
}

void tst_QSGListView::componentChanges()
{
    QSGView *canvas = createView();
    QTRY_VERIFY(canvas);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/propertychangestest.qml"));

    QSGListView *listView = canvas->rootObject()->findChild<QSGListView*>("listView");
    QTRY_VERIFY(listView);

    QDeclarativeComponent component(canvas->engine());
    component.setData("import QtQuick 2.0; Rectangle { color: \"blue\"; }", QUrl::fromLocalFile(""));

    QDeclarativeComponent delegateComponent(canvas->engine());
    delegateComponent.setData("import QtQuick 2.0; Text { text: '<b>Name:</b> ' + name }", QUrl::fromLocalFile(""));

    QSignalSpy highlightSpy(listView, SIGNAL(highlightChanged()));
    QSignalSpy delegateSpy(listView, SIGNAL(delegateChanged()));
    QSignalSpy headerSpy(listView, SIGNAL(headerChanged()));
    QSignalSpy footerSpy(listView, SIGNAL(footerChanged()));

    listView->setHighlight(&component);
    listView->setHeader(&component);
    listView->setFooter(&component);
    listView->setDelegate(&delegateComponent);

    QTRY_COMPARE(listView->highlight(), &component);
    QTRY_COMPARE(listView->header(), &component);
    QTRY_COMPARE(listView->footer(), &component);
    QTRY_COMPARE(listView->delegate(), &delegateComponent);

    QTRY_COMPARE(highlightSpy.count(),1);
    QTRY_COMPARE(delegateSpy.count(),1);
    QTRY_COMPARE(headerSpy.count(),1);
    QTRY_COMPARE(footerSpy.count(),1);

    listView->setHighlight(&component);
    listView->setHeader(&component);
    listView->setFooter(&component);
    listView->setDelegate(&delegateComponent);

    QTRY_COMPARE(highlightSpy.count(),1);
    QTRY_COMPARE(delegateSpy.count(),1);
    QTRY_COMPARE(headerSpy.count(),1);
    QTRY_COMPARE(footerSpy.count(),1);

    delete canvas;
}

void tst_QSGListView::modelChanges()
{
    QSGView *canvas = createView();
    QTRY_VERIFY(canvas);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/propertychangestest.qml"));

    QSGListView *listView = canvas->rootObject()->findChild<QSGListView*>("listView");
    QTRY_VERIFY(listView);

    QDeclarativeListModel *alternateModel = canvas->rootObject()->findChild<QDeclarativeListModel*>("alternateModel");
    QTRY_VERIFY(alternateModel);
    QVariant modelVariant = QVariant::fromValue<QObject *>(alternateModel);
    QSignalSpy modelSpy(listView, SIGNAL(modelChanged()));

    listView->setModel(modelVariant);
    QTRY_COMPARE(listView->model(), modelVariant);
    QTRY_COMPARE(modelSpy.count(),1);

    listView->setModel(modelVariant);
    QTRY_COMPARE(modelSpy.count(),1);

    listView->setModel(QVariant());
    QTRY_COMPARE(modelSpy.count(),2);

    delete canvas;
}

void tst_QSGListView::QTBUG_9791()
{
    QSGView *canvas = createView();

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/strictlyenforcerange.qml"));
    qApp->processEvents();

    QSGListView *listview = qobject_cast<QSGListView*>(canvas->rootObject());
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);
    QTRY_VERIFY(listview->delegate() != 0);
    QTRY_VERIFY(listview->model() != 0);

    QMetaObject::invokeMethod(listview, "fillModel");
    qApp->processEvents();

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    QCOMPARE(itemCount, 3);

    for (int i = 0; i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->x(), i*300.0);
    }

    // check that view is positioned correctly
    QTRY_COMPARE(listview->contentX(), 590.0);

    delete canvas;
}

void tst_QSGListView::manualHighlight()
{
    QSGView *canvas = new QSGView(0);
    canvas->setFixedSize(240,320);

    QString filename(SRCDIR "/data/manual-highlight.qml");
    canvas->setSource(QUrl::fromLocalFile(filename));

    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QTRY_COMPARE(listview->currentIndex(), 0);
    QTRY_COMPARE(listview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 0));
    QTRY_COMPARE(listview->highlightItem()->y() - 5, listview->currentItem()->y());

    listview->setCurrentIndex(2);

    QTRY_COMPARE(listview->currentIndex(), 2);
    QTRY_COMPARE(listview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 2));
    QTRY_COMPARE(listview->highlightItem()->y() - 5, listview->currentItem()->y());

    // QTBUG-15972
    listview->positionViewAtIndex(3, QSGListView::Contain);

    QTRY_COMPARE(listview->currentIndex(), 2);
    QTRY_COMPARE(listview->currentItem(), findItem<QSGItem>(contentItem, "wrapper", 2));
    QTRY_COMPARE(listview->highlightItem()->y() - 5, listview->currentItem()->y());

    delete canvas;
}

void tst_QSGListView::QTBUG_11105()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_VERIFY(item->y() == i*20);
    }

    listview->positionViewAtIndex(20, QSGListView::Beginning);
    QCOMPARE(listview->contentY(), 280.);

    TestModel model2;
    for (int i = 0; i < 5; i++)
        model2.addItem("Item" + QString::number(i), "");

    ctxt->setContextProperty("testModel", &model2);

    itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    QCOMPARE(itemCount, 5);

    delete canvas;
    delete testObject;
}

void tst_QSGListView::header()
{
    QFETCH(QSGListView::Orientation, orientation);
    QFETCH(Qt::LayoutDirection, layoutDirection);
    QFETCH(QPointF, initialHeaderPos);
    QFETCH(QPointF, firstDelegatePos);
    QFETCH(QPointF, initialContentPos);
    QFETCH(QPointF, changedHeaderPos);
    QFETCH(QPointF, changedContentPos);

    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/header.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);
    listview->setOrientation(orientation);
    listview->setLayoutDirection(layoutDirection);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGText *header = findItem<QSGText>(contentItem, "header");
    QVERIFY(header);

    QVERIFY(header == listview->headerItem());

    QCOMPARE(header->width(), 100.);
    QCOMPARE(header->height(), 30.);
    QCOMPARE(header->pos(), initialHeaderPos);
    QCOMPARE(QPointF(listview->contentX(), listview->contentY()), initialContentPos);

    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->pos(), firstDelegatePos);

    model.clear();
    QCOMPARE(header->pos(), initialHeaderPos); // header should stay where it is

    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QSignalSpy headerItemSpy(listview, SIGNAL(headerItemChanged()));
    QMetaObject::invokeMethod(canvas->rootObject(), "changeHeader");

    QCOMPARE(headerItemSpy.count(), 1);

    header = findItem<QSGText>(contentItem, "header");
    QVERIFY(!header);
    header = findItem<QSGText>(contentItem, "header2");
    QVERIFY(header);

    QVERIFY(header == listview->headerItem());

    QCOMPARE(header->pos(), changedHeaderPos);
    QCOMPARE(header->width(), 50.);
    QCOMPARE(header->height(), 20.);
    QTRY_COMPARE(QPointF(listview->contentX(), listview->contentY()), changedContentPos);
    QCOMPARE(item->pos(), firstDelegatePos);

    delete canvas;
}

void tst_QSGListView::header_data()
{
    QTest::addColumn<QSGListView::Orientation>("orientation");
    QTest::addColumn<Qt::LayoutDirection>("layoutDirection");
    QTest::addColumn<QPointF>("initialHeaderPos");
    QTest::addColumn<QPointF>("changedHeaderPos");
    QTest::addColumn<QPointF>("initialContentPos");
    QTest::addColumn<QPointF>("changedContentPos");
    QTest::addColumn<QPointF>("firstDelegatePos");

    // header1 = 100 x 30
    // header2 = 50 x 20
    // delegates = 240 x 20
    // view width = 240

    // header above items, top left
    QTest::newRow("vertical, left to right") << QSGListView::Vertical << Qt::LeftToRight
        << QPointF(0, -30)
        << QPointF(0, -20)
        << QPointF(0, -30)
        << QPointF(0, -20)
        << QPointF(0, 0);

    // header above items, top right
    QTest::newRow("vertical, layout right to left") << QSGListView::Vertical << Qt::RightToLeft
        << QPointF(0, -30)
        << QPointF(0, -20)
        << QPointF(0, -30)
        << QPointF(0, -20)
        << QPointF(0, 0);

    // header to left of items
    QTest::newRow("horizontal, layout left to right") << QSGListView::Horizontal << Qt::LeftToRight
        << QPointF(-100, 0)
        << QPointF(-50, 0)
        << QPointF(-100, 0)
        << QPointF(-50, 0)
        << QPointF(0, 0);

    // header to right of items
    QTest::newRow("horizontal, layout right to left") << QSGListView::Horizontal << Qt::RightToLeft
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(-240 + 100, 0)
        << QPointF(-240 + 50, 0)
        << QPointF(-240, 0);
}

void tst_QSGListView::header_delayItemCreation()
{
    QSGView *canvas = createView();

    TestModel model;

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/header1.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGText *header = findItem<QSGText>(contentItem, "header");
    QVERIFY(header);
    QCOMPARE(header->y(), -header->height());

    QCOMPARE(listview->contentY(), -header->height());

    model.clear();
    QTRY_COMPARE(header->y(), -header->height());

    delete canvas;
}

void tst_QSGListView::footer()
{
    QFETCH(QSGListView::Orientation, orientation);
    QFETCH(Qt::LayoutDirection, layoutDirection);
    QFETCH(QPointF, initialFooterPos);
    QFETCH(QPointF, firstDelegatePos);
    QFETCH(QPointF, initialContentPos);
    QFETCH(QPointF, changedFooterPos);
    QFETCH(QPointF, changedContentPos);

    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 3; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/footer.qml"));
    canvas->show();
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);
    listview->setOrientation(orientation);
    listview->setLayoutDirection(layoutDirection);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGText *footer = findItem<QSGText>(contentItem, "footer");
    QVERIFY(footer);

    QVERIFY(footer == listview->footerItem());

    QCOMPARE(footer->pos(), initialFooterPos);
    QCOMPARE(footer->width(), 100.);
    QCOMPARE(footer->height(), 30.);
    QCOMPARE(QPointF(listview->contentX(), listview->contentY()), initialContentPos);

    QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->pos(), firstDelegatePos);

    // remove one item
    model.removeItem(1);

    if (orientation == QSGListView::Vertical) {
        QTRY_COMPARE(footer->y(), initialFooterPos.y() - 20);   // delegate height = 20
    } else {
        QTRY_COMPARE(footer->x(), layoutDirection == Qt::LeftToRight ?
                initialFooterPos.x() - 40 : initialFooterPos.x() + 40);  // delegate width = 40
    }

    // remove all items
    model.clear();

    QPointF posWhenNoItems(0, 0);
    if (orientation == QSGListView::Horizontal && layoutDirection == Qt::RightToLeft)
        posWhenNoItems.setX(-100);
    QTRY_COMPARE(footer->pos(), posWhenNoItems);

    // if header is present, it's at a negative pos, so the footer should not move
    canvas->rootObject()->setProperty("showHeader", true);
    QTRY_COMPARE(footer->pos(), posWhenNoItems);
    canvas->rootObject()->setProperty("showHeader", false);

    // add 30 items
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QSignalSpy footerItemSpy(listview, SIGNAL(footerItemChanged()));
    QMetaObject::invokeMethod(canvas->rootObject(), "changeFooter");

    QCOMPARE(footerItemSpy.count(), 1);

    footer = findItem<QSGText>(contentItem, "footer");
    QVERIFY(!footer);
    footer = findItem<QSGText>(contentItem, "footer2");
    QVERIFY(footer);

    QVERIFY(footer == listview->footerItem());

    QCOMPARE(footer->pos(), changedFooterPos);
    QCOMPARE(footer->width(), 50.);
    QCOMPARE(footer->height(), 20.);
    QTRY_COMPARE(QPointF(listview->contentX(), listview->contentY()), changedContentPos);

    item = findItem<QSGItem>(contentItem, "wrapper", 0);
    QVERIFY(item);
    QCOMPARE(item->pos(), firstDelegatePos);

    delete canvas;
}

void tst_QSGListView::footer_data()
{
    QTest::addColumn<QSGListView::Orientation>("orientation");
    QTest::addColumn<Qt::LayoutDirection>("layoutDirection");
    QTest::addColumn<QPointF>("initialFooterPos");
    QTest::addColumn<QPointF>("changedFooterPos");
    QTest::addColumn<QPointF>("initialContentPos");
    QTest::addColumn<QPointF>("changedContentPos");
    QTest::addColumn<QPointF>("firstDelegatePos");

    // footer1 = 100 x 30
    // footer2 = 100 x 20
    // delegates = 40 x 20
    // view width = 240

    // footer below items, bottom left
    QTest::newRow("vertical, layout left to right") << QSGListView::Vertical << Qt::LeftToRight
        << QPointF(0, 3 * 20)
        << QPointF(0, 30 * 20)  // added 30 items
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(0, 0);

    // footer below items, bottom right
    QTest::newRow("vertical, layout right to left") << QSGListView::Vertical << Qt::RightToLeft
        << QPointF(0, 3 * 20)
        << QPointF(0, 30 * 20)
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(0, 0);

    // footer to right of items
    QTest::newRow("horizontal, layout left to right") << QSGListView::Horizontal << Qt::LeftToRight
        << QPointF(40 * 3, 0)
        << QPointF(40 * 30, 0)
        << QPointF(0, 0)
        << QPointF(0, 0)
        << QPointF(0, 0);

    // footer to left of items
    QTest::newRow("horizontal, layout right to left") << QSGListView::Horizontal << Qt::RightToLeft
        << QPointF(-(40 * 3) - 100, 0)
        << QPointF(-(40 * 30) - 50, 0)     // 50 = new footer width
        << QPointF(-240, 0)
        << QPointF(-240, 0)
        << QPointF(-40, 0);
}

class LVAccessor : public QSGListView
{
public:
    qreal minY() const { return minYExtent(); }
    qreal maxY() const { return maxYExtent(); }
    qreal minX() const { return minXExtent(); }
    qreal maxX() const { return maxXExtent(); }
};

void tst_QSGListView::headerFooter()
{
    {
        // Vertical
        QSGView *canvas = createView();

        TestModel model;
        QDeclarativeContext *ctxt = canvas->rootContext();
        ctxt->setContextProperty("testModel", &model);

        canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/headerfooter.qml"));
        qApp->processEvents();

        QSGListView *listview = qobject_cast<QSGListView*>(canvas->rootObject());
        QTRY_VERIFY(listview != 0);

        QSGItem *contentItem = listview->contentItem();
        QTRY_VERIFY(contentItem != 0);

        QSGItem *header = findItem<QSGItem>(contentItem, "header");
        QVERIFY(header);
        QCOMPARE(header->y(), -header->height());

        QSGItem *footer = findItem<QSGItem>(contentItem, "footer");
        QVERIFY(footer);
        QCOMPARE(footer->y(), 0.);

        QVERIFY(static_cast<LVAccessor*>(listview)->minY() == 0);
        QVERIFY(static_cast<LVAccessor*>(listview)->maxY() == 0);

        delete canvas;
    }
    {
        // Horizontal
        QSGView *canvas = createView();

        TestModel model;
        QDeclarativeContext *ctxt = canvas->rootContext();
        ctxt->setContextProperty("testModel", &model);

        canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/headerfooter.qml"));
        canvas->rootObject()->setProperty("horizontal", true);
        qApp->processEvents();

        QSGListView *listview = qobject_cast<QSGListView*>(canvas->rootObject());
        QTRY_VERIFY(listview != 0);

        QSGItem *contentItem = listview->contentItem();
        QTRY_VERIFY(contentItem != 0);

        QSGItem *header = findItem<QSGItem>(contentItem, "header");
        QVERIFY(header);
        QCOMPARE(header->x(), -header->width());

        QSGItem *footer = findItem<QSGItem>(contentItem, "footer");
        QVERIFY(footer);
        QCOMPARE(footer->x(), 0.);

        QVERIFY(static_cast<LVAccessor*>(listview)->minX() == 0);
        QVERIFY(static_cast<LVAccessor*>(listview)->maxX() == 0);

        delete canvas;
    }
    {
        // Horizontal RTL
        QSGView *canvas = createView();

        TestModel model;
        QDeclarativeContext *ctxt = canvas->rootContext();
        ctxt->setContextProperty("testModel", &model);

        canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/headerfooter.qml"));
        canvas->rootObject()->setProperty("horizontal", true);
        canvas->rootObject()->setProperty("rtl", true);
        qApp->processEvents();

        QSGListView *listview = qobject_cast<QSGListView*>(canvas->rootObject());
        QTRY_VERIFY(listview != 0);

        QSGItem *contentItem = listview->contentItem();
        QTRY_VERIFY(contentItem != 0);

        QSGItem *header = findItem<QSGItem>(contentItem, "header");
        QVERIFY(header);
        QCOMPARE(header->x(), 0.);

        QSGItem *footer = findItem<QSGItem>(contentItem, "footer");
        QVERIFY(footer);
        QCOMPARE(footer->x(), -footer->width());

        QCOMPARE(static_cast<LVAccessor*>(listview)->minX(), 240.);
        QCOMPARE(static_cast<LVAccessor*>(listview)->maxX(), 240.);

        delete canvas;
    }
}

void tst_QSGListView::resizeView()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 40; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*20.);
    }

    QVariant heightRatio;
    QMetaObject::invokeMethod(canvas->rootObject(), "heightRatio", Q_RETURN_ARG(QVariant, heightRatio));
    QCOMPARE(heightRatio.toReal(), 0.4);

    listview->setHeight(200);

    QMetaObject::invokeMethod(canvas->rootObject(), "heightRatio", Q_RETURN_ARG(QVariant, heightRatio));
    QCOMPARE(heightRatio.toReal(), 0.25);

    delete canvas;
    delete testObject;
}

void tst_QSGListView::sizeLessThan1()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/sizelessthan1.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    // Confirm items positioned correctly
    int itemCount = findItems<QSGItem>(contentItem, "wrapper").count();
    for (int i = 0; i < model.count() && i < itemCount; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        if (!item) qWarning() << "Item" << i << "not found";
        QTRY_VERIFY(item);
        QTRY_COMPARE(item->y(), i*0.5);
    }

    delete canvas;
    delete testObject;
}

void tst_QSGListView::QTBUG_14821()
{
    QSGView *canvas = createView();

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/qtbug14821.qml"));
    qApp->processEvents();

    QSGListView *listview = qobject_cast<QSGListView*>(canvas->rootObject());
    QVERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QVERIFY(contentItem != 0);

    listview->decrementCurrentIndex();
    QCOMPARE(listview->currentIndex(), 99);

    listview->incrementCurrentIndex();
    QCOMPARE(listview->currentIndex(), 0);

    delete canvas;
}

void tst_QSGListView::resizeDelegate()
{
    QSGView *canvas = createView();
    canvas->show();

    QStringList strings;
    for (int i = 0; i < 30; ++i)
        strings << QString::number(i);
    QStringListModel model(strings);

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/displaylist.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QVERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QVERIFY(contentItem != 0);

    QCOMPARE(listview->count(), model.rowCount());

    listview->setCurrentIndex(25);
    listview->setContentY(0);

    for (int i = 0; i < 16; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY(item != 0);
        QCOMPARE(item->y(), i*20.0);
    }

    QCOMPARE(listview->currentItem()->y(), 500.0);
    QTRY_COMPARE(listview->highlightItem()->y(), 500.0);

    canvas->rootObject()->setProperty("delegateHeight", 30);
    QTest::qWait(300);

    for (int i = 0; i < 11; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY(item != 0);
        QTRY_COMPARE(item->y(), i*30.0);
    }

    QTRY_COMPARE(listview->currentItem()->y(), 750.0);
    QTRY_COMPARE(listview->highlightItem()->y(), 750.0);

    listview->setCurrentIndex(1);
    listview->positionViewAtIndex(25, QSGListView::Beginning);
    listview->positionViewAtIndex(5, QSGListView::Beginning);

    for (int i = 5; i < 16; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY(item != 0);
        QCOMPARE(item->y(), i*30.0);
    }

    QTRY_COMPARE(listview->currentItem()->y(), 30.0);
    QTRY_COMPARE(listview->highlightItem()->y(), 30.0);

    canvas->rootObject()->setProperty("delegateHeight", 20);
    QTest::qWait(300);

    for (int i = 5; i < 11; ++i) {
        QSGItem *item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY(item != 0);
        QTRY_COMPARE(item->y(), 150 + (i-5)*20.0);
    }

    QTRY_COMPARE(listview->currentItem()->y(), 70.0);
    QTRY_COMPARE(listview->highlightItem()->y(), 70.0);

    delete canvas;
}

void tst_QSGListView::resizeFirstDelegate()
{
    // QTBUG-20712: Content Y jumps constantly if first delegate height == 0
    // and other delegates have height > 0

    QSGView *canvas = createView();
    canvas->show();

    // bug only occurs when all items in the model are visible
    TestModel model;
    for (int i = 0; i < 10; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QVERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QVERIFY(contentItem != 0);

    QSGItem *item = 0;
    for (int i = 0; i < model.count(); ++i) {
        item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY(item != 0);
        QCOMPARE(item->y(), i*20.0);
    }

    item = findItem<QSGItem>(contentItem, "wrapper", 0);
    item->setHeight(0);

    // check the content y has not jumped up and down
    QCOMPARE(listview->contentY(), 0.0);
    QSignalSpy spy(listview, SIGNAL(contentYChanged()));
    QTest::qWait(300);
    QCOMPARE(spy.count(), 0);

    for (int i = 1; i < model.count(); ++i) {
        item = findItem<QSGItem>(contentItem, "wrapper", i);
        QVERIFY(item != 0);
        QTRY_COMPARE(item->y(), (i-1)*20.0);
    }

    delete testObject;
    delete canvas;
}

void tst_QSGListView::QTBUG_16037()
{
    QSGView *canvas = createView();
    canvas->show();

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/qtbug16037.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "listview");
    QTRY_VERIFY(listview != 0);

    QVERIFY(listview->contentHeight() <= 0.0);

    QMetaObject::invokeMethod(canvas->rootObject(), "setModel");

    QTRY_COMPARE(listview->contentHeight(), 80.0);

    delete canvas;
}

void tst_QSGListView::indexAt()
{
    QSGView *canvas = createView();

    TestModel model;
    for (int i = 0; i < 30; i++)
        model.addItem("Item" + QString::number(i), "");

    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    TestObject *testObject = new TestObject;
    ctxt->setContextProperty("testObject", testObject);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/listviewtest.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QCOMPARE(listview->indexAt(0,0), 0);
    QCOMPARE(listview->indexAt(0,19), 0);
    QCOMPARE(listview->indexAt(239,19), 0);
    QCOMPARE(listview->indexAt(0,20), 1);
    QCOMPARE(listview->indexAt(240,20), -1);

    delete canvas;
    delete testObject;
}

void tst_QSGListView::incrementalModel()
{
    QSGView *canvas = createView();

    IncrementalModel model;
    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);

    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/displaylist.qml"));
    qApp->processEvents();

    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "list");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QTRY_COMPARE(listview->count(), 20);

    listview->positionViewAtIndex(10, QSGListView::Beginning);

    QTRY_COMPARE(listview->count(), 25);

    delete canvas;
}

void tst_QSGListView::onAdd()
{
    QFETCH(int, initialItemCount);
    QFETCH(int, itemsToAdd);

    const int delegateHeight = 10;
    TestModel2 model;

    // these initial items should not trigger ListView.onAdd
    for (int i=0; i<initialItemCount; i++)
        model.addItem("dummy value", "dummy value");

    QSGView *canvas = createView();
    canvas->setFixedSize(200, delegateHeight * (initialItemCount + itemsToAdd));
    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
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

void tst_QSGListView::onAdd_data()
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

void tst_QSGListView::onRemove()
{
    QFETCH(int, initialItemCount);
    QFETCH(int, indexToRemove);
    QFETCH(int, removeCount);

    const int delegateHeight = 10;
    TestModel2 model;
    for (int i=0; i<initialItemCount; i++)
        model.addItem(QString("value %1").arg(i), "dummy value");

    QSGView *canvas = createView();
    QDeclarativeContext *ctxt = canvas->rootContext();
    ctxt->setContextProperty("testModel", &model);
    ctxt->setContextProperty("delegateHeight", delegateHeight);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/attachedSignals.qml"));
    QObject *object = canvas->rootObject();

    qApp->processEvents();

    model.removeItems(indexToRemove, removeCount);
    qApp->processEvents();
    QCOMPARE(object->property("removedDelegateCount"), QVariant(removeCount));

    delete canvas;
}

void tst_QSGListView::onRemove_data()
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

void tst_QSGListView::rightToLeft()
{
    QSGView *canvas = createView();
    canvas->setFixedSize(640,320);
    canvas->setSource(QUrl::fromLocalFile(SRCDIR "/data/rightToLeft.qml"));
    qApp->processEvents();

    QVERIFY(canvas->rootObject() != 0);
    QSGListView *listview = findItem<QSGListView>(canvas->rootObject(), "view");
    QTRY_VERIFY(listview != 0);

    QSGItem *contentItem = listview->contentItem();
    QTRY_VERIFY(contentItem != 0);

    QSGVisualItemModel *model = canvas->rootObject()->findChild<QSGVisualItemModel*>("itemModel");
    QTRY_VERIFY(model != 0);

    QTRY_VERIFY(model->count() == 3);
    QTRY_COMPARE(listview->currentIndex(), 0);

    // initial position at first item, right edge aligned
    QCOMPARE(listview->contentX(), -640.);

    QSGItem *item = findItem<QSGItem>(contentItem, "item1");
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->x(), -100.0);
    QCOMPARE(item->height(), listview->height());

    QSGText *text = findItem<QSGText>(contentItem, "text1");
    QTRY_VERIFY(text);
    QTRY_COMPARE(text->text(), QLatin1String("index: 0"));

    listview->setCurrentIndex(2);

    item = findItem<QSGItem>(contentItem, "item3");
    QTRY_VERIFY(item);
    QTRY_COMPARE(item->x(), -540.0);

    text = findItem<QSGText>(contentItem, "text3");
    QTRY_VERIFY(text);
    QTRY_COMPARE(text->text(), QLatin1String("index: 2"));

    QCOMPARE(listview->contentX(), -640.);

    // Ensure resizing maintains position relative to right edge
    qobject_cast<QSGItem*>(canvas->rootObject())->setWidth(600);
    QTRY_COMPARE(listview->contentX(), -600.);

    delete canvas;
}

void tst_QSGListView::test_mirroring()
{
    QSGView *canvasA = createView();
    canvasA->setSource(QUrl::fromLocalFile(SRCDIR "/data/rightToLeft.qml"));
    QSGListView *listviewA = findItem<QSGListView>(canvasA->rootObject(), "view");
    QTRY_VERIFY(listviewA != 0);

    QSGView *canvasB = createView();
    canvasB->setSource(QUrl::fromLocalFile(SRCDIR "/data/rightToLeft.qml"));
    QSGListView *listviewB = findItem<QSGListView>(canvasB->rootObject(), "view");
    QTRY_VERIFY(listviewA != 0);
    qApp->processEvents();

    QList<QString> objectNames;
    objectNames << "item1" << "item2"; // << "item3"

    listviewA->setProperty("layoutDirection", Qt::LeftToRight);
    listviewB->setProperty("layoutDirection", Qt::RightToLeft);
    QCOMPARE(listviewA->layoutDirection(), listviewA->effectiveLayoutDirection());

    // LTR != RTL
    foreach(const QString objectName, objectNames)
        QVERIFY(findItem<QSGItem>(listviewA, objectName)->x() != findItem<QSGItem>(listviewB, objectName)->x());

    listviewA->setProperty("layoutDirection", Qt::LeftToRight);
    listviewB->setProperty("layoutDirection", Qt::LeftToRight);

    // LTR == LTR
    foreach(const QString objectName, objectNames)
        QCOMPARE(findItem<QSGItem>(listviewA, objectName)->x(), findItem<QSGItem>(listviewB, objectName)->x());

    QVERIFY(listviewB->layoutDirection() == listviewB->effectiveLayoutDirection());
    QSGItemPrivate::get(listviewB)->setLayoutMirror(true);
    QVERIFY(listviewB->layoutDirection() != listviewB->effectiveLayoutDirection());

    // LTR != LTR+mirror
    foreach(const QString objectName, objectNames)
        QVERIFY(findItem<QSGItem>(listviewA, objectName)->x() != findItem<QSGItem>(listviewB, objectName)->x());

    listviewA->setProperty("layoutDirection", Qt::RightToLeft);

    // RTL == LTR+mirror
    foreach(const QString objectName, objectNames)
        QCOMPARE(findItem<QSGItem>(listviewA, objectName)->x(), findItem<QSGItem>(listviewB, objectName)->x());

    listviewB->setProperty("layoutDirection", Qt::RightToLeft);

    // RTL != RTL+mirror
    foreach(const QString objectName, objectNames)
        QVERIFY(findItem<QSGItem>(listviewA, objectName)->x() != findItem<QSGItem>(listviewB, objectName)->x());

    listviewA->setProperty("layoutDirection", Qt::LeftToRight);

    // LTR == RTL+mirror
    foreach(const QString objectName, objectNames)
        QCOMPARE(findItem<QSGItem>(listviewA, objectName)->x(), findItem<QSGItem>(listviewB, objectName)->x());

    delete canvasA;
    delete canvasB;
}

void tst_QSGListView::qListModelInterface_items()
{
    items<TestModel>();
}

void tst_QSGListView::qAbstractItemModel_items()
{
    items<TestModel2>();
}

void tst_QSGListView::qListModelInterface_changed()
{
    changed<TestModel>();
}

void tst_QSGListView::qAbstractItemModel_changed()
{
    changed<TestModel2>();
}

void tst_QSGListView::qListModelInterface_inserted()
{
    inserted<TestModel>();
}

void tst_QSGListView::qAbstractItemModel_inserted()
{
    inserted<TestModel2>();
}

void tst_QSGListView::qListModelInterface_removed()
{
    removed<TestModel>(false);
    removed<TestModel>(true);
}

void tst_QSGListView::qAbstractItemModel_removed()
{
    removed<TestModel2>(false);
    removed<TestModel2>(true);
}

void tst_QSGListView::qListModelInterface_moved()
{
    moved<TestModel>();
}

void tst_QSGListView::qListModelInterface_moved_data()
{
    moved_data();
}

void tst_QSGListView::qAbstractItemModel_moved()
{
    moved<TestModel2>();
}

void tst_QSGListView::qAbstractItemModel_moved_data()
{
    moved_data();
}

void tst_QSGListView::qListModelInterface_clear()
{
    clear<TestModel>();
}

void tst_QSGListView::qAbstractItemModel_clear()
{
    clear<TestModel2>();
}

QSGView *tst_QSGListView::createView()
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
T *tst_QSGListView::findItem(QSGItem *parent, const QString &objectName, int index)
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
                QDeclarativeExpression e(qmlContext(item), item, "index");
                if (e.evaluate().toInt() == index)
                    return static_cast<T*>(item);
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
QList<T*> tst_QSGListView::findItems(QSGItem *parent, const QString &objectName)
{
    QList<T*> items;
    const QMetaObject &mo = T::staticMetaObject;
    //qDebug() << parent->childItems().count() << "children";
    for (int i = 0; i < parent->childItems().count(); ++i) {
        QSGItem *item = qobject_cast<QSGItem*>(parent->childItems().at(i));
        if(!item || !item->isVisible())
            continue;
        //qDebug() << "try" << item;
        if (mo.cast(item) && (objectName.isEmpty() || item->objectName() == objectName))
            items.append(static_cast<T*>(item));
        items += findItems<T>(item, objectName);
    }

    return items;
}

void tst_QSGListView::dumpTree(QSGItem *parent, int depth)
{
    static QString padding("                       ");
    for (int i = 0; i < parent->childItems().count(); ++i) {
        QSGItem *item = qobject_cast<QSGItem*>(parent->childItems().at(i));
        if(!item)
            continue;
        qDebug() << padding.left(depth*2) << item;
        dumpTree(item, depth+1);
    }
}


QTEST_MAIN(tst_QSGListView)

#include "tst_qsglistview.moc"

