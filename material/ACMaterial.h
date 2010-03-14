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

#ifndef ACMATERIAL_H
#define ACMATERIAL_H

#include <QObject>
#include <QSharedPointer>
#include <QStringList>

#include "ACTexture.h"

#include <PRP/KeyedObject/plKey.h>

class hsGMaterial;

class ACMaterial : public QObject
{
  Q_OBJECT
public:
  ACMaterial(plKey);
  ACMaterial(const QString &name);
  
  void addLayer(const QString &name);
  QStringList layerNames();
  
  plKey key() const;
  void setLocation(plLocation loc);

protected:
  hsGMaterial *mat;
  QList<QSharedPointer<ACTexture> > textures;
};

#endif // ACMATERIAL_H