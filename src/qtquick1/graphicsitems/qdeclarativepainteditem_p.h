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

#ifndef QDECLARATIVEIMAGEITEM_H
#define QDECLARATIVEIMAGEITEM_H

#include "qdeclarativeitem.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarative1PaintedItemPrivate;
class Q_AUTOTEST_EXPORT QDeclarative1PaintedItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QSize contentsSize READ contentsSize WRITE setContentsSize NOTIFY contentsSizeChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(int pixelCacheSize READ pixelCacheSize WRITE setPixelCacheSize)
    Q_PROPERTY(bool smoothCache READ smoothCache WRITE setSmoothCache)
    Q_PROPERTY(qreal contentsScale READ contentsScale WRITE setContentsScale NOTIFY contentsScaleChanged)


public:
    QDeclarative1PaintedItem(QDeclarativeItem *parent=0);
    ~QDeclarative1PaintedItem();

    QSize contentsSize() const;
    void setContentsSize(const QSize &);

    qreal contentsScale() const;
    void setContentsScale(qreal);

    int pixelCacheSize() const;
    void setPixelCacheSize(int pixels);

    bool smoothCache() const;
    void setSmoothCache(bool on);

    QColor fillColor() const;
    void setFillColor(const QColor&);

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

protected:
    QDeclarative1PaintedItem(QDeclarative1PaintedItemPrivate &dd, QDeclarativeItem *parent);

    virtual void drawContents(QPainter *p, const QRect &) = 0;
    virtual void geometryChanged(const QRectF &newGeometry,
                                 const QRectF &oldGeometry);
    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant &value);

    void setCacheFrozen(bool);
    QRectF boundingRect() const;

Q_SIGNALS:
    void fillColorChanged();
    void contentsSizeChanged();
    void contentsScaleChanged();

protected Q_SLOTS:
    void dirtyCache(const QRect &);
    void clearCache();

private:
    Q_DISABLE_COPY(QDeclarative1PaintedItem)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarative1PaintedItem)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarative1PaintedItem)

QT_END_HEADER

#endif
