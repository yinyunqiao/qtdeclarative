// Commit: ac5c099cc3c5b8c7eec7a49fdeb8a21037230350
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

#ifndef QSGIMAGE_P_H
#define QSGIMAGE_P_H

#include "qsgimagebase_p.h"
#include <private/qsgtextureprovider_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QSGImagePrivate;
class Q_AUTOTEST_EXPORT QSGImage : public QSGImageBase, public QSGTextureProvider
{
    Q_OBJECT
    Q_ENUMS(FillMode)
    Q_ENUMS(HAlignment)
    Q_ENUMS(VAlignment)

    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedGeometryChanged)
    Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedGeometryChanged)
    Q_PROPERTY(QSGTexture *texture READ texture)
    Q_PROPERTY(HAlignment horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment NOTIFY horizontalAlignmentChanged)
    Q_PROPERTY(VAlignment verticalAlignment READ verticalAlignment WRITE setVerticalAlignment NOTIFY verticalAlignmentChanged)

    Q_INTERFACES(QSGTextureProvider)

public:
    QSGImage(QSGItem *parent=0);
    ~QSGImage();

    enum HAlignment { AlignLeft = Qt::AlignLeft,
                       AlignRight = Qt::AlignRight,
                       AlignHCenter = Qt::AlignHCenter };
    enum VAlignment { AlignTop = Qt::AlignTop,
                       AlignBottom = Qt::AlignBottom,
                       AlignVCenter = Qt::AlignVCenter };

    enum FillMode { Stretch, PreserveAspectFit, PreserveAspectCrop, Tile, TileVertically, TileHorizontally, Pad };

    FillMode fillMode() const;
    void setFillMode(FillMode);

    qreal paintedWidth() const;
    qreal paintedHeight() const;

    QRectF boundingRect() const;

    virtual QSGTexture *texture() const;

    HAlignment horizontalAlignment() const;
    void setHorizontalAlignment(HAlignment align);

    VAlignment verticalAlignment() const;
    void setVerticalAlignment(VAlignment align);

Q_SIGNALS:
    void fillModeChanged();
    void paintedGeometryChanged();
    void horizontalAlignmentChanged(HAlignment alignment);
    void verticalAlignmentChanged(VAlignment alignment);

protected:
    QSGImage(QSGImagePrivate &dd, QSGItem *parent);
    void pixmapChange();
    void updatePaintedGeometry();

    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    virtual QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    Q_DISABLE_COPY(QSGImage)
    Q_DECLARE_PRIVATE(QSGImage)
};

QT_END_NAMESPACE
QML_DECLARE_TYPE(QSGImage)
QT_END_HEADER

#endif // QSGIMAGE_P_H
