/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#ifndef QDECLARATIVETYPENAMECACHE_P_H
#define QDECLARATIVETYPENAMECACHE_P_H

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

#include "private/qdeclarativerefcount_p.h"
#include "private/qdeclarativecleanup_p.h"
#include "private/qdeclarativemetatype_p.h"

#include <private/qhashedstring_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeType;
class QDeclarativeEngine;
class QDeclarativeTypeNameCache : public QDeclarativeRefCount, public QDeclarativeCleanup
{
public:
    QDeclarativeTypeNameCache(QDeclarativeEngine *);
    virtual ~QDeclarativeTypeNameCache();

    struct Data {
        inline Data();
        inline Data(const Data &);
        inline ~Data();
        inline Data &operator=(const Data &);
        QDeclarativeType *type;
        QDeclarativeTypeNameCache *typeNamespace;
        int importedScriptIndex;
    };

    void add(const QString &, int);
    void add(const QString &, QDeclarativeType *);
    void add(const QString &, QDeclarativeTypeNameCache *);

    Data *data(const QString &) const;
    inline Data *data(const QHashedV8String &) const;
    inline bool isEmpty() const;

    inline QDeclarativeMetaType::ModuleApiInstance *moduleApi() const;
    void setModuleApi(QDeclarativeMetaType::ModuleApiInstance *);

protected:
    virtual void clear();

private:
    typedef QStringHash<Data> StringCache;

    StringCache stringCache;

    QDeclarativeEngine *engine;
    QDeclarativeMetaType::ModuleApiInstance *m_moduleApi;
};

QDeclarativeTypeNameCache::Data::Data()
: type(0), typeNamespace(0), importedScriptIndex(-1)
{
}

QDeclarativeTypeNameCache::Data::~Data()
{
    if (typeNamespace) typeNamespace->release();
}

bool QDeclarativeTypeNameCache::isEmpty() const
{
    return stringCache.isEmpty();
}

QDeclarativeTypeNameCache::Data::Data(const QDeclarativeTypeNameCache::Data &o)
: type(o.type), typeNamespace(o.typeNamespace), importedScriptIndex(o.importedScriptIndex)
{
    if (typeNamespace) typeNamespace->addref();
}

QDeclarativeTypeNameCache::Data &QDeclarativeTypeNameCache::Data::operator=(const QDeclarativeTypeNameCache::Data &o)
{
    if (o.typeNamespace) o.typeNamespace->addref();
    if (typeNamespace) typeNamespace->release();
    type = o.type;
    typeNamespace = o.typeNamespace;
    importedScriptIndex = o.importedScriptIndex;
    return *this;
}

QDeclarativeMetaType::ModuleApiInstance *QDeclarativeTypeNameCache::moduleApi() const
{
    return m_moduleApi;
}

QDeclarativeTypeNameCache::Data *QDeclarativeTypeNameCache::data(const QHashedV8String &name) const
{
    return stringCache.value(name);
}

QT_END_NAMESPACE

#endif // QDECLARATIVETYPENAMECACHE_P_H

