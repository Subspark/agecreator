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

#include "ACAge.h"
#include "ACLayer.h"
#include "ACUtil.h"

#include "ACObject.h"
#include "ACSpawnPoint.h"
#include "ACPhysicalDrawable.h"

#include "glew.h"

#include <ResManager/plResManager.h>
#include <ResManager/plAgeInfo.h>
#include <PRP/KeyedObject/plLocation.h>
#include <PRP/Surface/hsGMaterial.h>
#include <PRP/Surface/plLayer.h>
#include <PRP/Surface/plMipmap.h>
#include <PRP/plPageInfo.h>
#include <PRP/plSceneNode.h>
#include <Stream/plEncryptedStream.h>

#include <QDir>
#include <QFileDialog>
#include <QGLWidget>
#include <QIcon>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPointer>
#include <QRegExpValidator>
#include <QStringList>

#define COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

//TODO: Generate this from actual user fog settings
const char *fni_string=
"#--Fog settings--\n"
"#Graphics.Renderer.SetYon float yon\n"
"#Visibility distance\n"
"Graphics.Renderer.SetYon 100000\n"
"\n"
"#Graphics.Renderer.Fog.SetDefLinear float start, float end, float density\n"
"#Fog depth\n"
"Graphics.Renderer.Fog.SetDefLinear 1 1000 1\n"
"\n"
"#Graphics.Renderer.Fog.SetDefExp2 float end, float density\n"
"#Graphics.Renderer.Fog.SetDefExp2 100000 20\n"
"\n"
"#Graphics.Renderer.Fog.SetDefColor float r, float g, float b\n"
"Graphics.Renderer.Fog.SetDefColor 0.15 0.13 0.19\n"
"\n"
"#Graphics.Renderer.SetClearColor float r, float g, float b\n"
"Graphics.Renderer.SetClearColor 0.15 0.13 0.19\n";

ACAge::ACAge(const QString& name)
  : QAbstractItemModel(0)
{
  age = new plAgeInfo;
  age->setAgeName(toPlasma(name));
  manager->AddAge(age);
  // a "NULL" page, where objects that have been created but not yet assigned a page are stored
  plPageInfo *info = new plPageInfo;
  info->setLocation(virtual_loc);
  manager->AddPage(info);
  plSceneNode *null_node = new plSceneNode;
  null_node->init(toPlasma(name));
  manager->AddObject(virtual_loc, null_node);
  // An Age created in-memory is always dirty
  textures = new ACPage(ascii("Textures"), -1, this);
  builtins = new ACPage(ascii("BuiltIn"), -2, this);
  addLayer(ascii("mainRoom"));
  layers[0]->addObject(new ACSpawnPoint(ascii("LinkInPointDefault")));
  dirty = true;
}

ACAge::ACAge(const QString &filename, QObject *parent)
  : QAbstractItemModel(parent)
{
  age = manager->ReadAge(filename.toLocal8Bit().constData(), true);
  // a "NULL" page, where objects that have been created but not yet assinged a page are stored
  plPageInfo *info = new plPageInfo;
  info->setLocation(virtual_loc);
  manager->AddPage(info);
  plSceneNode *null_node = new plSceneNode;
  null_node->init(toPlasma(name()));
  manager->AddObject(virtual_loc, null_node);
  std::vector<plLocation> locs = manager->getLocations();
  std::vector<plLocation> age_locs;
  for(int i = 0; i < locs.size(); i++) {
    if(locs[i].getSeqPrefix() == sequencePrefix() && manager->FindPage(locs[i]))
      age_locs.push_back(locs[i]);
  }
  size_t num_pages = age_locs.size();
  beginInsertRows(QModelIndex(), 0, num_pages-1);
  for(size_t i = 0; i < num_pages; i++) {
    layers.append(new ACLayer(age_locs[i], this));
  }
  endInsertRows();
  // Manually load textures and Builtin PRPs
  QString district;
  if(manager->getVer() < pvEoa)
    district = ascii("_District");
  QDir filepath = filename;
  filepath.cdUp();
  QString textures_name = filepath.absoluteFilePath(name()+district+ascii("_Textures.prp"));
  QString builtins_name = filepath.absoluteFilePath(name()+district+ascii("_BuiltIn.prp"));
  textures = new ACPage(manager->ReadPage(toPlasma(textures_name))->getLocation(), this);
  builtins = new ACPage(manager->ReadPage(toPlasma(builtins_name))->getLocation(), this);
  dirty = false;
}

ACAge::~ACAge()
{
  manager->DelAge(toPlasma(name()));
  manager->DelPage(virtual_loc);
}

