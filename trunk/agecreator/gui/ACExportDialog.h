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

#ifndef ACEXPORTDIALOG_H
#define ACEXPORTDIALOG_H

#include <QDialog>

#include "ui_ACExportDialog.h"

#include <Util/PlasmaVersions.h>

class ACExportDialog : public QDialog
{
  Q_OBJECT
public:
  ACExportDialog(QWidget *parent = 0);
  
  QString exportPath() const;
  PlasmaVer plasmaVersion() const;

public slots:
  void browse();

private:
  Ui_ACExportDialog ui;
};

#endif // ACEXPORTDIALOG_H
