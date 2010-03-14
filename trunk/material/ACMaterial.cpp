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

#include "ACMaterial.h"
#include "ACUtil.h"

#include <PRP/Surface/hsGMaterial.h>
#include <PRP/Surface/plLayer.h>

ACMaterial::ACMaterial(const QString &name)
{
  mat = new hsGMaterial;
  mat->init(toPlasma(name));
  manager->AddObject(plLocation(), mat);
  addLayer(name + ascii("_layer"));
  mat->setCompFlags(hsGMaterial::kCompSpecular);
}

void ACMaterial::addLayer(const QString &name)
{
  QSharedPointer<ACTexture> tex(new ACTexture(name));
  tex->setParent(this);
  mat->addLayer(tex->key());
  textures.append(tex);
}

plKey ACMaterial::key() const
{
  return mat->getKey();
}

void ACMaterial::setLocation(plLocation loc)
{
  manager->MoveKey(key(), loc);
  foreach(QSharedPointer<ACTexture> t, textures)
    t->setLocation(loc);
}
