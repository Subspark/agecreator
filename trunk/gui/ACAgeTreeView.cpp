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
#include "ACObject.h"
#include "ACObjectNameDelegate.h"
#include "ACUtil.h"

#include <QAction>

ACAgeTreeView::ACAgeTreeView(QWidget *parent)
  : QTreeView(parent)
{
  setDragEnabled(true);
  setDragDropMode(InternalMove);
  setSelectionMode(ExtendedSelection);
  setHeaderHidden(true);
  setEditTriggers(editTriggers() | SelectedClicked);
  ACObjectNameDelegate *delegate = new ACObjectNameDelegate(this);
  setItemDelegate(delegate);
}

void ACAgeTreeView::setModel(QAbstractItemModel *model)
{
  QItemSelectionModel *m = selectionModel();
  QTreeView::setModel(model);
  delete m;
  m = selectionModel();
}

void ACAgeTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  QTreeView::rowsInserted(parent, start, end);
  QModelIndex index = model()->index(start, 0, parent);
  setCurrentIndex(index);
  edit(index);
}

void ACAgeTreeView::contextMenuEvent(QContextMenuEvent *event)
{
  ACObject *obj = qobject_cast<ACObject*>(static_cast<QObject*>(currentIndex().internalPointer()));
  if(obj) {
    obj->contextMenu(event);
  }
}
