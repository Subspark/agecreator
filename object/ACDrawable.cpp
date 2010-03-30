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
#include "ACMaterialEditor.h"
#include "ACUtil.h"

#include <PRP/Geometry/plDrawableSpans.h>
#include <PRP/Geometry/plGBufferGroup.h>
#include <PRP/Object/plCoordinateInterface.h>
#include <PRP/Object/plDrawInterface.h>
#include <PRP/Object/plSceneObject.h>
#include <PRP/plSceneNode.h>

#include <PRP/Surface/hsGMaterial.h>
#include <PRP/Surface/plLayer.h>

#include <climits>

#include <QFile>
#include <QMenu>
#include <QPointer>

#include "glew.h"

#include <iostream>

QMap<plLocation, QWeakPointer<ACDrawableSpans> > ACDrawable::weak_spans;

ACDrawable::ACDrawable(const QString &name)
  : ACObject(name)
{
  drawi = new plDrawInterface;
  drawi->init(toPlasma(name));
  drawi->setOwner(scene_object->getKey());
  spans = getSpans(virtual_loc);
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  manager->AddObject(virtual_loc, drawi);
  scene_object->setDrawInterface(drawi->getKey());
  
  QAction *action;
  menu->addAction(tr("Mesh Properties"));
  action = menu->addAction(tr("Material Properties"));
  connect(action, SIGNAL(triggered(bool)), this, SLOT(editMaterial()));
}

ACDrawable::ACDrawable(plKey key)
  : ACObject(key)
{
  spans = getSpans(key->getLocation());
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  drawi = static_cast<plDrawInterface*>(scene_object->getDrawInterface()->getObj());
  plDrawableSpans *span = static_cast<plDrawableSpans*>(drawi->getDrawable(0)->getObj());
  material = span->getMaterial(span->getSpan(drawi->getDrawableKey(0))->getMaterialIdx());
  
  QAction *action;
  menu->addAction(tr("Mesh Properties"));
  action = menu->addAction(tr("Material Properties"));
  connect(action, SIGNAL(triggered(bool)), this, SLOT(editMaterial()));
}

ACDrawable::ACDrawable::~ACDrawable()
{
  clearMeshData();
  manager->DelObject(drawi->getKey());
}

void ACDrawable::setMaterial(plKey mat)
{
  material = mat;
  if(drawi->getNumDrawables()) {
    plDrawableSpans *span = static_cast<plDrawableSpans*>(drawi->getDrawable(0)->getObj());
    span->getSpan(drawi->getDrawableKey(0))->setMaterialIdx(spans->getMaterialId(span, mat));
  }
}

void ACDrawable::setMeshData(const hsTArray<plGBufferVertex> &verts, const hsTArray<unsigned short> &indices, unsigned char fmt)
{
  plDrawableSpans *span = spans->getSpan(0, 0);
  clearMeshData();
  format = fmt;

  // insert the new data
  hsTArray<plGBufferVertex> current_verts;
  hsTArray<unsigned short> current_indices;
  hsTArray<plGBufferCell> current_cells;
  size_t group = spans->getGroupId(span, format);
  if(span->getBuffer(group)->getNumVertBuffers()) {
    current_verts = span->getBuffer(group)->getVertices(0);
    current_cells = span->getBuffer(group)->getCells(0);
  } else {
    plGBufferCell cell;
    cell.fVtxStart = 0;
    cell.fColorStart = UINT_MAX;
    cell.fLength = 0;
    current_cells.append(cell);
  }
  if(span->getBuffer(group)->getNumIdxBuffers())
    current_indices = span->getBuffer(group)->getIndices(0);
  size_t vertex_offset = current_verts.getSize();
  size_t index_offset = current_indices.getSize();
  span->getBuffer(group)->clearVertices();
  span->getBuffer(group)->clearIndices();
  span->getBuffer(group)->clearCells();
  current_verts.append(verts);
  current_indices.setSize(index_offset + indices.getSize());
  for(size_t i = 0; i < indices.getSize(); i++)
      current_indices[index_offset+i] = indices[i] + vertex_offset;
  current_cells[0].fLength = current_verts.getSize();
  span->addVerts(group, current_verts);
  span->addIndices(group, current_indices);
  span->addCells(group, current_cells);
  plIcicle icicle;
  float min_x = 0.0f, min_y = 0.0f, min_z = 0.0f, max_x = 0.0f, max_y = 0.0f, max_z = 0.0f;
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
  bounds.setFlags(hsBounds3Ext::kAxisAligned);
  span->setLocalBounds(bounds);
  span->setWorldBounds(bounds);
  span->setMaxWorldBounds(bounds);
  bounds.setMins(hsVector3(min_x, min_y, min_z));
  bounds.setMaxs(hsVector3(max_x, max_y, max_z));
  icicle.setLocalBounds(bounds);
  icicle.setWorldBounds(bounds);
  icicle.setGroupIdx(group);
  icicle.setMaterialIdx(0);
  icicle.setVBufferIdx(0);
  icicle.setCellIdx(0);
  icicle.setCellOffset(vertex_offset);
  icicle.setIStartIdx(index_offset);
  icicle.setVStartIdx(vertex_offset);
  icicle.setILength(indices.getSize());
  icicle.setVLength(verts.getSize());
  icicle.setMinDist(-1.0f);
  icicle.setMaxDist(-1.0f);
  icicle.setMaterialIdx(spans->getMaterialId(span, material));
  size_t id = span->addIcicle(icicle);
  plDISpanIndex di_index;
  di_index.fIndices.append(id);
  span->addDIIndex(di_index);
  drawi->addDrawable(span->getKey(), span->getNumDIIndices()-1);
  
  emit meshDataUpdated(verts.getSize(), &(verts[0]), indices.getSize(), &(indices[0]));
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
      v.fColor = 0xFFFFFFFF;
      verts.append(v);
    } else if(line.startsWith('f')) {
      hsVector3 p0, p1, p2;
      unsigned short i0, i1, i2;
      i0 = line.split(' ')[1].split('/')[0].toShort() - 1;
      i1 = line.split(' ')[2].split('/')[0].toShort() - 1;
      i2 = line.split(' ')[3].split('/')[0].toShort() - 1;
      p0 = verts[i0].fPos;
      p1 = verts[i1].fPos;
      p2 = verts[i2].fPos;
      hsVector3 n = (p1-p0).crossP(p2-p0);
      verts[i0].fNormal = verts[i0].fNormal + n;
      verts[i1].fNormal = verts[i1].fNormal + n;
      verts[i2].fNormal = verts[i2].fNormal + n;
      indices.append(i0);
      indices.append(i1);
      indices.append(i2);
    }
  }
  // Normalize all the normals;
  for(size_t i = 0; i < verts.getSize(); i++) {
    verts[i].fNormal = verts[i].fNormal * (1.0 / verts[i].fNormal.magnitude());
  }
  setMeshData(verts, indices, (unsigned char)plGBufferGroup::kEncoded);
  return true;
}

