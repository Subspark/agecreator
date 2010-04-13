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
#include "glew.h"

#include "ACMesh.h"
#include "ACUtil.h"

#include <PRP/Geometry/plDrawableSpans.h>
#include <PRP/Object/plCoordinateInterface.h>
#include <PRP/Surface/hsGMaterial.h>
#include <PRP/Surface/plLayer.h>
#include <PRP/plSceneNode.h>

#include <QSet>

#include <climits>

QSet<ACMesh*> meshes;

ACMesh::ACMesh(const hsTArray<plGBufferVertex> &verts, const hsTArray<unsigned short> &indices, unsigned char fmt, plDrawableSpans *spans)
{
  size_t group_id;
  bool found = false;
  for(size_t i = 0; i < spans->getNumBufferGroups(); i++) {
    plGBufferGroup *group = spans->getBuffer(i);
    if(group->getFormat() == fmt) {
      found = true;
      group_id = i;
    }
  }
  if(!found) {
    group_id = spans->createBufferGroup(fmt);
  }
  
  hsTArray<plGBufferVertex> current_verts;
  hsTArray<unsigned short> current_indices;
  hsTArray<plGBufferCell> current_cells;
  if(spans->getBuffer(group_id)->getNumVertBuffers()) {
    current_verts = spans->getBuffer(group_id)->getVertices(0);
    current_cells = spans->getBuffer(group_id)->getCells(0);
  } else {
    plGBufferCell cell;
    cell.fVtxStart = 0;
    cell.fColorStart = UINT_MAX;
    cell.fLength = 0;
    current_cells.append(cell);
  }
  if(spans->getBuffer(group_id)->getNumIdxBuffers())
    current_indices = spans->getBuffer(group_id)->getIndices(0);
  size_t vertex_offset = current_verts.getSize();
  size_t index_offset = current_indices.getSize();
  spans->getBuffer(group_id)->clearVertices();
  spans->getBuffer(group_id)->clearIndices();
  spans->getBuffer(group_id)->clearCells();
  current_verts.append(verts);
  current_indices.setSize(index_offset + indices.getSize());
  for(size_t i = 0; i < indices.getSize(); i++)
      current_indices[index_offset+i] = indices[i] + vertex_offset;
  current_cells[0].fLength = current_verts.getSize();
  spans->addVerts(group_id, current_verts);
  spans->addIndices(group_id, current_indices);
  spans->addCells(group_id, current_cells);
  float min_x = 0.0f, min_y = 0.0f, min_z = 0.0f, max_x = 0.0f, max_y = 0.0f, max_z = 0.0f;
  for(unsigned int i = 0; i < verts.getSize(); i++) {
    if(verts[i].fPos.X > max_x)
      max_x = verts[i].fPos.X;
    if(verts[i].fPos.X < min_x)
      min_x = verts[i].fPos.X;
    if(verts[i].fPos.Y > max_y)
      max_y = verts[i].fPos.Y;
    if(verts[i].fPos.Y < min_y)
      min_y = verts[i].fPos.Y;
    if(verts[i].fPos.Z > max_z)
      max_z = verts[i].fPos.Z;
    if(verts[i].fPos.Z < min_z)
      min_z = verts[i].fPos.Z;
  }
  hsBounds3Ext bounds;
  bounds.setFlags(hsBounds3Ext::kAxisAligned);
  plIcicle icicle;
  icicle.setLocalBounds(bounds);
  icicle.setWorldBounds(bounds);
  icicle.setGroupIdx(group_id);
  icicle.setMaterialIdx(0);
  icicle.setVBufferIdx(0);
  icicle.setIBufferIdx(0);
  icicle.setCellIdx(0);
  icicle.setCellOffset(vertex_offset);
  icicle.setIStartIdx(index_offset);
  icicle.setVStartIdx(vertex_offset);
  icicle.setILength(indices.getSize());
  icicle.setVLength(verts.getSize());
  icicle.setMinDist(-1.0f);
  icicle.setMaxDist(-1.0f);
  icicle_id = spans->addIcicle(icicle);
  
  this->spans = spans->getKey();
  
  meshes.insert(this);
}

ACMesh::ACMesh(size_t icicle_id_, plKey spans_)
  : spans(spans_), icicle_id(icicle_id_)
{
  meshes.insert(this);
}

