/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Declarative module of the Qt Toolkit.
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

#include "qsgparticleemitter_p.h"
QT_BEGIN_NAMESPACE



/*!
    \qmlclass Emitter QSGParticleEmitter
    \inqmlmodule QtQuick.Particles 2
    \brief The Emitter element allows you to emit logical particles.

    This element emits logical particles into the ParticleSystem, with the
    given starting attributes.

    Note that logical particles are not
    automatically rendered, you will need to have one or more
    ParticlePainter elements visualizing them.

    Note that the given starting attributes can be modified at any point
    in the particle's lifetime by any Affector element in the same
    ParticleSystem. This includes attributes like lifespan.
*/


/*!
    \qmlproperty ParticleSystem QtQuick.Particles2::Emitter::system

    This is the Particle system that the Emitter will emit into.
    This can be omitted if the Emitter is a direct child of the ParticleSystem
*/
/*!
    \qmlproperty string QtQuick.Particles2::Emitter::particle

    This is the type of logical particle which it will emit.

    Default value is "" (empty string).
*/
/*!
    \qmlproperty Shape QtQuick.Particles2::Emitter::shape

    This shape is applied to the bounding box of the emitter. Particles are then emitting
    from inside the area of the shape.

*/
/*!
    \qmlproperty bool QtQuick.Particles2::Emitter::emitting

    If set to false, the emitter will cease emissions until it is set to true.

    Default value is true.
*/
/*!
    \qmlproperty real QtQuick.Particles2::Emitter::emitRate

    Number of particles emitted per second.

    Default value is 10 particles per second.
*/
/*!
    \qmlproperty int QtQuick.Particles2::Emitter::lifeSpan

    The time in milliseconds each emitted particle should last for.

    Default value is 1000 (one second).
*/
/*!
    \qmlproperty int QtQuick.Particles2::Emitter::lifeSpanVariation

    Particle lifespans will vary by up to this much in either direction.

    Default value is 0.
*/

/*!
    \qmlproperty int QtQuick.Particles2::Emitter::emitCap

    The maximum number of particles at a time that this emitter will have alive.

    This can be set as a performance optimization (when using burst and pulse) or
    to stagger emissions. The default value is emitRate * lifeSpan in seconds, which
    is the number of particles that would be alive at any one time given the default settings.
*/
/*!
    \qmlproperty bool QtQuick.Particles2::Emitter::noCap

    If set to true, the emitCap will be ignored and this emitter will never skip emitting
    a particle based on how many it has alive.

    Default value is false.
*/
/*!
    \qmlproperty int QtQuick.Particles2::Emitter::startTime

    If this value is set when the emitter is loaded, then it will emit particles from the
    past, up to startTime milliseconds ago. These will simulate as if they were emitted then,
    but will not have any affectors applied to them. Affectors will take effect from the present time.
*/
/*!
    \qmlproperty real QtQuick.Particles2::Emitter::size

    The size in pixels of the particles at the start of their life.

    Default value is 16.
*/
/*!
    \qmlproperty real QtQuick.Particles2::Emitter::endSize

    The size in pixels of the particles at the end of their life. Size will
    be linearly interpolated during the life of the particle from this value and
    size. If endSize is -1, then the size of the particle will remain constant at
    the starting size.

    Default value is -1.
*/
/*!
    \qmlproperty real QtQuick.Particles2::Emitter::sizeVariation

    The size of a particle can vary by this much up or down from size/endSize. The same
    random addition is made to both size and endSize for a single particle.

    Default value is 0.
*/
/*!
    \qmlproperty StochasticDirection QtQuick.Particles2::Emitter::speed

    The starting speed of the particles emitted.
*/
/*!
    \qmlproperty StochasticDirection QtQuick.Particles2::Emitter::acceleration

    The starting acceleraton of the particles emitted.
*/
/*!
    \qmlproperty qreal QtQuick.Particles2::Emitter::speedFromMovement

    If this value is non-zero, then any movement of the emitter will provide additional
    starting velocity to the particles based on the movement. The additional vector will be the
    same angle as the emitter's movement, with a magnitude that is the magnitude of the emitters
    movement multiplied by speedFromMovement.

    Default value is 0.
*/

