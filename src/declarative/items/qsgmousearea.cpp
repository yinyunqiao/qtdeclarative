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

#include "qsgmousearea_p.h"
#include "qsgmousearea_p_p.h"
#include "qsgcanvas.h"
#include "qsgevent.h"
#include "qsgevents_p_p.h"

#include <QtGui/qgraphicssceneevent.h>
#include <QtGui/qapplication.h>

#include <float.h>

QT_BEGIN_NAMESPACE
static const int PressAndHoldDelay = 800;

QSGDrag::QSGDrag(QObject *parent)
: QObject(parent), _target(0), _dropItem(0), _grabItem(0), _axis(XandYAxis), _xmin(-FLT_MAX),
_xmax(FLT_MAX), _ymin(-FLT_MAX), _ymax(FLT_MAX), _active(false), _filterChildren(false)
{
}

QSGDrag::~QSGDrag()
{
}

QSGItem *QSGDrag::target() const
{
    return _target;
}

void QSGDrag::setTarget(QSGItem *t)
{
    if (_target == t)
        return;
    _target = t;
    emit targetChanged();
}

void QSGDrag::resetTarget()
{
    if (_target == 0)
        return;
    _target = 0;
    emit targetChanged();
}

/*!
    \qmlproperty Item QtQuick2::MouseArea::drag.dropItem

    This property holds the item an active drag will be dropped on if released
    at the current position.
*/

QSGItem *QSGDrag::dropItem() const
{
    return _dropItem;
}

void QSGDrag::setDropItem(QSGItem *item)
{
    if (_dropItem != item) {
        _dropItem = item;
        emit dropItemChanged();
    }
}

QSGItem *QSGDrag::grabItem() const
{
    return _grabItem;
}

void QSGDrag::setGrabItem(QSGItem *item)
{
    _grabItem = item;
}

/*!
    \qmlproperty variant QtQuick2::MouseArea::drag.data

    This property holds the data sent to recipients of drag events generated
    by a MouseArea.
*/

QVariant QSGDrag::data() const
{
    return _data;
}

void QSGDrag::setData(const QVariant &data)
{
    if (_data != data) {
        _data = data;
        emit dataChanged();
    }
}

void QSGDrag::resetData()
{
    if (!_data.isNull()) {
        _data = QVariant();
        emit dataChanged();
    }
}

QSGDrag::Axis QSGDrag::axis() const
{
    return _axis;
}

void QSGDrag::setAxis(QSGDrag::Axis a)
{
    if (_axis == a)
        return;
    _axis = a;
    emit axisChanged();
}

qreal QSGDrag::xmin() const
{
    return _xmin;
}

void QSGDrag::setXmin(qreal m)
{
    if (_xmin == m)
        return;
    _xmin = m;
    emit minimumXChanged();
}

qreal QSGDrag::xmax() const
{
    return _xmax;
}

void QSGDrag::setXmax(qreal m)
{
    if (_xmax == m)
        return;
    _xmax = m;
    emit maximumXChanged();
}

qreal QSGDrag::ymin() const
{
    return _ymin;
}

void QSGDrag::setYmin(qreal m)
{
    if (_ymin == m)
        return;
    _ymin = m;
    emit minimumYChanged();
}

qreal QSGDrag::ymax() const
{
    return _ymax;
}

void QSGDrag::setYmax(qreal m)
{
    if (_ymax == m)
        return;
    _ymax = m;
    emit maximumYChanged();
}

bool QSGDrag::active() const
{
    return _active;
}

void QSGDrag::setActive(bool drag)
{
    if (_active == drag)
        return;
    _active = drag;
    emit activeChanged();
}

bool QSGDrag::filterChildren() const
{
    return _filterChildren;
}

void QSGDrag::setFilterChildren(bool filter)
{
    if (_filterChildren == filter)
        return;
    _filterChildren = filter;
    emit filterChildrenChanged();
}

/*!
    \qmlproperty stringlist QtQuick2::MouseArea::drag.keys

    This property holds a list of keys drag recipients can use to identify the
    source or data type of a drag event.
*/

QStringList QSGDrag::keys() const
{
    return _keys;
}

void QSGDrag::setKeys(const QStringList &keys)
{
    if (_keys != keys) {
        _keys = keys;
        emit keysChanged();
    }
}

QSGMouseAreaPrivate::QSGMouseAreaPrivate()
: absorb(true), hovered(false), pressed(false), longPress(false),
  moved(false), stealMouse(false), doubleClick(false), preventStealing(false), dragRejected(false),
  drag(0)
{
}

QSGMouseAreaPrivate::~QSGMouseAreaPrivate()
{
    delete drag;
}

