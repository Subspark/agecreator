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
#include "ACMesh.h"
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

ACDrawable::ACDrawable(const QString &name)
  : ACObject(name)
{
  drawi = new plDrawInterface;
  drawi->init(toPlasma(name));
  drawi->setOwner(scene_object->getKey());
  manager->AddObject(virtual_loc, drawi);
  scene_object->setDrawInterface(drawi->getKey());
  
  QAction *action;
  menu->addAction(tr("World Object Properties"));
  action = menu->addAction(tr("Manage Submeshes"));
  connect(action, SIGNAL(triggered(bool)), this, SLOT(manageMeshes()));
}

ACDrawable::ACDrawable(plKey key)
  : ACObject(key)
{
  drawi = static_cast<plDrawInterface*>(scene_object->getDrawInterface()->getObj());
  for(size_t i = 0; i < drawi->getNumDrawables(); i++) {
    plDrawableSpans *spans = static_cast<plDrawableSpans*>(drawi->getDrawable(i)->getObj());
    if(drawi->getDrawableKey(i) < 0)
      continue;
    plDISpanIndex idx = spans->getDIIndex(drawi->getDrawableKey(i));
    for(size_t j = 0; j < idx.fIndices.getSize(); j++)
      if(idx.fIndices[j] < spans->getNumSpans())
        meshes.append(new ACMesh(static_cast<plIcicle*>(spans->getSpan(idx.fIndices[j])), spans->getKey()));
  }
  
  QAction *action;
  menu->addAction(tr("World Object Properties"));
  action = menu->addAction(tr("Manage Submeshes"));
  connect(action, SIGNAL(triggered(bool)), this, SLOT(addMeshFromFile()));
}

ACDrawable::ACDrawable::~ACDrawable()
{
  foreach(ACMesh *mesh, meshes)
    delete mesh;
  manager->DelObject(drawi->getKey());
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
  plDrawableSpans *spans = findSpans(0, 0);
  meshes.append(new ACMesh(verts, indices, (unsigned char)plGBufferGroup::kEncoded, spans));
  return true;
}

void ACDrawable::draw(DrawMode mode, unsigned int shader) const
{
  //TODO: draw only for the correct renderlevel
  foreach(ACMesh *mesh, meshes) {
    if(mode == Draw3D)
      mesh->draw(shader, scene_object->getCoordInterface(), true, true);
    else
      mesh->draw(shader, scene_object->getCoordInterface());
  }
}

QIcon ACDrawable::icon() const
{
  return ACIcon("draw-triangle");
}

void ACDrawable::registerWithPage(ACPage* page)
{
  manager->MoveKey(drawi->getKey(), page->location());
  //TODO: move all mesh data
  ACObject::registerWithPage(page);
}

void ACDrawable::unregisterFromPage(ACPage* page)
{
  manager->MoveKey(drawi->getKey(), virtual_loc);
  // TODO: move all mesh data
  ACObject::unregisterFromPage(page);
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

plDrawableSpans *ACDrawable::findSpans(unsigned int render_level, unsigned int criteria)
{
  std::vector<plKey> keys = manager->getKeys(location(), kDrawableSpans);
  for(int i = 0; i < keys.size(); i++) {
    plDrawableSpans *spans = static_cast<plDrawableSpans*>(keys[i]->getObj());
    if(spans->getRenderLevel() == render_level && spans->getCriteria() == criteria)
      return spans;
  }
  plDrawableSpans *spans = new plDrawableSpans;
  QString basename = toQt(manager->getSceneNode(location())->getKey()->getName());
  QString rlevel = QString(ascii("%1")).arg(render_level, 8, 16);
  QString crit = QString(ascii("%1")).arg(criteria, 1, 16);
  QString span;
  if(render_level)
    span = ascii("BlendSpans");
  else
    span = ascii("Spans");
  spans->init(toPlasma(basename+ascii("_")+rlevel+ascii("_")+crit+span));
  spans->setSceneNode(manager->getSceneNode(location())->getKey());
  spans->getKey()->setLocation(location());
  spans->setRenderLevel(render_level);
  spans->setCriteria(criteria);
  return spans;
}