// Model/View functions

int ACAge::columnCount(const QModelIndex &/*parent*/) const
{
  return 1;
}

Qt::ItemFlags ACAge::flags(const QModelIndex &index) const
{
  if(!index.isValid())
    return Qt::ItemIsEnabled;
  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant ACAge::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole && role != Qt::DecorationRole && role != Qt::EditRole)
    return QVariant();
  
  if(index == QModelIndex())
    return QVariant();

  if(index.parent() == QModelIndex())
    if(role == Qt::DisplayRole || role == Qt::EditRole)
      return QVariant(layers[index.row()]->name());
    else
      if(layers[index.row()]->isDirty())
        return QVariant(ACIcon("folder-important"));
      else
        return QVariant(ACIcon("folder"));
  else {
    ACObject *obj = static_cast<ACObject*>(index.internalPointer());
    if(role == Qt::DisplayRole || role == Qt::EditRole)
      return QVariant(obj->name());
    else
      return QVariant(obj->icon());
  }
}

bool ACAge::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(!index.isValid() || role != Qt::EditRole)
    return false;
  if(index.parent() == QModelIndex()) {
    layers[index.row()]->setName(value.toString());
    layers[index.row()]->makeDirty();
  } else {
    ACObject *obj = static_cast<ACObject*>(index.internalPointer());
    obj->setName(value.toString());
    layers[index.parent().row()]->makeDirty();
  }
  return true;
}

QModelIndex ACAge::index(int row, int column, const QModelIndex &parent) const
{
  if(0 != column)
    return QModelIndex();

  if(parent == QModelIndex()) {
    if(row < 0 || row >= layers.size())
      return QModelIndex();
    return createIndex(row, 0, layers[row]);
  }  else {
    ACLayer *layer = static_cast<ACLayer*>(parent.internalPointer());
    if(row < 0 || row >= layer->objectCount())
      return QModelIndex();
    ACObject *object = layer->getObject(row);
    return createIndex(row, 0, object);
  }
}

QModelIndex ACAge::parent(const QModelIndex &index) const
{
  if(index == QModelIndex())
    return QModelIndex();
  
  ACLayer *layer = qobject_cast<ACLayer*>(static_cast<QObject*>(index.internalPointer()));
  if(layer)
    return QModelIndex();
  else {
    ACObject *obj = qobject_cast<ACObject*>(static_cast<QObject*>(index.internalPointer()));
    layer = qobject_cast<ACLayer*>(obj->parent());
    return createIndex(layers.indexOf(layer), 0, layer);
  }
}

int ACAge::rowCount(const QModelIndex &parent) const
{
  if(parent == QModelIndex())
    return layers.size();
  else if(qobject_cast<ACLayer*>(static_cast<QObject*>(parent.internalPointer())))
    return layers[parent.row()]->objectCount();
  return 0;
}

// Getters

QString ACAge::name() const
{
  return toQt(age->getAgeName());
}

int ACAge::sequencePrefix() const
{
  return age->getSeqPrefix();
}

PlasmaVer ACAge::plasmaVersion() const
{
  return manager->getVer();
}

plKey ACAge::defaultMaterial()
{
  plLocation texture_page;
  texture_page.setSeqPrefix(sequencePrefix());
  texture_page.setPageNum(-1);
  std::vector<plKey> keys = manager->getKeys(texture_page, kGMaterial);
  plKey material_key;
  for(size_t i = 0; i < keys.size(); i++)
    if(keys[i]->getName() == plString("DEFAULT")) {
      material_key = keys[i];
      break;
    }
  if(! material_key.Exists()) {
    hsGMaterial *mat = new hsGMaterial;
    mat->init(plString("DEFAULT"));
    manager->AddObject(texture_page, mat);
    plLayer *l = new plLayer;
    l->init(plString("DEFAULT_layer0"));
    manager->AddObject(texture_page, l);
    l->setAmbient(hsColorRGBA());
    l->setPreshade(hsColorRGBA(1.0f, 1.0f, 1.0f));
    l->setRuntime(hsColorRGBA(1.0f, 1.0f, 1.0f));
    l->setSpecular(hsColorRGBA(1.0f, 1.0f, 1.0f));
    l->getState().fShadeFlags = hsGMatState::kShadeSpecular | hsGMatState::kShadeSpecularAlpha | hsGMatState::kShadeSpecularColor
                              | hsGMatState::kShadeSpecularHighlight;
    l->setLODBias(-1);
    mat->addLayer(l->getKey());
    material_key = mat->getKey();
  }
  return material_key;
}

