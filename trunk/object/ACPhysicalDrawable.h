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

#ifndef ACPHYSICALDRAWABLE_H
#define ACPHYSICALDRAWABLE_H

#include "ACDrawable.h"

#include "Math/hsGeometry3.h"

class plGenericPhysical;
class plSimulationInterface;

class ACPhysicalDrawable : public ACDrawable
{
  Q_OBJECT
public:
  ACPhysicalDrawable(const QString &name);
  ACPhysicalDrawable(plKey key);
  virtual ~ACPhysicalDrawable();

  void setColliderVerts(const hsTArray<hsVector3>& verts);
  void setColliderIndices(const hsTArray<unsigned int> &indices);

  virtual QIcon icon() const;
  virtual void registerWithPage(ACPage* page);
  virtual void unregisterFromPage(ACPage* page);

public slots:
  virtual void setName(const QString &name);

protected:
  plGenericPhysical *phys;
  plSimulationInterface *sim;

private slots:
  void updateCollider(size_t, const plGBufferVertex*, size_t, const unsigned short*);
};

#endif // ACPHYSICALDRAWABLE_H
