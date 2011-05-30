/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL-ONLY$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QJSVALUE_P_H
#define QJSVALUE_P_H

#include <private/qv8_p.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmath.h>
#include <QtCore/qvarlengtharray.h>
#include <qdebug.h>

#include "qscripttools_p.h"
#include "qscriptshareddata_p.h"
#include "qjsvalue.h"

QT_BEGIN_NAMESPACE

/*!
  \internal
  \class QJSValuePrivate
*/
class QJSValuePrivate
        : public QSharedData
        , public QScriptLinkedNode
{
public:
    inline QJSValuePrivate();
    inline static QJSValuePrivate* get(const QJSValue& q);
    inline static QJSValue get(const QJSValuePrivate* d);
    inline static QJSValue get(QJSValuePrivate* d);
    inline static QJSValue get(QScriptPassPointer<QJSValuePrivate> d);
    inline ~QJSValuePrivate();

    inline QJSValuePrivate(bool value);
    inline QJSValuePrivate(int value);
    inline QJSValuePrivate(uint value);
    inline QJSValuePrivate(double value);
    inline QJSValuePrivate(const QString& value);
    inline QJSValuePrivate(QJSValue::SpecialValue value);

    inline QJSValuePrivate(QV8Engine *engine, bool value);
    inline QJSValuePrivate(QV8Engine *engine, int value);
    inline QJSValuePrivate(QV8Engine *engine, uint value);
    inline QJSValuePrivate(QV8Engine *engine, double value);
    inline QJSValuePrivate(QV8Engine *engine, const QString& value);
    inline QJSValuePrivate(QV8Engine *engine, QJSValue::SpecialValue value);
    inline QJSValuePrivate(QV8Engine *engine, v8::Handle<v8::Value>);
    inline void reinitialize();
    inline void reinitialize(QV8Engine *engine, v8::Handle<v8::Value> value);

    inline bool toBool() const;
    inline double toNumber() const;
    inline QScriptPassPointer<QJSValuePrivate> toObject() const;
    inline QScriptPassPointer<QJSValuePrivate> toObject(QV8Engine *engine) const;
    inline QString toString() const;
    inline double toInteger() const;
    inline qint32 toInt32() const;
    inline quint32 toUInt32() const;
    inline quint16 toUInt16() const;
    inline QDateTime toDataTime() const;
    inline QRegExp toRegExp() const;
    inline QObject *toQObject() const;
    inline QVariant toVariant() const;

    inline bool isArray() const;
    inline bool isBool() const;
    inline bool isCallable() const;
    inline bool isError() const;
    inline bool isFunction() const;
    inline bool isNull() const;
    inline bool isNumber() const;
    inline bool isObject() const;
    inline bool isString() const;
    inline bool isUndefined() const;
    inline bool isValid() const;
    inline bool isVariant() const;
    inline bool isDate() const;
    inline bool isRegExp() const;
    inline bool isQObject() const;

    inline bool equals(QJSValuePrivate* other);
    inline bool strictlyEquals(QJSValuePrivate* other);
    inline bool lessThan(QJSValuePrivate *other) const;
    inline bool instanceOf(QJSValuePrivate*) const;
    inline bool instanceOf(v8::Handle<v8::Object> other) const;

    inline QScriptPassPointer<QJSValuePrivate> prototype() const;
    inline void setPrototype(QJSValuePrivate* prototype);

    inline void setProperty(const QString &name, QJSValuePrivate *value, uint attribs = 0);
    inline void setProperty(v8::Handle<v8::String> name, QJSValuePrivate *value, uint attribs = 0);
    inline void setProperty(quint32 index, QJSValuePrivate* value, uint attribs = 0);
    inline QScriptPassPointer<QJSValuePrivate> property(const QString& name) const;
    inline QScriptPassPointer<QJSValuePrivate> property(v8::Handle<v8::String> name) const;
    inline QScriptPassPointer<QJSValuePrivate> property(quint32 index) const;
    template<typename T>
    inline QScriptPassPointer<QJSValuePrivate> property(T name) const;
    inline bool deleteProperty(const QString& name);
    inline QJSValue::PropertyFlags propertyFlags(const QString& name) const;
    inline QJSValue::PropertyFlags propertyFlags(v8::Handle<v8::String> name) const;

    inline QScriptPassPointer<QJSValuePrivate> call(QJSValuePrivate* thisObject, const QJSValueList& args);
    inline QScriptPassPointer<QJSValuePrivate> call(QJSValuePrivate* thisObject, const QJSValue& arguments);
    inline QScriptPassPointer<QJSValuePrivate> call(QJSValuePrivate* thisObject, int argc, v8::Handle< v8::Value >* argv);
    inline QScriptPassPointer<QJSValuePrivate> construct(int argc, v8::Handle<v8::Value> *argv);
    inline QScriptPassPointer<QJSValuePrivate> construct(const QJSValueList& args);
    inline QScriptPassPointer<QJSValuePrivate> construct(const QJSValue& arguments);

    inline bool assignEngine(QV8Engine *engine);
    inline QV8Engine *engine() const;

    inline operator v8::Handle<v8::Value>() const;
    inline operator v8::Handle<v8::Object>() const;
    inline v8::Handle<v8::Value> asV8Value(QV8Engine *engine);
private:
    QV8Engine *m_engine;

    // Please, update class documentation when you change the enum.
    enum State {
        Invalid = 0,
        CString = 0x1000,
        CNumber,
        CBool,
        CNull,
        CUndefined,
        JSValue = 0x2000, // V8 values are equal or higher then this value.
        // JSPrimitive,
        // JSObject
    } m_state;

    union CValue {
        bool m_bool;
        double m_number;
        QString* m_string;

        CValue() : m_number(0) {}
        CValue(bool value) : m_bool(value) {}
        CValue(int number) : m_number(number) {}
        CValue(uint number) : m_number(number) {}
        CValue(double number) : m_number(number) {}
        CValue(QString* string) : m_string(string) {}
    } u;
    // v8::Persistent is not a POD, so can't be part of the union.
    v8::Persistent<v8::Value> m_value;

    Q_DISABLE_COPY(QJSValuePrivate)
    inline bool isJSBased() const;
    inline bool isNumberBased() const;
    inline bool isStringBased() const;
    inline bool prepareArgumentsForCall(v8::Handle<v8::Value> argv[], const QJSValueList& arguments) const;
};

// This template is used indirectly by the Q_GLOBAL_STATIC macro below
template<>
class QGlobalStaticDeleter<QJSValuePrivate>
{
public:
    QGlobalStatic<QJSValuePrivate> &globalStatic;
    QGlobalStaticDeleter(QGlobalStatic<QJSValuePrivate> &_globalStatic)
        : globalStatic(_globalStatic)
    {
        globalStatic.pointer->ref.ref();
    }

    inline ~QGlobalStaticDeleter()
    {
        if (!globalStatic.pointer->ref.deref()) { // Logic copy & paste from SharedDataPointer
            delete globalStatic.pointer;
        }
        globalStatic.pointer = 0;
        globalStatic.destroyed = true;
    }
};

Q_GLOBAL_STATIC(QJSValuePrivate, InvalidValue)

QT_END_NAMESPACE

#endif