QSGParticleEmitter::QSGParticleEmitter(QSGItem *parent) :
    QSGItem(parent)
  , m_particlesPerSecond(10)
  , m_particleDuration(1000)
  , m_particleDurationVariation(0)
  , m_emitting(true)
  , m_system(0)
  , m_extruder(0)
  , m_defaultExtruder(0)
  , m_speed(&m_nullVector)
  , m_acceleration(&m_nullVector)
  , m_particleSize(16)
  , m_particleEndSize(-1)
  , m_particleSizeVariation(0)
  , m_maxParticleCount(-1)
  , m_burstLeft(0)
  , m_speed_from_movement(0)
  , m_particle_count(0)
  , m_reset_last(true)
  , m_last_timestamp(-1)
  , m_last_emission(0)
  , m_startTime(0)
  , m_overwrite(false)

{
    //TODO: Reset speed/acc back to null vector? Or allow null pointer?
    connect(this, SIGNAL(maxParticleCountChanged(int)),
            this, SIGNAL(particleCountChanged()));
    connect(this, SIGNAL(particlesPerSecondChanged(qreal)),
            this, SIGNAL(particleCountChanged()));
    connect(this, SIGNAL(particleDurationChanged(int)),
            this, SIGNAL(particleCountChanged()));
}

QSGParticleEmitter::~QSGParticleEmitter()
{
    if (m_defaultExtruder)
        delete m_defaultExtruder;
}

void QSGParticleEmitter::componentComplete()
{
    if (!m_system && qobject_cast<QSGParticleSystem*>(parentItem()))
        setSystem(qobject_cast<QSGParticleSystem*>(parentItem()));
    if (!m_system)
        qWarning() << "Emitter created without a particle system specified";//TODO: useful QML warnings, like line number?
    QSGItem::componentComplete();
}

void QSGParticleEmitter::setEmitting(bool arg)
{
    if (m_emitting != arg) {
        m_emitting = arg;
        emit emittingChanged(arg);
    }
}


QSGParticleExtruder* QSGParticleEmitter::effectiveExtruder()
{
    if (m_extruder)
        return m_extruder;
    if (!m_defaultExtruder)
        m_defaultExtruder = new QSGParticleExtruder;
    return m_defaultExtruder;
}

void QSGParticleEmitter::pulse(qreal seconds)
{
    if (!particleCount())
        qWarning() << "pulse called on an emitter with a particle count of zero";
    if (!m_emitting)
        m_burstLeft = seconds*1000.0;//TODO: Change name to match
}

void QSGParticleEmitter::burst(int num)
{
    if (!particleCount())
        qWarning() << "burst called on an emitter with a particle count of zero";
    m_burstQueue << qMakePair(num, QPointF(x(), y()));
}

void QSGParticleEmitter::burst(int num, qreal x, qreal y)
{
    if (!particleCount())
        qWarning() << "burst called on an emitter with a particle count of zero";
    m_burstQueue << qMakePair(num, QPointF(x, y));
}

void QSGParticleEmitter::setMaxParticleCount(int arg)
{
    if (m_maxParticleCount != arg) {
        if (arg < 0 && m_maxParticleCount >= 0){
            connect(this, SIGNAL(particlesPerSecondChanged(qreal)),
                    this, SIGNAL(particleCountChanged()));
            connect(this, SIGNAL(particleDurationChanged(int)),
                    this, SIGNAL(particleCountChanged()));
        }else if (arg >= 0 && m_maxParticleCount < 0){
            disconnect(this, SIGNAL(particlesPerSecondChanged(qreal)),
                    this, SIGNAL(particleCountChanged()));
            disconnect(this, SIGNAL(particleDurationChanged(int)),
                    this, SIGNAL(particleCountChanged()));
        }
        m_maxParticleCount = arg;
        emit maxParticleCountChanged(arg);
    }
}

int QSGParticleEmitter::particleCount() const
{
    if (m_maxParticleCount >= 0)
        return m_maxParticleCount;
    return m_particlesPerSecond*((m_particleDuration+m_particleDurationVariation)/1000.0);
}

void QSGParticleEmitter::setSpeedFromMovement(qreal t)
{
    if (t == m_speed_from_movement)
        return;
    m_speed_from_movement = t;
    emit speedFromMovementChanged();
}

