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

#include "ACPhysicalDrawable.h"
#include "ACLayer.h"
#include "ACUtil.h"

#include <PRP/Geometry/plDrawableSpans.h>
#include <PRP/Physics/plGenericPhysical.h>
#include <PRP/Object/plDrawInterface.h>
#include <PRP/Object/plSimulationInterface.h>
#include <PRP/Object/plSceneObject.h>
#include <PRP/plSceneNode.h>

ACPhysicalDrawable::ACPhysicalDrawable(const QString& name)
  : ACDrawable(name)
{
  phys = new plGenericPhysical;
  phys->init(toPlasma(name));
  phys->setSceneNode(manager->getSceneNode(plLocation())->getKey());
  phys->setObject(scene_object->getKey());
  phys->setRestitution(-1.0f);
  phys->setLOSDBs(0x44);
  phys->setBoundsType(plSimDefs::kProxyBounds);
  phys->setCategory(0x02000000);
  phys->setPos(hsVector3(0.0f, 0.0f, 0.0f));
  phys->setRot(hsQuat(0.0f, 0.0f, 1.0f, 0.0f));
  manager->AddObject(plLocation(), phys);
  sim = new plSimulationInterface;
  sim->init(toPlasma(name));
  sim->setOwner(scene_object->getKey());
  sim->setPhysical(phys->getKey());
  manager->AddObject(plLocation(), sim);
  scene_object->setSimInterface(sim->getKey());
  connect(this, SIGNAL(meshDataUpdated(size_t, const plGBufferVertex*, size_t, const unsigned short*)),
          this, SLOT(updateCollider(size_t, const plGBufferVertex*, size_t, const unsigned short*)));
}

ACPhysicalDrawable::ACPhysicalDrawable(plKey key)
  : ACDrawable(key)
{
  sim = static_cast<plSimulationInterface*>(scene_object->getSimInterface()->getObj());
  phys = static_cast<plGenericPhysical*>(sim->getPhysical()->getObj());
  connect(this, SIGNAL(meshDataUpdated(size_t, const plGBufferVertex*, size_t, const unsigned short*)),
          this, SLOT(updateCollider(size_t, const plGBufferVertex*, size_t, const unsigned short*)));
}

ACPhysicalDrawable::~ACPhysicalDrawable()
{
  manager->DelObject(sim->getKey());
  manager->DelObject(phys->getKey());
}

void ACPhysicalDrawable::setColliderVerts(const hsTArray<hsVector3>& verts)
{
  phys->setVerts(verts.getSize(), &(verts[0]));
}

void ACPhysicalDrawable::setColliderIndices(const hsTArray<unsigned int>& indices)
{
  phys->setIndices(indices.getSize(), &(indices[0]));
}

QIcon ACPhysicalDrawable::icon() const
{
  return ACIcon("draw-rectangle");
}

void ACPhysicalDrawable::registerWithPage(ACPage* page)
{
  manager->MoveKey(sim->getKey(), page->location());
  manager->MoveKey(phys->getKey(), page->location());
  ACLayer *layer;
  if((layer = qobject_cast<ACLayer*>(page)))
    phys->setSceneNode(layer->scene()->getKey());
  ACDrawable::registerWithPage(page);
}

void ACPhysicalDrawable::unregisterFromPage(ACPage* page)
{
  manager->MoveKey(sim->getKey(), plLocation());
  manager->MoveKey(sim->getKey(), plLocation());
  phys->setSceneNode(manager->getSceneNode(plLocation())->getKey());
  ACDrawable::unregisterFromPage(page);
}

void ACPhysicalDrawable::updateCollider(size_t vs, const plGBufferVertex *v, size_t is, const unsigned short *i)
{
  hsTArray<hsVector3> phys_verts;
  hsTArray<unsigned int> phys_indices;
  phys_verts.setSize(vs);
  phys_indices.setSize(is);
  for(size_t j = 0; j < vs; j++)
    phys_verts[j] = v[j].fPos;
  for(size_t j = 0; j < is; j++)
    phys_indices[j] = i[j];
  setColliderIndices(phys_indices);
  setColliderVerts(phys_verts);
}

void ACPhysicalDrawable::setName(const QString& name)
{
  ACDrawable::setName(name);
  sim->getKey()->setName(toPlasma(name));
  phys->getKey()->setName(toPlasma(name));
}
