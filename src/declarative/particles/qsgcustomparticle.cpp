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

#include "qsgcustomparticle_p.h"
#include <private/qsgshadereffectmesh_p.h>
#include <cstdlib>

QT_BEGIN_NAMESPACE

//TODO: Can we make the code such that you don't have to copy the whole vertex shader just to add one little calculation?
//Includes comments because the code isn't self explanatory
static const char qt_particles_default_vertex_code[] =
        "attribute highp vec2 vPos;                                                         \n"
        "attribute highp vec2 vTex;                                                         \n"
        "attribute highp vec4 vData; //  x = time,  y = lifeSpan, z = size,  w = endSize    \n"
        "attribute highp vec4 vVec; // x,y = constant speed,  z,w = acceleration            \n"
        "uniform highp mat4 qt_Matrix;                                                      \n"
        "uniform highp float timestamp;                                                     \n"
        "varying highp vec2 fTex;                                                           \n"
        "void main() {                                                                      \n"
        "    fTex = vTex;                                                                   \n"
        "    highp float size = vData.z;                                                    \n"
        "    highp float endSize = vData.w;                                                 \n"
        "    highp float t = (timestamp - vData.x) / vData.y;                               \n"
        "    highp float currentSize = mix(size, endSize, t * t);                           \n"
        "    if (t < 0. || t > 1.)                                                          \n"
        "        currentSize = 0.;                                                          \n"
        "    highp vec2 pos = vPos                                                          \n"
        "                   - currentSize / 2. + currentSize * vTex          // adjust size \n"
        "                   + vVec.xy * t * vData.y         // apply speed vector..         \n"
        "                   + 0.5 * vVec.zw * pow(t * vData.y, 2.);                         \n"
        "    gl_Position = qt_Matrix * vec4(pos.x, pos.y, 0, 1);                            \n"
        "}";

static const char qt_particles_default_fragment_code[] =//TODO: Default frag requires source?
        "uniform sampler2D source;                                  \n"
        "varying highp vec2 fTex;                                   \n"
        "uniform lowp float qt_Opacity;                             \n"
        "void main() {                                              \n"
        "    gl_FragColor = texture2D(source, fTex) * qt_Opacity;   \n"
        "}";

static const char qt_position_attribute_name[] = "qt_Vertex";
static const char qt_texcoord_attribute_name[] = "qt_MultiTexCoord0";

static QSGGeometry::Attribute PlainParticle_Attributes[] = {
    { 0, 2, GL_FLOAT },             // Position
    { 1, 2, GL_FLOAT },             // TexCoord
    { 2, 4, GL_FLOAT },             // Data
    { 3, 4, GL_FLOAT },             // Vectors
    { 4, 1, GL_FLOAT }              // r
};

static QSGGeometry::AttributeSet PlainParticle_AttributeSet =
{
    5, // Attribute Count
    (2 + 2 + 4 + 4 + 1) * sizeof(float),
    PlainParticle_Attributes
};

struct PlainVertex {
    float x;
    float y;
    float tx;
    float ty;
    float t;
    float lifeSpan;
    float size;
    float endSize;
    float vx;
    float vy;
    float ax;
    float ay;
    float r;
};

struct PlainVertices {
    PlainVertex v1;
    PlainVertex v2;
    PlainVertex v3;
    PlainVertex v4;
};

/*!
    \qmlclass CustomParticle QSGCustomParticle
    \inqmlmodule QtQuick.Particles 2
    \inherits ParticlePainter
    \brief The CustomParticle element allows you to specify your own shader to paint particles.

*/

QSGCustomParticle::QSGCustomParticle(QSGItem* parent)
    : QSGParticlePainter(parent)
    , m_pleaseReset(true)
    , m_dirtyData(true)
    , m_rootNode(0)
{
    setFlag(QSGItem::ItemHasContents);
}

void QSGCustomParticle::componentComplete()
{
    reset();
    QSGParticlePainter::componentComplete();
}


//Trying to keep the shader conventions the same as in qsgshadereffectitem
/*!
    \qmlproperty string QtQuick.Particles2::CustomParticle::fragmentShader

    This property holds the fragment shader's GLSL source code.
    The default shader passes the texture coordinate along to the fragment
    shader as "varying highp vec2 qt_TexCoord0".
*/

void QSGCustomParticle::setFragmentShader(const QByteArray &code)
{
    if (m_source.fragmentCode.constData() == code.constData())
        return;
    m_source.fragmentCode = code;
    if (isComponentComplete()) {
        reset();
    }
    emit fragmentShaderChanged();
}