void QSGMouseAreaPrivate::init()
{
    Q_Q(QSGMouseArea);
    q->setAcceptedMouseButtons(Qt::LeftButton);
    q->setFiltersChildMouseEvents(true);
}

void QSGMouseAreaPrivate::saveEvent(QGraphicsSceneMouseEvent *event)
{
    lastPos = event->pos();
    lastScenePos = event->scenePos();
    lastButton = event->button();
    lastButtons = event->buttons();
    lastModifiers = event->modifiers();
}

void QSGMouseAreaPrivate::forwardEvent(QGraphicsSceneMouseEvent* event)
{
    Q_Q(QSGMouseArea);
    event->setPos(q->mapFromScene(event->scenePos()));
}

bool QSGMouseAreaPrivate::isPressAndHoldConnected()
{
    Q_Q(QSGMouseArea);
    static int idx = QObjectPrivate::get(q)->signalIndex("pressAndHold(QSGMouseEvent*)");
    return QObjectPrivate::get(q)->isSignalConnected(idx);
}

bool QSGMouseAreaPrivate::isDoubleClickConnected()
{
    Q_Q(QSGMouseArea);
    static int idx = QObjectPrivate::get(q)->signalIndex("doubleClicked(QSGMouseEvent*)");
    return QObjectPrivate::get(q)->isSignalConnected(idx);
}

bool QSGMouseAreaPrivate::isClickConnected()
{
    Q_Q(QSGMouseArea);
    static int idx = QObjectPrivate::get(q)->signalIndex("clicked(QSGMouseEvent*)");
    return QObjectPrivate::get(q)->isSignalConnected(idx);
}

void QSGMouseAreaPrivate::propagate(QSGMouseEvent* event, PropagateType t)
{
    Q_Q(QSGMouseArea);
    QPointF scenePos = q->mapToScene(QPointF(event->x(), event->y()));
    propagateHelper(event, canvas->rootItem(), scenePos, t);
}

bool QSGMouseAreaPrivate::propagateHelper(QSGMouseEvent *ev, QSGItem *item,const QPointF &sp, PropagateType sig)
{
    //Based off of QSGCanvas::deliverInitialMousePressEvent
    //But specific to MouseArea, so doesn't belong in canvas
    Q_Q(const QSGMouseArea);
    QSGItemPrivate *itemPrivate = QSGItemPrivate::get(item);
    if (itemPrivate->opacity == 0.0)
        return false;

    if (itemPrivate->flags & QSGItem::ItemClipsChildrenToShape) {
        QPointF p = item->mapFromScene(sp);
        if (!QRectF(0, 0, item->width(), item->height()).contains(p))
            return false;
    }

    QList<QSGItem *> children = itemPrivate->paintOrderChildItems();
    for (int ii = children.count() - 1; ii >= 0; --ii) {
        QSGItem *child = children.at(ii);
        if (!child->isVisible() || !child->isEnabled())
            continue;
        if (propagateHelper(ev, child, sp, sig))
            return true;
    }

    QSGMouseArea* ma = qobject_cast<QSGMouseArea*>(item);
    if (ma && ma != q && itemPrivate->acceptedMouseButtons & ev->button()) {
        switch(sig){
        case Click:
            if (!ma->d_func()->isClickConnected())
                return false;
            break;
        case DoubleClick:
            if (!ma->d_func()->isDoubleClickConnected())
                return false;
            break;
        case PressAndHold:
            if (!ma->d_func()->isPressAndHoldConnected())
                return false;
            break;
        }
        QPointF p = item->mapFromScene(sp);
        if (QRectF(0, 0, item->width(), item->height()).contains(p)) {
            ev->setX(p.x());
            ev->setY(p.y());
            ev->setAccepted(true);//It is connected, they have to explicitly ignore to let it slide
            switch(sig){
            case Click: emit ma->clicked(ev); break;
            case DoubleClick: emit ma->doubleClicked(ev); break;
            case PressAndHold: emit ma->pressAndHold(ev); break;
            }
            if (ev->isAccepted())
                return true;
        }
    }
    return false;

}

