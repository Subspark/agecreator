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

#ifndef ACOBJECT_H
#define ACOBJECT_H

#include <QObject>
#include <QIcon>

#include "PRP/KeyedObject/plKey.h"

class plSceneObject;
class plPageInfo;

class ACPage;

class ACObject : public QObject
{
  Q_OBJECT;
public:
  ACObject(const QString& name);
  ACObject(plKey key);
  ~ACObject();

  ACPage *page() const;
  QString name() const;

  virtual QIcon icon() const;
  virtual void registerWithPage(ACPage *page);
  virtual void unregisterFromPage(ACPage *page);
protected:
  plSceneObject *scene_object;
};

#endif // ACOBJECT_H
