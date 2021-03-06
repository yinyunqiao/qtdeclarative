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

#include "qsgvisualitemmodel_p.h"
#include "qsgitem.h"

#include <QtDeclarative/qdeclarativecontext.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativeexpression.h>
#include <QtDeclarative/qdeclarativeinfo.h>

#include <private/qdeclarativecontext_p.h>
#include <private/qdeclarativepackage_p.h>
#include <private/qdeclarativeopenmetaobject_p.h>
#include <private/qdeclarativelistaccessor_p.h>
#include <private/qdeclarativedata_p.h>
#include <private/qdeclarativepropertycache_p.h>
#include <private/qdeclarativeguard_p.h>
#include <private/qdeclarativeglobal_p.h>
#include <private/qlistmodelinterface_p.h>
#include <private/qmetaobjectbuilder_p.h>
#include <private/qdeclarativeproperty_p.h>
#include <private/qobject_p.h>

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

QHash<QObject*, QSGVisualItemModelAttached*> QSGVisualItemModelAttached::attachedProperties;


class QSGVisualItemModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSGVisualItemModel)
public:
    QSGVisualItemModelPrivate() : QObjectPrivate() {}

    static void children_append(QDeclarativeListProperty<QSGItem> *prop, QSGItem *item) {
        QDeclarative_setParent_noEvent(item, prop->object);
        static_cast<QSGVisualItemModelPrivate *>(prop->data)->children.append(Item(item));
        static_cast<QSGVisualItemModelPrivate *>(prop->data)->itemAppended();
        static_cast<QSGVisualItemModelPrivate *>(prop->data)->emitChildrenChanged();
    }

    static int children_count(QDeclarativeListProperty<QSGItem> *prop) {
        return static_cast<QSGVisualItemModelPrivate *>(prop->data)->children.count();
    }

    static QSGItem *children_at(QDeclarativeListProperty<QSGItem> *prop, int index) {
        return static_cast<QSGVisualItemModelPrivate *>(prop->data)->children.at(index).item;
    }

    void itemAppended() {
        Q_Q(QSGVisualItemModel);
        QSGVisualItemModelAttached *attached = QSGVisualItemModelAttached::properties(children.last().item);
        attached->setIndex(children.count()-1);
        emit q->itemsInserted(children.count()-1, 1);
        emit q->countChanged();
    }

    void emitChildrenChanged() {
        Q_Q(QSGVisualItemModel);
        emit q->childrenChanged();
    }

    int indexOf(QSGItem *item) const {
        for (int i = 0; i < children.count(); ++i)
            if (children.at(i).item == item)
                return i;
        return -1;
    }

    class Item {
    public:
        Item(QSGItem *i) : item(i), ref(0) {}

        void addRef() { ++ref; }
        bool deref() { return --ref == 0; }

        QSGItem *item;
        int ref;
    };

    QList<Item> children;
};


/*!
    \qmlclass VisualItemModel QSGVisualItemModel
    \inqmlmodule QtQuick 2
    \ingroup qml-working-with-data
    \brief The VisualItemModel allows items to be provided to a view.

    A VisualItemModel contains the visual items to be used in a view.
    When a VisualItemModel is used in a view, the view does not require
    a delegate since the VisualItemModel already contains the visual
    delegate (items).

    An item can determine its index within the
    model via the \l{VisualItemModel::index}{index} attached property.

    The example below places three colored rectangles in a ListView.
    \code
    import QtQuick 1.0

    Rectangle {
        VisualItemModel {
            id: itemModel
            Rectangle { height: 30; width: 80; color: "red" }
            Rectangle { height: 30; width: 80; color: "green" }
            Rectangle { height: 30; width: 80; color: "blue" }
        }

        ListView {
            anchors.fill: parent
            model: itemModel
        }
    }
    \endcode

    \image visualitemmodel.png

    \sa {declarative/modelviews/visualitemmodel}{VisualItemModel example}
*/
QSGVisualItemModel::QSGVisualItemModel(QObject *parent)
    : QSGVisualModel(*(new QSGVisualItemModelPrivate), parent)
{
}

/*!
    \qmlattachedproperty int VisualItemModel::index
    This attached property holds the index of this delegate's item within the model.

    It is attached to each instance of the delegate.
*/

QDeclarativeListProperty<QSGItem> QSGVisualItemModel::children()
{
    Q_D(QSGVisualItemModel);
    return QDeclarativeListProperty<QSGItem>(this, d, d->children_append,
                                                      d->children_count, d->children_at);
}

/*!
    \qmlproperty int QtQuick2::VisualItemModel::count

    The number of items in the model.  This property is readonly.
*/
int QSGVisualItemModel::count() const
{
    Q_D(const QSGVisualItemModel);
    return d->children.count();
}

bool QSGVisualItemModel::isValid() const
{
    return true;
}

QSGItem *QSGVisualItemModel::item(int index, bool)
{
    Q_D(QSGVisualItemModel);
    QSGVisualItemModelPrivate::Item &item = d->children[index];
    item.addRef();
    return item.item;
}

QSGVisualModel::ReleaseFlags QSGVisualItemModel::release(QSGItem *item)
{
    Q_D(QSGVisualItemModel);
    int idx = d->indexOf(item);
    if (idx >= 0) {
        if (d->children[idx].deref()) {
            // XXX todo - the original did item->scene()->removeItem().  Why?
            item->setParentItem(0);
            QDeclarative_setParent_noEvent(item, this);
        }
    }
    return 0;
}

bool QSGVisualItemModel::completePending() const
{
    return false;
}

void QSGVisualItemModel::completeItem()
{
    // Nothing to do
}

QString QSGVisualItemModel::stringValue(int index, const QString &name)
{
    Q_D(QSGVisualItemModel);
    if (index < 0 || index >= d->children.count())
        return QString();
    return QDeclarativeEngine::contextForObject(d->children.at(index).item)->contextProperty(name).toString();
}

int QSGVisualItemModel::indexOf(QSGItem *item, QObject *) const
{
    Q_D(const QSGVisualItemModel);
    return d->indexOf(item);
}

QSGVisualItemModelAttached *QSGVisualItemModel::qmlAttachedProperties(QObject *obj)
{
    return QSGVisualItemModelAttached::properties(obj);
}

//============================================================================

class VDMDelegateDataType : public QDeclarativeRefCount
{
public:
    VDMDelegateDataType()
        : metaObject(0)
        , propertyCache(0)
        , propertyOffset(0)
        , signalOffset(0)
        , shared(true)
    {
    }

    VDMDelegateDataType(const VDMDelegateDataType &type)
        : metaObject(0)
        , propertyCache(0)
        , propertyOffset(type.propertyOffset)
        , signalOffset(type.signalOffset)
        , shared(false)
        , builder(type.metaObject, QMetaObjectBuilder::Properties
                | QMetaObjectBuilder::Signals
                | QMetaObjectBuilder::SuperClass
                | QMetaObjectBuilder::ClassName)
    {
        builder.setFlags(QMetaObjectBuilder::DynamicMetaObject);
    }

