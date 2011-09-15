/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of QtUiTest.
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

Item {
    id: doublevalidatorelementtest
    anchors.fill: parent
    property string testtext: ""

    DoubleValidator {
        id: doublevalidatorelement
        top: 30.06; bottom: 30.02
        //Behavior on top { NumberAnimation { duration: 1000 } }
    }

    Rectangle {
        id: doublevalidatorelementbackground
        color: doublevalidatorelementinput.acceptableInput ? "green" : "red"; height: 50; width: parent.width *.8
        border.color: "gray"; opacity: 0.7; radius: 5
        anchors.centerIn: parent

        TextInput {
            id: doublevalidatorelementinput
            font.pointSize: 12; width: parent.width; text: "0"; horizontalAlignment: Text.AlignHCenter; validator: doublevalidatorelement
            anchors.centerIn: parent
            Behavior on font.pointSize { NumberAnimation { duration: 1000 } }
            Behavior on color { ColorAnimation { duration: 1000 } }
        }
    }

    Text{
        anchors.top: doublevalidatorelementbackground.bottom; anchors.topMargin: 50; anchors.horizontalCenter: parent.horizontalCenter
        text: "Top: " + doublevalidatorelement.top + " Bottom: " + doublevalidatorelement.bottom
    }

    SystemTestHelp { id: helpbubble; visible: statenum != 0
        anchors { top: parent.top; horizontalCenter: parent.horizontalCenter; topMargin: 50 }
    }
    BugPanel { id: bugpanel }



    states: [
        State { name: "start"; when: statenum == 1
            PropertyChanges { target: doublevalidatorelementinput; text: "30.03" }
            PropertyChanges { target: doublevalidatorelementtest
                testtext: "This is a TextInput element using an DoubleValidator for input masking. At present it should be indicating 30.03.\n"+
                "Next, let's attempt to enter a number smaller than the lowest allowable value: 30.01" }
        },
        State { name: "lowerthan"; when: statenum == 2
            PropertyChanges { target: doublevalidatorelementinput; text: "30.01" }
            PropertyChanges { target: doublevalidatorelementtest
                testtext: "The TextInput background should be showing red - input is not acceptable.\n"+
                "Next, let's enter a value equal to the lowest allowable value: 30.02." }
        },
        State { name: "equaltobottom"; when: statenum == 3
            PropertyChanges { target: doublevalidatorelementinput; text: "30.02" }
            PropertyChanges { target: doublevalidatorelementtest
                testtext: "The TextInput background should be showing green - input is acceptable.\n"+
                "Next, let's attempt to enter a number greater than the highest allowable value: 30.08" }
        },
        State { name: "greaterthan"; when: statenum == 4
            PropertyChanges { target: doublevalidatorelementinput; text: "30.08" }
            PropertyChanges { target: doublevalidatorelementtest
                testtext: "The TextInput background should be showing red - input is not acceptable.\n"+
                "Next, let's change the value of top to be greater than the entered number." }
        },
        State { name: "increasedtop"; when: statenum == 5
            PropertyChanges { target: doublevalidatorelementinput; text: "30.08" }
            PropertyChanges { target: doublevalidatorelement; top: 30.09 }
            PropertyChanges { target: doublevalidatorelementtest
                testtext: "The highest value should have increased to 30.09, thus making the number valid and turning the input background green.\n"+
                "Next, let's change the value of bottom to be greater than the entered number." }
            PropertyChanges { target: bugpanel; bugnumber: "19956" }
        },
        State { name: "increasedbottom"; when: statenum == 6
            PropertyChanges { target: doublevalidatorelementinput; text: "30.08" }
            PropertyChanges { target: doublevalidatorelement; top: 30.09; bottom: 40.05 }
            PropertyChanges { target: doublevalidatorelementtest
                testtext: "The lowest value should have increased to 36, thus making the validator invalid and turning the input background red.\n"+
                "Press advance to restart the test." }
        }
    ]

}