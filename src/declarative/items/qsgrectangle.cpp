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

#include "qsgrectangle_p.h"
#include "qsgrectangle_p_p.h"

#include <private/qsgcontext_p.h>
#include <private/qsgadaptationlayer_p.h>

#include <QtGui/qpixmapcache.h>
#include <QtCore/qstringbuilder.h>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

// XXX todo - should we change rectangle to draw entirely within its width/height?
/*!
    \internal
    \class QSGPen
    \brief The QSGPen class provides a pen used for drawing rectangle borders on a QSGView.

    By default, the pen is invalid and nothing is drawn. You must either set a color (then the default
    width is 1) or a width (then the default color is black).

    A width of 1 indicates is a single-pixel line on the border of the item being painted.

    Example:
    \qml
    Rectangle {
        border.width: 2
        border.color: "red"
    }
    \endqml
*/

QSGPen::QSGPen(QObject *parent)
    : QObject(parent)
    , m_width(1)
    , m_color("#000000")
    , m_aligned(true)
    , m_valid(false)
{
}

qreal QSGPen::width() const
{
    return m_width;
}

void QSGPen::setWidth(qreal w)
{
    if (m_width == w && m_valid)
        return;

    m_width = w;
    m_valid = m_color.alpha() && (qRound(m_width) >= 1 || (!m_aligned && m_width > 0));
    emit penChanged();
}

QColor QSGPen::color() const
{
    return m_color;
}

void QSGPen::setColor(const QColor &c)
{
    m_color = c;
    m_valid = m_color.alpha() && (qRound(m_width) >= 1 || (!m_aligned && m_width > 0));
    emit penChanged();
}

bool QSGPen::aligned() const
{
    return m_aligned;
}

void QSGPen::setAligned(bool aligned)
{
    if (aligned == m_aligned)
        return;
    m_aligned = aligned;
    m_valid = m_color.alpha() && (qRound(m_width) >= 1 || (!m_aligned && m_width > 0));
    emit penChanged();
}

bool QSGPen::isValid() const
{
    return m_valid;
}

/*!
    \qmlclass GradientStop QSGGradientStop
    \inqmlmodule QtQuick 2
    \ingroup qml-basic-visual-elements
    \brief The GradientStop item defines the color at a position in a Gradient.

    \sa Gradient
*/

/*!
    \qmlproperty real QtQuick2::GradientStop::position
    \qmlproperty color QtQuick2::GradientStop::color

    The position and color properties describe the color used at a given
    position in a gradient, as represented by a gradient stop.

    The default position is 0.0; the default color is black.

    \sa Gradient
*/
QSGGradientStop::QSGGradientStop(QObject *parent)
    : QObject(parent)
{
}

qreal QSGGradientStop::position() const
{
    return m_position;
}

void QSGGradientStop::setPosition(qreal position)
{
    m_position = position; updateGradient();
}

QColor QSGGradientStop::color() const
{
    return m_color;
}

void QSGGradientStop::setColor(const QColor &color)
{
    m_color = color; updateGradient();
}

void QSGGradientStop::updateGradient()
{
    if (QSGGradient *grad = qobject_cast<QSGGradient*>(parent()))
        grad->doUpdate();
}

/*!
    \qmlclass Gradient QSGGradient
    \inqmlmodule QtQuick 2
    \ingroup qml-basic-visual-elements
    \brief The Gradient item defines a gradient fill.

    A gradient is defined by two or more colors, which will be blended seamlessly.

    The colors are specified as a set of GradientStop child items, each of
    which defines a position on the gradient from 0.0 to 1.0 and a color.
    The position of each GradientStop is defined by setting its
    \l{GradientStop::}{position} property; its color is defined using its
    \l{GradientStop::}{color} property.

    A gradient without any gradient stops is rendered as a solid white fill.

    Note that this item is not a visual representation of a gradient. To display a
    gradient, use a visual element (like \l Rectangle) which supports the use
    of gradients.

    \section1 Example Usage

    \div {class="float-right"}
    \inlineimage qml-gradient.png
    \enddiv

    The following example declares a \l Rectangle item with a gradient starting
    with red, blending to yellow at one third of the height of the rectangle,
    and ending with green:

    \snippet doc/src/snippets/declarative/gradient.qml code

    \clearfloat
    \section1 Performance and Limitations

    Calculating gradients can be computationally expensive compared to the use
    of solid color fills or images. Consider using gradients for static items
    in a user interface.

    In Qt 4.7, only vertical, linear gradients can be applied to items. If you
    need to apply different orientations of gradients, a combination of rotation
    and clipping will need to be applied to the relevant items. This can
    introduce additional performance requirements for your application.

    The use of animations involving gradient stops may not give the desired
    result. An alternative way to animate gradients is to use pre-generated
    images or SVG drawings containing gradients.

    \sa GradientStop
*/

