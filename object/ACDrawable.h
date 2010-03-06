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

class ACDrawableSpans : public QObject
{
  Q_OBJECT
public:
  ACDrawableSpans();
  ~ACDrawableSpans();
  void load(plLocation);
  plDrawableSpans *getSpan(unsigned int render_level, unsigned int criteria);
  size_t getGroupId(plDrawableSpans *span, unsigned char format);
  void meshRemoved(int id, unsigned char format);
  
signals:
  void idUpdated(int, unsigned char format);
private:
  QMap<QPair<unsigned int, unsigned int>, plDrawableSpans*> spans;
  QMap<QPair<plDrawableSpans *, unsigned char>, size_t> group_ids;
  plKey scene_node;
  plString span_name(unsigned int render_level, unsigned int criteria);
};

class ACDrawable : public ACObject
{
  Q_OBJECT
public:
  ACDrawable(const QString &name);
  ACDrawable(plKey key);
  virtual ~ACDrawable();
  
  void setMeshData(const hsTArray<plGBufferVertex> &verts, const hsTArray<unsigned short> &indices, unsigned char fmt);
  bool loadFromFile(const QString &filename);
  
  virtual QIcon icon() const;
  virtual void registerWithPage(ACPage* page);
  virtual void unregisterFromPage(ACPage* page);

protected:
  plDrawInterface *draw;
  unsigned char format;
  QSharedPointer<ACDrawableSpans> spans;
  
  void clearMeshData();
  void moveMeshData(plLocation loc);
  
  static QMap<plLocation, QWeakPointer<ACDrawableSpans> > weak_spans;
  static QSharedPointer<ACDrawableSpans> getSpans(plLocation loc);

private slots:
  void idUpdated(int id, unsigned char fmt);
};

#endif // ACDRAWABLE_H
