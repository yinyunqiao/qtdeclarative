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

import QtQuick 2.0

Rectangle {

    property string qmlfile: ""
    height: parent.height *.95; width: parent.width *.95; anchors.centerIn: parent; radius: 5

    onQmlfileChanged: { qmlapp.source = qmlfile; if (qmlfile != "") { starttimer.start(); } }

    Loader {
        id: qmlapp
        property int statenum: 0
        property int statecount
        statecount: qmlfile != "" ? children[0].states.length : 0
        anchors.fill: parent; focus: true
        function advance() { statenum = statenum == statecount ? 1 : ++statenum; }
    }

    Timer { id: starttimer; interval: 500; onTriggered: { qmlapp.advance(); } }

    Rectangle {
        anchors { top: parent.top; right: parent.right; topMargin: 3; rightMargin: 3 }
        height: 30; width: 30; color: "red"; radius: 5
        Text { text: "X"; anchors.centerIn: parent; font.pointSize: 12 }
        MouseArea { anchors.fill: parent; onClicked: { elementsapp.qmlfiletoload = "" } }
    }

    Text { anchors.centerIn: parent; visible: qmlapp.status == Loader.Error; text: qmlfile+" failed to load.\n"; }

}