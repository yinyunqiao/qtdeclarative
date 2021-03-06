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

import QtQuick 2.0
import QtQuick.Particles 2.0

Item{
    id: container
    property alias model: mp.model
    property alias delegate: mp.delegate
    property bool jumpStarted: false
    ParticleSystem{ 
        id: sys 
        anchors.fill:parent
    }
    ModelParticle{
        id: mp
        fade: false
        system: sys
        anchors.fill: parent
        onModelCountChanged: {
            if(!jumpStarted && modelCount > 0){
                console.log("Jumping");
                jumpStarted = true;
                sys.fastForward(8000);
            }
        }
    }
    property real emitterSpacing: parent.width/3
    Emitter{
        system: sys
        width: emitterSpacing - 64
        x: emitterSpacing*0 + 32
        y: -128
        height: 32
        speed: PointDirection{ y: (container.height + 128)/12 }
        emitRate: 0.4
        lifeSpan: 1000000//eventually -1 should mean a million seconds for neatness
        emitCap: 15
    }
    Emitter{
        system: sys
        width: emitterSpacing - 64
        x: emitterSpacing*1 + 32
        y: -128
        height: 32
        speed: PointDirection{ y: (container.height + 128)/12 }
        emitRate: 0.4
        lifeSpan: 1000000//eventually -1 should mean a million seconds for neatness
        emitCap: 15
    }
    Emitter{
        system: sys
        width: emitterSpacing - 64
        x: emitterSpacing*2 + 32
        y: -128
        height: 32
        speed: PointDirection{ y: (container.height + 128)/12 }
        emitRate: 0.4
        lifeSpan: 1000000//eventually -1 should mean a million seconds for neatness
        emitCap: 15
    }
    Kill{
        system: sys
        y: container.height + 64
        width: container.width
        height: 6400
    }
}
