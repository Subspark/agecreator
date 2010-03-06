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
#include "ACAge.h"
#include "ACLayer.h"
#include "ACUtil.h"

#include <PRP/Modifier/plSpawnModifier.h>
#include <PRP/Object/plCoordinateInterface.h>
#include <PRP/Object/plSceneObject.h>

ACSpawnPoint::ACSpawnPoint(const QString& name)
  : ACObject(name)
{
  spawn = new plSpawnModifier;
  spawn->init(toPlasma(name));
  coord= new plCoordinateInterface;
  coord->init(toPlasma(name));
  manager->AddObject(plLocation(), spawn);
  manager->AddObject(plLocation(), coord);
  scene_object->setCoordInterface(coord->getKey());
  coord->setOwner(scene_object->getKey());
  scene_object->addModifier(spawn->getKey());
}

ACSpawnPoint::ACSpawnPoint(plKey key)
  : ACObject(key)
{
  // TODO: make this more robust to allow spawn points with other modifiers
  spawn = static_cast<plSpawnModifier*>(scene_object->getModifier(0)->getObj());
  coord = static_cast<plCoordinateInterface*>(scene_object->getCoordInterface()->getObj());
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

void ACSpawnPoint::registerWithPage(ACPage *page)
{
  manager->MoveKey(spawn->getKey(), page->location());
  manager->MoveKey(coord->getKey(), page->location());
  ACObject::registerWithPage(page);
}

void ACSpawnPoint::unregisterFromPage(ACPage *page)
{
  manager->MoveKey(spawn->getKey(), plLocation());
  manager->MoveKey(coord->getKey(), plLocation());
  ACObject::unregisterFromPage(page);
}