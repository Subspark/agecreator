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

#include "ACAgeTreeView.h"
#include "ACUtil.h"

#include <QAction>

ACAgeTreeView::ACAgeTreeView(QWidget *parent)
  : QTreeView(parent)
{
  setDragEnabled(true);
  setDragDropMode(InternalMove);
  setDefaultDropAction(Qt::MoveAction);
  setSelectionMode(ExtendedSelection);
  setHeaderHidden(true);
}

void ACAgeTreeView::setModel(QAbstractItemModel *model)
{
  QItemSelectionModel *m = selectionModel();
  QTreeView::setModel(model);
  delete m;
  m = selectionModel();
  connect(m, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateActions(QModelIndex)));
}

void ACAgeTreeView::updateActions(const QModelIndex &index)
{
  //TODO: special actions for certain object types will be handled here.
}