/*!
    \qmlproperty string QtQuick.Particles2::CustomParticle::vertexShader

    This property holds the vertex shader's GLSL source code.
    The default shader expects the texture coordinate to be passed from the
    vertex shader as "varying highp vec2 qt_TexCoord0", and it samples from a
    sampler2D named "source".
*/

void QSGCustomParticle::setVertexShader(const QByteArray &code)
{
    if (m_source.vertexCode.constData() == code.constData())
        return;
    m_source.vertexCode = code;
    if (isComponentComplete()) {
        reset();
    }
    emit vertexShaderChanged();
}

void QSGCustomParticle::reset()
{
    disconnectPropertySignals();

    m_source.attributeNames.clear();
    m_source.uniformNames.clear();
    m_source.respectsOpacity = false;
    m_source.respectsMatrix = false;
    m_source.className = metaObject()->className();

    for (int i = 0; i < m_sources.size(); ++i) {
        const SourceData &source = m_sources.at(i);
        delete source.mapper;
        if (source.item && source.item->parentItem() == this)
            source.item->setParentItem(0);
    }
    m_sources.clear();

    QSGParticlePainter::reset();
    m_pleaseReset = true;
    update();
}


void QSGCustomParticle::changeSource(int index)
{
    Q_ASSERT(index >= 0 && index < m_sources.size());
    QVariant v = property(m_sources.at(index).name.constData());
    setSource(v, index);
}

void QSGCustomParticle::updateData()
{
    m_dirtyData = true;
    update();
}

void QSGCustomParticle::setSource(const QVariant &var, int index)
{
    Q_ASSERT(index >= 0 && index < m_sources.size());

    SourceData &source = m_sources[index];

    source.item = 0;
    if (var.isNull()) {
        return;
    } else if (!qVariantCanConvert<QObject *>(var)) {
        qWarning("Could not assign source of type '%s' to property '%s'.", var.typeName(), source.name.constData());
        return;
    }

    QObject *obj = qVariantValue<QObject *>(var);

    QSGTextureProvider *int3rface = QSGTextureProvider::from(obj);
    if (!int3rface) {
        qWarning("Could not assign property '%s', did not implement QSGTextureProvider.", source.name.constData());
    }

    source.item = qobject_cast<QSGItem *>(obj);

    // TODO: Copy better solution in QSGShaderEffect when they find it.
    // 'source.item' needs a canvas to get a scenegraph node.
    // The easiest way to make sure it gets a canvas is to
    // make it a part of the same item tree as 'this'.
    if (source.item && source.item->parentItem() == 0) {
        source.item->setParentItem(this);
        source.item->setVisible(false);
    }
}

void QSGCustomParticle::disconnectPropertySignals()
{
    disconnect(this, 0, this, SLOT(updateData()));
    for (int i = 0; i < m_sources.size(); ++i) {
        SourceData &source = m_sources[i];
        disconnect(this, 0, source.mapper, 0);
        disconnect(source.mapper, 0, this, 0);
    }
}

void QSGCustomParticle::connectPropertySignals()
{
    QSet<QByteArray>::const_iterator it;
    for (it = m_source.uniformNames.begin(); it != m_source.uniformNames.end(); ++it) {
        int pi = metaObject()->indexOfProperty(it->constData());
        if (pi >= 0) {
            QMetaProperty mp = metaObject()->property(pi);
            if (!mp.hasNotifySignal())
                qWarning("QSGCustomParticle: property '%s' does not have notification method!", it->constData());
            QByteArray signalName("2");
            signalName.append(mp.notifySignal().signature());
            connect(this, signalName, this, SLOT(updateData()));
        } else {
            qWarning("QSGCustomParticle: '%s' does not have a matching property!", it->constData());
        }
    }
    for (int i = 0; i < m_sources.size(); ++i) {
        SourceData &source = m_sources[i];
        int pi = metaObject()->indexOfProperty(source.name.constData());
        if (pi >= 0) {
            QMetaProperty mp = metaObject()->property(pi);
            QByteArray signalName("2");
            signalName.append(mp.notifySignal().signature());
            connect(this, signalName, source.mapper, SLOT(map()));
            source.mapper->setMapping(this, i);
            connect(source.mapper, SIGNAL(mapped(int)), this, SLOT(changeSource(int)));
        } else {
            qWarning("QSGCustomParticle: '%s' does not have a matching source!", source.name.constData());
        }
    }
}

