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

Package {
    function photoClicked() {
        imageDetails.photoTitle = title;
        imageDetails.photoTags = tags;
        imageDetails.photoWidth = photoWidth;
        imageDetails.photoHeight = photoHeight;
        imageDetails.photoType = photoType;
        imageDetails.photoAuthor = photoAuthor;
        imageDetails.photoDate = photoDate;
        imageDetails.photoUrl = url;
        imageDetails.rating = 0;
        scaleMe.state = "Details";
    }

    Item {
        id: gridwrapper;
        width: GridView.view.cellWidth; height: GridView.view.cellHeight
        Package.name: "grid"
    }
    Item {
        id: streamwrapper;
        width: 80; height: 80
        Package.name: "stream"
        function pleaseFreeze(){
            ModelParticle.particle.freeze(streamwrapper);
        }
        function pleaseUnfreeze(){
            ModelParticle.particle.unfreeze(streamwrapper);
        }
    }
    Item {
        //anchors.centerIn: parent//Doesn't animate :(
        width: 80; height: 80
        scale: 0.0
        Behavior on scale { NumberAnimation { easing.type: Easing.InOutQuad} }
        id: scaleMe

        Item {
            id: whiteRectContainer
            width: 77; height: 77; anchors.centerIn: parent
            Rectangle {
                id: whiteRect; width: 77; height: 77; color: "#dddddd"; smooth: true
                x:0; y:0
                Image { id: thumb; source: imagePath; x: 1; y: 1; smooth: true }
                Image { source: "images/gloss.png" }
                MouseArea { anchors.fill: parent; onClicked: photoClicked() }
            }
        }

        Connections {
            target: toolBar
            onButton2Clicked: if (scaleMe.state == 'Details' ) scaleMe.state = 'Show'
        }

            state: 'inStream'
        states: [
            State {
                name: "Show"; when: thumb.status == Image.Ready
                PropertyChanges { target: scaleMe; scale: 1; }
            },
            State {
                name: "Details"
                PropertyChanges { target: scaleMe; scale: 1 }
                ParentChange { target: whiteRect; x: 10; y: 20; parent: imageDetails.frontContainer }
                PropertyChanges { target: background; state: "DetailedView" }
            }
        ]
        transitions: [
            Transition {
                from: "Show"; to: "Details"
                ScriptAction{ script: streamwrapper.pleaseFreeze(); }
                ParentAnimation {
                    via: foreground
                    NumberAnimation { properties: "x,y"; duration: 500; easing.type: Easing.InOutQuad }
                }
            },
            Transition {
                from: "Details"; to: "Show"
                SequentialAnimation{
                    ParentAnimation {
                        via: foreground
                        NumberAnimation { properties: "x,y"; duration: 500; easing.type: Easing.InOutQuad }
                    }
                    ScriptAction{ script: streamwrapper.pleaseUnfreeze(); }
                }
            }
        ]
        Item{
            id: stateContainer
            states: [
                State {
                    name: 'inStream'
                    when: screen.inGridView == false
                    ParentChange { 
                        target: scaleMe; parent: streamwrapper 
                        x: 0; y: 0; 
                    }
                },
                State {
                    name: 'inGrid'
                    when: screen.inGridView == true
                    ParentChange {
                        target: scaleMe; parent: gridwrapper
                        x: 0; y: 0;
                    }
                }
            ]

            transitions: [
                Transition {
                    ParentAnimation {
                        NumberAnimation { target: scaleMe; properties: 'x,y,width,height'; duration: 300 }
                    }
                }
            ]
        }
    }
}