    ~VDMDelegateDataType()
    {
        if (propertyCache)
            propertyCache->release();
        qFree(metaObject);
    }

    QMetaObject *metaObject;
    QDeclarativePropertyCache *propertyCache;
    int propertyOffset;
    int signalOffset;
    bool shared : 1;
    QMetaObjectBuilder builder;
};

class QSGVisualDataModelParts;
class QSGVisualDataModelData;
class QSGVisualDataModelDataMetaObject;
class QSGVisualDataModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSGVisualDataModel)
public:
    QSGVisualDataModelPrivate(QDeclarativeContext *);

    static QSGVisualDataModelPrivate *get(QSGVisualDataModel *m) {
        return static_cast<QSGVisualDataModelPrivate *>(QObjectPrivate::get(m));
    }

    QDeclarativeGuard<QListModelInterface> m_listModelInterface;
    QDeclarativeGuard<QAbstractItemModel> m_abstractItemModel;
    QDeclarativeGuard<QSGVisualDataModel> m_visualItemModel;
    QString m_part;

    QDeclarativeComponent *m_delegate;
    QDeclarativeGuard<QDeclarativeContext> m_context;
    QList<int> m_roles;
    QHash<QByteArray,int> m_roleNames;

    void addProperty(int role, int propertyId, const char *propertyName, const char *propertyType, bool isModelData = false);
    template <typename T> void setModelDataType()
    {
        createModelData = &T::create;
        m_delegateDataType->builder.setFlags(QMetaObjectBuilder::DynamicMetaObject);
        m_delegateDataType->builder.setClassName(T::staticMetaObject.className());
        m_delegateDataType->builder.setSuperClass(&T::staticMetaObject);
        m_delegateDataType->propertyOffset = T::staticMetaObject.propertyCount();
        m_delegateDataType->signalOffset = T::staticMetaObject.methodCount();
    }
    QSGVisualDataModelData *createMetaObject(int index, QSGVisualDataModel *model);

    static QSGVisualDataModelData *initializeModelData(int index, QSGVisualDataModel *model) {
        return get(model)->createMetaObject(index, model);
    }

    typedef QSGVisualDataModelData *(*CreateModelData)(int index, QSGVisualDataModel *model);

    struct PropertyData {
        int role;
        bool isModelData : 1;
    };

    struct ObjectRef {
        ObjectRef(QObject *object=0) : obj(object), ref(1) {}
        QObject *obj;
        int ref;
    };
    class Cache : public QHash<int, ObjectRef> {
    public:
        QObject *getItem(int index) {
            QObject *item = 0;
            QHash<int,ObjectRef>::iterator it = find(index);
            if (it != end()) {
                (*it).ref++;
                item = (*it).obj;
            }
            return item;
        }
        QObject *item(int index) {
            QObject *item = 0;
            QHash<int, ObjectRef>::const_iterator it = find(index);
            if (it != end())
                item = (*it).obj;
            return item;
        }
        void insertItem(int index, QObject *obj) {
            insert(index, ObjectRef(obj));
        }
        bool releaseItem(QObject *obj) {
            QHash<int, ObjectRef>::iterator it = begin();
            for (; it != end(); ++it) {
                ObjectRef &objRef = *it;
                if (objRef.obj == obj) {
                    if (--objRef.ref == 0) {
                        erase(it);
                        return true;
                    }
                    break;
                }
            }
            return false;
        }
    };

    int modelCount() const {
        if (m_visualItemModel)
            return m_visualItemModel->count();
        if (m_listModelInterface)
            return m_listModelInterface->count();
        if (m_abstractItemModel)
            return m_abstractItemModel->rowCount(m_root);
        if (m_listAccessor)
            return m_listAccessor->count();
        return 0;
    }

    Cache m_cache;
    QHash<QObject *, QDeclarativePackage*> m_packaged;

    QSGVisualDataModelParts *m_parts;
    friend class QSGVisualItemParts;

    VDMDelegateDataType *m_delegateDataType;
    CreateModelData createModelData;

    friend class QSGVisualDataModelData;
    bool m_delegateValidated : 1;
    bool m_completePending : 1;
    bool m_objectList : 1;

    QSGVisualDataModelData *data(QObject *item);

    QVariant m_modelVariant;
    QDeclarativeListAccessor *m_listAccessor;

    QModelIndex m_root;
    QList<QByteArray> watchedRoles;
    QList<int> watchedRoleIds;

    QVector<PropertyData> m_propertyData;
};

class QSGVisualDataModelData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int index READ index NOTIFY indexChanged)
public:
    QSGVisualDataModelData(int index, QSGVisualDataModel *model);
    ~QSGVisualDataModelData();

    int index() const;
    void setIndex(int index);

Q_SIGNALS:
    void indexChanged();

public:
    int m_index;
    QDeclarativeGuard<QSGVisualDataModel> m_model;
};

class QSGVisualDataModelDataMetaObject : public QAbstractDynamicMetaObject
{
public:
    QSGVisualDataModelDataMetaObject(QSGVisualDataModelData *data, VDMDelegateDataType *type)
        : m_data(data)
        , m_type(type)
    {
        QObjectPrivate *op = QObjectPrivate::get(m_data);
        *static_cast<QMetaObject *>(this) = *type->metaObject;
        op->metaObject = this;
        m_type->addref();
    }

    ~QSGVisualDataModelDataMetaObject() { m_type->release(); }

    QSGVisualDataModelData *m_data;
    VDMDelegateDataType *m_type;
};

class QSGVDMAbstractItemModelDataMetaObject : public QSGVisualDataModelDataMetaObject
{
public:
    QSGVDMAbstractItemModelDataMetaObject(QSGVisualDataModelData *object, VDMDelegateDataType *type)
        : QSGVisualDataModelDataMetaObject(object, type) {}

    int metaCall(QMetaObject::Call call, int id, void **arguments)
    {
        if (call == QMetaObject::ReadProperty && id >= m_type->propertyOffset) {
            QSGVisualDataModelPrivate *model = QSGVisualDataModelPrivate::get(m_data->m_model);
            if (m_data->m_index == -1 || !model->m_abstractItemModel)
                return -1;
            *static_cast<QVariant *>(arguments[0]) = model->m_abstractItemModel->index(
                    m_data->m_index, 0, model->m_root).data(model->m_propertyData.at(id - m_type->propertyOffset).role);
            return -1;
        } else {
            return m_data->qt_metacall(call, id, arguments);
        }
    }
};

class QSGVDMAbstractItemModelData : public QSGVisualDataModelData
{
    Q_OBJECT
    Q_PROPERTY(bool hasModelChildren READ hasModelChildren CONSTANT)
public:
    bool hasModelChildren() const
    {
        QSGVisualDataModelPrivate *model = QSGVisualDataModelPrivate::get(m_model);
        return model->m_abstractItemModel->hasChildren(model->m_abstractItemModel->index(m_index, 0, model->m_root));
    }

