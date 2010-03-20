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

#ifndef ACAGETREEVIEW_H
#define ACAGETREEVIEW_H

#include <QTreeView>

class ACAgeTreeView : public QTreeView
{
  Q_OBJECT
public:
  ACAgeTreeView(QWidget *parent = 0);
  void setModel(QAbstractItemModel *model);
protected:
  void rowsInserted(const QModelIndex &parent, int start, int end);
  void contextMenuEvent(QContextMenuEvent *event);
};

#endif //ACAGETREEVIEW_H
