// This file is part of IRC Chatter, the first IRC Client for MeeGo.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Copyright (C) 2012, Timur Kristóf <venemo@fedoraproject.org>

import QtQuick 2.0
import "../components"
import "../misc"

Item {
    id: serverSettingEntry

    property alias serverName: serverNameText.text
    property alias userName: userNameText.text
    property alias serverEnabled: theSwitch.value

    signal clicked

    implicitHeight: serverNameText.font.pixelSize * 1.5 + userNameText.font.pixelSize * 1.5 + 10
    width: parent.width

    MenuButton {
        id: button
        anchors {
            fill: parent
            leftMargin: -10
            rightMargin: -10
        }
        onEntered: tooltip.visible = true
        onExited: tooltip.visible = false
        onClicked: serverSettingEntry.clicked()

        Bubble {
            id: tooltip
            visible: false
            anchors.left: button.right
            anchors.leftMargin: -50
            anchors.verticalCenter: parent.verticalCenter
            padding: 10
            showLeftTab: true

            Text {
                width: parent.width
                wrapMode: Text.Wrap
                id: toolTipTextItem
                text: "Edit the server settings, or use the switch to decide whether to connect to the server."
            }
        }
    }
    Switch {
        id: theSwitch
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
    }
    Text {
        id: serverNameText
        text: "Your server name"
        font.bold: true
        color: "#fff"
        anchors.left: theSwitch.right
        anchors.top: serverSettingEntry.top
        anchors.leftMargin: 10
        anchors.topMargin: 5
    }
    Text {
        id: userNameText
        text: "Your user name"
        color: "#fff"
        anchors.left: theSwitch.right
        anchors.top: serverNameText.bottom
        anchors.leftMargin: 10
    }

}
