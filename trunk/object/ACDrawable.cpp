/*
    AgeCreator: A level editor for Uru
    Copyright (C) 2010 Branan Riley

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "ACDrawable.h"
#include "ACLayer.h"
#include "ACUtil.h"

#include <PRP/Geometry/plDrawableSpans.h>
#include <PRP/Geometry/plGBufferGroup.h>
#include <PRP/Object/plDrawInterface.h>
#include <PRP/Object/plSceneObject.h>
#include <PRP/plSceneNode.h>

#include <climits>

#include <QFile>
#include <QDebug>

QMap<plLocation, QWeakPointer<ACDrawableSpans> > ACDrawable::weak_spans;

ACDrawable::ACDrawable(const QString &name)
  : ACObject(name)
{
  draw = new plDrawInterface;
  draw->init(toPlasma(name));
  spans = getSpans(plLocation());
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  manager->AddObject(plLocation(), draw);
  scene_object->setDrawInterface(draw->getKey());
}

ACDrawable::ACDrawable(plKey key)
  : ACObject(key)
{
  spans = getSpans(key->getLocation());
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  draw = static_cast<plDrawInterface*>(scene_object->getDrawInterface()->getObj());
}

ACDrawable::ACDrawable::~ACDrawable()
{
  clearMeshData();
  manager->DelObject(draw->getKey());
}

void ACDrawable::setMeshData(const hsTArray<plGBufferVertex> &verts, const hsTArray<unsigned short> &indices, unsigned char fmt)
{
  plDrawableSpans *span = spans->getSpan(0, 0);
  clearMeshData();
  format = fmt;

  // insert the new data
  size_t group = spans->getGroupId(span, format);
  size_t vertex_offset = 0;
  for(size_t i = 0; i < span->getBuffer(group)->getNumVertBuffers(); i++)
    vertex_offset += span->getBuffer(group)->getVertBufferSize(i);
  size_t index_offset = 0;
  for(size_t i = 0; i < span->getBuffer(group)->getNumIdxBuffers(); i++)
    index_offset += span->getBuffer(group)->getIdxBufferCount(i);
  span->addVerts(group, verts);
  span->addIndices(group, indices);
  plGBufferCell cell;
  cell.fVtxStart = 0;
  cell.fColorStart=UINT_MAX;
  cell.fLength = vertex_offset + verts.getSize();
  hsTArray<plGBufferCell> cells;
  cells.append(cell);
  span->getBuffer(group)->clearCells();
  span->addCells(group, cells);
  plIcicle icicle;
  float min_x, min_y, min_z, max_x, max_y, max_z;
  for(unsigned int i = 0; i < verts.getSize(); i++) {
    if(verts[i].fPos.X > max_x)
      max_x = verts[i].fPos.X;
    if(verts[i].fPos.X < min_x)
      min_x = verts[i].fPos.X;
    if(verts[i].fPos.Y > max_y)
      max_y = verts[i].fPos.Y;
    if(verts[i].fPos.Y < min_y)
      min_y = verts[i].fPos.Y;
    if(verts[i].fPos.Z > max_z)
      max_z = verts[i].fPos.Z;
    if(verts[i].fPos.Z < min_z)
      min_z = verts[i].fPos.Z;
  }
  hsBounds3Ext bounds;
  bounds.setMins(hsVector3(min_x, min_y, min_z));
  bounds.setMaxs(hsVector3(max_x, max_y, max_z));
  icicle.setLocalBounds(bounds);
  icicle.setWorldBounds(bounds);
  //TODO: Material support
  icicle.setMaterialIdx(0);
  icicle.setIStartIdx(index_offset);
  icicle.setVStartIdx(vertex_offset);
  icicle.setILength(indices.getSize());
  icicle.setVLength(verts.getSize());
  size_t id = span->addIcicle(icicle);
  plDISpanIndex di_index;
  di_index.fIndices.append(id);
  span->addDIIndex(di_index);
  draw->addDrawable(span->getKey(), id);
}

bool ACDrawable::loadFromFile(const QString &filename)
{
  QFile model_file(filename);
  if(!model_file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;
  hsTArray<plGBufferVertex> verts;
  hsTArray<unsigned short> indices;
  while(!model_file.atEnd()) {
    QByteArray line = model_file.readLine();
    line = line.simplified();
    if(line.startsWith('v')) {
      plGBufferVertex v;
      v.fPos.X = line.split(' ')[1].toFloat();
      v.fPos.Y = line.split(' ')[2].toFloat();
      v.fPos.Z = line.split(' ')[3].toFloat();
      v.fNormal.X = 0.0f;
      v.fNormal.Y = 0.0f;
      v.fNormal.Z = 1.0f;
      verts.append(v);
    } else if(line.startsWith('f')) {
     indices.append(line.split(' ')[1].toShort());
     indices.append(line.split(' ')[2].toShort());
     indices.append(line.split(' ')[3].toShort());
    }
  }
  setMeshData(verts, indices, (unsigned char)plGBufferGroup::kEncoded);
  return true;
}

QIcon ACDrawable::icon() const
{
  return ACIcon("draw-triangle");
}

void ACDrawable::registerWithPage(ACPage* page)
{
  manager->MoveKey(draw->getKey(), page->location());
  disconnect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  moveMeshData(page->location()); // This updates spans as a side effect
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  spans->getSpan(0, 0);
  ACObject::registerWithPage(page);
}

void ACDrawable::unregisterFromPage(ACPage* page)
{
  manager->MoveKey(draw->getKey(), plLocation());
  disconnect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  moveMeshData(plLocation()); // This updates spans as a side effect
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  ACObject::unregisterFromPage(page);
}

void ACDrawable::clearMeshData()
{
  if(draw->getNumDrawables()) {
    //TODO: backup all mesh data for the current format, and re-insert all data but the one for this mesh
    spans->meshRemoved(draw->getDrawableKey(0), format);
    draw->delDrawable(0);
  }
  return;
}

void ACDrawable::moveMeshData(plLocation loc)
{
  // If there's not actually any mesh data here, return
  if(0 == draw->getNumDrawables()) {
    spans = getSpans(loc);
    return;
  }
  plDrawableSpans *span = spans->getSpan(0, 0);
  hsTArray<plGBufferVertex> verts = span->getVerts(static_cast<plIcicle*>(span->getSpan(draw->getDrawableKey(0))));
  hsTArray<unsigned short> indices = span->getIndices(static_cast<plIcicle*>(span->getSpan(draw->getDrawableKey(0))));
  clearMeshData();
  spans = getSpans(loc);
  setMeshData(verts, indices, format);
}

QSharedPointer<ACDrawableSpans> ACDrawable::getSpans(plLocation loc)
{
  QSharedPointer<ACDrawableSpans> span;
  if(!weak_spans.contains(loc) || weak_spans.value(loc).isNull()) {
    span = QSharedPointer<ACDrawableSpans>(new ACDrawableSpans);
    span->load(loc);
    weak_spans.insert(loc, span);
  }
  return weak_spans.value(loc);
}

void ACDrawable::idUpdated(int id, unsigned char fmt)
{
  if(format == fmt && draw->getNumDrawables() && id < draw->getDrawableKey(0)) {
    plKey key = draw->getDrawable(0);
    int key_id = draw->getDrawableKey(0) - 1;
    draw->delDrawable(0);
    draw->addDrawable(key, key_id);
  }
}

ACDrawableSpans::ACDrawableSpans()
{}
  
ACDrawableSpans::~ACDrawableSpans()
{
  QMapIterator<QPair<unsigned int, unsigned int>, plDrawableSpans*> i(spans);
  while(i.hasNext()) {
    i.next();
    manager->DelObject(i.value()->getKey());
  }
}
  
void ACDrawableSpans::load(plLocation loc)
{
  scene_node = manager->getSceneNode(loc)->getKey();
  std::vector<plKey> keys = manager->getKeys(loc, kDrawableSpans);
  // If this is being created on a page with no previous drawable spans, none of the below will run
  // HOWEVER, this function still sets the scene node, so it must be called before using the class
  for(unsigned int i = 0; i < keys.size(); i++) {
    plDrawableSpans *span = static_cast<plDrawableSpans*>(keys[i]->getObj());
    QPair<unsigned int, unsigned int> pair(span->getRenderLevel(), span->getCriteria());
    spans.insert(pair, span);
    for(size_t j = 0; j < span->getNumBufferGroups(); j++) {
      QPair<plDrawableSpans*, unsigned char> pair(span, span->getBuffer(j)->getFormat());
      group_ids.insert(pair, j);
      qDebug() << span->getBuffer(j)->getNumVertBuffers();
    }
  }
}

plDrawableSpans *ACDrawableSpans::getSpan(unsigned int render_level, unsigned int criteria)
{
  QPair<unsigned int, unsigned int> key(render_level, criteria);
  if(!spans.contains(key)) {
    plDrawableSpans *span = new plDrawableSpans;
    span = new plDrawableSpans;
    span->setRenderLevel(render_level);
    span->setCriteria(criteria);
    span->setSceneNode(scene_node);
    span->init(span_name(render_level, criteria));
    manager->AddObject(scene_node->getLocation(), span);
    spans.insert(key, span);
  }
  return spans.value(key);
}

size_t ACDrawableSpans::getGroupId(plDrawableSpans *span, unsigned char format)
{
  QPair<plDrawableSpans *, unsigned char> key(span, format);
  if(!group_ids.contains(key)) {
    size_t id = span->createBufferGroup(format);
    group_ids.insert(key, id);
  }
  return group_ids.value(key);
}

void ACDrawableSpans::meshRemoved(int id, unsigned char format)
{
  emit idUpdated(id, format);
}

plString ACDrawableSpans::span_name(unsigned int render_level, unsigned int criteria)
{
  QString rlevel = ascii("%1").arg(render_level, 8, 16, QLatin1Char('0'));
  QString cteria = ascii("%1").arg(criteria, 1, 16);
  QString name = rlevel + ascii("_") + cteria + ascii("Spans");
  if(scene_node.Exists()) {
    name = toQt(scene_node->getName()) + ascii("_") + name;
  }
  return toPlasma(name);
}
