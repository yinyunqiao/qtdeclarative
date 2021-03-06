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

#ifndef QSGRECTANGLE_P_H
#define QSGRECTANGLE_P_H

#include "qsgitem.h"

#include <QtGui/qbrush.h>

#include <private/qdeclarativeglobal_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)
class Q_DECLARATIVE_PRIVATE_EXPORT QSGPen : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY penChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY penChanged)
    Q_PROPERTY(bool aligned READ aligned WRITE setAligned NOTIFY penChanged)
public:
    QSGPen(QObject *parent=0);

    qreal width() const;
    void setWidth(qreal w);

    QColor color() const;
    void setColor(const QColor &c);

    bool aligned() const;
    void setAligned(bool aligned);

    bool isValid() const;

Q_SIGNALS:
    void penChanged();

private:
    qreal m_width;
    QColor m_color;
    bool m_aligned : 1;
    bool m_valid : 1;
};

class Q_AUTOTEST_EXPORT QSGGradientStop : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal position READ position WRITE setPosition)
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    QSGGradientStop(QObject *parent=0);

    qreal position() const;
    void setPosition(qreal position);

    QColor color() const;
    void setColor(const QColor &color);

private:
    void updateGradient();

private:
    qreal m_position;
    QColor m_color;
};

class Q_AUTOTEST_EXPORT QSGGradient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QSGGradientStop> stops READ stops)
    Q_CLASSINFO("DefaultProperty", "stops")

public:
    QSGGradient(QObject *parent=0);
    ~QSGGradient();

    QDeclarativeListProperty<QSGGradientStop> stops();

    const QGradient *gradient() const;

Q_SIGNALS:
    void updated();

private:
    void doUpdate();

private:
    QList<QSGGradientStop *> m_stops;
    mutable QGradient *m_gradient;
    friend class QSGRectangle;
    friend class QSGGradientStop;
};

class QSGRectanglePrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QSGRectangle : public QSGItem
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QSGGradient *gradient READ gradient WRITE setGradient)
    Q_PROPERTY(QSGPen * border READ border CONSTANT)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
public:
    QSGRectangle(QSGItem *parent=0);

    QColor color() const;
    void setColor(const QColor &);

    QSGPen *border();

    QSGGradient *gradient() const;
    void setGradient(QSGGradient *gradient);

    qreal radius() const;
    void setRadius(qreal radius);

    virtual QRectF boundingRect() const;

Q_SIGNALS:
    void colorChanged();
    void radiusChanged();

protected:
    virtual QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private Q_SLOTS:
    void doUpdate();

private:
    Q_DISABLE_COPY(QSGRectangle)
    Q_DECLARE_PRIVATE(QSGRectangle)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QSGPen)
QML_DECLARE_TYPE(QSGGradientStop)
QML_DECLARE_TYPE(QSGGradient)
QML_DECLARE_TYPE(QSGRectangle)

QT_END_HEADER

#endif // QSGRECTANGLE_P_H