void QSGCustomParticle::updateProperties()
{
    QByteArray vertexCode = m_source.vertexCode;
    QByteArray fragmentCode = m_source.fragmentCode;
    if (vertexCode.isEmpty())
        vertexCode = qt_particles_default_vertex_code;
    if (fragmentCode.isEmpty())
        fragmentCode = qt_particles_default_fragment_code;

    m_source.attributeNames.clear();
    m_source.attributeNames << "vPos" << "vTex" << "vData" << "vVec" << "r";

    lookThroughShaderCode(vertexCode);
    lookThroughShaderCode(fragmentCode);

    if (!m_source.attributeNames.contains(qt_position_attribute_name))
        qWarning("QSGCustomParticle: Missing reference to \'%s\'.", qt_position_attribute_name);
    if (!m_source.attributeNames.contains(qt_texcoord_attribute_name))
        qWarning("QSGCustomParticle: Missing reference to \'%s\'.", qt_texcoord_attribute_name);
    if (!m_source.respectsMatrix)
        qWarning("QSGCustomParticle: Missing reference to \'qt_Matrix\'.");
    if (!m_source.respectsOpacity)
        qWarning("QSGCustomParticle: Missing reference to \'qt_Opacity\'.");

    for (int i = 0; i < m_sources.size(); ++i) {
        QVariant v = property(m_sources.at(i).name);
        setSource(v, i);
    }

    connectPropertySignals();
}

void QSGCustomParticle::lookThroughShaderCode(const QByteArray &code)
{
    // Regexp for matching attributes and uniforms.
    // In human readable form: attribute|uniform [lowp|mediump|highp] <type> <name>
    static QRegExp re(QLatin1String("\\b(attribute|uniform)\\b\\s*\\b(?:lowp|mediump|highp)?\\b\\s*\\b(\\w+)\\b\\s*\\b(\\w+)"));
    Q_ASSERT(re.isValid());

    int pos = -1;

    QString wideCode = QString::fromLatin1(code.constData(), code.size());

    while ((pos = re.indexIn(wideCode, pos + 1)) != -1) {
        QByteArray decl = re.cap(1).toLatin1(); // uniform or attribute
        QByteArray type = re.cap(2).toLatin1(); // type
        QByteArray name = re.cap(3).toLatin1(); // variable name

        if (decl == "attribute") {
            if (!m_source.attributeNames.contains(name))//TODO: Can they add custom attributes?
                qWarning() << "Custom Particle: Unknown attribute " << name;
        } else {
            Q_ASSERT(decl == "uniform");//TODO: Shouldn't assert

            if (name == "qt_Matrix") {
                m_source.respectsMatrix = true;
            } else if (name == "qt_ModelViewProjectionMatrix") {
                // TODO: Remove after grace period.
                qWarning("ShaderEffect: qt_ModelViewProjectionMatrix is deprecated. Use qt_Matrix instead.");
                m_source.respectsMatrix = true;
            } else if (name == "qt_Opacity") {
                m_source.respectsOpacity = true;
            } else if (name == "timestamp") {
                //TODO: Copy the whole thing just because I have one more uniform?
            } else {
                m_source.uniformNames.insert(name);
                if (type == "sampler2D") {
                    SourceData d;
                    d.mapper = new QSignalMapper;
                    d.name = name;
                    d.item = 0;
                    m_sources.append(d);
                }
            }
        }
    }
}

QSGNode *QSGCustomParticle::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (m_pleaseReset){

        //delete m_material;//Shader effect item doesn't regen material?

        delete m_rootNode;//Automatically deletes children
        m_rootNode = 0;
        m_nodes.clear();
        m_pleaseReset = false;
        m_dirtyData = false;
    }

    if (m_system && m_system->isRunning())
        prepareNextFrame();
    if (m_rootNode){
        update();
        //### Should I be using dirty geometry too/instead?
        foreach (QSGShaderEffectNode* node, m_nodes)
            node->markDirty(QSGNode::DirtyMaterial); //done in buildData?
    }

    return m_rootNode;
}

void QSGCustomParticle::prepareNextFrame(){
    if (!m_rootNode)
        m_rootNode = buildCustomNodes();
    if (!m_rootNode)
        return;

    m_lastTime = m_system->systemSync(this) / 1000.;
    if (m_dirtyData || true)//Currently this is how we update timestamp... potentially over expensive.
        buildData();
}

