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

#include <QMap>
#include <QPair>
#include <QSharedPointer>

#include "Util/hsTArray.hpp"

class plDrawableSpans;
class plDrawInterface;
class plGBufferVertex;

class ACMesh;

class ACDrawable : public ACObject
{
  Q_OBJECT
public:
  ACDrawable(const QString &name);
  ACDrawable(plKey key);
  virtual ~ACDrawable();
  
  //TODO: make this more generalized towards dealing with sub-meshes
  bool loadFromFile(const QString &filename);
  
  virtual void draw(DrawMode draw, unsigned int rlevel = 0xFFFFFFFF) const;
  virtual QIcon icon() const;
  virtual void registerWithPage(ACPage* page);
  virtual void unregisterFromPage(ACPage* page);

public slots:
  void setName(const QString &name);

signals:
  void meshDataUpdated(size_t, const plGBufferVertex*, size_t, const unsigned short*);

protected:
  plDrawInterface *drawi;
  unsigned char format;
  QList<ACMesh*> meshes;
  plDrawableSpans *findSpans(unsigned int render_level, unsigned int criteria);

private slots:
  void idUpdated(int id, unsigned char fmt);
};

#endif // ACDRAWABLE_H
