import QtQuick 2.0

XmlListModel {
    query: "/data"
    XmlRole { name: "stringValue"; query: "a-string/string()" }
    XmlRole { name: "numberValue"; query: "a-number/number()" }
}

