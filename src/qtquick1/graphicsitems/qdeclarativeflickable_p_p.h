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

#ifndef QDECLARATIVEFLICKABLE_P_H
#define QDECLARATIVEFLICKABLE_P_H

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

#include "private/qdeclarativeflickable_p.h"

#include "private/qdeclarativeitem_p.h"
#include "private/qdeclarativeitemchangelistener_p.h"

#include <QtDeclarative/qdeclarative.h>
#include <QtQuick1/private/qdeclarativetimeline_p_p.h>
#include <QtQuick1/private/qdeclarativeanimation_p_p.h>

#include <qdatetime.h>

QT_BEGIN_NAMESPACE

// Really slow flicks can be annoying.
const qreal MinimumFlickVelocity = 75.0;

class QDeclarative1FlickableVisibleArea;
class QDeclarative1FlickablePrivate : public QDeclarativeItemPrivate, public QDeclarativeItemChangeListener
{
    Q_DECLARE_PUBLIC(QDeclarative1Flickable)

public:
    QDeclarative1FlickablePrivate();
    void init();

    struct Velocity : public QDeclarative1TimeLineValue
    {
        Velocity(QDeclarative1FlickablePrivate *p)
            : parent(p) {}
        virtual void setValue(qreal v) {
            if (v != value()) {
                QDeclarative1TimeLineValue::setValue(v);
                parent->updateVelocity();
            }
        }
        QDeclarative1FlickablePrivate *parent;
    };

    struct AxisData {
        AxisData(QDeclarative1FlickablePrivate *fp, void (QDeclarative1FlickablePrivate::*func)(qreal))
            : move(fp, func), viewSize(-1), smoothVelocity(fp), atEnd(false), atBeginning(true)
            , fixingUp(false), inOvershoot(false)
        {}

        void reset() {
            velocityBuffer.clear();
            dragStartOffset = 0;
            fixingUp = false;
            inOvershoot = false;
        }

        void addVelocitySample(qreal v, qreal maxVelocity);
        void updateVelocity();

        QDeclarative1TimeLineValueProxy<QDeclarative1FlickablePrivate> move;
        qreal viewSize;
        qreal pressPos;
        qreal dragStartOffset;
        qreal dragMinBound;
        qreal dragMaxBound;
        qreal velocity;
        qreal flickTarget;
        QDeclarative1FlickablePrivate::Velocity smoothVelocity;
        QPODVector<qreal,10> velocityBuffer;
        bool atEnd : 1;
        bool atBeginning : 1;
        bool fixingUp : 1;
        bool inOvershoot : 1;
    };

    void flickX(qreal velocity);
    void flickY(qreal velocity);
    virtual void flick(AxisData &data, qreal minExtent, qreal maxExtent, qreal vSize,
                        QDeclarative1TimeLineCallback::Callback fixupCallback, qreal velocity);

    void fixupX();
    void fixupY();
    virtual void fixup(AxisData &data, qreal minExtent, qreal maxExtent);

    void updateBeginningEnd();

    bool isOutermostPressDelay() const;
    void captureDelayedPress(QGraphicsSceneMouseEvent *event);
    void clearDelayedPress();

    void setRoundedViewportX(qreal x);
    void setRoundedViewportY(qreal y);

    qreal overShootDistance(qreal size);

    void itemGeometryChanged(QDeclarativeItem *, const QRectF &, const QRectF &);

public:
    QDeclarativeItem *contentItem;

    AxisData hData;
    AxisData vData;

    QDeclarative1TimeLine timeline;
    bool flickingHorizontally : 1;
    bool flickingVertically : 1;
    bool hMoved : 1;
    bool vMoved : 1;
    bool movingHorizontally : 1;
    bool movingVertically : 1;
    bool stealMouse : 1;
    bool pressed : 1;
    bool interactive : 1;
    bool calcVelocity : 1;
    QElapsedTimer lastPosTime;
    QPointF lastPos;
    QPointF pressPos;
    QElapsedTimer pressTime;
    qreal deceleration;
    qreal maxVelocity;
    QElapsedTimer velocityTime;
    QPointF lastFlickablePosition;
    qreal reportedVelocitySmoothing;
    QGraphicsSceneMouseEvent *delayedPressEvent;
    QGraphicsItem *delayedPressTarget;
    QBasicTimer delayedPressTimer;
    int pressDelay;
    int fixupDuration;

    enum FixupMode { Normal, Immediate, ExtentChanged };
    FixupMode fixupMode;

    static void fixupY_callback(void *);
    static void fixupX_callback(void *);

    void updateVelocity();
    int vTime;
    QDeclarative1TimeLine velocityTimeline;
    QDeclarative1FlickableVisibleArea *visibleArea;
    QDeclarative1Flickable::FlickableDirection flickableDirection;
    QDeclarative1Flickable::BoundsBehavior boundsBehavior;

    void handleMousePressEvent(QGraphicsSceneMouseEvent *);
    void handleMouseMoveEvent(QGraphicsSceneMouseEvent *);
    void handleMouseReleaseEvent(QGraphicsSceneMouseEvent *);

    // flickableData property
    static void data_append(QDeclarativeListProperty<QObject> *, QObject *);
    static int data_count(QDeclarativeListProperty<QObject> *);
    static QObject *data_at(QDeclarativeListProperty<QObject> *, int);
    static void data_clear(QDeclarativeListProperty<QObject> *);
};

class QDeclarative1FlickableVisibleArea : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal xPosition READ xPosition NOTIFY xPositionChanged)
    Q_PROPERTY(qreal yPosition READ yPosition NOTIFY yPositionChanged)
    Q_PROPERTY(qreal widthRatio READ widthRatio NOTIFY widthRatioChanged)
    Q_PROPERTY(qreal heightRatio READ heightRatio NOTIFY heightRatioChanged)

public:
    QDeclarative1FlickableVisibleArea(QDeclarative1Flickable *parent=0);

    qreal xPosition() const;
    qreal widthRatio() const;
    qreal yPosition() const;
    qreal heightRatio() const;

    void updateVisible();

signals:
    void xPositionChanged(qreal xPosition);
    void yPositionChanged(qreal yPosition);
    void widthRatioChanged(qreal widthRatio);
    void heightRatioChanged(qreal heightRatio);

private:
    QDeclarative1Flickable *flickable;
    qreal m_xPosition;
    qreal m_widthRatio;
    qreal m_yPosition;
    qreal m_heightRatio;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarative1FlickableVisibleArea)

#endif