/*!
    \qmlclass MouseArea QSGMouseArea
    \inqmlmodule QtQuick 2
    \ingroup qml-basic-interaction-elements
    \brief The MouseArea item enables simple mouse handling.
    \inherits Item

    A MouseArea is an invisible item that is typically used in conjunction with
    a visible item in order to provide mouse handling for that item.
    By effectively acting as a proxy, the logic for mouse handling can be
    contained within a MouseArea item.

    For basic key handling, see the \l{Keys}{Keys attached property}.

    The \l enabled property is used to enable and disable mouse handling for
    the proxied item. When disabled, the mouse area becomes transparent to
    mouse events.

    The \l pressed read-only property indicates whether or not the user is
    holding down a mouse button over the mouse area. This property is often
    used in bindings between properties in a user interface. The containsMouse
    read-only property indicates the presence of the mouse cursor over the
    mouse area but, by default, only when a mouse button is held down; see below
    for further details.

    Information about the mouse position and button clicks are provided via
    signals for which event handler properties are defined. The most commonly
    used involved handling mouse presses and clicks: onClicked, onDoubleClicked,
    onPressed, onReleased and onPressAndHold.

    By default, MouseArea items only report mouse clicks and not changes to the
    position of the mouse cursor. Setting the hoverEnabled property ensures that
    handlers defined for onPositionChanged, onEntered and onExited are used and
    that the containsMouse property is updated even when no mouse buttons are
    pressed.

    \section1 Example Usage

    \div {class="float-right"}
    \inlineimage qml-mousearea-snippet.png
    \enddiv

    The following example uses a MouseArea in a \l Rectangle that changes
    the \l Rectangle color to red when clicked:

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml import
    \codeline
    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml intro

    \clearfloat
    Many MouseArea signals pass a \l{MouseEvent}{mouse} parameter that contains
    additional information about the mouse event, such as the position, button,
    and any key modifiers.

    Here is an extension of the previous example that produces a different
    color when the area is right clicked:

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml intro-extended

  Behavioral Change in QtQuick 2.0

  From QtQuick 2.0, the signals clicked, doubleClicked and pressAndHold have a different interaction
  model with regards to the delivery of events to multiple overlapping MouseAreas. These signals will now propagate
  to all MouseAreas in the area, in painting order, until accepted by one of them. A signal is accepted by
  default if there is a signal handler for it, use mouse.accepted = false; to ignore. This propagation
  can send the signal to MouseAreas other than the one which accepted the press event, although that MouseArea
  will receive the signal first.

  Note that to get the same behavior as a QtQuick 1.0 MouseArea{} with regard to absorbing all mouse events, you will
  now need to add empty signal handlers for these three signals.

    \sa MouseEvent, {declarative/touchinteraction/mousearea}{MouseArea example}
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onEntered()

    This handler is called when the mouse enters the mouse area.

    By default the onEntered handler is only called while a button is
    pressed. Setting hoverEnabled to true enables handling of
    onEntered when no mouse button is pressed.

    \sa hoverEnabled
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onExited()

    This handler is called when the mouse exits the mouse area.

    By default the onExited handler is only called while a button is
    pressed. Setting hoverEnabled to true enables handling of
    onExited when no mouse button is pressed.

    The example below shows a fairly typical relationship between
    two MouseAreas, with \c mouseArea2 on top of \c mouseArea1. Moving the
    mouse into \c mouseArea2 from \c mouseArea1 will cause \c onExited
    to be called for \c mouseArea1.
    \qml
    Rectangle {
        width: 400; height: 400
        MouseArea {
            id: mouseArea1
            anchors.fill: parent
            hoverEnabled: true
        }
        MouseArea {
            id: mouseArea2
            width: 100; height: 100
            anchors.centerIn: parent
            hoverEnabled: true
        }
    }
    \endqml

    If instead you give the two mouseAreas a parent-child relationship,
    moving the mouse into \c mouseArea2 from \c mouseArea1 will \b not
    cause \c onExited to be called for \c mouseArea1. Instead, they will
    both be considered to be simultaneously hovered.

    \sa hoverEnabled
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onPositionChanged(MouseEvent mouse)

    This handler is called when the mouse position changes.

    The \l {MouseEvent}{mouse} parameter provides information about the mouse, including the x and y
    position, and any buttons currently pressed.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.

    By default the onPositionChanged handler is only called while a button is
    pressed.  Setting hoverEnabled to true enables handling of
    onPositionChanged when no mouse button is pressed.
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onClicked(MouseEvent mouse)

    This handler is called when there is a click. A click is defined as a press followed by a release,
    both inside the MouseArea (pressing, moving outside the MouseArea, and then moving back inside and
    releasing is also considered a click).

    The \l {MouseEvent}{mouse} parameter provides information about the click, including the x and y
    position of the release of the click, and whether the click was held.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onPressed(MouseEvent mouse)

    This handler is called when there is a press.
    The \l {MouseEvent}{mouse} parameter provides information about the press, including the x and y
    position and which button was pressed.

    The \e accepted property of the MouseEvent parameter determines whether this MouseArea
    will handle the press and all future mouse events until release.  The default is to accept
    the event and not allow other MouseArea beneath this one to handle the event.  If \e accepted
    is set to false, no further events will be sent to this MouseArea until the button is next
    pressed.
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onReleased(MouseEvent mouse)

    This handler is called when there is a release.
    The \l {MouseEvent}{mouse} parameter provides information about the click, including the x and y
    position of the release of the click, and whether the click was held.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.

    \sa onCanceled
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onPressAndHold(MouseEvent mouse)

    This handler is called when there is a long press (currently 800ms).
    The \l {MouseEvent}{mouse} parameter provides information about the press, including the x and y
    position of the press, and which button is pressed.

    The \e accepted property of the MouseEvent parameter is ignored in this handler.
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onDoubleClicked(MouseEvent mouse)

    This handler is called when there is a double-click (a press followed by a release followed by a press).
    The \l {MouseEvent}{mouse} parameter provides information about the click, including the x and y
    position of the release of the click, and whether the click was held.

    If the \e accepted property of the \l {MouseEvent}{mouse} parameter is set to false
    in the handler, the onPressed/onReleased/onClicked handlers will be called for the second
    click; otherwise they are suppressed.  The accepted property defaults to true.
*/