/*!
    \qmlproperty list<GradientStop> QtQuick2::Gradient::stops
    \default

    This property holds the gradient stops describing the gradient.

    By default, this property contains an empty list.

    To set the gradient stops, define them as children of the Gradient element.
*/
QSGGradient::QSGGradient(QObject *parent)
: QObject(parent), m_gradient(0)
{
}

QSGGradient::~QSGGradient()
{
    delete m_gradient;
}

QDeclarativeListProperty<QSGGradientStop> QSGGradient::stops()
{
    return QDeclarativeListProperty<QSGGradientStop>(this, m_stops);
}

const QGradient *QSGGradient::gradient() const
{
    if (!m_gradient && !m_stops.isEmpty()) {
        m_gradient = new QLinearGradient(0,0,0,1.0);
        for (int i = 0; i < m_stops.count(); ++i) {
            const QSGGradientStop *stop = m_stops.at(i);
            m_gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
            m_gradient->setColorAt(stop->position(), stop->color());
        }
    }

    return m_gradient;
}

void QSGGradient::doUpdate()
{
    delete m_gradient;
    m_gradient = 0;
    emit updated();
}

int QSGRectanglePrivate::doUpdateSlotIdx = -1;

/*!
    \qmlclass Rectangle QSGRectangle
    \inqmlmodule QtQuick 2
    \ingroup qml-basic-visual-elements
    \brief The Rectangle item provides a filled rectangle with an optional border.
    \inherits Item

    Rectangle items are used to fill areas with solid color or gradients, and are
    often used to hold other items.

    \section1 Appearance

    Each Rectangle item is painted using either a solid fill color, specified using
    the \l color property, or a gradient, defined using a Gradient element and set
    using the \l gradient property. If both a color and a gradient are specified,
    the gradient is used.

    You can add an optional border to a rectangle with its own color and thickness
    by settting the \l border.color and \l border.width properties.

    You can also create rounded rectangles using the \l radius property. Since this
    introduces curved edges to the corners of a rectangle, it may be appropriate to
    set the \l smooth property to improve its appearance.

    \section1 Example Usage

    \div {class="float-right"}
    \inlineimage declarative-rect.png
    \enddiv

    The following example shows the effects of some of the common properties on a
    Rectangle item, which in this case is used to create a square:

    \snippet doc/src/snippets/declarative/rectangle/rectangle.qml document

    \clearfloat
    \section1 Performance

    Using the \l smooth property improves the appearance of a rounded rectangle at
    the cost of rendering performance. You should consider unsetting this property
    for rectangles in motion, and only set it when they are stationary.

    \sa Image
*/

QSGRectangle::QSGRectangle(QSGItem *parent)
: QSGItem(*(new QSGRectanglePrivate), parent)
{
    setFlag(ItemHasContents);
}

void QSGRectangle::doUpdate()
{
    Q_D(QSGRectangle);
    const int pw = d->pen && d->pen->isValid() ? d->pen->width() : 0;
    d->setPaintMargin((pw+1)/2);
    update();
}

/*!
    \qmlproperty int QtQuick2::Rectangle::border.width
    \qmlproperty color QtQuick2::Rectangle::border.color

    The width and color used to draw the border of the rectangle.

    A width of 1 creates a thin line. For no line, use a width of 0 or a transparent color.

    \note The width of the rectangle's border does not affect the geometry of the
    rectangle itself or its position relative to other items if anchors are used.

    If \c border.width is an odd number, the rectangle is painted at a half-pixel offset to retain
    border smoothness. Also, the border is rendered evenly on either side of the
    rectangle's boundaries, and the spare pixel is rendered to the right and below the
    rectangle (as documented for QRect rendering). This can cause unintended effects if
    \c border.width is 1 and the rectangle is \l{Item::clip}{clipped} by a parent item:

    \div {class="float-right"}
    \inlineimage rect-border-width.png
    \enddiv

    \snippet doc/src/snippets/declarative/rectangle/rect-border-width.qml 0

    \clearfloat
    Here, the innermost rectangle's border is clipped on the bottom and right edges by its
    parent. To avoid this, the border width can be set to two instead of one.
*/
QSGPen *QSGRectangle::border()
{
    Q_D(QSGRectangle);
    return d->getPen();
}

/*!
    \qmlproperty Gradient QtQuick2::Rectangle::gradient

    The gradient to use to fill the rectangle.

    This property allows for the construction of simple vertical gradients.
    Other gradients may by formed by adding rotation to the rectangle.

    \div {class="float-left"}
    \inlineimage declarative-rect_gradient.png
    \enddiv

    \snippet doc/src/snippets/declarative/rectangle/rectangle-gradient.qml rectangles
    \clearfloat

    If both a gradient and a color are specified, the gradient will be used.

    \sa Gradient, color
*/
QSGGradient *QSGRectangle::gradient() const
{
    Q_D(const QSGRectangle);
    return d->gradient;
}

