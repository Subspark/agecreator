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

#ifndef ACAGE_H
#define ACAGE_H

#include "ACLayer.h"

#include <QAbstractItemModel>
#include <QList>

#include "Util/PlasmaVersions.h"

class plAgeInfo;
class plPageInfo;
class plResManager;

class QItemSelectionModel;

class ACAge : public QAbstractItemModel
{
  Q_OBJECT
  Q_PROPERTY(QString name READ name)
  Q_PROPERTY(bool dirty READ isDirty)
  Q_PROPERTY(int sequencePrefix READ sequencePrefix)
  Q_PROPERTY(PlasmaVer plasmaVersion READ plasmaVersion WRITE setPlasmaVersion)
public:
  enum ACObjectType {
    idSpawnPoint,
    idPhysDrawable,
    idDrawable
  };

  ACAge(const QString &);
  ACAge(const QString &, QObject *parent);
  ~ACAge();

  QStringList getLayers() const;
  int layerCount() const;
  ACLayer *getLayer(int) const;
  ACLayer *getLayer(const QString&) const;
  void updatePageName(int id, const QString &new_name);
  
  void exportAge(const QString&);

  QString name() const;
  int sequencePrefix() const;
  PlasmaVer plasmaVersion() const;

  // All the remaining plAgeInfo and plResManager properties need to be here somehow

  // non-plasma attributes
  bool isDirty() const;

  // Model/View functions
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &index) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

public slots:
  void addLayer(QString name = QString());
  void addObject(int type);
  void setPlasmaVersion(PlasmaVer ver);
  void setName(const QString &);
  void setSequencePrefix(int);
  void setMaxCapacity(short);
  void setSelectionModel(QItemSelectionModel *model);
//  void delObject();

private:
  plAgeInfo *age;
  QList<ACLayer*> layers;
  ACPage *textures;
  ACPage *builtins;
  bool dirty;
  QItemSelectionModel *selection_model;
};

#endif // ACAGE_H
