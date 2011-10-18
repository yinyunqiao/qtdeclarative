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

#ifndef QPARALLELANIMATIONGROUP2_P_H
#define QPARALLELANIMATIONGROUP2_P_H

#include "private/qanimationgroup2_p.h"
#include <QtCore/qhash.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)



class Q_DECLARATIVE_EXPORT QParallelAnimationGroup2 : public QAnimationGroup2
{
public:
    QParallelAnimationGroup2(QDeclarativeAbstractAnimation *animation=0);
    QParallelAnimationGroup2(const QParallelAnimationGroup2 &other);
    ~QParallelAnimationGroup2();

    int duration() const;

protected:
    void updateCurrentTime(int currentTime);
    void updateState(QAbstractAnimation2::State newState, QAbstractAnimation2::State oldState);
    void updateDirection(QAbstractAnimation2::Direction direction);
    void uncontrolledAnimationFinished(QAbstractAnimation2Pointer animation);

private:
    Q_DISABLE_COPY(QParallelAnimationGroup2)
    int m_lastLoop;
    int m_lastCurrentTime;
    bool shouldAnimationStart(QAbstractAnimation2Pointer animation, bool startIfAtEnd) const;
    void applyGroupState(QAbstractAnimation2Pointer animation);
    void animationRemoved(int index, QAbstractAnimation2Pointer );
};



QT_END_NAMESPACE

QT_END_HEADER

#endif // QPARALLELANIMATIONGROUP2_P_H
