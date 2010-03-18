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

#include <ResManager/plResManager.h>
#include <ResManager/plAgeInfo.h>
#include <PRP/KeyedObject/plLocation.h>
#include <PRP/plPageInfo.h>
#include <PRP/plSceneNode.h>
#include <Stream/plEncryptedStream.h>

#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPointer>
#include <QRegExpValidator>
#include <QStringList>

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
  // An Age created in-memory is always dirty
  textures = new ACPage(ascii("Textures"), -1, this);
  builtins = new ACPage(ascii("BuiltIn"), -2, this);
  addLayer(ascii("mainRoom"));
  layers[0]->addObject(new ACSpawnPoint(ascii("LinkInPointDefault")));
  // a "NULL" page, where objects that have been created but not yet assigned a page are stored
  plPageInfo *info = new plPageInfo;
  info->setLocation(plLocation());
  manager->AddPage(info);
  plSceneNode *null_node = new plSceneNode;
  null_node->init(toPlasma(name));
  manager->AddObject(plLocation(), null_node);
  dirty = true;
}

ACAge::ACAge(const QString &filename, QObject *parent)
  : QAbstractItemModel(parent)
{
  age = manager->ReadAge(filename.toLocal8Bit().constData(), true);
  size_t num_pages = age->getNumPages();
  beginInsertRows(QModelIndex(), 0, num_pages-1);
  for(size_t i = 0; i < num_pages; i++) {
    plLocation loc;
    loc.setSeqPrefix(sequencePrefix());
    loc.setPageNum(age->getPage(i).fSeqSuffix);
    layers.append(new ACLayer(loc, this));
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
  // a "NULL" page, where objects that have been created but not yet assinged a page are stored
  plPageInfo *info = new plPageInfo;
  info->setLocation(plLocation());
  manager->AddPage(info);
  plSceneNode *null_node = new plSceneNode;
  null_node->init(toPlasma(name()));
  manager->AddObject(plLocation(), null_node);
  dirty = false;
}

ACAge::~ACAge()
{
  manager->DelAge(toPlasma(name()));
  manager->DelPage(plLocation());
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
  ACObject *new_object;
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
}
