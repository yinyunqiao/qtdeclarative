/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#ifndef QSGPAINTEDITEM_P_H
#define QSGPAINTEDITEM_P_H

#include <qsgitem.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QSGPaintedItemPrivate;
class Q_DECLARATIVE_EXPORT QSGPaintedItem : public QSGItem
{
    Q_OBJECT
    Q_ENUMS(RenderTarget)

    Q_PROPERTY(QSize contentsSize READ contentsSize WRITE setContentsSize NOTIFY contentsSizeChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(qreal contentsScale READ contentsScale WRITE setContentsScale NOTIFY contentsScaleChanged)
    Q_PROPERTY(RenderTarget renderTarget READ renderTarget WRITE setRenderTarget NOTIFY renderTargetChanged)
public:
    QSGPaintedItem(QSGItem *parent = 0);
    virtual ~QSGPaintedItem();

    enum RenderTarget {
        Image,
        FramebufferObject
    };

    void update(const QRect &rect = QRect());

    bool opaquePainting() const;
    void setOpaquePainting(bool opaque);

    bool antialiasing() const;
    void setAntialiasing(bool enable);

    bool mipmap() const;
    void setMipmap(bool enable);

    QRectF contentsBoundingRect() const;

    QSize contentsSize() const;
    void setContentsSize(const QSize &);
    void resetContentsSize();

    qreal contentsScale() const;
    void setContentsScale(qreal);

    QColor fillColor() const;
    void setFillColor(const QColor&);

    RenderTarget renderTarget() const;
    void setRenderTarget(RenderTarget target);

    virtual void paint(QPainter *painter) = 0;

Q_SIGNALS:
    void fillColorChanged();
    void contentsSizeChanged();
    void contentsScaleChanged();
    void renderTargetChanged();

protected:
    QSGPaintedItem(QSGPaintedItemPrivate &dd, QSGItem *parent = 0);
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    virtual QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    Q_DISABLE_COPY(QSGPaintedItem);
    Q_DECLARE_PRIVATE(QSGPaintedItem);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSGPAINTEDITEM_P_H