    static QSGVisualDataModelData *create(int index, QSGVisualDataModel *model) {
        return new QSGVDMAbstractItemModelData(index, model); }
private:
    QSGVDMAbstractItemModelData(int index, QSGVisualDataModel *model)
        : QSGVisualDataModelData(index, model)
    {
        new QSGVDMAbstractItemModelDataMetaObject(
                this, QSGVisualDataModelPrivate::get(m_model)->m_delegateDataType);
    }
};

class QSGVDMListModelInterfaceDataMetaObject : public QSGVisualDataModelDataMetaObject
{
public:
    QSGVDMListModelInterfaceDataMetaObject(QSGVisualDataModelData *object, VDMDelegateDataType *type)
        : QSGVisualDataModelDataMetaObject(object, type) {}

    int metaCall(QMetaObject::Call call, int id, void **arguments)
    {
        if (call == QMetaObject::ReadProperty && id >= m_type->propertyOffset) {
            QSGVisualDataModelPrivate *model = QSGVisualDataModelPrivate::get(m_data->m_model);
            if (m_data->m_index == -1 || !model->m_listModelInterface)
                return -1;
            *static_cast<QVariant *>(arguments[0]) = model->m_listModelInterface->data(
                    m_data->m_index, model->m_propertyData.at(id - m_type->propertyOffset).role);
            return -1;
        } else {
            return m_data->qt_metacall(call, id, arguments);
        }
    }
};

class QSGVDMListModelInterfaceData : public QSGVisualDataModelData
{
public:
    static QSGVisualDataModelData *create(int index, QSGVisualDataModel *model) {
        return new QSGVDMListModelInterfaceData(index, model); }
private:
    QSGVDMListModelInterfaceData(int index, QSGVisualDataModel *model)
        : QSGVisualDataModelData(index, model)
    {
        new QSGVDMListModelInterfaceDataMetaObject(
                this, QSGVisualDataModelPrivate::get(m_model)->m_delegateDataType);
    }
};

class QSGVDMListAccessorData : public QSGVisualDataModelData
{
    Q_OBJECT
    Q_PROPERTY(QVariant modelData READ modelData CONSTANT)
public:
    QVariant modelData() const {
        return QSGVisualDataModelPrivate::get(m_model)->m_listAccessor->at(m_index); }

    static QSGVisualDataModelData *create(int index, QSGVisualDataModel *model) {
        return new QSGVDMListAccessorData(index, model); }
private:
    QSGVDMListAccessorData(int index, QSGVisualDataModel *model)
        : QSGVisualDataModelData(index, model)
    {
    }
};

class QSGVDMObjectDataMetaObject : public QSGVisualDataModelDataMetaObject
{
public:
    QSGVDMObjectDataMetaObject(QSGVisualDataModelData *data, VDMDelegateDataType *type)
        : QSGVisualDataModelDataMetaObject(data, type)
        , m_object(QSGVisualDataModelPrivate::get(data->m_model)->m_listAccessor->at(data->m_index).value<QObject *>())
    {}

    int metaCall(QMetaObject::Call call, int id, void **arguments)
    {
        if (id >= m_type->propertyOffset
                && (call == QMetaObject::ReadProperty
                || call == QMetaObject::WriteProperty
                || call == QMetaObject::ResetProperty)) {
            if (m_object)
                QMetaObject::metacall(m_object, call, id - m_type->propertyOffset + 1, arguments);
            return -1;
        } else if (id >= m_type->signalOffset && call == QMetaObject::InvokeMetaMethod) {
            QMetaObject::activate(m_data, this, id, 0);
            return -1;
        } else {
            return m_data->qt_metacall(call, id, arguments);
        }
    }

    int createProperty(const char *name, const char *)
    {
        if (!m_object)
            return -1;
        const QMetaObject *metaObject = m_object->metaObject();

        const int previousPropertyCount = propertyCount() - propertyOffset();
        int propertyIndex = metaObject->indexOfProperty(name);
        if (propertyIndex == -1)
            return -1;
        if (previousPropertyCount + 1 == metaObject->propertyCount())
            return propertyIndex + m_type->propertyOffset - 1;

        if (m_type->shared) {
            VDMDelegateDataType *type = m_type;
            m_type = new VDMDelegateDataType(*m_type);
            type->release();
        }

        const int previousMethodCount = methodCount();
        int notifierId = previousMethodCount;
        for (int propertyId = previousPropertyCount; propertyId < metaObject->propertyCount() - 1; ++propertyId) {
            QMetaProperty property = metaObject->property(propertyId + 1);
            QMetaPropertyBuilder propertyBuilder;
            if (property.hasNotifySignal()) {
                m_type->builder.addSignal("__" + QByteArray::number(propertyId) + "()");
                propertyBuilder = m_type->builder.addProperty(property.name(), property.typeName(), notifierId);
                ++notifierId;
            } else {
                propertyBuilder = m_type->builder.addProperty(property.name(), property.typeName());
            }
            propertyBuilder.setWritable(property.isWritable());
            propertyBuilder.setResettable(property.isResettable());
            propertyBuilder.setConstant(property.isConstant());
        }

        if (m_type->metaObject)
            qFree(m_type->metaObject);
        m_type->metaObject = m_type->builder.toMetaObject();
        *static_cast<QMetaObject *>(this) = *m_type->metaObject;

        notifierId = previousMethodCount;
        for (int i = previousPropertyCount; i < metaObject->propertyCount(); ++i) {
            QMetaProperty property = metaObject->property(i);
            if (property.hasNotifySignal()) {
                QDeclarativePropertyPrivate::connect(
                        m_object, property.notifySignalIndex(), m_data, notifierId);
                ++notifierId;
            }
        }
        return propertyIndex + m_type->propertyOffset - 1;
    }

    QDeclarativeGuard<QObject> m_object;
};

class QSGVDMObjectData : public QSGVisualDataModelData
{
    Q_OBJECT
    Q_PROPERTY(QObject *modelData READ modelData CONSTANT)
public:
    QObject *modelData() const { return m_metaObject->m_object; }

    static QSGVisualDataModelData *create(int index, QSGVisualDataModel *model) {
        return new QSGVDMObjectData(index, model); }

private:
    QSGVDMObjectData(int index, QSGVisualDataModel *model)
        : QSGVisualDataModelData(index, model)
        , m_metaObject(new QSGVDMObjectDataMetaObject(this, QSGVisualDataModelPrivate::get(m_model)->m_delegateDataType))
    {
    }

    QSGVDMObjectDataMetaObject *m_metaObject;
};