void QSGParticleEmitter::emitWindow(int timeStamp)
{
    if (m_system == 0)
        return;
    if ((!m_emitting || !m_particlesPerSecond)&& !m_burstLeft && m_burstQueue.isEmpty()){
        m_reset_last = true;
        return;
    }

    if (m_reset_last) {
        m_last_emitter = m_last_last_emitter = QPointF(x(), y());
        if (m_last_timestamp == -1)
            m_last_timestamp = timeStamp/1000. - m_startTime;
        else
            m_last_timestamp = timeStamp/1000.;
        m_last_emission = m_last_timestamp;
        m_reset_last = false;
    }

    if (m_burstLeft){
        m_burstLeft -= timeStamp - m_last_timestamp * 1000.;
        if (m_burstLeft < 0){
            if (!m_emitting)
                timeStamp += m_burstLeft;
            m_burstLeft = 0;
        }
    }

    qreal time = timeStamp / 1000.;

    qreal particleRatio = 1. / m_particlesPerSecond;
    qreal pt = m_last_emission;

    qreal opt = pt; // original particle time
    qreal dt = time - m_last_timestamp; // timestamp delta...
    if (!dt)
        dt = 0.000001;

    // emitter difference since last...
    qreal dex = (x() - m_last_emitter.x());
    qreal dey = (y() - m_last_emitter.y());

    qreal ax = (m_last_last_emitter.x() + m_last_emitter.x()) / 2;
    qreal bx = m_last_emitter.x();
    qreal cx = (x() + m_last_emitter.x()) / 2;
    qreal ay = (m_last_last_emitter.y() + m_last_emitter.y()) / 2;
    qreal by = m_last_emitter.y();
    qreal cy = (y() + m_last_emitter.y()) / 2;

    qreal sizeAtEnd = m_particleEndSize >= 0 ? m_particleEndSize : m_particleSize;
    qreal emitter_x_offset = m_last_emitter.x() - x();
    qreal emitter_y_offset = m_last_emitter.y() - y();
    if (!m_burstQueue.isEmpty() && !m_burstLeft && !m_emitting)//'outside time' emissions only
        pt = time;
    while (pt < time || !m_burstQueue.isEmpty()) {
        //int pos = m_last_particle % m_particle_count;
        QSGParticleData* datum = m_system->newDatum(m_system->m_groupIds[m_particle], !m_overwrite);
        if (datum){//actually emit(otherwise we've been asked to skip this one)
            datum->e = this;//###useful?
            qreal t = 1 - (pt - opt) / dt;
            qreal vx =
              - 2 * ax * (1 - t)
              + 2 * bx * (1 - 2 * t)
              + 2 * cx * t;
            qreal vy =
              - 2 * ay * (1 - t)
              + 2 * by * (1 - 2 * t)
              + 2 * cy * t;


            // Particle timestamp
            datum->t = pt;
            datum->lifeSpan = //TODO:Promote to base class?
                    (m_particleDuration
                     + ((rand() % ((m_particleDurationVariation*2) + 1)) - m_particleDurationVariation))
                    / 1000.0;

            // Particle position
            QRectF boundsRect;
            if (!m_burstQueue.isEmpty()){
                boundsRect = QRectF(m_burstQueue.first().second.x() - x(), m_burstQueue.first().second.y() - y(),
                        width(), height());
            } else {
                boundsRect = QRectF(emitter_x_offset + dex * (pt - opt) / dt, emitter_y_offset + dey * (pt - opt) / dt
                              , width(), height());
            }
            QPointF newPos = effectiveExtruder()->extrude(boundsRect);
            datum->x = newPos.x();
            datum->y = newPos.y();

            // Particle speed
            const QPointF &speed = m_speed->sample(newPos);
            datum->vx = speed.x()
                    + m_speed_from_movement * vx;
            datum->vy = speed.y()
                    + m_speed_from_movement * vy;

            // Particle acceleration
            const QPointF &accel = m_acceleration->sample(newPos);
            datum->ax = accel.x();
            datum->ay = accel.y();

            // Particle size
            float sizeVariation = -m_particleSizeVariation
                    + rand() / float(RAND_MAX) * m_particleSizeVariation * 2;

            float size = qMax((qreal)0.0 , m_particleSize + sizeVariation);
            float endSize = qMax((qreal)0.0 , sizeAtEnd + sizeVariation);

            datum->size = size;// * float(m_emitting);
            datum->endSize = endSize;// * float(m_emitting);

            m_system->emitParticle(datum);
        }
        if (m_burstQueue.isEmpty()){
            pt += particleRatio;
        }else{
            m_burstQueue.first().first--;
            if (m_burstQueue.first().first <= 0)
                m_burstQueue.pop_front();
        }
    }
    m_last_emission = pt;

    m_last_last_last_emitter = m_last_last_emitter;
    m_last_last_emitter = m_last_emitter;
    m_last_emitter = QPointF(x(), y());
    m_last_timestamp = time;
}


QT_END_NAMESPACE
