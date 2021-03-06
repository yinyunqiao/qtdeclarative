/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qsgareaallocator_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

namespace
{
    enum SplitType
    {
        VerticalSplit,
        HorizontalSplit
    };

    static const int maxMargin = 2;
}

struct QSGAreaAllocatorNode
{
    QSGAreaAllocatorNode(QSGAreaAllocatorNode *parent);
    ~QSGAreaAllocatorNode();
    inline bool isLeaf();

    QSGAreaAllocatorNode *parent;
    QSGAreaAllocatorNode *left;
    QSGAreaAllocatorNode *right;
    int split; // only valid for inner nodes.
    SplitType splitType;
    bool isOccupied; // only valid for leaf nodes.
};

QSGAreaAllocatorNode::QSGAreaAllocatorNode(QSGAreaAllocatorNode *parent)
    : parent(parent)
    , left(0)
    , right(0)
    , isOccupied(false)
{
}

QSGAreaAllocatorNode::~QSGAreaAllocatorNode()
{
    delete left;
    delete right;
}

bool QSGAreaAllocatorNode::isLeaf()
{
    Q_ASSERT((left != 0) == (right != 0));
    return !left;
}


QSGAreaAllocator::QSGAreaAllocator(const QSize &size) : m_size(size)
{
    m_root = new QSGAreaAllocatorNode(0);
}

QSGAreaAllocator::~QSGAreaAllocator()
{
    delete m_root;
}

QRect QSGAreaAllocator::allocate(const QSize &size)
{
    QPoint point;
    bool result = allocateInNode(size, point, QRect(QPoint(0, 0), m_size), m_root);
    return result ? QRect(point, size) : QRect();
}

bool QSGAreaAllocator::deallocate(const QRect &rect)
{
    return deallocateInNode(rect.topLeft(), m_root);
}

bool QSGAreaAllocator::allocateInNode(const QSize &size, QPoint &result, const QRect &currentRect, QSGAreaAllocatorNode *node)
{
    if (size.width() > currentRect.width() || size.height() > currentRect.height())
        return false;

    if (node->isLeaf()) {
        if (node->isOccupied)
            return false;
        if (size.width() + maxMargin >= currentRect.width() && size.height() + maxMargin >= currentRect.height()) {
            //Snug fit, occupy entire rectangle.
            node->isOccupied = true;
            result = currentRect.topLeft();
            return true;
        }
        // TODO: Reuse nodes.
        // Split node.
        node->left = new QSGAreaAllocatorNode(node);
        node->right = new QSGAreaAllocatorNode(node);
        QRect splitRect = currentRect;
        if ((currentRect.width() - size.width()) * currentRect.height() < (currentRect.height() - size.height()) * currentRect.width()) {
            node->splitType = HorizontalSplit;
            node->split = currentRect.top() + size.height();
            splitRect.setHeight(size.height());
        } else {
            node->splitType = VerticalSplit;
            node->split = currentRect.left() + size.width();
            splitRect.setWidth(size.width());
        }
        return allocateInNode(size, result, splitRect, node->left);
    } else {
        // TODO: avoid unnecessary recursion.
        //  has been split.
        QRect leftRect = currentRect;
        QRect rightRect = currentRect;
        if (node->splitType == HorizontalSplit) {
            leftRect.setHeight(node->split - leftRect.top());
            rightRect.setTop(node->split);
        } else {
            leftRect.setWidth(node->split - leftRect.left());
            rightRect.setLeft(node->split);
        }
        if (allocateInNode(size, result, leftRect, node->left))
            return true;
        if (allocateInNode(size, result, rightRect, node->right))
            return true;
        return false;
    }
}

bool QSGAreaAllocator::deallocateInNode(const QPoint &pos, QSGAreaAllocatorNode *node)
{
    while (!node->isLeaf()) {
        //  has been split.
        int cmp = node->splitType == HorizontalSplit ? pos.y() : pos.x();
        node = cmp < node->split ? node->left : node->right;
    }
    if (!node->isOccupied)
        return false;
    node->isOccupied = false;
    mergeNodeWithNeighbors(node);
    return true;
}

void QSGAreaAllocator::mergeNodeWithNeighbors(QSGAreaAllocatorNode *node)
{
    bool done = false;
    QSGAreaAllocatorNode *parent = 0;
    QSGAreaAllocatorNode *current = 0;
    QSGAreaAllocatorNode *sibling;
    while (!done) {
        Q_ASSERT(node->isLeaf());
        Q_ASSERT(!node->isOccupied);
        if (node->parent == 0)
            return; // No neighbours.

        SplitType splitType = SplitType(node->parent->splitType);
        done = true;

        /* Special case. Might be faster than going through the general code path.
        // Merge with sibling.
        parent = node->parent;
        sibling = (node == parent->left ? parent->right : parent->left);
        if (sibling->isLeaf() && !sibling->isOccupied) {
            Q_ASSERT(!sibling->right);
            node = parent;
            parent->isOccupied = false;
            delete parent->left;
            delete parent->right;
            parent->left = parent->right = 0;
            done = false;
            continue;
        }
        */

        // Merge with left neighbour.
        current = node;
        parent = current->parent;
        while (parent && current == parent->left && parent->splitType == splitType) {
            current = parent;
            parent = parent->parent;
        }

        if (parent && parent->splitType == splitType) {
            Q_ASSERT(current == parent->right);
            Q_ASSERT(parent->left);

            QSGAreaAllocatorNode *neighbor = parent->left;
            while (neighbor->right && neighbor->splitType == splitType)
                neighbor = neighbor->right;

            if (neighbor->isLeaf() && neighbor->parent->splitType == splitType && !neighbor->isOccupied) {
                // Left neighbour can be merged.
                parent->split = neighbor->parent->split;

                parent = neighbor->parent;
                sibling = neighbor == parent->left ? parent->right : parent->left;
                QSGAreaAllocatorNode **nodeRef = &m_root;
                if (parent->parent) {
                    if (parent == parent->parent->left)
                        nodeRef = &parent->parent->left;
                    else
                        nodeRef = &parent->parent->right;
                }
                sibling->parent = parent->parent;
                *nodeRef = sibling;
                parent->left = parent->right = 0;
                delete parent;
                delete neighbor;
                done = false;
            }
        }

        // Merge with right neighbour.
        current = node;
        parent = current->parent;
        while (parent && current == parent->right && parent->splitType == splitType) {
            current = parent;
            parent = parent->parent;
        }

        if (parent && parent->splitType == splitType) {
            Q_ASSERT(current == parent->left);
            Q_ASSERT(parent->right);

            QSGAreaAllocatorNode *neighbor = parent->right;
            while (neighbor->left && neighbor->splitType == splitType)
                neighbor = neighbor->left;

            if (neighbor->isLeaf() && neighbor->parent->splitType == splitType && !neighbor->isOccupied) {
                // Right neighbour can be merged.
                parent->split = neighbor->parent->split;

                parent = neighbor->parent;
                sibling = neighbor == parent->left ? parent->right : parent->left;
                QSGAreaAllocatorNode **nodeRef = &m_root;
                if (parent->parent) {
                    if (parent == parent->parent->left)
                        nodeRef = &parent->parent->left;
                    else
                        nodeRef = &parent->parent->right;
                }
                sibling->parent = parent->parent;
                *nodeRef = sibling;
                parent->left = parent->right = 0;
                delete parent;
                delete neighbor;
                done = false;
            }
        }
    } // end while(!done)
}

QT_END_NAMESPACE
