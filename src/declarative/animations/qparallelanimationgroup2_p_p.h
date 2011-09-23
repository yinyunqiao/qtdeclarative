/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QPARALLELANIMATIONGROUP2_P_P_H
#define QPARALLELANIMATIONGROUP2_P_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QIODevice. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qparallelanimationgroup2_p.h"
#include "private/qanimationgroup2_p_p.h"
#include <QtCore/qhash.h>



QT_BEGIN_NAMESPACE

class QParallelAnimationGroup2Private : public QAnimationGroup2Private
{
    Q_DECLARE_PUBLIC(QParallelAnimationGroup2)
public:
    QParallelAnimationGroup2Private()
        : lastLoop(0), lastCurrentTime(0)
    {
    }

    QHash<QAbstractAnimation2*, int> uncontrolledFinishTime;
    int lastLoop;
    int lastCurrentTime;

    bool shouldAnimationStart(QAbstractAnimation2 *animation, bool startIfAtEnd) const;
    void applyGroupState(QAbstractAnimation2 *animation);
    bool isUncontrolledAnimationFinished(QAbstractAnimation2 *anim) const;
    void connectUncontrolledAnimations();
    void disconnectUncontrolledAnimations();

    void animationRemoved(int index, QAbstractAnimation2 *);

    // private slot
    void _q_uncontrolledAnimationFinished();
};

QT_END_NAMESPACE



#endif //QPARALLELANIMATIONGROUP2_P_P_H