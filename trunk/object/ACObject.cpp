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

#include "ACObject.h"
#include "ACAge.h"
#include "ACLayer.h"
#include "ACUtil.h"

#include "PRP/Object/plSceneObject.h"
#include "PRP/plSceneNode.h"
#include "Util/hsTArray.hpp"

#include <QMenu>
#include <QContextMenuEvent>

ACObject::ACObject(const QString& name)
  : QObject(0)
{
  scene_object = new plSceneObject;
  scene_object->init(toPlasma(name));
  manager->AddObject(virtual_loc, scene_object);
  menu = new QMenu;
}

ACObject::ACObject(plKey key)
  : QObject(0)
{
  scene_object = plPointer<plSceneObject>(key);
  menu = new QMenu;
}

ACObject::~ACObject()
{
  manager->DelObject(scene_object->getKey());
}

ACPage *ACObject::page() const
{
  return qobject_cast<ACLayer*>(parent());
}

QString ACObject::name() const
{
  return toQt(scene_object->getKey()->getName());
}

plKey ACObject::key() const
{
  return scene_object->getKey();
}

plLocation ACObject::location() const
{
  return scene_object->getKey()->getLocation();
}

void ACObject::setName(const QString &name)
{
  scene_object->getKey()->setName(toPlasma(name));
}

void ACObject::contextMenu(QContextMenuEvent *event)
{
  menu->exec(event->globalPos());
}

void ACObject::draw(DrawMode /*draw*/) const
{}

QIcon ACObject::icon() const
{
  // TODO: replace this with a better "generic object" icon
  return ACIcon("dialog-error");
}

void ACObject::registerWithPage(ACPage *page)
{
  manager->MoveKey(scene_object->getKey(), page->location());
  ACLayer *layer;
  if((layer = qobject_cast<ACLayer*>(page))) {
    layer->scene()->addSceneObject(scene_object->getKey());
    scene_object->setSceneNode(layer->scene()->getKey());
  }
  setParent(page);
}

void ACObject::unregisterFromPage(ACPage *page)
{
  manager->MoveKey(scene_object->getKey(), virtual_loc);
  ACLayer *layer;
  if((layer = qobject_cast<ACLayer*>(page))) {
    layer->scene()->delSceneObject(layer->scene()->getSceneObjects().find(scene_object->getKey()));
    scene_object->setSceneNode(plKey());
  }
  setParent(0);
}