void QSGVisualDataModelPrivate::addProperty(
        int role, int propertyId, const char *propertyName, const char *propertyType, bool isModelData)
{
    PropertyData propertyData;
    propertyData.role = role;
    propertyData.isModelData = isModelData;
    m_delegateDataType->builder.addSignal("__" + QByteArray::number(propertyId) + "()");
    QMetaPropertyBuilder property = m_delegateDataType->builder.addProperty(
            propertyName, propertyType, propertyId);
    property.setWritable(false);

    m_propertyData.append(propertyData);
}

QSGVisualDataModelData *QSGVisualDataModelPrivate::createMetaObject(int index, QSGVisualDataModel *model)
{
    Q_ASSERT(!m_delegateDataType);

    m_objectList = false;
    m_propertyData.clear();
    if (m_listAccessor
            && m_listAccessor->type() != QDeclarativeListAccessor::ListProperty
            && m_listAccessor->type() != QDeclarativeListAccessor::Instance) {
        createModelData = &QSGVDMListAccessorData::create;
        return QSGVDMListAccessorData::create(index, model);
    }

    m_delegateDataType = new VDMDelegateDataType;
    if (m_listModelInterface) {
        setModelDataType<QSGVDMListModelInterfaceData>();
        QList<int> roles = m_listModelInterface->roles();
        for (int propertyId = 0; propertyId < roles.count(); ++propertyId) {
            const int role = roles.at(propertyId);
            const QByteArray propertyName = m_listModelInterface->toString(role).toUtf8();
            addProperty(role, propertyId, propertyName, "QVariant");
            m_roleNames.insert(propertyName, role);
        }
        if (m_propertyData.count() == 1)
            addProperty(roles.first(), 1, "modelData", "QVariant", true);
    } else if (m_abstractItemModel) {
        setModelDataType<QSGVDMAbstractItemModelData>();
        QHash<int, QByteArray> roleNames = m_abstractItemModel->roleNames();
        for (QHash<int, QByteArray>::const_iterator it = roleNames.begin(); it != roleNames.end(); ++it) {
            addProperty(it.key(), m_propertyData.count(), it.value(), "QVariant");
            m_roleNames.insert(it.value(), it.key());
        }
        if (m_propertyData.count() == 1)
            addProperty(roleNames.begin().key(), 1, "modelData", "QVariant", true);
    } else if (m_listAccessor) {
        setModelDataType<QSGVDMObjectData>();
        m_objectList = true;
    } else {
        Q_ASSERT(!"No model set on VisualDataModel");
        return 0;
    }
    m_delegateDataType->metaObject = m_delegateDataType->builder.toMetaObject();
    if (!m_objectList) {
        m_delegateDataType->propertyCache = new QDeclarativePropertyCache(
                m_context ? m_context->engine() : qmlEngine(q_func()), m_delegateDataType->metaObject);
    }
    return createModelData(index, model);
}

QSGVisualDataModelData::QSGVisualDataModelData(int index, QSGVisualDataModel *model)
    : m_index(index)
    , m_model(model)
{
}

QSGVisualDataModelData::~QSGVisualDataModelData()
{
}

int QSGVisualDataModelData::index() const
{
    return m_index;
}

// This is internal only - it should not be set from qml
void QSGVisualDataModelData::setIndex(int index)
{
    m_index = index;
    emit indexChanged();
}

//---------------------------------------------------------------------------

class QSGVisualDataModelPartsMetaObject : public QDeclarativeOpenMetaObject
{
public:
    QSGVisualDataModelPartsMetaObject(QObject *parent)
    : QDeclarativeOpenMetaObject(parent) {}

    virtual void propertyCreated(int, QMetaPropertyBuilder &);
    virtual QVariant initialValue(int);
};

class QSGVisualDataModelParts : public QObject
{
Q_OBJECT
public:
    QSGVisualDataModelParts(QSGVisualDataModel *parent);

private:
    friend class QSGVisualDataModelPartsMetaObject;
    QSGVisualDataModel *model;
};

void QSGVisualDataModelPartsMetaObject::propertyCreated(int, QMetaPropertyBuilder &prop)
{
    prop.setWritable(false);
}

QVariant QSGVisualDataModelPartsMetaObject::initialValue(int id)
{
    QSGVisualDataModel *m = new QSGVisualDataModel;
    m->setParent(object());
    m->setPart(QString::fromUtf8(name(id)));
    m->setModel(QVariant::fromValue(static_cast<QSGVisualDataModelParts *>(object())->model));

    QVariant var = QVariant::fromValue((QObject *)m);
    return var;
}

QSGVisualDataModelParts::QSGVisualDataModelParts(QSGVisualDataModel *parent)
: QObject(parent), model(parent)
{
    new QSGVisualDataModelPartsMetaObject(this);
}

QSGVisualDataModelPrivate::QSGVisualDataModelPrivate(QDeclarativeContext *ctxt)
: m_listModelInterface(0), m_abstractItemModel(0), m_visualItemModel(0), m_delegate(0)
, m_context(ctxt), m_parts(0), m_delegateDataType(0), createModelData(&initializeModelData)
, m_delegateValidated(false), m_completePending(false), m_objectList(false), m_listAccessor(0)
{
}

QSGVisualDataModelData *QSGVisualDataModelPrivate::data(QObject *item)
{
    QSGVisualDataModelData *dataItem =
        item->findChild<QSGVisualDataModelData *>();
    Q_ASSERT(dataItem);
    return dataItem;
}

//---------------------------------------------------------------------------

/*!
    \qmlclass VisualDataModel QSGVisualDataModel
    \inqmlmodule QtQuick 2
    \ingroup qml-working-with-data
    \brief The VisualDataModel encapsulates a model and delegate

    A VisualDataModel encapsulates a model and the delegate that will
    be instantiated for items in the model.

    It is usually not necessary to create VisualDataModel elements.
    However, it can be useful for manipulating and accessing the \l modelIndex
    when a QAbstractItemModel subclass is used as the
    model. Also, VisualDataModel is used together with \l Package to
    provide delegates to multiple views.

    The example below illustrates using a VisualDataModel with a ListView.

    \snippet doc/src/snippets/declarative/visualdatamodel.qml 0
*/

QSGVisualDataModel::QSGVisualDataModel()
: QSGVisualModel(*(new QSGVisualDataModelPrivate(0)))
{
}

QSGVisualDataModel::QSGVisualDataModel(QDeclarativeContext *ctxt, QObject *parent)
: QSGVisualModel(*(new QSGVisualDataModelPrivate(ctxt)), parent)
{
}

QSGVisualDataModel::~QSGVisualDataModel()
{
    Q_D(QSGVisualDataModel);
    if (d->m_listAccessor)
        delete d->m_listAccessor;
    if (d->m_delegateDataType)
        d->m_delegateDataType->release();
}

