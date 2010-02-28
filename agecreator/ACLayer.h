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

#ifndef ACLAYER_H
#define ACLAYER_H

#include "ACPage.h"
class ACObject;

class plSceneNode;

class ACLayer : public ACPage
{
  Q_OBJECT
public:
  ACLayer(const QString &name, int page, ACAge *age);
  ACLayer(const plLocation &loc, ACAge *age);

  plSceneNode *scene() const;
  int objectCount() const;
  int findObject(const QString &name) const;
  ACObject *getObject(int) const;
  void addObject(ACObject *obj);

private:
  plSceneNode *node;
  QList<ACObject*> objects;
  ACObject* createPlasmaObject(plKey);
};

#endif // ACLAYER_H
