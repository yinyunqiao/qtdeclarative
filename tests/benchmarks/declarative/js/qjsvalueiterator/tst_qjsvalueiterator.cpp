/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qtest.h>
#include <QtDeclarative/qjsengine.h>
#include <QtDeclarative/qjsvalue.h>
#include <QtDeclarative/qjsvalueiterator.h>

//TESTED_FILES=

class tst_QJSValueIterator : public QObject
{
    Q_OBJECT

public:
    tst_QJSValueIterator();
    virtual ~tst_QJSValueIterator();

    void dataHelper();

private slots:
    void init();
    void cleanup();

    void hasNextAndNext();

    void constructAndNext_data();
    void constructAndNext();

    void name_data();
    void name();
#if 0 // No string handle
    void scriptName_data();
    void scriptName();
#endif

    void value_data();
    void value();
#if 0 // no setValue
    void setValue_data();
    void setValue();
#endif
#if 0 // no flags
    void flags();
#endif

#if 0 // no array index
    void iterateArrayAndConvertNameToIndex();
#endif
#if 0 // no setValue
    void iterateArrayAndDoubleElements();
#endif
#if 0 // no remove
    void iterateArrayAndRemoveAllElements();
#endif
};

tst_QJSValueIterator::tst_QJSValueIterator()
{
}

tst_QJSValueIterator::~tst_QJSValueIterator()
{
}

void tst_QJSValueIterator::init()
{
}

void tst_QJSValueIterator::cleanup()
{
}

void tst_QJSValueIterator::dataHelper()
{
    QTest::addColumn<QString>("code");
    QTest::newRow("{ foo: 123 }") << QString::fromLatin1("({ foo: 123 })");
    QTest::newRow("Math") << QString::fromLatin1("Math");
    QTest::newRow("Array.prototype") << QString::fromLatin1("Array.prototype");
    QTest::newRow("Global Object") << QString::fromLatin1("this");
    QTest::newRow("['foo']") << QString::fromLatin1("['foo']");
    QTest::newRow("array with 1000 elements")
        << QString::fromLatin1("(function() {"
                               "  var a = new Array;"
                               "  for (i = 0; i < 1000; ++i)"
                               "    a[i] = i;"
                               "  return a;"
                               "})()");
}

void tst_QJSValueIterator::hasNextAndNext()
{
    QJSEngine engine;
    QJSValue object = engine.newObject();
    for (int i = 0; i < 2000; ++i)
        object.setProperty(i, i);
    QBENCHMARK {
        for (int i = 0; i < 1000; ++i) {
            QJSValueIterator it(object);
            while (it.hasNext())
                it.next();
        }
    }
}

void tst_QJSValueIterator::constructAndNext_data()
{
    dataHelper();
}

void tst_QJSValueIterator::constructAndNext()
{
    QFETCH(QString, code);
    QJSEngine engine;
    QJSValue object = engine.evaluate(code);
    Q_ASSERT(object.isObject());

    QBENCHMARK {
        for (int i = 0; i < 100; ++i) {
            QJSValueIterator it(object);
            it.next();
        }
    }
}

void tst_QJSValueIterator::name_data()
{
    dataHelper();
}

void tst_QJSValueIterator::name()
{
    QFETCH(QString, code);
    QJSEngine engine;
    QJSValue object = engine.evaluate(code);
    Q_ASSERT(object.isObject());

    QJSValueIterator it(object);
    it.next();
    QBENCHMARK {
        for (int i = 0; i < 200000; ++i)
            it.name();
    }
}

#if 0
void tst_QJSValueIterator::scriptName_data()
{
    dataHelper();
}

void tst_QJSValueIterator::scriptName()
{
    QFETCH(QString, code);
    QJSEngine engine;
    QJSValue object = engine.evaluate(code);
    Q_ASSERT(object.isObject());

    QJSValueIterator it(object);
    it.next();
    QBENCHMARK {
        for (int i = 0; i < 50000; ++i)
            it.scriptName();
    }
}
#endif

void tst_QJSValueIterator::value_data()
{
    dataHelper();
}

void tst_QJSValueIterator::value()
{
    QFETCH(QString, code);
    QJSEngine engine;
    QJSValue object = engine.evaluate(code);
    Q_ASSERT(object.isObject());

    QJSValueIterator it(object);
    it.next();
    QBENCHMARK {
        for (int i = 0; i < 50000; ++i)
            it.value();
    }
}

#if 0
void tst_QJSValueIterator::setValue_data()
{
    dataHelper();
}

void tst_QJSValueIterator::setValue()
{
    QFETCH(QString, code);
    QJSEngine engine;
    QJSValue object = engine.evaluate(code);
    Q_ASSERT(object.isObject());

    QJSValueIterator it(object);
    it.next();
    QJSValue newValue(&engine, 456);
    QBENCHMARK {
        for (int i = 0; i < 50000; ++i)
            it.setValue(newValue);
    }
}

void tst_QJSValueIterator::flags()
{
    QJSEngine engine;
    QJSValue object = engine.newObject();
    QJSValue::PropertyFlags flags = flags;
    object.setProperty("foo", 123, QJSValue::SkipInEnumeration | QJSValue::ReadOnly | QJSValue::Undeletable);
    QJSValueIterator it(object);
    it.next();
    QBENCHMARK {
        for (int i = 0; i < 50000; ++i)
            it.flags();
    }
}
#endif

#if 0
void tst_QJSValueIterator::iterateArrayAndConvertNameToIndex()
{
    QJSEngine engine;
    QJSValue array = engine.newArray();
    for (int i = 0; i < 20000; ++i)
        array.setProperty(i, i);
    QBENCHMARK {
        QJSValueIterator it(array);
        while (it.hasNext()) {
            it.next();
            it.scriptName().toArrayIndex();
        }
    }
}

void tst_QJSValueIterator::iterateArrayAndDoubleElements()
{
    QJSEngine engine;
    QJSValue array = engine.newArray();
    for (int i = 0; i < 20000; ++i)
        array.setProperty(i, i);
    QBENCHMARK {
        QJSValueIterator it(array);
        while (it.hasNext()) {
            it.next();
            it.setValue(QJSValue(&engine, it.value().toNumber() * 2));
        }
    }
}

void tst_QJSValueIterator::iterateArrayAndRemoveAllElements()
{
    QJSEngine engine;
    QJSValue array = engine.newArray();
    for (int i = 0; i < 20000; ++i)
        array.setProperty(i, i);
    QBENCHMARK {
        QJSValueIterator it(array);
        while (it.hasNext()) {
            it.next();
            it.remove();
        }
    }
}
#endif

QTEST_MAIN(tst_QJSValueIterator)
#include "tst_qjsvalueiterator.moc"
