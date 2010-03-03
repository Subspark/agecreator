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

#ifndef ACDRAWABLE_H
#define ACDRAWABLE_H

#include "ACObject.h"

#include "Util/hsTArray.hpp"

class plDrawableSpans;
class plDrawInterface;
class plGBufferVertex;

//TODO: Make this class more robust. Currently it only supports the most basic drawables
class ACDrawable : public ACObject
{
  Q_OBJECT
public:
  ACDrawable(const QString &name);
  ACDrawable(plKey key);
  
  void setMeshData(const hsTArray<plGBufferVertex> &verts, const hsTArray<unsigned short> &indices, unsigned char format);
  
  virtual QIcon icon() const;
  virtual void registerWithPage(ACPage* page);
  virtual void unregisterFromPage(ACPage* page);

//   Not yet implemented. Will support multi-material meshes and other cool stuff in an advanced
//   format, whenever it gets implemented
//   TODO: implement loadFromFile
//   static ACDrawable *loadFromFile(const QString &filename);

protected:
  plDrawableSpans *span;
  plDrawInterface *draw;
};

#endif // ACPHYSICSDRAWABLE_H
