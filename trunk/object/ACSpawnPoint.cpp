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

#include "ACSpawnPoint.h"
#include "ACDrawable.h"
#include "ACPage.h"
#include "ACUtil.h"

#include <PRP/Modifier/plSpawnModifier.h>
#include <PRP/Object/plCoordinateInterface.h>
#include <PRP/Object/plSceneObject.h>

#include <GL/gl.h>

ACSpawnPoint::ACSpawnPoint(const QString& name)
  : ACObject(name)
{
  spawn = new plSpawnModifier;
  spawn->init(toPlasma(name));
  coord= new plCoordinateInterface;
  coord->init(toPlasma(name));
  manager->AddObject(virtual_loc, spawn);
  manager->AddObject(virtual_loc, coord);
  scene_object->setCoordInterface(coord->getKey());
  coord->setOwner(scene_object->getKey());
  scene_object->addModifier(spawn->getKey());
  
  hector = new ACDrawable(ascii("hector_")+name);
  hector->loadFromFile(ascii(":/data/hector.obj"));
  plSceneObject *obj = static_cast<plSceneObject*>(hector->key()->getObj());
  obj->setCoordInterface(coord->getKey());
}

ACSpawnPoint::ACSpawnPoint(plKey key)
  : ACObject(key)
{
  // TODO: make this more robust to allow spawn points with other modifiers
  spawn = static_cast<plSpawnModifier*>(scene_object->getModifier(0)->getObj());
  coord = static_cast<plCoordinateInterface*>(scene_object->getCoordInterface()->getObj());

  hector = new ACDrawable(ascii("hector_")+name());
  hector->loadFromFile(ascii(":/data/hector.obj"));
  plSceneObject *obj = static_cast<plSceneObject*>(hector->key()->getObj());
  obj->setCoordInterface(coord->getKey());
}

ACSpawnPoint::~ACSpawnPoint()
{
  manager->DelObject(spawn->getKey());
  manager->DelObject(coord->getKey());
}

QIcon ACSpawnPoint::icon() const
{
  return ACIcon("list-add-user");
}

void ACSpawnPoint::draw(DrawMode mode) const
{
  GLboolean current_cull;
  glGetBooleanv(GL_CULL_FACE, &current_cull);
  if(current_cull) {
    glDisable(GL_CULL_FACE);
    hector->draw(mode);
    glEnable(GL_CULL_FACE);
  } else {
    hector->draw(mode);
  }
}

void ACSpawnPoint::registerWithPage(ACPage *page)
{
  manager->MoveKey(spawn->getKey(), page->location());
  manager->MoveKey(coord->getKey(), page->location());
  ACObject::registerWithPage(page);
}

void ACSpawnPoint::unregisterFromPage(ACPage *page)
{
  manager->MoveKey(spawn->getKey(), virtual_loc);
  manager->MoveKey(coord->getKey(), virtual_loc);
  ACObject::unregisterFromPage(page);
}

void ACSpawnPoint::setName(const QString &name)
{
  ACObject::setName(name);
  spawn->getKey()->setName(toPlasma(name));
  coord->getKey()->setName(toPlasma(name));
}
