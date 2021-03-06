/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtQuick.Particles 2.0

Rectangle {
    id: root
    width: 360
    height: 540
    color: "black"

    ParticleSystem{
        id: particles
    }

    /*
    ImageParticle{
        id: fireball
        anchors.fill: parent
        particles: ["E"]
        system: particles
        source: "content/particleA.png"
        colorVariation: 0.2
        color: "#00ff400f"
    }
    */
    ImageParticle{
        id: smoke
        system: particles
        anchors.fill: parent
        particles: ["A", "B"]
        source: "content/particle.png"
        colorVariation: 0
        color: "#00111111"
    }
    ImageParticle{
        id: flame
        anchors.fill: parent
        system: particles
        particles: ["C", "D"]
        source: "content/particle.png"
        colorVariation: 0.1
        color: "#00ff400f"
    }
    Emitter{
        id: fire
        system: particles
        particle: "C"

        y: parent.height
        width: parent.width

        emitRate: 350
        lifeSpan: 3500

        acceleration: PointDirection{ y: -17; xVariation: 3 }
        speed: PointDirection{xVariation: 3}

        size: 24
        sizeVariation: 8
        endSize: 4
    }
    FollowEmitter{
        id: fireSmoke
        particle: "B"
        system: particles
        follow: "C"
        width: root.width
        height: root.height - 68

        emitRatePerParticle: 1
        lifeSpan: 2000

        speed: PointDirection{y:-17*6; yVariation: -17; xVariation: 3}
        acceleration: PointDirection{xVariation: 3}

        size: 36
        sizeVariation: 8
        endSize: 16
    }
    FollowEmitter{
        id: fireballFlame
        anchors.fill: parent
        system: particles
        particle: "D"
        follow: "E"

        emitRatePerParticle: 120
        lifeSpan: 180
        emitWidth: 8
        emitHeight: 8

        size: 16
        sizeVariation: 4
        endSize: 4
    }
    
    FollowEmitter{
        id: fireballSmoke
        anchors.fill: parent
        system: particles
        particle: "A"
        follow: "E"

        emitRatePerParticle: 128
        lifeSpan: 2400
        emitWidth: 16
        emitHeight: 16

        speed: PointDirection{yVariation: 16; xVariation: 16}
        acceleration: PointDirection{y: -16}

        size: 24
        sizeVariation: 8
        endSize: 8
    }
    Emitter{
        id: balls
        system: particles
        particle: "E"

        y: parent.height
        width: parent.width

        emitRate: 2
        lifeSpan: 7000

        speed: PointDirection{y:-17*4*2; xVariation: 6*6}
        acceleration: PointDirection{y: 17*2; xVariation: 6*6}

        size: 12
        sizeVariation: 4
    }

}

