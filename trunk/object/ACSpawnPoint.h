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

#ifndef ACSPAWNPOINT_H
#define ACSPAWNPOINT_H

#include "ACObject.h"

class plCoordinateInterface;
class plSpawnModifier;

class ACSpawnPoint : public ACObject
{
  Q_OBJECT
public:
  ACSpawnPoint(const QString &name);
  ACSpawnPoint(plKey key);
  virtual ~ACSpawnPoint();

  virtual QIcon icon() const;
  virtual void registerWithPage(ACPage *page);
  virtual void unregisterFromPage(ACPage *page);

public slots:
  virtual void setName(const QString &name);

private:
  plSpawnModifier *spawn;
  plCoordinateInterface *coord;
};

#endif // ACSPAWNPOINT_H