/*!
    \qmlsignal QtQuick2::MouseArea::onCanceled()

    This handler is called when mouse events have been canceled, either because an event was not accepted, or
    because another element stole the mouse event handling.

    This signal is for advanced use: it is useful when there is more than one MouseArea
    that is handling input, or when there is a MouseArea inside a \l Flickable. In the latter
    case, if you execute some logic on the pressed signal and then start dragging, the
    \l Flickable will steal the mouse handling from the MouseArea. In these cases, to reset
    the logic when the MouseArea has lost the mouse handling to the \l Flickable,
    \c onCanceled should be used in addition to onReleased.
*/
QSGMouseArea::QSGMouseArea(QSGItem *parent)
  : QSGItem(*(new QSGMouseAreaPrivate), parent)
{
    Q_D(QSGMouseArea);
    d->init();
}

QSGMouseArea::~QSGMouseArea()
{
}

/*!
    \qmlproperty real QtQuick2::MouseArea::mouseX
    \qmlproperty real QtQuick2::MouseArea::mouseY
    These properties hold the coordinates of the mouse cursor.

    If the hoverEnabled property is false then these properties will only be valid
    while a button is pressed, and will remain valid as long as the button is held
    down even if the mouse is moved outside the area.

    By default, this property is false.

    If hoverEnabled is true then these properties will be valid when:
    \list
        \i no button is pressed, but the mouse is within the MouseArea (containsMouse is true).
        \i a button is pressed and held, even if it has since moved out of the area.
    \endlist

    The coordinates are relative to the MouseArea.
*/
qreal QSGMouseArea::mouseX() const
{
    Q_D(const QSGMouseArea);
    return d->lastPos.x();
}

qreal QSGMouseArea::mouseY() const
{
    Q_D(const QSGMouseArea);
    return d->lastPos.y();
}

/*!
    \qmlproperty bool QtQuick2::MouseArea::enabled
    This property holds whether the item accepts mouse events.

    By default, this property is true.
*/
bool QSGMouseArea::isEnabled() const
{
    Q_D(const QSGMouseArea);
    return d->absorb;
}

void QSGMouseArea::setEnabled(bool a)
{
    Q_D(QSGMouseArea);
    if (a != d->absorb) {
        d->absorb = a;
        emit enabledChanged();
    }
}

/*!
    \qmlproperty bool QtQuick2::MouseArea::preventStealing
    This property holds whether the mouse events may be stolen from this
    MouseArea.

    If a MouseArea is placed within an item that filters child mouse
    events, such as Flickable, the mouse
    events may be stolen from the MouseArea if a gesture is recognized
    by the parent element, e.g. a flick gesture.  If preventStealing is
    set to true, no element will steal the mouse events.

    Note that setting preventStealing to true once an element has started
    stealing events will have no effect until the next press event.

    By default this property is false.
*/
bool QSGMouseArea::preventStealing() const
{
    Q_D(const QSGMouseArea);
    return d->preventStealing;
}

void QSGMouseArea::setPreventStealing(bool prevent)
{
    Q_D(QSGMouseArea);
    if (prevent != d->preventStealing) {
        d->preventStealing = prevent;
        setKeepMouseGrab(d->preventStealing && d->absorb);
        emit preventStealingChanged();
    }
}

/*!
    \qmlproperty MouseButtons QtQuick2::MouseArea::pressedButtons
    This property holds the mouse buttons currently pressed.

    It contains a bitwise combination of:
    \list
    \o Qt.LeftButton
    \o Qt.RightButton
    \o Qt.MiddleButton
    \endlist

    The code below displays "right" when the right mouse buttons is pressed:

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml mousebuttons

    \sa acceptedButtons
*/
Qt::MouseButtons QSGMouseArea::pressedButtons() const
{
    Q_D(const QSGMouseArea);
    return d->lastButtons;
}

void QSGMouseArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QSGMouseArea);
    d->moved = false;
    d->stealMouse = d->preventStealing;
    if (!d->absorb)
        QSGItem::mousePressEvent(event);
    else {
        d->longPress = false;
        d->dragRejected = false;
        d->saveEvent(event);
        if (d->drag) {
            d->dragX = drag()->axis() & QSGDrag::XAxis;
            d->dragY = drag()->axis() & QSGDrag::YAxis;
        }
        if (d->drag)
            d->drag->setActive(false);
        setHovered(true);
        d->startScene = event->scenePos();
        d->pressAndHoldTimer.start(PressAndHoldDelay, this);
        setKeepMouseGrab(d->stealMouse);
        event->setAccepted(setPressed(true));

    }
}

void QSGMouseArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QSGMouseArea);
    if (!d->absorb) {
        QSGItem::mouseMoveEvent(event);
        return;
    }

    d->saveEvent(event);

    // ### we should skip this if these signals aren't used
    // ### can GV handle this for us?
    bool contains = boundingRect().contains(d->lastPos);
    if (d->hovered && !contains)
        setHovered(false);
    else if (!d->hovered && contains)
        setHovered(true);

    if (d->drag && d->drag->target()) {

        if (!d->moved) {
            d->targetStartPos = d->drag->target()->parentItem()
                    ? d->drag->target()->parentItem()->mapToScene(d->drag->target()->pos())
                    : d->drag->target()->pos();
        }

        QPointF startLocalPos;
        QPointF curLocalPos;
        if (drag()->target()->parentItem()) {
            startLocalPos = drag()->target()->parentItem()->mapFromScene(d->startScene);
            curLocalPos = drag()->target()->parentItem()->mapFromScene(event->scenePos());
        } else {
            startLocalPos = d->startScene;
            curLocalPos = event->scenePos();
        }

        const int dragThreshold = QApplication::startDragDistance();
        qreal dx = qAbs(curLocalPos.x() - startLocalPos.x());
        qreal dy = qAbs(curLocalPos.y() - startLocalPos.y());

        if (keepMouseGrab() && d->stealMouse && !d->dragRejected && !d->drag->active()) {
            QSGMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, false, d->longPress);
            d->drag->emitDragged(&me);
            if (me.isAccepted()) {
                d->drag->setActive(true);
                QSGDragEvent dragEvent(
                        QSGEvent::SGDragEnter,
                        d->startScene,
                        d->drag->data(),
                        d->drag->keys());
                QCoreApplication::sendEvent(canvas(), &dragEvent);

                d->drag->setGrabItem(dragEvent.grabItem());
                d->drag->setDropItem(dragEvent.dropItem());
            } else {
                d->dragRejected = true;
            }
        }

        QPointF startPos = d->drag->target()->parentItem()
                ? d->drag->target()->parentItem()->mapFromScene(d->targetStartPos)
                : d->targetStartPos;

        if (d->dragX && d->drag->active()) {
            qreal x = (curLocalPos.x() - startLocalPos.x()) + startPos.x();
            if (x < drag()->xmin())
                x = drag()->xmin();
            else if (x > drag()->xmax())
                x = drag()->xmax();
            drag()->target()->setX(x);
        }
        if (d->dragY && d->drag->active()) {
            qreal y = (curLocalPos.y() - startLocalPos.y()) + startPos.y();
            if (y < drag()->ymin())
                y = drag()->ymin();
            else if (y > drag()->ymax())
                y = drag()->ymax();
            drag()->target()->setY(y);
        }

        if (!keepMouseGrab()) {
            if ((!d->dragY && dy < dragThreshold && d->dragX && dx > dragThreshold)
                || (!d->dragX && dx < dragThreshold && d->dragY && dy > dragThreshold)
                || (d->dragX && d->dragY && (dx > dragThreshold || dy > dragThreshold))) {
                setKeepMouseGrab(true);
                d->stealMouse = true;
            }
        }

        d->moved = true;

        if (d->drag->active()) {
            QSGDragEvent dragEvent(
                    QSGEvent::SGDragMove,
                    event->scenePos(),
                    d->drag->data(),
                    d->drag->keys(),
                    d->drag->grabItem());
            QCoreApplication::sendEvent(canvas(), &dragEvent);
            d->drag->setGrabItem(dragEvent.grabItem());
            d->drag->setDropItem(dragEvent.dropItem());
        }
    }
    QSGMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, false, d->longPress);
    emit mouseXChanged(&me);
    me.setPosition(d->lastPos);
    emit mouseYChanged(&me);
    me.setPosition(d->lastPos);
    emit positionChanged(&me);
}

void QSGMouseArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QSGMouseArea);
    d->stealMouse = false;
    if (!d->absorb) {
        QSGItem::mouseReleaseEvent(event);
    } else {
        d->saveEvent(event);
        setPressed(false);
        if (d->drag && d->drag->active()) {
            QSGDragEvent dragEvent(
                    QSGEvent::SGDragDrop,
                    event->scenePos(),
                    d->drag->data(),
                    d->drag->keys(),
                    d->drag->grabItem());
            QCoreApplication::sendEvent(canvas(), &dragEvent);
            d->drag->setGrabItem(0);
            if (dragEvent.isAccepted()) {
                d->drag->setDropItem(dragEvent.dropItem());
                d->drag->emitDropped(dragEvent.dropItem());
            } else {
                d->drag->emitCanceled();
            }
            d->drag->setDropItem(0);
            d->drag->setActive(false);
        }
        // If we don't accept hover, we need to reset containsMouse.
        if (!acceptHoverEvents())
            setHovered(false);
        QSGCanvas *c = canvas();
        if (c && c->mouseGrabberItem() == this)
            ungrabMouse();
        setKeepMouseGrab(false);

    }
    d->doubleClick = false;
}

void QSGMouseArea::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QSGMouseArea);
    if (!d->absorb) {
        QSGItem::mouseDoubleClickEvent(event);
    } else {
        d->saveEvent(event);
        QSGMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, true, false);
        me.setAccepted(d->isDoubleClickConnected());
        emit this->doubleClicked(&me);
        if (!me.isAccepted())
            d->propagate(&me, QSGMouseAreaPrivate::DoubleClick);
        d->doubleClick = d->isDoubleClickConnected() || me.isAccepted();
        QSGItem::mouseDoubleClickEvent(event);
    }
}

void QSGMouseArea::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(QSGMouseArea);
    if (!d->absorb) {
        QSGItem::hoverEnterEvent(event);
    } else {
        d->lastPos = event->posF();
        d->lastModifiers = event->modifiers();
        setHovered(true);
        QSGMouseEvent me(d->lastPos.x(), d->lastPos.y(), Qt::NoButton, Qt::NoButton, d->lastModifiers, false, false);
        emit mouseXChanged(&me);
        me.setPosition(d->lastPos);
        emit mouseYChanged(&me);
        me.setPosition(d->lastPos);
    }
}

void QSGMouseArea::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(QSGMouseArea);
    if (!d->absorb) {
        QSGItem::hoverMoveEvent(event);
    } else {
        d->lastPos = event->posF();
        d->lastModifiers = event->modifiers();
        QSGMouseEvent me(d->lastPos.x(), d->lastPos.y(), Qt::NoButton, Qt::NoButton, d->lastModifiers, false, false);
        emit mouseXChanged(&me);
        me.setPosition(d->lastPos);
        emit mouseYChanged(&me);
        me.setPosition(d->lastPos);
        emit positionChanged(&me);
    }
}

void QSGMouseArea::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(QSGMouseArea);
    if (!d->absorb)
        QSGItem::hoverLeaveEvent(event);
    else
        setHovered(false);
}

void QSGMouseArea::ungrabMouse()
{
    Q_D(QSGMouseArea);
    if (d->pressed) {
        // if our mouse grab has been removed (probably by Flickable), fix our
        // state
        d->pressed = false;
        d->stealMouse = false;
        setKeepMouseGrab(false);
        emit canceled();
        emit pressedChanged();
        if (d->hovered) {
            d->hovered = false;
            emit hoveredChanged();
        }
    }
}

void QSGMouseArea::mouseUngrabEvent()
{
    ungrabMouse();
}