void QSGRectangle::setGradient(QSGGradient *gradient)
{
    Q_D(QSGRectangle);
    if (d->gradient == gradient)
        return;
    static int updatedSignalIdx = -1;
    if (updatedSignalIdx < 0)
        updatedSignalIdx = QSGGradient::staticMetaObject.indexOfSignal("updated()");
    if (d->doUpdateSlotIdx < 0)
        d->doUpdateSlotIdx = QSGRectangle::staticMetaObject.indexOfSlot("doUpdate()");
    if (d->gradient)
        QMetaObject::disconnect(d->gradient, updatedSignalIdx, this, d->doUpdateSlotIdx);
    d->gradient = gradient;
    if (d->gradient)
        QMetaObject::connect(d->gradient, updatedSignalIdx, this, d->doUpdateSlotIdx);
    update();
}

/*!
    \qmlproperty real QtQuick2::Rectangle::radius
    This property holds the corner radius used to draw a rounded rectangle.

    If radius is non-zero, the rectangle will be painted as a rounded rectangle, otherwise it will be
    painted as a normal rectangle. The same radius is used by all 4 corners; there is currently
    no way to specify different radii for different corners.
*/
qreal QSGRectangle::radius() const
{
    Q_D(const QSGRectangle);
    return d->radius;
}

void QSGRectangle::setRadius(qreal radius)
{
    Q_D(QSGRectangle);
    if (d->radius == radius)
        return;

    d->radius = radius;
    update();
    emit radiusChanged();
}

/*!
    \qmlproperty color QtQuick2::Rectangle::color
    This property holds the color used to fill the rectangle.

    The default color is white.

    \div {class="float-right"}
    \inlineimage rect-color.png
    \enddiv

    The following example shows rectangles with colors specified
    using hexadecimal and named color notation:

    \snippet doc/src/snippets/declarative/rectangle/rectangle-colors.qml rectangles

    \clearfloat
    If both a gradient and a color are specified, the gradient will be used.

    \sa gradient
*/
QColor QSGRectangle::color() const
{
    Q_D(const QSGRectangle);
    return d->color;
}

void QSGRectangle::setColor(const QColor &c)
{
    Q_D(QSGRectangle);
    if (d->color == c)
        return;

    d->color = c;
    update();
    emit colorChanged();
}

QSGNode *QSGRectangle::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    Q_D(QSGRectangle);

    if (width() <= 0 || height() <= 0) {
        delete oldNode;
        return 0;
    }

    QSGRectangleNode *rectangle = static_cast<QSGRectangleNode *>(oldNode);
    if (!rectangle) rectangle = d->sceneGraphContext()->createRectangleNode();

    rectangle->setRect(QRectF(0, 0, width(), height()));
    rectangle->setColor(d->color);

    if (d->pen && d->pen->isValid()) {
        rectangle->setPenColor(d->pen->color());
        rectangle->setPenWidth(d->pen->width());
        rectangle->setAligned(d->pen->aligned());
    } else {
        rectangle->setPenWidth(0);
    }

    rectangle->setRadius(d->radius);

    QGradientStops stops;
    if (d->gradient) {
        QList<QSGGradientStop *> qxstops = d->gradient->m_stops;
        for (int i = 0; i < qxstops.size(); ++i){
            int j = 0;
            while (j < stops.size() && stops.at(j).first < qxstops[i]->position())
                j++;
            stops.insert(j, QGradientStop(qxstops.at(i)->position(), qxstops.at(i)->color()));
        }
    }
    rectangle->setGradientStops(stops);

    rectangle->update();

    return rectangle;
}
/*!
    \qmlproperty bool QtQuick2::Rectangle::smooth

    Set this property if you want the item to be smoothly scaled or
    transformed.  Smooth filtering gives better visual quality, but is slower.  If
    the item is displayed at its natural size, this property has no visual or
    performance effect.

    \note Generally scaling artifacts are only visible if the item is stationary on
    the screen.  A common pattern when animating an item is to disable smooth
    filtering at the beginning of the animation and reenable it at the conclusion.

    \image rect-smooth.png
    On this image, smooth is turned off on the top half and on on the bottom half.
*/

QRectF QSGRectangle::boundingRect() const
{
    Q_D(const QSGRectangle);
    return QRectF(-d->paintmargin, -d->paintmargin, width()+d->paintmargin*2, height()+d->paintmargin*2);
}

QT_END_NAMESPACE