void ACDrawable::draw(DrawMode draw, unsigned int shader) const
{
  unsigned int uvw_id;
  int plasma_mat_id = -1;
  int plasma_color_id = -1;
  if(shader) {
    plasma_mat_id = glGetUniformLocation(shader, "plasma_matrix");
    plasma_color_id = glGetUniformLocation(shader, "layer_color");
  }
  for(size_t draw_id = 0; draw_id < drawi->getNumDrawables(); draw_id++) {
    // Only do material setup for 3D preview
    plDrawableSpans *span = static_cast<plDrawableSpans*>(drawi->getDrawable(draw_id)->getObj());
    plDISpanIndex diIndex = span->getDIIndex(drawi->getDrawableKey(draw_id));
    for(size_t idx_id = 0; idx_id < diIndex.fIndices.getSize() ; idx_id ++) {
      plIcicle* icicle = static_cast<plIcicle*>(span->getSpan(diIndex.fIndices[idx_id]));
      if(draw == Draw3D && material.Exists()) {
        hsGMaterial *mat = static_cast<hsGMaterial*>(span->getMaterial(icicle->getMaterialIdx())->getObj());
        plLayer *layer = static_cast<plLayer*>(mat->getLayer(0)->getObj());
        unsigned int new_layer = 1;
        while(!(layer->getTexture().Exists()) || layer->getTexture()->getType() != kMipmap) {
          if(new_layer < mat->getNumLayers())
            layer = static_cast<plLayer*>(mat->getLayer(new_layer)->getObj());
          else
            break;
          new_layer++;
        }
        while(layer->getUnderLay().Exists()) {
          layer = static_cast<plLayer*>(layer->getUnderLay()->getObj());
        }
        hsColorRGBA color = layer->getRuntime();
        if(shader != 0 && plasma_color_id != -1)
          glUniform4fARB(plasma_color_id, color.r, color.g, color.b, color.a);
        glEnable(GL_TEXTURE_2D);
        if(layer->getTexture().Exists())
          glBindTexture(GL_TEXTURE_2D, texture_ids.value(layer->getTexture(), 0));
        uvw_id = layer->getUVWSrc();
        glMatrixMode(GL_TEXTURE);
          glLoadMatrixf(layer->getTransform().glMatrix());
        glMatrixMode(GL_MODELVIEW);
      } else if(draw == Draw3D) {
        glDisable(GL_TEXTURE_2D);
        if(shader != 0 && plasma_color_id != -1)
          glUniform4fARB(plasma_color_id, 1.0f, 1.0f, 1.0f, 1.0f);
      }
      hsMatrix44 mat;
      if(scene_object->getCoordInterface().Exists()) {
        plCoordinateInterface *coord = static_cast<plCoordinateInterface*>(scene_object->getCoordInterface()->getObj());
        mat = coord->getLocalToWorld();
      } else {
        mat = icicle->getLocalToWorld();
      }
      if(shader != 0 && plasma_mat_id != -1) {
        glUniformMatrix4fvARB(plasma_mat_id, 1, GL_FALSE, mat.glMatrix());
      }
      plGBufferGroup *buff = span->getBuffer(icicle->getGroupIdx());
      const unsigned char *vertex_data = buff->getVertBufferStorage(icicle->getVBufferIdx());
      const unsigned short *index_data = buff->getIdxBufferStorage(icicle->getIBufferIdx());
      size_t normal_offset = 3*sizeof(float);
      unsigned char fmt = buff->getFormat();
      int weightCount = (fmt & plGBufferGroup::kSkinWeightMask) >> 4;
      if(weightCount > 0) {
        normal_offset += sizeof(float) * weightCount;
        if(fmt & plGBufferGroup::kSkinIndices)
          normal_offset += sizeof(int);
      }
      glVertexPointer(3, GL_FLOAT, buff->getStride(), vertex_data);
      glNormalPointer(GL_FLOAT, buff->getStride(), vertex_data + normal_offset);
      if(uvw_id <= 9) {
        size_t uv_offset = normal_offset + 3*sizeof(float) + 2*sizeof(int) + 3*sizeof(float)*uvw_id;
        glTexCoordPointer(3, GL_FLOAT, buff->getStride(), vertex_data+uv_offset);
      } else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoord3f(0.0f, 0.0f, 0.0f);
      }
      if(draw == Draw3D) {
        glColorPointer(4, GL_UNSIGNED_BYTE, buff->getStride(), vertex_data + normal_offset + 3*sizeof(float));
      }
      glDrawElements(GL_TRIANGLES, icicle->getILength(), GL_UNSIGNED_SHORT, &(index_data[icicle->getIStartIdx()]));
      if(uvw_id > 9) {
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      }
    }
  }
}

