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

#include "ACMainWindow.h"
#include "ACAge.h"
#include "ACAgeWizard.h"
#include "ACExportDialog.h"
#include "ACGLWidget.h"
#include "ACUtil.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QPointer>

static const char *age_file_type = QT_TR_NOOP("Age Files (*.age)");

ACMainWindow::ACMainWindow()
  : current_age(NULL)
{
  ui.setupUi(this);
  ui.actionFileNew->setIcon(ACIcon("document-new"));
  ui.actionFileOpen->setIcon(ACIcon("document-open"));
  ui.actionFileSave->setIcon(ACIcon("document-save"));
  ui.actionFileSaveAs->setIcon(ACIcon("document-save-as"));
  ui.actionFileClose->setIcon(ACIcon("document-close"));
  ui.actionFileQuit->setIcon(ACIcon("application-exit"));

  ui.actionAgeImport->setIcon(ACIcon("document-import"));
  ui.actionAgeExport->setIcon(ACIcon("document-export"));
  ui.actionAgeProperties->setIcon(ACIcon("configure"));
  
  ui.buttonAddObject->setIcon(ACIcon("list-add"));
  ui.buttonDelObject->setIcon(ACIcon("list-remove"));
  ui.buttonAddLayer->setIcon(ACIcon("folder-new"));
  
  // This function sets things up for when there is no age loaded
  teardownAgeGui();
}

ACMainWindow::~ACMainWindow()
{
  if(current_age)
    delete current_age;
}

void ACMainWindow::closeAge()
{
  if(current_age) {
    if(current_age->isDirty()) {
      //TODO: Ask to save
    }
    delete current_age;
    current_age = 0;
    teardownAgeGui();
  }
}

void ACMainWindow::newAge()
{
  QPointer<ACAgeWizard> wizard = new ACAgeWizard(this);
  if(wizard->exec() == QDialog::Rejected)
    return;
  closeAge();
  current_age = wizard->getAge();
  current_age->setParent(this);
  setupAgeGui();
}

void ACMainWindow::importAge()
{
  QString age_file = QFileDialog::getOpenFileName(this, tr("Import Age"), QString(), tr(age_file_type));
  if(!age_file.isEmpty()) {
    closeAge();
    current_age = new ACAge(age_file, this);
    setupAgeGui();
  }
}

void ACMainWindow::exportAge()
{
  QPointer<ACExportDialog> dialog = new ACExportDialog(this);
  if(dialog->exec() == QDialog::Rejected)
    return;
  current_age->setPlasmaVersion(dialog->plasmaVersion());
  current_age->exportAge(dialog->exportPath());
}

void ACMainWindow::setupAgeGui()
{
  ui.actionFileClose->setEnabled(true);
  ui.actionAgeExport->setEnabled(true);
  ui.actionAgeProperties->setEnabled(true);
  ui.buttonAddObject->setEnabled(true);
  ui.buttonDelObject->setEnabled(true);
  ui.buttonAddLayer->setEnabled(true);
  connect(ui.buttonAddObject, SIGNAL(clicked()), current_age, SLOT(addObject()));
  connect(ui.buttonDelObject, SIGNAL(clicked()), current_age, SLOT(delObject()));
  connect(ui.buttonAddLayer, SIGNAL(clicked()), current_age, SLOT(addLayer()));
  connect(current_age, SIGNAL(rowsInserted()), ui.preview3d, SLOT(updateGL()));
  connect(current_age, SIGNAL(rowsRemoved()), ui.preview3d, SLOT(updateGL()));
  ui.sceneTreeView->setModel(current_age);
  current_age->setSelectionModel(ui.sceneTreeView->selectionModel());
  ui.preview3d->setAge(current_age);
  setWindowTitle(tr("Age Creator - %1").arg(current_age->name()));
}

void ACMainWindow::teardownAgeGui()
{
  ui.actionFileClose->setEnabled(false);
  ui.actionAgeExport->setEnabled(false);
  ui.actionAgeProperties->setEnabled(false);
  ui.buttonAddObject->setEnabled(false);
  ui.buttonDelObject->setEnabled(false);
  ui.buttonAddLayer->setEnabled(false);
  ui.preview3d->setAge(0);
}