bool ACAge::isDirty() const
{
  if(dirty)
    return true;
  bool dirty_page = false;
  foreach(ACLayer* layer, layers) {
    if(layer->isDirty())
      dirty_page = true;
  }
  return dirty_page;
}

int ACAge::layerCount() const
{
  return layers.size();
}

ACLayer *ACAge::getLayer(int i) const
{
  return layers[i];
}

// Setters
void ACAge::setPlasmaVersion(PlasmaVer ver)
{
  manager->setVer(ver);
}

void ACAge::setName(const QString &name)
{
  age->setAgeName(toPlasma(name));
}

void ACAge::setSequencePrefix(int prefix)
{
  age->setSeqPrefix(prefix);
  foreach(ACLayer *layer, layers) {
    plLocation old_loc = layer->page()->getLocation();
    plLocation new_loc(old_loc);
    new_loc.setSeqPrefix(prefix);
    layer->page()->setLocation(new_loc);
    manager->ChangeLocation(old_loc, new_loc);
  }
  plLocation old_loc = textures->page()->getLocation();
  plLocation new_loc(old_loc);
  new_loc.setSeqPrefix(prefix);
  textures->page()->setLocation(new_loc);
  manager->ChangeLocation(old_loc, new_loc);
  old_loc = builtins->page()->getLocation();
  new_loc = old_loc;
  new_loc.setSeqPrefix(prefix);
  builtins->page()->setLocation(new_loc);
  manager->ChangeLocation(old_loc, new_loc);
}

void ACAge::setMaxCapacity(short cap)
{
  age->setMaxCapacity(cap);
}

void ACAge::setSelectionModel(QItemSelectionModel *model)
{
  selection_model = model;
}

// Non-setters Slots

void ACAge::addLayer(QString name)
{
  ACLayer *l;
  int largest_suffix = 0;
  if(!layers.empty()) {
    foreach(ACLayer *layer, layers) {
      if(layer->suffix() > largest_suffix)
	largest_suffix = layer->suffix();
    }
    largest_suffix++;
  }
  if(name.isEmpty())
    name = ascii("Layer")+ascii("%1").arg(largest_suffix);
  l = new ACLayer(name, largest_suffix, this);
  plAgeInfo::PageEntry pe;
  pe.fName = toPlasma(l->name());
  pe.fSeqSuffix = l->suffix();
  age->addPage(pe);
  beginInsertRows(QModelIndex(), layers.size(), layers.size());
  layers.append(l);
  endInsertRows();
  dirty = true;
}

void ACAge::addObject(int object_type)
{
  ACLayer *current_layer;
  QModelIndex idx = selection_model->currentIndex();
  if(idx == QModelIndex())
    return;
  if(idx.parent() != QModelIndex())
    idx = idx.parent();
  current_layer = static_cast<ACLayer*>(idx.internalPointer());
  QString name;
  int id = 0;
  bool ok = false;
  while(!ok) {
    name = ascii("Object_")+ascii("%1").arg(id);
    if(current_layer->findObject(name) == -1)
      ok = true;
    id++;
  }
  ACObject *new_object = 0;
  switch(object_type) {
    case idSpawnPoint: {
      new_object = new ACSpawnPoint(name);
      break;
    }
    case idPhysDrawable: {
      new_object = new ACPhysicalDrawable(name);
      QString model_file = QFileDialog::getOpenFileName(NULL, tr("Import Age"), QString(), tr("Model Files (*.obj)"));
      if(model_file.isEmpty()) {
        // this tells the generic addObject() function to return
        return;
      } else if(!(qobject_cast<ACPhysicalDrawable*>(new_object)->loadFromFile(model_file))) {
        QMessageBox::critical(NULL, tr("AgeCreator Error"), tr("Could not load model file: %1").arg(model_file));
        return;
      }
      break;
    }
    case idDrawable: {
      new_object = new ACDrawable(name);
      QString model_file = QFileDialog::getOpenFileName(NULL, tr("Import Age"), QString(), tr("Model Files (*.obj)"));
      if(model_file.isEmpty()) {
        // this tells the generic addObject() function to return
        return;
      } else if(!(qobject_cast<ACDrawable*>(new_object)->loadFromFile(model_file))) {
        QMessageBox::critical(NULL, tr("AgeCreator Error"), tr("Could not load model file: %1").arg(model_file));
        return;
      }
      break;
    }
    default:
      break;
  }
  if(new_object) {
    beginInsertRows(idx, current_layer->objectCount(), current_layer->objectCount());
    current_layer->addObject(new_object);
    endInsertRows();
  }
  
}