QIcon ACDrawable::icon() const
{
  return ACIcon("draw-triangle");
}

void ACDrawable::registerWithPage(ACPage* page)
{
  manager->MoveKey(drawi->getKey(), page->location());
  disconnect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  moveMeshData(page->location()); // This updates spans as a side effect
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  ACObject::registerWithPage(page);
}

void ACDrawable::unregisterFromPage(ACPage* page)
{
  manager->MoveKey(drawi->getKey(), virtual_loc);
  disconnect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  moveMeshData(virtual_loc); // This updates spans as a side effect
  connect(spans.operator->(), SIGNAL(idUpdated(int, unsigned char)), this, SLOT(idUpdated(int, unsigned char)));
  ACObject::unregisterFromPage(page);
}

void ACDrawable::clearMeshData()
{
  if(drawi->getNumDrawables()) {
    //TODO: backup all mesh data for the current format, and re-insert all data but the one for this mesh
    //spans->meshRemoved(drawi->getDrawableKey(0), format);
    drawi->delDrawable(0);
  }
  return;
}

void ACDrawable::moveMeshData(plLocation loc)
{
  // If there's not actually any mesh data here, return
  if(0 == drawi->getNumDrawables()) {
    spans = getSpans(loc);
    return;
  }
  //TODO get the same pass index as our current span
  plDrawableSpans *span = spans->getSpan(0, 0);
  hsTArray<plGBufferVertex> verts = span->getVerts(static_cast<plIcicle*>(span->getSpan(drawi->getDrawableKey(0))));
  hsTArray<unsigned short> indices = span->getIndices(static_cast<plIcicle*>(span->getSpan(drawi->getDrawableKey(0))));
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

void ACDrawable::setName(const QString &name)
{
  ACObject::setName(name);
  drawi->getKey()->setName(toPlasma(name));
}

void ACDrawable::idUpdated(int id, unsigned char fmt)
{
  if(format == fmt && drawi->getNumDrawables() && id < drawi->getDrawableKey(0)) {
    plKey key = drawi->getDrawable(0);
    int key_id = drawi->getDrawableKey(0) - 1;
    drawi->delDrawable(0);
    drawi->addDrawable(key, key_id);
  }
}

void ACDrawable::editMaterial()
{
  QPointer<ACMaterialEditor> editor = new ACMaterialEditor(material);
  editor->exec();
  plKey new_mat = editor->selectedMaterial();
  if(new_mat != material)
    setMaterial(new_mat);
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
  // If this is being created on a page with no previous drawiable spans, none of the below will run
  // HOWEVER, this function still sets the scene node, so it must be called before using the class
  for(unsigned int i = 0; i < keys.size(); i++) {
    plDrawableSpans *span = static_cast<plDrawableSpans*>(keys[i]->getObj());
    QPair<unsigned int, unsigned int> pair(span->getRenderLevel(), span->getCriteria());
    spans.insert(pair, span);
    for(size_t j = 0; j < span->getNumBufferGroups(); j++) {
      QPair<plDrawableSpans*, unsigned char> pair(span, span->getBuffer(j)->getFormat());
      group_ids.insert(pair, j);
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

int ACDrawableSpans::getMaterialId(plDrawableSpans *span, plKey mat)
{
  if(!mat.Exists())
    return -1;
  size_t mat_count = span->getNumMaterials();
  size_t i;
  for(i = 0; i < mat_count; i++)
    if(span->getMaterial(i) == mat)
      break;
  if(i == mat_count)
    span->addMaterial(mat);
  return i;
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