ACMesh::~ACMesh()
{
  if(spans.Exists()) {
    plDrawableSpans *span = plPointer<plDrawableSpans>(spans);
    //TODO: remove mesh data
//    foreach(ACMesh* mesh, meshes)
//      if(mesh->spans == spans && mesh->icicle_id > icicle_id) {
//        mesh->icicle_id--;
//      }
  }
  meshes.remove(this);
}

void ACMesh::draw(plKey ci, unsigned int draw_flags) const
{
  plDrawableSpans *span = plPointer<plDrawableSpans>(spans);
  unsigned int uvw_id;
  plIcicle *icicle = span->getIcicle(icicle_id);
  if(draw_flags & DRAW_MATERIAL && icicle->getMaterialIdx() < span->getMaterials().getSize()) {
    hsGMaterial *mat = plPointer<hsGMaterial>(span->getMaterials()[icicle->getMaterialIdx()]);
    plLayer *layer = plPointer<plLayer>(mat->getLayers()[0]);
    unsigned int new_layer = 1;
    while(!(layer->getTexture().Exists()) || layer->getTexture()->getType() != kMipmap) {
      if(new_layer < mat->getLayers().getSize())
        layer = plPointer<plLayer>(mat->getLayers()[new_layer]);
      else
        break;
      new_layer++;
    }
    while(layer->getUnderLay().Exists()) {
      layer = plPointer<plLayer>(layer->getUnderLay());
    }
    hsColorRGBA color = layer->getRuntime();
    glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, color.r, color.g, color.b, color.a);
    glEnable(GL_TEXTURE_2D);
    if(layer->getTexture().Exists())
      glBindTexture(GL_TEXTURE_2D, texture_ids.value(layer->getTexture(), 0));
    uvw_id = layer->getUVWSrc();
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(layer->getTransform().glMatrix());
    glMatrixMode(GL_MODELVIEW);
  } else if(draw_flags & DRAW_COLOR) {
    glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, 1.0f, 1.0f, 1.0f, 1.0f);
  }
  hsMatrix44 mat;
  if(ci.Exists()) {
    plCoordinateInterface *coord = plPointer<plCoordinateInterface>(ci);
    mat = coord->getLocalToWorld();
  } else {
    mat = icicle->getLocalToWorld();
  }
  glPushMatrix();
  glMultMatrixf(mat.glMatrix());
  plGBufferGroup *buff = span->getBuffer(icicle->getGroupIdx());
  const unsigned char *vertex_data = buff->getVertBufferStorage(icicle->getVBufferIdx());
  const unsigned short *index_data = buff->getIdxBufferStorage(icicle->getIBufferIdx());
  size_t normal_offset = 3*sizeof(float);
  unsigned char fmt = buff->getFormat();
  int weightCount = (fmt & plGBufferGroup::kSkinWeightMask) >> 4;
  if(weightCount > 0) {
    normal_offset += sizeof(float) * weightCount;
    if(fmt & plGBufferGroup::kSkinIndices)
      normal_offset += sizeof(int);
  }
  glVertexPointer(3, GL_FLOAT, buff->getStride(), vertex_data);
  glNormalPointer(GL_FLOAT, buff->getStride(), vertex_data + normal_offset);
  if(uvw_id <= 9) {
    size_t uv_offset = normal_offset + 3*sizeof(float) + 2*sizeof(int) + 3*sizeof(float)*uvw_id;
    glTexCoordPointer(3, GL_FLOAT, buff->getStride(), vertex_data+uv_offset);
  } else {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoord3f(0.0f, 0.0f, 0.0f);
  }
  if(draw_flags & DRAW_COLOR) {
    glColorPointer(4, GL_UNSIGNED_BYTE, buff->getStride(), vertex_data + normal_offset + 3*sizeof(float));
  }
  glDrawElements(GL_TRIANGLES, icicle->getILength(), GL_UNSIGNED_SHORT, &(index_data[icicle->getIStartIdx()]));
  if(uvw_id > 9) {
     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }
  glPopMatrix();
}

unsigned int ACMesh::renderLevel() const
{
  return plPointer<plDrawableSpans>(spans)->getRenderLevel();
}

plKey ACMesh::spansKey() const
{
  return spans;
}

unsigned int ACMesh::spanId() const
{
  return icicle_id;
}