bool QSGMouseArea::sendMouseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QSGMouseArea);
    QGraphicsSceneMouseEvent mouseEvent(event->type());
    QRectF myRect = mapRectToScene(QRectF(0, 0, width(), height()));

    QSGCanvas *c = canvas();
    QSGItem *grabber = c ? c->mouseGrabberItem() : 0;
    bool stealThisEvent = d->stealMouse;
    if ((stealThisEvent || myRect.contains(event->scenePos().toPoint())) && (!grabber || !grabber->keepMouseGrab())) {
        mouseEvent.setAccepted(false);
        for (int i = 0x1; i <= 0x10; i <<= 1) {
            if (event->buttons() & i) {
                Qt::MouseButton button = Qt::MouseButton(i);
                mouseEvent.setButtonDownPos(button, mapFromScene(event->buttonDownPos(button)));
            }
        }
        mouseEvent.setScenePos(event->scenePos());
        mouseEvent.setLastScenePos(event->lastScenePos());
        mouseEvent.setPos(mapFromScene(event->scenePos()));
        mouseEvent.setLastPos(mapFromScene(event->lastScenePos()));

        switch(mouseEvent.type()) {
        case QEvent::GraphicsSceneMouseMove:
            mouseMoveEvent(&mouseEvent);
            break;
        case QEvent::GraphicsSceneMousePress:
            mousePressEvent(&mouseEvent);
            break;
        case QEvent::GraphicsSceneMouseRelease:
            mouseReleaseEvent(&mouseEvent);
            break;
        default:
            break;
        }
        grabber = c->mouseGrabberItem();
        if (grabber && stealThisEvent && !grabber->keepMouseGrab() && grabber != this)
            grabMouse();

        return stealThisEvent;
    }
    if (mouseEvent.type() == QEvent::GraphicsSceneMouseRelease) {
        if (d->pressed) {
            d->pressed = false;
            d->stealMouse = false;
            if (c && c->mouseGrabberItem() == this)
                ungrabMouse();
            emit canceled();
            emit pressedChanged();
            if (d->hovered) {
                d->hovered = false;
                emit hoveredChanged();
            }
        }
    }
    return false;
}

bool QSGMouseArea::childMouseEventFilter(QSGItem *i, QEvent *e)
{
    Q_D(QSGMouseArea);
    if (!d->absorb || !isVisible() || !d->drag || !d->drag->filterChildren())
        return QSGItem::childMouseEventFilter(i, e);
    switch (e->type()) {
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        return sendMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
    default:
        break;
    }

    return QSGItem::childMouseEventFilter(i, e);
}

void QSGMouseArea::timerEvent(QTimerEvent *event)
{
    Q_D(QSGMouseArea);
    if (event->timerId() == d->pressAndHoldTimer.timerId()) {
        d->pressAndHoldTimer.stop();
        bool dragged = d->drag && d->drag->active();
        if (d->pressed && dragged == false && d->hovered == true) {
            d->longPress = true;
            QSGMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, false, d->longPress);
            me.setAccepted(d->isPressAndHoldConnected());
            emit pressAndHold(&me);
            if (!me.isAccepted())
                d->propagate(&me, QSGMouseAreaPrivate::PressAndHold);
            if (!me.isAccepted()) // no one handled the long press - allow click
                d->longPress = false;
        }
    }
}

void QSGMouseArea::windowDeactivateEvent()
{
    ungrabMouse();
    QSGItem::windowDeactivateEvent();
}

void QSGMouseArea::geometryChanged(const QRectF &newGeometry,
                                            const QRectF &oldGeometry)
{
    Q_D(QSGMouseArea);
    QSGItem::geometryChanged(newGeometry, oldGeometry);

    if (d->lastScenePos.isNull)
        d->lastScenePos = mapToScene(d->lastPos);
    else if (newGeometry.x() != oldGeometry.x() || newGeometry.y() != oldGeometry.y())
        d->lastPos = mapFromScene(d->lastScenePos);
}

void QSGMouseArea::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(QSGMouseArea);
    switch (change) {
    case ItemVisibleHasChanged:
        if (acceptHoverEvents() && d->hovered != (isVisible() && isUnderMouse()))
            setHovered(!d->hovered);
        break;
    default:
        break;
    }

    QSGItem::itemChange(change, value);
}

/*!
    \qmlproperty bool QtQuick2::MouseArea::hoverEnabled
    This property holds whether hover events are handled.

    By default, mouse events are only handled in response to a button event, or when a button is
    pressed.  Hover enables handling of all mouse events even when no mouse button is
    pressed.

    This property affects the containsMouse property and the onEntered, onExited and
    onPositionChanged signals.
*/
bool QSGMouseArea::hoverEnabled() const
{
    return acceptHoverEvents();
}

void QSGMouseArea::setHoverEnabled(bool h)
{
    Q_D(QSGMouseArea);
    if (h == acceptHoverEvents())
        return;

    setAcceptHoverEvents(h);
    emit hoverEnabledChanged();
}


/*!
    \qmlproperty bool QtQuick2::MouseArea::containsMouse
    This property holds whether the mouse is currently inside the mouse area.

    \warning This property is not updated if the area moves under the mouse: \e containsMouse will not change.
    In addition, if hoverEnabled is false, containsMouse will only be valid when the mouse is pressed.
*/
bool QSGMouseArea::hovered() const
{
    Q_D(const QSGMouseArea);
    return d->hovered;
}

