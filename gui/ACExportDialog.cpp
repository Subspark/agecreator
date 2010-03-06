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

#include "ACExportDialog.h"

#include <QFileDialog>

#include "Util/PlasmaVersions.h"

// This aray must be kept in sync with the textual descriptions in the UI file
PlasmaVer plasma_versions[] = {
  pvPots,
  pvLive,
  pvPrime,
  pvEoa,
};

ACExportDialog::ACExportDialog(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);
}

void ACExportDialog::browse()
{
  QString target = QFileDialog::getExistingDirectory(this, tr("Export Location"));
  ui.ExportDirectory->setText(target);
}

QString ACExportDialog::exportPath() const
{
  return ui.ExportDirectory->text();
}

PlasmaVer ACExportDialog::plasmaVersion() const
{
  return plasma_versions[ui.PlasmaVersion->currentIndex()];
}
