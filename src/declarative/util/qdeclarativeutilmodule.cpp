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

#include "private/qdeclarativeutilmodule_p.h"
#include "private/qdeclarativeanimation_p.h"
#include "private/qdeclarativeanimation_p_p.h"
#include "private/qdeclarativebehavior_p.h"
#include "private/qdeclarativebind_p.h"
#include "private/qdeclarativeconnections_p.h"
#include "private/qdeclarativesmoothedanimation_p.h"
#include "private/qdeclarativefontloader_p.h"
#include "private/qdeclarativelistaccessor_p.h"
#include "private/qdeclarativelistmodel_p.h"
#include "private/qdeclarativenullablevalue_p_p.h"
#include "private/qdeclarativeopenmetaobject_p.h"
#include "private/qdeclarativepackage_p.h"
#include "private/qdeclarativepixmapcache_p.h"
#include "private/qdeclarativepropertychanges_p.h"
#include "qdeclarativepropertymap.h"
#include "private/qdeclarativespringanimation_p.h"
#include "private/qdeclarativestategroup_p.h"
#include "private/qdeclarativestateoperations_p.h"
#include "private/qdeclarativestate_p.h"
#include "private/qdeclarativestate_p_p.h"
#include "private/qdeclarativestyledtext_p.h"
#include "private/qdeclarativesystempalette_p.h"
#include "private/qdeclarativetimeline_p_p.h"
#include "private/qdeclarativetimer_p.h"
#include "private/qdeclarativetransitionmanager_p_p.h"
#include "private/qdeclarativetransition_p.h"
#include "private/qdeclarativeapplication_p.h"
#include "qdeclarativeinfo.h"
#include "private/qdeclarativetypenotavailable_p.h"
#ifndef QT_NO_XMLPATTERNS
#include "private/qdeclarativexmllistmodel_p.h"
#endif

void QDeclarativeUtilModule::registerBaseTypes(const char *uri, int versionMajor, int versionMinor)
{
    qmlRegisterType<QDeclarativeListElement>(uri, versionMajor, versionMinor,"ListElement");
    qmlRegisterCustomType<QDeclarativeListModel>(uri, versionMajor, versionMinor,"ListModel", new QDeclarativeListModelParser);
}

void QDeclarativeUtilModule::defineModule()
{
    qmlRegisterUncreatableType<QDeclarativeApplication>("QtQuick",2,0,"Application", QDeclarativeApplication::tr("Application is an abstract class"));

    qmlRegisterUncreatableType<QDeclarativeAbstractAnimation>("QtQuick",2,0,"Animation",QDeclarativeAbstractAnimation::tr("Animation is an abstract class"));

    qmlRegisterType<QDeclarativeBehavior>("QtQuick",2,0,"Behavior");
    qmlRegisterType<QDeclarativeBind>("QtQuick",2,0,"Binding");
    qmlRegisterType<QDeclarativeColorAnimation>("QtQuick",2,0,"ColorAnimation");
    qmlRegisterType<QDeclarativeConnections>("QtQuick",2,0,"Connections");
    qmlRegisterType<QDeclarativeSmoothedAnimation>("QtQuick",2,0,"SmoothedAnimation");
    qmlRegisterType<QDeclarativeFontLoader>("QtQuick",2,0,"FontLoader");
    qmlRegisterType<QDeclarativeNumberAnimation>("QtQuick",2,0,"NumberAnimation");
    qmlRegisterType<QDeclarativePackage>("QtQuick",2,0,"Package");
    qmlRegisterType<QDeclarativeParallelAnimation>("QtQuick",2,0,"ParallelAnimation");
    qmlRegisterType<QDeclarativePauseAnimation>("QtQuick",2,0,"PauseAnimation");
    qmlRegisterType<QDeclarativePropertyAction>("QtQuick",2,0,"PropertyAction");
    qmlRegisterType<QDeclarativePropertyAnimation>("QtQuick",2,0,"PropertyAnimation");
    qmlRegisterType<QDeclarativeRotationAnimation>("QtQuick",2,0,"RotationAnimation");
    qmlRegisterType<QDeclarativeScriptAction>("QtQuick",2,0,"ScriptAction");
    qmlRegisterType<QDeclarativeSequentialAnimation>("QtQuick",2,0,"SequentialAnimation");
    qmlRegisterType<QDeclarativeSpringAnimation>("QtQuick",2,0,"SpringAnimation");
    qmlRegisterType<QDeclarativeStateChangeScript>("QtQuick",2,0,"StateChangeScript");
    qmlRegisterType<QDeclarativeStateGroup>("QtQuick",2,0,"StateGroup");
    qmlRegisterType<QDeclarativeState>("QtQuick",2,0,"State");
    qmlRegisterType<QDeclarativeSystemPalette>("QtQuick",2,0,"SystemPalette");
    qmlRegisterType<QDeclarativeTimer>("QtQuick",2,0,"Timer");
    qmlRegisterType<QDeclarativeTransition>("QtQuick",2,0,"Transition");
    qmlRegisterType<QDeclarativeVector3dAnimation>("QtQuick",2,0,"Vector3dAnimation");
#ifdef QT_NO_XMLPATTERNS
    qmlRegisterTypeNotAvailable("QtQuick",2,0,"XmlListModel",
        qApp->translate("QDeclarativeXmlListModel","Qt was built without support for xmlpatterns"));
    qmlRegisterTypeNotAvailable("QtQuick",2,0,"XmlRole",
        qApp->translate("QDeclarativeXmlListModel","Qt was built without support for xmlpatterns"));
#else
    qmlRegisterType<QDeclarativeXmlListModel>("QtQuick",2,0,"XmlListModel");
    qmlRegisterType<QDeclarativeXmlListModelRole>("QtQuick",2,0,"XmlRole");
#endif

    qmlRegisterType<QDeclarativeStateOperation>();

    qmlRegisterCustomType<QDeclarativePropertyChanges>("QtQuick",2,0,"PropertyChanges", new QDeclarativePropertyChangesParser);
    qmlRegisterCustomType<QDeclarativeConnections>("QtQuick",2,0,"Connections", new QDeclarativeConnectionsParser);
    registerBaseTypes("QtQuick",2,0);
}