/*!
    \qmlproperty model QtQuick2::VisualDataModel::model
    This property holds the model providing data for the VisualDataModel.

    The model provides a set of data that is used to create the items
    for a view.  For large or dynamic datasets the model is usually
    provided by a C++ model object.  The C++ model object must be a \l
    {QAbstractItemModel} subclass or a simple list.

    Models can also be created directly in QML, using a \l{ListModel} or
    \l{XmlListModel}.

    \sa {qmlmodels}{Data Models}
*/
QVariant QSGVisualDataModel::model() const
{
    Q_D(const QSGVisualDataModel);
    return d->m_modelVariant;
}

void QSGVisualDataModel::setModel(const QVariant &model)
{
    Q_D(QSGVisualDataModel);
    delete d->m_listAccessor;
    d->m_listAccessor = 0;
    d->m_modelVariant = model;
    if (d->m_listModelInterface) {
        // Assume caller has released all items.
        QObject::disconnect(d->m_listModelInterface, SIGNAL(itemsChanged(int,int,QList<int>)),
                this, SLOT(_q_itemsChanged(int,int,QList<int>)));
        QObject::disconnect(d->m_listModelInterface, SIGNAL(itemsInserted(int,int)),
                this, SLOT(_q_itemsInserted(int,int)));
        QObject::disconnect(d->m_listModelInterface, SIGNAL(itemsRemoved(int,int)),
                this, SLOT(_q_itemsRemoved(int,int)));
        QObject::disconnect(d->m_listModelInterface, SIGNAL(itemsMoved(int,int,int)),
                this, SLOT(_q_itemsMoved(int,int,int)));
        d->m_listModelInterface = 0;
    } else if (d->m_abstractItemModel) {
        QObject::disconnect(d->m_abstractItemModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                            this, SLOT(_q_rowsInserted(QModelIndex,int,int)));
        QObject::disconnect(d->m_abstractItemModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                            this, SLOT(_q_rowsRemoved(QModelIndex,int,int)));
        QObject::disconnect(d->m_abstractItemModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                            this, SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
        QObject::disconnect(d->m_abstractItemModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                            this, SLOT(_q_rowsMoved(QModelIndex,int,int,QModelIndex,int)));
        QObject::disconnect(d->m_abstractItemModel, SIGNAL(modelReset()), this, SLOT(_q_modelReset()));
        QObject::disconnect(d->m_abstractItemModel, SIGNAL(layoutChanged()), this, SLOT(_q_layoutChanged()));
        d->m_abstractItemModel = 0;
    } else if (d->m_visualItemModel) {
        QObject::disconnect(d->m_visualItemModel, SIGNAL(itemsInserted(int,int)),
                         this, SIGNAL(itemsInserted(int,int)));
        QObject::disconnect(d->m_visualItemModel, SIGNAL(itemsRemoved(int,int)),
                         this, SIGNAL(itemsRemoved(int,int)));
        QObject::disconnect(d->m_visualItemModel, SIGNAL(itemsMoved(int,int,int)),
                         this, SIGNAL(itemsMoved(int,int,int)));
        QObject::disconnect(d->m_visualItemModel, SIGNAL(createdPackage(int,QDeclarativePackage*)),
                         this, SLOT(_q_createdPackage(int,QDeclarativePackage*)));
        QObject::disconnect(d->m_visualItemModel, SIGNAL(destroyingPackage(QDeclarativePackage*)),
                         this, SLOT(_q_destroyingPackage(QDeclarativePackage*)));
        d->m_visualItemModel = 0;
    }

    d->m_roles.clear();
    d->m_roleNames.clear();
    if (d->m_delegateDataType)
        d->m_delegateDataType->release();
    d->m_delegateDataType = 0;
    d->createModelData = &QSGVisualDataModelPrivate::initializeModelData;

    QObject *object = qvariant_cast<QObject *>(model);
    if (object && (d->m_listModelInterface = qobject_cast<QListModelInterface *>(object))) {
        QObject::connect(d->m_listModelInterface, SIGNAL(itemsChanged(int,int,QList<int>)),
                         this, SLOT(_q_itemsChanged(int,int,QList<int>)));
        QObject::connect(d->m_listModelInterface, SIGNAL(itemsInserted(int,int)),
                         this, SLOT(_q_itemsInserted(int,int)));
        QObject::connect(d->m_listModelInterface, SIGNAL(itemsRemoved(int,int)),
                         this, SLOT(_q_itemsRemoved(int,int)));
        QObject::connect(d->m_listModelInterface, SIGNAL(itemsMoved(int,int,int)),
                         this, SLOT(_q_itemsMoved(int,int,int)));
        if (d->m_delegate && d->m_listModelInterface->count())
            emit itemsInserted(0, d->m_listModelInterface->count());
        return;
    } else if (object && (d->m_abstractItemModel = qobject_cast<QAbstractItemModel *>(object))) {
        QObject::connect(d->m_abstractItemModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                            this, SLOT(_q_rowsInserted(QModelIndex,int,int)));
        QObject::connect(d->m_abstractItemModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                            this, SLOT(_q_rowsRemoved(QModelIndex,int,int)));
        QObject::connect(d->m_abstractItemModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                            this, SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
        QObject::connect(d->m_abstractItemModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                            this, SLOT(_q_rowsMoved(QModelIndex,int,int,QModelIndex,int)));
        QObject::connect(d->m_abstractItemModel, SIGNAL(modelReset()), this, SLOT(_q_modelReset()));
        QObject::connect(d->m_abstractItemModel, SIGNAL(layoutChanged()), this, SLOT(_q_layoutChanged()));
        if (d->m_abstractItemModel->canFetchMore(d->m_root))
            d->m_abstractItemModel->fetchMore(d->m_root);
        return;
    }
    if ((d->m_visualItemModel = qvariant_cast<QSGVisualDataModel *>(model))) {
        QObject::connect(d->m_visualItemModel, SIGNAL(countChanged()),
                         this, SIGNAL(countChanged()));
        QObject::connect(d->m_visualItemModel, SIGNAL(itemsInserted(int,int)),
                         this, SIGNAL(itemsInserted(int,int)));
        QObject::connect(d->m_visualItemModel, SIGNAL(itemsRemoved(int,int)),
                         this, SIGNAL(itemsRemoved(int,int)));
        QObject::connect(d->m_visualItemModel, SIGNAL(itemsMoved(int,int,int)),
                         this, SIGNAL(itemsMoved(int,int,int)));
        QObject::connect(d->m_visualItemModel, SIGNAL(createdPackage(int,QDeclarativePackage*)),
                         this, SLOT(_q_createdPackage(int,QDeclarativePackage*)));
        QObject::connect(d->m_visualItemModel, SIGNAL(destroyingPackage(QDeclarativePackage*)),
                         this, SLOT(_q_destroyingPackage(QDeclarativePackage*)));
        return;
    }
    d->m_listAccessor = new QDeclarativeListAccessor;
    d->m_listAccessor->setList(model, d->m_context?d->m_context->engine():qmlEngine(this));
    if (d->m_listAccessor->type() != QDeclarativeListAccessor::ListProperty)
    if (d->m_delegate && d->modelCount()) {
        emit itemsInserted(0, d->modelCount());
        emit countChanged();
    }
}

/*!
    \qmlproperty Component QtQuick2::VisualDataModel::delegate

    The delegate provides a template defining each item instantiated by a view.
    The index is exposed as an accessible \c index property.  Properties of the
    model are also available depending upon the type of \l {qmlmodels}{Data Model}.
*/
QDeclarativeComponent *QSGVisualDataModel::delegate() const
{
    Q_D(const QSGVisualDataModel);
    if (d->m_visualItemModel)
        return d->m_visualItemModel->delegate();
    return d->m_delegate;
}

void QSGVisualDataModel::setDelegate(QDeclarativeComponent *delegate)
{
    Q_D(QSGVisualDataModel);
    bool wasValid = d->m_delegate != 0;
    d->m_delegate = delegate;
    d->m_delegateValidated = false;
    if (!wasValid && d->modelCount() && d->m_delegate) {
        emit itemsInserted(0, d->modelCount());
        emit countChanged();
    }
    if (wasValid && !d->m_delegate && d->modelCount()) {
        emit itemsRemoved(0, d->modelCount());
        emit countChanged();
    }
}

/*!
    \qmlproperty QModelIndex QtQuick2::VisualDataModel::rootIndex

    QAbstractItemModel provides a hierarchical tree of data, whereas
    QML only operates on list data.  \c rootIndex allows the children of
    any node in a QAbstractItemModel to be provided by this model.

    This property only affects models of type QAbstractItemModel that
    are hierarchical (e.g, a tree model).

    For example, here is a simple interactive file system browser.
    When a directory name is clicked, the view's \c rootIndex is set to the
    QModelIndex node of the clicked directory, thus updating the view to show
    the new directory's contents.

    \c main.cpp:
    \snippet doc/src/snippets/declarative/visualdatamodel_rootindex/main.cpp 0

    \c view.qml:
    \snippet doc/src/snippets/declarative/visualdatamodel_rootindex/view.qml 0

    If the \l model is a QAbstractItemModel subclass, the delegate can also
    reference a \c hasModelChildren property (optionally qualified by a
    \e model. prefix) that indicates whether the delegate's model item has
    any child nodes.


    \sa modelIndex(), parentModelIndex()
*/
QVariant QSGVisualDataModel::rootIndex() const
{
    Q_D(const QSGVisualDataModel);
    return QVariant::fromValue(d->m_root);
}

void QSGVisualDataModel::setRootIndex(const QVariant &root)
{
    Q_D(QSGVisualDataModel);
    QModelIndex modelIndex = qvariant_cast<QModelIndex>(root);
    if (d->m_root != modelIndex) {
        int oldCount = d->modelCount();
        d->m_root = modelIndex;
        if (d->m_abstractItemModel && d->m_abstractItemModel->canFetchMore(modelIndex))
            d->m_abstractItemModel->fetchMore(modelIndex);
        int newCount = d->modelCount();
        if (d->m_delegate && oldCount)
            emit itemsRemoved(0, oldCount);
        if (d->m_delegate && newCount)
            emit itemsInserted(0, newCount);
        if (newCount != oldCount)
            emit countChanged();
        emit rootIndexChanged();
    }
}

/*!
    \qmlmethod QModelIndex QtQuick2::VisualDataModel::modelIndex(int index)

    QAbstractItemModel provides a hierarchical tree of data, whereas
    QML only operates on list data.  This function assists in using
    tree models in QML.

    Returns a QModelIndex for the specified index.
    This value can be assigned to rootIndex.

    \sa rootIndex
*/
QVariant QSGVisualDataModel::modelIndex(int idx) const
{
    Q_D(const QSGVisualDataModel);
    if (d->m_abstractItemModel)
        return QVariant::fromValue(d->m_abstractItemModel->index(idx, 0, d->m_root));
    return QVariant::fromValue(QModelIndex());
}

/*!
    \qmlmethod QModelIndex QtQuick2::VisualDataModel::parentModelIndex()

    QAbstractItemModel provides a hierarchical tree of data, whereas
    QML only operates on list data.  This function assists in using
    tree models in QML.

    Returns a QModelIndex for the parent of the current rootIndex.
    This value can be assigned to rootIndex.

    \sa rootIndex
*/
QVariant QSGVisualDataModel::parentModelIndex() const
{
    Q_D(const QSGVisualDataModel);
    if (d->m_abstractItemModel)
        return QVariant::fromValue(d->m_abstractItemModel->parent(d->m_root));
    return QVariant::fromValue(QModelIndex());
}

/*!
    \qmlproperty object QtQuick2::VisualDataModel::parts

    The \a parts property selects a VisualDataModel which creates
    delegates from the part named.  This is used in conjunction with
    the \l Package element.

    For example, the code below selects a model which creates
    delegates named \e list from a \l Package:

    \code
    VisualDataModel {
        id: visualModel
        delegate: Package {
            Item { Package.name: "list" }
        }
        model: myModel
    }

    ListView {
        width: 200; height:200
        model: visualModel.parts.list
    }
    \endcode

    \sa Package
*/
QString QSGVisualDataModel::part() const
{
    Q_D(const QSGVisualDataModel);
    return d->m_part;
}

void QSGVisualDataModel::setPart(const QString &part)
{
    Q_D(QSGVisualDataModel);
    d->m_part = part;
}

int QSGVisualDataModel::count() const
{
    Q_D(const QSGVisualDataModel);
    if (d->m_visualItemModel)
        return d->m_visualItemModel->count();
    if (!d->m_delegate)
        return 0;
    return d->modelCount();
}

QSGItem *QSGVisualDataModel::item(int index, bool complete)
{
    Q_D(QSGVisualDataModel);
    if (d->m_visualItemModel)
        return d->m_visualItemModel->item(index, d->m_part.toUtf8(), complete);
    return item(index, QByteArray(), complete);
}

/*
  Returns ReleaseStatus flags.
*/
QSGVisualDataModel::ReleaseFlags QSGVisualDataModel::release(QSGItem *item)
{
    Q_D(QSGVisualDataModel);
    if (d->m_visualItemModel)
        return d->m_visualItemModel->release(item);

    ReleaseFlags stat = 0;
    QObject *obj = item;
    bool inPackage = false;

    QHash<QObject*,QDeclarativePackage*>::iterator it = d->m_packaged.find(item);
    if (it != d->m_packaged.end()) {
        QDeclarativePackage *package = *it;
        d->m_packaged.erase(it);
        if (d->m_packaged.contains(item))
            stat |= Referenced;
        inPackage = true;
        obj = package; // fall through and delete
    }

    if (d->m_cache.releaseItem(obj)) {
        // Remove any bindings to avoid warnings due to parent change.
        QObjectPrivate *p = QObjectPrivate::get(obj);
        Q_ASSERT(p->declarativeData);
        QDeclarativeData *d = static_cast<QDeclarativeData*>(p->declarativeData);
        if (d->ownContext && d->context)
            d->context->clearContext();

        if (inPackage) {
            emit destroyingPackage(qobject_cast<QDeclarativePackage*>(obj));
        } else {
            // XXX todo - the original did item->scene()->removeItem().  Why?
            item->setParentItem(0);
        }
        stat |= Destroyed;
        obj->deleteLater();
    } else if (!inPackage) {
        stat |= Referenced;
    }

    return stat;
}

QObject *QSGVisualDataModel::parts()
{
    Q_D(QSGVisualDataModel);
    if (!d->m_parts)
        d->m_parts = new QSGVisualDataModelParts(this);
    return d->m_parts;
}

QSGItem *QSGVisualDataModel::item(int index, const QByteArray &viewId, bool complete)
{
    Q_D(QSGVisualDataModel);
    if (d->m_visualItemModel)
        return d->m_visualItemModel->item(index, viewId, complete);

    if (d->modelCount() <= 0 || !d->m_delegate)
        return 0;
    QObject *nobj = d->m_cache.getItem(index);
    bool needComplete = false;
    if (!nobj) {
        QDeclarativeContext *ccontext = d->m_context;
        if (!ccontext) ccontext = qmlContext(this);
        QDeclarativeContext *ctxt = new QDeclarativeContext(ccontext);
        if (d->m_objectList) {
            ctxt->setContextObject(d->m_listAccessor->at(index).value<QObject *>());
            ctxt = new QDeclarativeContext(ctxt);
        }
        QSGVisualDataModelData *data = d->createModelData(index, this);
        ctxt->setContextProperty(QLatin1String("model"), data);
        ctxt->setContextObject(data);
        d->m_completePending = false;
        nobj = d->m_delegate->beginCreate(ctxt);
        if (complete) {
            d->m_delegate->completeCreate();
        } else {
            d->m_completePending = true;
            needComplete = true;
        }
        if (nobj) {
            QDeclarative_setParent_noEvent(ctxt, nobj);
            QDeclarative_setParent_noEvent(data, nobj);
            d->m_cache.insertItem(index, nobj);
            if (QDeclarativePackage *package = qobject_cast<QDeclarativePackage *>(nobj))
                emit createdPackage(index, package);
        } else {
            delete data;
            delete ctxt;
            qmlInfo(this, d->m_delegate->errors()) << "Error creating delegate";
        }
    }
    QSGItem *item = qobject_cast<QSGItem *>(nobj);
    if (!item) {
        QDeclarativePackage *package = qobject_cast<QDeclarativePackage *>(nobj);
        if (package) {
            QObject *o = package->part(QString::fromUtf8(viewId));
            item = qobject_cast<QSGItem *>(o);
            if (item)
                d->m_packaged.insertMulti(item, package);
        }
    }
    if (!item) {
        if (needComplete)
            d->m_delegate->completeCreate();
        d->m_cache.releaseItem(nobj);
        if (!d->m_delegateValidated) {
            qmlInfo(d->m_delegate) << QSGVisualDataModel::tr("Delegate component must be Item type.");
            d->m_delegateValidated = true;
        }
    }
    if (d->modelCount()-1 == index && d->m_abstractItemModel && d->m_abstractItemModel->canFetchMore(d->m_root))
        d->m_abstractItemModel->fetchMore(d->m_root);

    return item;
}

bool QSGVisualDataModel::completePending() const
{
    Q_D(const QSGVisualDataModel);
    if (d->m_visualItemModel)
        return d->m_visualItemModel->completePending();
    return d->m_completePending;
}

void QSGVisualDataModel::completeItem()
{
    Q_D(QSGVisualDataModel);
    if (d->m_visualItemModel) {
        d->m_visualItemModel->completeItem();
        return;
    }

    d->m_delegate->completeCreate();
    d->m_completePending = false;
}

QString QSGVisualDataModel::stringValue(int index, const QString &name)
{
    Q_D(QSGVisualDataModel);
    if (d->m_visualItemModel)
        return d->m_visualItemModel->stringValue(index, name);

    if ((!d->m_listModelInterface || !d->m_abstractItemModel) && d->m_listAccessor) {
        if (QObject *object = d->m_listAccessor->at(index).value<QObject*>())
            return object->property(name.toUtf8()).toString();
    }

    if ((!d->m_listModelInterface && !d->m_abstractItemModel) || !d->m_delegate)
        return QString();

    QString val;
    QObject *data = 0;
    bool tempData = false;

    if (QObject *nobj = d->m_cache.item(index))
        data = d->data(nobj);
    if (!data) {
        data = d->createModelData(index, this);
        tempData = true;
    }

    QDeclarativeData *ddata = QDeclarativeData::get(data);
    if (ddata && ddata->propertyCache) {
        QDeclarativePropertyCache::Data *prop = ddata->propertyCache->property(name);
        if (prop) {
            if (prop->propType == QVariant::String) {
                void *args[] = { &val, 0 };
                QMetaObject::metacall(data, QMetaObject::ReadProperty, prop->coreIndex, args);
            } else if (prop->propType == qMetaTypeId<QVariant>()) {
                QVariant v;
                void *args[] = { &v, 0 };
                QMetaObject::metacall(data, QMetaObject::ReadProperty, prop->coreIndex, args);
                val = v.toString();
            }
        } else {
            val = data->property(name.toUtf8()).toString();
        }
    } else {
        val = data->property(name.toUtf8()).toString();
    }

    if (tempData)
        delete data;

    return val;
}

int QSGVisualDataModel::indexOf(QSGItem *item, QObject *) const
{
    QVariant val = QDeclarativeEngine::contextForObject(item)->contextProperty(QLatin1String("index"));
        return val.toInt();
    return -1;
}

void QSGVisualDataModel::setWatchedRoles(QList<QByteArray> roles)
{
    Q_D(QSGVisualDataModel);
    d->watchedRoles = roles;
    d->watchedRoleIds.clear();
}

void QSGVisualDataModel::_q_itemsChanged(int index, int count,
                                         const QList<int> &roles)
{
    Q_D(QSGVisualDataModel);
    bool changed = false;
    if (!d->watchedRoles.isEmpty() && d->watchedRoleIds.isEmpty()) {
        foreach (QByteArray r, d->watchedRoles) {
            if (d->m_roleNames.contains(r))
                d->watchedRoleIds << d->m_roleNames.value(r);
        }
    }

    QVector<int> signalIndexes;
    for (int i = 0; i < roles.count(); ++i) {
        const int role = roles.at(i);
        if (!changed && d->watchedRoleIds.contains(role))
            changed = true;
        for (int propertyId = 0; propertyId < d->m_propertyData.count(); ++propertyId) {
            if (d->m_propertyData.at(propertyId).role == role)
                signalIndexes.append(propertyId + d->m_delegateDataType->signalOffset);
        }
    }
    if (roles.isEmpty()) {
        for (int propertyId = 0; propertyId < d->m_propertyData.count(); ++propertyId)
            signalIndexes.append(propertyId + d->m_delegateDataType->signalOffset);
    }

    for (QHash<int,QSGVisualDataModelPrivate::ObjectRef>::ConstIterator iter = d->m_cache.begin();
        iter != d->m_cache.end(); ++iter) {
        const int idx = iter.key();

        if (idx >= index && idx < index+count) {
            QSGVisualDataModelPrivate::ObjectRef objRef = *iter;
            QSGVisualDataModelData *data = d->data(objRef.obj);
            for (int i = 0; i < signalIndexes.count(); ++i)
                QMetaObject::activate(data, signalIndexes.at(i), 0);
        }
    }
    if (changed)
        emit itemsChanged(index, count);
}

void QSGVisualDataModel::_q_itemsInserted(int index, int count)
{
    Q_D(QSGVisualDataModel);
    if (!count)
        return;
    // XXX - highly inefficient
    QHash<int,QSGVisualDataModelPrivate::ObjectRef> items;
    for (QHash<int,QSGVisualDataModelPrivate::ObjectRef>::Iterator iter = d->m_cache.begin();
        iter != d->m_cache.end(); ) {

        if (iter.key() >= index) {
            QSGVisualDataModelPrivate::ObjectRef objRef = *iter;
            int index = iter.key() + count;
            iter = d->m_cache.erase(iter);

            items.insert(index, objRef);

            QSGVisualDataModelData *data = d->data(objRef.obj);
            data->setIndex(index);
        } else {
            ++iter;
        }
    }
    d->m_cache.unite(items);

    emit itemsInserted(index, count);
    emit countChanged();
}

void QSGVisualDataModel::_q_itemsRemoved(int index, int count)
{
    Q_D(QSGVisualDataModel);
    if (!count)
        return;
    // XXX - highly inefficient
    QHash<int, QSGVisualDataModelPrivate::ObjectRef> items;
    for (QHash<int, QSGVisualDataModelPrivate::ObjectRef>::Iterator iter = d->m_cache.begin();
        iter != d->m_cache.end(); ) {
        if (iter.key() >= index && iter.key() < index + count) {
            QSGVisualDataModelPrivate::ObjectRef objRef = *iter;
            iter = d->m_cache.erase(iter);
            items.insertMulti(-1, objRef); //XXX perhaps better to maintain separately
            QSGVisualDataModelData *data = d->data(objRef.obj);
            data->setIndex(-1);
        } else if (iter.key() >= index + count) {
            QSGVisualDataModelPrivate::ObjectRef objRef = *iter;
            int index = iter.key() - count;
            iter = d->m_cache.erase(iter);
            items.insert(index, objRef);
            QSGVisualDataModelData *data = d->data(objRef.obj);
            data->setIndex(index);
        } else {
            ++iter;
        }
    }

    d->m_cache.unite(items);
    emit itemsRemoved(index, count);
    emit countChanged();
}

void QSGVisualDataModel::_q_itemsMoved(int from, int to, int count)
{
    Q_D(QSGVisualDataModel);
    // XXX - highly inefficient
    QHash<int,QSGVisualDataModelPrivate::ObjectRef> items;
    for (QHash<int,QSGVisualDataModelPrivate::ObjectRef>::Iterator iter = d->m_cache.begin();
        iter != d->m_cache.end(); ) {

        if (iter.key() >= from && iter.key() < from + count) {
            QSGVisualDataModelPrivate::ObjectRef objRef = *iter;
            int index = iter.key() - from + to;
            iter = d->m_cache.erase(iter);

            items.insert(index, objRef);

            QSGVisualDataModelData *data = d->data(objRef.obj);
            data->setIndex(index);
        } else {
            ++iter;
        }
    }
    for (QHash<int,QSGVisualDataModelPrivate::ObjectRef>::Iterator iter = d->m_cache.begin();
        iter != d->m_cache.end(); ) {

        int diff = from > to ? count : -count;
        if (iter.key() >= qMin(from,to) && iter.key() < qMax(from+count,to+count)) {
            QSGVisualDataModelPrivate::ObjectRef objRef = *iter;
            int index = iter.key() + diff;
            iter = d->m_cache.erase(iter);

            items.insert(index, objRef);

            QSGVisualDataModelData *data = d->data(objRef.obj);
            data->setIndex(index);
        } else {
            ++iter;
        }
    }
    d->m_cache.unite(items);

    emit itemsMoved(from, to, count);
}

void QSGVisualDataModel::_q_rowsInserted(const QModelIndex &parent, int begin, int end)
{
    Q_D(QSGVisualDataModel);
    if (parent == d->m_root)
        _q_itemsInserted(begin, end - begin + 1);
}

void QSGVisualDataModel::_q_rowsRemoved(const QModelIndex &parent, int begin, int end)
{
    Q_D(QSGVisualDataModel);
    if (parent == d->m_root)
        _q_itemsRemoved(begin, end - begin + 1);
}

void QSGVisualDataModel::_q_rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    Q_D(QSGVisualDataModel);
    const int count = sourceEnd - sourceStart + 1;
    if (destinationParent == d->m_root && sourceParent == d->m_root) {
        _q_itemsMoved(sourceStart, sourceStart > destinationRow ? destinationRow : destinationRow-1, count);
    } else if (sourceParent == d->m_root) {
        _q_itemsRemoved(sourceStart, count);
    } else if (destinationParent == d->m_root) {
        _q_itemsInserted(destinationRow, count);
    }
}

void QSGVisualDataModel::_q_dataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    Q_D(QSGVisualDataModel);
    if (begin.parent() == d->m_root)
        _q_itemsChanged(begin.row(), end.row() - begin.row() + 1, d->m_roles);
}

void QSGVisualDataModel::_q_layoutChanged()
{
    Q_D(QSGVisualDataModel);
    _q_itemsChanged(0, count(), d->m_roles);
}

void QSGVisualDataModel::_q_modelReset()
{
    Q_D(QSGVisualDataModel);
    d->m_root = QModelIndex();
    emit modelReset();
    emit rootIndexChanged();
    if (d->m_abstractItemModel && d->m_abstractItemModel->canFetchMore(d->m_root))
        d->m_abstractItemModel->fetchMore(d->m_root);
}

void QSGVisualDataModel::_q_createdPackage(int index, QDeclarativePackage *package)
{
    Q_D(QSGVisualDataModel);
    emit createdItem(index, qobject_cast<QSGItem*>(package->part(d->m_part)));
}

void QSGVisualDataModel::_q_destroyingPackage(QDeclarativePackage *package)
{
    Q_D(QSGVisualDataModel);
    emit destroyingItem(qobject_cast<QSGItem*>(package->part(d->m_part)));
}

QT_END_NAMESPACE

QML_DECLARE_TYPE(QListModelInterface)

#include <qsgvisualitemmodel.moc>