/*!
    \qmlproperty bool QtQuick2::MouseArea::pressed
    This property holds whether the mouse area is currently pressed.
*/
bool QSGMouseArea::pressed() const
{
    Q_D(const QSGMouseArea);
    return d->pressed;
}

void QSGMouseArea::setHovered(bool h)
{
    Q_D(QSGMouseArea);
    if (d->hovered != h) {
        d->hovered = h;
        emit hoveredChanged();
        d->hovered ? emit entered() : emit exited();
    }
}
/*!
    \qmlproperty QtQuick2::Qt::MouseButtons MouseArea::acceptedButtons
    This property holds the mouse buttons that the mouse area reacts to.

    The available buttons are:
    \list
    \o Qt.LeftButton
    \o Qt.RightButton
    \o Qt.MiddleButton
    \endlist

    To accept more than one button the flags can be combined with the
    "|" (or) operator:

    \code
    MouseArea { acceptedButtons: Qt.LeftButton | Qt.RightButton }
    \endcode

    The default value is \c Qt.LeftButton.
*/
Qt::MouseButtons QSGMouseArea::acceptedButtons() const
{
    return acceptedMouseButtons();
}

void QSGMouseArea::setAcceptedButtons(Qt::MouseButtons buttons)
{
    if (buttons != acceptedMouseButtons()) {
        setAcceptedMouseButtons(buttons);
        emit acceptedButtonsChanged();
    }
}

bool QSGMouseArea::setPressed(bool p)
{
    Q_D(QSGMouseArea);
    bool dragged = d->drag && d->drag->active();
    bool isclick = d->pressed == true && p == false && dragged == false && d->hovered == true;

    if (d->pressed != p) {
        d->pressed = p;
        QSGMouseEvent me(d->lastPos.x(), d->lastPos.y(), d->lastButton, d->lastButtons, d->lastModifiers, isclick, d->longPress);
        if (d->pressed) {
            if (!d->doubleClick)
                emit pressed(&me);
            me.setPosition(d->lastPos);
            emit mouseXChanged(&me);
            me.setPosition(d->lastPos);
            emit mouseYChanged(&me);
            emit pressedChanged();
        } else {
            emit released(&me);
            me.setPosition(d->lastPos);
            emit pressedChanged();
            if (isclick && !d->longPress && !d->doubleClick){
                me.setAccepted(d->isClickConnected());
                emit clicked(&me);
                if (!me.isAccepted())
                    d->propagate(&me, QSGMouseAreaPrivate::Click);
            }
        }

        return me.isAccepted();
    }
    return false;
}

/*!
    \qmlproperty Item QtQuick2::MouseArea::drag.target
    \qmlproperty bool QtQuick2::MouseArea::drag.active
    \qmlproperty enumeration QtQuick2::MouseArea::drag.axis
    \qmlproperty real QtQuick2::MouseArea::drag.minimumX
    \qmlproperty real QtQuick2::MouseArea::drag.maximumX
    \qmlproperty real QtQuick2::MouseArea::drag.minimumY
    \qmlproperty real QtQuick2::MouseArea::drag.maximumY
    \qmlproperty bool QtQuick2::MouseArea::drag.filterChildren

    \c drag provides a convenient way to make an item draggable.

    \list
    \i \c drag.target specifies the id of the item to drag.
    \i \c drag.active specifies if the target item is currently being dragged.
    \i \c drag.axis specifies whether dragging can be done horizontally (\c Drag.XAxis), vertically (\c Drag.YAxis), or both (\c Drag.XandYAxis)
    \i \c drag.minimum and \c drag.maximum limit how far the target can be dragged along the corresponding axes.
    \endlist

    The following example displays a \l Rectangle that can be dragged along the X-axis. The opacity
    of the rectangle is reduced when it is dragged to the right.

    \snippet doc/src/snippets/declarative/mousearea/mousearea.qml drag

    \note Items cannot be dragged if they are anchored for the requested
    \c drag.axis. For example, if \c anchors.left or \c anchors.right was set
    for \c rect in the above example, it cannot be dragged along the X-axis.
    This can be avoided by settng the anchor value to \c undefined in
    an \l onPressed handler.

    If \c drag.filterChildren is set to true, a drag can override descendant MouseAreas.  This
    enables a parent MouseArea to handle drags, for example, while descendants handle clicks:

    \snippet doc/src/snippets/declarative/mousearea/mouseareadragfilter.qml dragfilter

*/

QSGDrag *QSGMouseArea::drag()
{
    Q_D(QSGMouseArea);
    if (!d->drag)
        d->drag = new QSGDrag;
    return d->drag;
}

QT_END_NAMESPACE
