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

#include <PRP/KeyedObject/plKey.h>

#include <QVector>

class plDrawableSpans;
class plGBufferVertex;

class ACMesh
{
public:
  enum {
    DRAW_MATERIAL=1,
    DRAW_COLOR=2,
    DRAW_HILIGHT=4,
  };
  ACMesh(const hsTArray<plGBufferVertex> &verts, const hsTArray<unsigned short> &indices, unsigned char fmt, plDrawableSpans *spans);
  //ACMesh(const ACMesh &other, plDrawableSpans *new_spans);
  ACMesh(size_t icicle_id_, plKey spans_);
  ~ACMesh();

  void draw(plKey ci = plKey(), unsigned int draw_flags = 0) const;
  //plKey material() const;
  //void setMaterial(plKey new_mat);
  unsigned int renderLevel() const;
  plKey spansKey() const;
  unsigned int spanId() const;

private:
  ACMesh(const ACMesh &); // unimplemented
  void operator=(const ACMesh&); // unimplemented

  size_t icicle_id;
  plKey spans;

  friend class ACMeshManager;
};