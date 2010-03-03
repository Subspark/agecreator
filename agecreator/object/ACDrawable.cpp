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

ACDrawable::ACDrawable(const QString &name)
  : ACObject(name)
{
  draw = new plDrawInterface;
  draw->init(toPlasma(name));
  span = new plDrawableSpans;
  span->init(toPlasma(name));
  manager->AddObject(plLocation(), draw);
  manager->AddObject(plLocation(), span);
  scene_object->setDrawInterface(draw->getKey());
  draw->addDrawable(span->getKey(), 0);
  span->createBufferGroup(plGBufferGroup::kEncoded);
}

ACDrawable::ACDrawable(plKey key)
  : ACObject(key)
{
  draw = static_cast<plDrawInterface*>(scene_object->getDrawInterface()->getObj());
  span = static_cast<plDrawableSpans*>(draw->getDrawable(0)->getObj());
}

void ACDrawable::setMeshData(const hsTArray<plGBufferVertex> &verts, const hsTArray<unsigned short> &indices, unsigned char format)
{
  // Clear out all the old data
  span->clearSpans();
  span->clearDIIndices();
  span->deleteBufferGroup(0);

  // insert the new data
  span->createBufferGroup(format);
  span->addVerts(0, verts);
  span->addIndices(0, indices);
  plGBufferCell cell;
  cell.fVtxStart=0;
  cell.fColorStart=UINT_MAX;
  cell.fLength = verts.getSize();
  hsTArray<plGBufferCell> cells;
  cells.append(cell);
  span->addCells(0, cells);
  plDISpanIndex di_index;
  di_index.fIndices.append(0);
  span->addDIIndex(di_index);
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
  icicle.setILength(indices.getSize());
  icicle.setVLength(verts.getSize());
  span->addIcicle(icicle);
}

QIcon ACDrawable::icon() const
{
  return ACIcon("draw-triangle");
}

void ACDrawable::registerWithPage(ACPage* page)
{
  manager->MoveKey(draw->getKey(), page->location());
  manager->MoveKey(span->getKey(), page->location());
  ACLayer *layer;
  if((layer = qobject_cast<ACLayer*>(page))) {
    span->setSceneNode(layer->scene()->getKey());
  }
  ACObject::registerWithPage(page);
}

void ACDrawable::unregisterFromPage(ACPage* page)
{
  manager->MoveKey(draw->getKey(), plLocation());
  manager->MoveKey(span->getKey(), plLocation());
  ACLayer *layer;
  if((layer = qobject_cast<ACLayer*>(page))) {
    span->setSceneNode(plKey());
  }
  ACObject::unregisterFromPage(page);
}

