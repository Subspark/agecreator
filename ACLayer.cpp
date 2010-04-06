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

#include "ACLayer.h"
#include "ACAge.h"
#include "ACPhysicalDrawable.h"
#include "ACSpawnPoint.h"
#include "ACUtil.h"

#include <PRP/KeyedObject/plLocation.h>
#include <PRP/plPageInfo.h>
#include <PRP/plSceneNode.h>
#include <ResManager/plResManager.h>

#include <QPointer>

ACLayer::ACLayer(const QString &name, int page, ACAge *age)
  : ACPage(name, page, age)
{
  node = new plSceneNode();
  // Assuming node names match file names in Eoa, like they do in PotS
  if(manager->getVer() < pvEoa)
    node->init(toPlasma(age->name()+ascii("_District_")+name));
  else
    node->init(toPlasma(age->name()+name));
  manager->AddObject(location(), node);
}

ACLayer::ACLayer(const plLocation &loc, ACAge *age)
  : ACPage(loc, age)
{
  node = manager->getSceneNode(loc);
  if(node) {
    hsTArray<plKey>& array = node->getSceneObjects();
    for(unsigned int i = 0; i < array.getSize(); i++) {
      ACObject *obj = createPlasmaObject(array[i]);
      if(!obj)
        continue;
      obj->setParent(this);
      objects.append(obj);
    }
  }
}

plSceneNode *ACLayer::scene() const
{
  return node;
}

int ACLayer::objectCount() const
{
  return objects.size();
}

int ACLayer::findObject(const QString &name) const
{
  for(int i = 0; i < objects.size(); i++)
    if(objects[i]->name() == name)
      return i;
  return -1;
}

ACObject *ACLayer::getObject(int i) const
{
  return objects[i];
}

void ACLayer::addObject(ACObject *obj)
{
  objects.append(obj);
  obj->registerWithPage(this);
}

void ACLayer::setName(const QString &name)
{
  ACPage::setName(name);
  if(manager->getVer() < pvEoa)
    node->init(toPlasma(age()->name()+ascii("_District_")+name));
  else
    node->init(toPlasma(age()->name()+name));
}

// This function could get much more complicated over time.
// Need to find a better way to do this
ACObject *ACLayer::createPlasmaObject(plKey key)
{
  if(!key.Exists())
    return 0;
  plSceneObject *obj = static_cast<plSceneObject*>(key->getObj());
  if(!obj)
    return 0;
  if(obj->getCoordInterface().Exists() && obj->getModifiers().getSize() == 1) {
    if(obj->getModifiers()[0]->getType() == kSpawnModifier)
      return new ACSpawnPoint(key);
  }
  if(obj->getDrawInterface().Exists() && obj->getSimInterface().Exists()) {
    return new ACPhysicalDrawable(key);
  } else if(obj->getDrawInterface().Exists()) {
    return new ACDrawable(key);
  }
  return new ACObject(key);
}