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
    id: gridviewelement; radius: 5; clip: true
    property int cellsize
    cellsize: Math.floor(elementview.width / 3)

    GridView {
        id: elementview
        height: parent.height * .98; width: parent.width * .98
        anchors.centerIn: parent
        delegate: griddelegate; model: elements; visible: { qmlfiletoload == "" }
        highlightFollowsCurrentItem: true; highlightRangeMode: ListView.StrictlyEnforceRange
        cellWidth: cellsize; cellHeight: cellsize
    }

    // Delegates for the launcher grid
    Component {
        id: griddelegate
        Item {
            height: cellsize; width: cellsize
            Column { anchors.fill: parent; spacing: 2
                Rectangle {
                    height: parent.height * .8; width: height
                    color: "lightgray"; radius: 5; smooth: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    Image {
                        height: parent.height*.75; width: height
                        anchors.centerIn: parent
                        source: "pics/logo.png"; fillMode: Image.PreserveAspectFit
                    }
                }
                Text {
                    height: parent.height * .8; width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter; text: model.label; elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true // For desktop testing
                onClicked: { runapp(model.label+"Element.qml"); }
                onEntered: { helptext = model.help }
                onExited: { helptext = "" }
            }
        }
    }

    // Elements list
    ListModel {
        id: elements
        ListElement { label: "Rectangle"; help: "The Rectangle item provides a filled rectangle with an optional border." }
        ListElement { label: "Image"; help: "The Image element displays an image in a declarative user interface" }
        ListElement { label: "AnimatedImage"; help: "An image element that supports animations" }
        ListElement { label: "BorderImage"; help: "The BorderImage element provides an image that can be used as a border." }
        ListElement { label: "SystemPalette"; help: "The SystemPalette element provides access to the Qt palettes." }
        ListElement { label: "Text"; help: "The Text item allows you to add formatted text to a scene." }
        ListElement { label: "FontLoader"; help: "The FontLoader element allows fonts to be loaded by name or URL." }
        ListElement { label: "TextInput"; help: "The TextInput item displays an editable line of text." }
        ListElement { label: "TextEdit"; help: "The TextEdit item displays multiple lines of editable formatted text." }
        ListElement { label: "ListView"; help: "The ListView item provides a list view of items provided by a model." }
        ListElement { label: "Flipable"; help: "The Flipable item provides a surface that can be flipped" }
        ListElement { label: "Column"; help: "The Column item arranges its children vertically." }
        ListElement { label: "Row"; help: "The Row item arranges its children horizontally." }
        ListElement { label: "Grid"; help: "The Grid item positions its children in a grid." }
        ListElement { label: "Flow"; help: "The Flow item arranges its children side by side, wrapping as necessary." }
        ListElement { label: "Repeater"; help: "The Repeater element allows you to repeat an Item-based component using a model." }
        ListElement { label: "IntValidator"; help: "This element provides a validator for integer values." }
        ListElement { label: "DoubleValidator"; help: "This element provides a validator for non-integer values." }
        ListElement { label: "RegExpValidator"; help: "This element provides a validator, which counts as valid any string which matches a specified regular expression." }
        ListElement { label: "Flickable"; help: "The Flickable item provides a surface that can be \"flicked\"." }
        ListElement { label: "Keys"; help: "The Keys attached property provides key handling to Items." }
        ListElement { label: "MouseArea"; help: "The MouseArea item enables simple mouse handling." }
        ListElement { label: "SequentialAnimation"; help: "The SequentialAnimation element allows animations to be run sequentially." }
        ListElement { label: "ParallelAnimation"; help: "The ParallelAnimation element allows animations to be run in parallel." }
        ListElement { label: "XmlListModel"; help: "The XmlListModel element is used to specify a read-only model using XPath expressions." }
        ListElement { label: "Scale"; help: "The Scale element provides a way to scale an Item." }
    }
}