QSGShaderEffectNode* QSGCustomParticle::buildCustomNodes()
{
    if (m_count * 4 > 0xffff) {
        printf("CustomParticle: Too many particles... \n");//####Why is this here?
        return 0;
    }

    if (m_count <= 0) {
        printf("CustomParticle: Too few particles... \n");
        return 0;
    }

    updateProperties();

    QSGShaderEffectProgram s = m_source;
    if (s.fragmentCode.isEmpty())
        s.fragmentCode = qt_particles_default_fragment_code;
    if (s.vertexCode.isEmpty())
        s.vertexCode = qt_particles_default_vertex_code;
    m_material.setProgramSource(s);
    foreach (const QString &str, m_particles){
        int gIdx = m_system->m_groupIds[str];
        int count = m_system->m_groupData[gIdx]->size();
        //Create Particle Geometry
        int vCount = count * 4;
        int iCount = count * 6;
        QSGGeometry *g = new QSGGeometry(PlainParticle_AttributeSet, vCount, iCount);
        g->setDrawingMode(GL_TRIANGLES);
        PlainVertex *vertices = (PlainVertex *) g->vertexData();
        for (int p=0; p < count; ++p) {
            commit(gIdx, p);
            vertices[0].tx = 0;
            vertices[0].ty = 0;

            vertices[1].tx = 1;
            vertices[1].ty = 0;

            vertices[2].tx = 0;
            vertices[2].ty = 1;

            vertices[3].tx = 1;
            vertices[3].ty = 1;
            vertices += 4;
        }
        quint16 *indices = g->indexDataAsUShort();
        for (int i=0; i < count; ++i) {
            int o = i * 4;
            indices[0] = o;
            indices[1] = o + 1;
            indices[2] = o + 2;
            indices[3] = o + 1;
            indices[4] = o + 3;
            indices[5] = o + 2;
            indices += 6;
        }

        QSGShaderEffectNode* node = new QSGShaderEffectNode();

        node->setGeometry(g);
        node->setMaterial(&m_material);
        node->markDirty(QSGNode::DirtyMaterial);

        m_nodes.insert(gIdx, node);
    }
    foreach (QSGShaderEffectNode* node, m_nodes){
        if (node == *(m_nodes.begin()))
                continue;
        (*(m_nodes.begin()))->appendChildNode(node);
    }

    return *(m_nodes.begin());
}

static const QByteArray timestampName("timestamp");

void QSGCustomParticle::buildData()
{
    if (!m_rootNode)
        return;
    QVector<QPair<QByteArray, QVariant> > values;
    QVector<QPair<QByteArray, QPointer<QSGItem> > > textures;
    const QVector<QPair<QByteArray, QPointer<QSGItem> > > &oldTextures = m_material.textureProviders();
    for (int i = 0; i < oldTextures.size(); ++i) {
        QSGTextureProvider *oldSource = QSGTextureProvider::from(oldTextures.at(i).second);
        if (oldSource && oldSource->textureChangedSignal())
            foreach (QSGShaderEffectNode* node, m_nodes)
                disconnect(oldTextures.at(i).second, oldSource->textureChangedSignal(), node, SLOT(markDirtyTexture()));
    }
    for (int i = 0; i < m_sources.size(); ++i) {
        const SourceData &source = m_sources.at(i);
        textures.append(qMakePair(source.name, source.item));
        QSGTextureProvider *t = QSGTextureProvider::from(source.item);
        if (t && t->textureChangedSignal())
            foreach (QSGShaderEffectNode* node, m_nodes)
                connect(source.item, t->textureChangedSignal(), node, SLOT(markDirtyTexture()), Qt::DirectConnection);
    }
    for (QSet<QByteArray>::const_iterator it = m_source.uniformNames.begin();
         it != m_source.uniformNames.end(); ++it) {
        values.append(qMakePair(*it, property(*it)));
    }
    values.append(qMakePair(timestampName, QVariant(m_lastTime)));
    m_material.setUniforms(values);
    m_material.setTextureProviders(textures);
    m_dirtyData = false;
    foreach (QSGShaderEffectNode* node, m_nodes)
        node->markDirty(QSGNode::DirtyMaterial);
}

void QSGCustomParticle::initialize(int gIdx, int pIdx)
{
    QSGParticleData* datum = m_system->m_groupData[gIdx]->data[pIdx];
    datum->r = rand()/(qreal)RAND_MAX;
}

void QSGCustomParticle::commit(int gIdx, int pIdx)
{
    if (m_nodes[gIdx] == 0)
        return;

    QSGParticleData* datum = m_system->m_groupData[gIdx]->data[pIdx];
    PlainVertices *particles = (PlainVertices *) m_nodes[gIdx]->geometry()->vertexData();
    PlainVertex *vertices = (PlainVertex *)&particles[pIdx];
    for (int i=0; i<4; ++i) {
        vertices[i].x = datum->x - m_systemOffset.x();
        vertices[i].y = datum->y - m_systemOffset.y();
        vertices[i].t = datum->t;
        vertices[i].lifeSpan = datum->lifeSpan;
        vertices[i].size = datum->size;
        vertices[i].endSize = datum->endSize;
        vertices[i].vx = datum->vx;
        vertices[i].vy = datum->vy;
        vertices[i].ax = datum->ax;
        vertices[i].ay = datum->ay;
        vertices[i].r = datum->r;
    }
}

QT_END_NAMESPACE
