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

#ifndef QSEQUENTIALANIMATIONGROUP2_P_H
#define QSEQUENTIALANIMATIONGROUP2_P_H

#include <private/qanimationgroup2_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QPauseAnimation2;
class Q_DECLARATIVE_EXPORT QSequentialAnimationGroup2 : public QAnimationGroup2
{
public:
    QSequentialAnimationGroup2(QDeclarativeAbstractAnimation *animation=0);
    QSequentialAnimationGroup2(const QSequentialAnimationGroup2 &other);
    ~QSequentialAnimationGroup2();

    QAbstractAnimation2Pointer currentAnimation() const;
    int duration() const;

//Q_SIGNALS:
//    void currentAnimationChanged(QAbstractAnimation2Pointer current);

protected:
    void updateCurrentTime(int);
    void updateState(QAbstractAnimation2::State newState, QAbstractAnimation2::State oldState);
    void updateDirection(QAbstractAnimation2::Direction direction);
    void uncontrolledAnimationFinished(QAbstractAnimation2Pointer animation);
private:
    Q_DISABLE_COPY(QSequentialAnimationGroup2)
    struct AnimationIndex
    {
        AnimationIndex() : index(0), timeOffset(0) {}
        // index points to the animation at timeOffset, skipping 0 duration animations.
        // Note that the index semantic is slightly different depending on the direction.
        int index; // the index of the animation in timeOffset
        int timeOffset; // time offset when the animation at index starts.
    };

    int animationActualTotalDuration(int index) const;
    AnimationIndex indexForCurrentTime() const;

    void setCurrentAnimation(int index, bool intermediate = false);
    void activateCurrentAnimation(bool intermediate = false);

    void animationInsertedAt(int index);
    void animationRemoved(int index, QAbstractAnimation2Pointer anim);

    bool atEnd() const;

    QAbstractAnimation2Pointer m_currentAnimation;
    int m_currentAnimationIndex;

    // this is the actual duration of uncontrolled animations
    // it helps seeking and even going forward
    QList<int> m_actualDuration;

    void restart();
    int m_lastLoop;

    // handle time changes
    void rewindForwards(const AnimationIndex &newAnimationIndex);
    void advanceForwards(const AnimationIndex &newAnimationIndex);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif //QSEQUENTIALANIMATIONGROUP2_P_H
