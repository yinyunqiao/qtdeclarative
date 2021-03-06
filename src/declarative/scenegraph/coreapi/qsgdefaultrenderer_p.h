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

#ifndef QMLRENDERER_H
#define QMLRENDERER_H

#include "qsgrenderer_p.h"

#include <QtGui/private/qdatabuffer_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class IndexGeometryNodePair : public QPair<int, QSGGeometryNode *>
{
public:
    IndexGeometryNodePair(int i, QSGGeometryNode *n);
    bool operator < (const IndexGeometryNodePair &other) const;
};


// Minimum heap.
class IndexGeometryNodePairHeap
{
public:
    IndexGeometryNodePairHeap();
    void insert(const IndexGeometryNodePair &x);
    const IndexGeometryNodePair &top() const { return v.first(); }
    IndexGeometryNodePair pop();
    bool isEmpty() const { return v.isEmpty(); }
private:
    static int parent(int i) { return (i - 1) >> 1; }
    static int left(int i) { return (i << 1) | 1; }
    static int right(int i) { return (i + 1) << 1; }
    QDataBuffer<IndexGeometryNodePair> v;
};


class QSGDefaultRenderer : public QSGRenderer
{
    Q_OBJECT
public:
    QSGDefaultRenderer(QSGContext *context);

    void render();

    void nodeChanged(QSGNode *node, QSGNode::DirtyFlags flags);

    void setSortFrontToBackEnabled(bool sort);
    bool isSortFrontToBackEnabled() const;

private:
    void buildLists(QSGNode *node);
    void renderNodes(const QDataBuffer<QSGGeometryNode *> &list);

    const QSGClipNode *m_currentClip;
    QSGMaterial *m_currentMaterial;
    QSGMaterialShader *m_currentProgram;
    const QMatrix4x4 *m_currentMatrix;
    QMatrix4x4 m_renderOrderMatrix;
    QDataBuffer<QSGGeometryNode *> m_opaqueNodes;
    QDataBuffer<QSGGeometryNode *> m_transparentNodes;
    QDataBuffer<QSGGeometryNode *> m_tempNodes;
    IndexGeometryNodePairHeap m_heap;

    bool m_rebuild_lists;
    bool m_needs_sorting;
    bool m_sort_front_to_back;
    int m_currentRenderOrder;

#ifdef QML_RUNTIME_TESTING
    bool m_render_opaque_nodes;
    bool m_render_alpha_nodes;
#endif
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMLRENDERER_H