QStringList ACAge::getLayers() const
{
  QStringList layers;
  for(size_t i = 0; i < age->getNumPages(); i++) {
    plAgeInfo::PageEntry pe = age->getPage(i);
    layers.append(toQt(pe.fName));
  }
  return layers;
}

void ACAge::updatePageName(int id, const QString& new_name)
{
  hsTArray<plAgeInfo::PageEntry> temp_pages;
  for(size_t i = 0; i < age->getNumPages(); i++) {
    plAgeInfo::PageEntry pe = age->getPage(i);
    if(pe.fSeqSuffix == id)
      pe.fName = toPlasma(new_name);
  }
  age->clearPages();
  for(size_t i = 0; i < temp_pages.getSize(); i++)
    age->addPage(temp_pages[i]);
}

void ACAge::loadTextures(QGLContext *context)
{
  context->makeCurrent();
  plLocation texture_page;
  texture_page.setSeqPrefix(sequencePrefix());
  texture_page.setPageNum(-1);
  std::vector<plKey> textures = manager->getKeys(texture_page, kMipmap);
  GLuint *texids = new GLuint[textures.size()];
  glGenTextures(textures.size(), texids);
  for(unsigned int i = 0; i < textures.size(); i++) {
    glBindTexture(GL_TEXTURE_2D, texids[i]);
    plMipmap *mip = plPointer<plMipmap>(textures[i]);
    if(!mip)
      continue;
    texture_ids.insert(textures[i], texids[i]);
    glBindTexture(GL_TEXTURE_2D, texids[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    switch(mip->getCompressionType()) {
      case plBitmap::kJPEGCompression: {
        char *data = new char[mip->GetUncompressedSize(0)];
        mip->DecompressImage(0, data, mip->GetUncompressedSize(0));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mip->getLevelWidth(0), mip->getLevelHeight(0), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        break;
      }
      case plBitmap::kUncompressed: {
        GLenum data_type = GL_UNSIGNED_BYTE;
        GLenum data_format = GL_RGBA;
        switch(mip->getARGBType()) {
          case plBitmap::kRGB4444:
            data_type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
          case plBitmap::kRGB1555:
            data_type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
          case plBitmap::kInten8:
            data_format = GL_LUMINANCE;
            break;
          case plBitmap::kAInten88:
            data_format = GL_LUMINANCE_ALPHA;
            break;
          default:
            break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, data_format, mip->getLevelWidth(0), mip->getLevelHeight(0), 0, data_format, data_type, mip->getLevelData(0));
        break;
      }
      case plBitmap::kDirectXCompression: {
        GLenum dx_compression;
        switch(mip->getDXCompression()) {
          case plBitmap::kDXT1:
          case plBitmap::kDXT2:
            dx_compression = COMPRESSED_RGBA_S3TC_DXT1_EXT;
            break;
          case plBitmap::kDXT3:
          case plBitmap::kDXT4:
            dx_compression = COMPRESSED_RGBA_S3TC_DXT3_EXT;
            break;
          case plBitmap::kDXT5:
            dx_compression = COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
          default:
            break;
        }
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, dx_compression, mip->getLevelWidth(0), mip->getLevelHeight(0), 0, mip->getLevelSize(0), mip->getLevelData(0));
        break;
      }
      default:
        break;
    }
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  delete[] texids;
  context->doneCurrent();
}

void ACAge::exportAge(const QString &path)
{
  manager->WriteAge((path+QDir::separator()+name()+ascii(".age")).toLocal8Bit().data(), age);
  foreach(ACLayer *layer, layers) {
    plPageInfo *info = layer->page();
    manager->WritePage((path+QDir::separator()+toQt(info->getFilename(plasmaVersion()))).toLocal8Bit().data(), info);
  }
  plPageInfo *info = textures->page();
  manager->WritePage((path+QDir::separator()+toQt(info->getFilename(plasmaVersion()))).toLocal8Bit().data(), info);
  info = builtins->page();
  manager->WritePage((path+QDir::separator()+toQt(info->getFilename(plasmaVersion()))).toLocal8Bit().data(), info);
  plEncryptedStream stream(plasmaVersion());
  stream.open((path+QDir::separator()+name()+ascii(".fni")).toLocal8Bit().data(), fmCreate, plEncryptedStream::kEncAuto);
  stream.write(strlen(fni_string), fni_string);
  stream.close();
  stream.open((path+QDir::separator()+name()+ascii(".sum")).toLocal8Bit().data(), fmCreate, plEncryptedStream::kEncAuto);
  stream.write(8, "\0\0\0\0\0\0\0\0");
  stream.close();
}
