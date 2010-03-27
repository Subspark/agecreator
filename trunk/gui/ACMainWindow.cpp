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
#include <QSignalMapper>
#include <qmessagebox.h>

#include <ResManager/plResManager.h>

static const char *age_file_type = QT_TR_NOOP("Age Files (*.age)");

static const char *ac_license = QT_TR_NOOP(
"<html>AgeCreator: A level editor for Uru<br>"
"Copyright (C) 2010 Branan Riley<br>"
"<a href=\"http://code.google.com/p/agecreator\">http://code.google.com/p/agecreator</a><br>"
"<br>"
"This program is free software: you can redistribute it and/or modify"
"it under the terms of the GNU General Public License as published by"
"the Free Software Foundation, either version 3 of the License, or"
"(at your option) any later version.<br>"
"<br>"
"This program is distributed in the hope that it will be useful,"
"but WITHOUT ANY WARRANTY; without even the implied warranty of"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
"GNU General Public License for more details.<br>"
"<br>"
"You should have received a copy of the GNU General Public License"
"along with this program.  If not, see < a href=http://www.gnu.org/licenses/>http://www.gnu.org/licenses/</a></html>");

static const char *hsplasma_license = QT_TR_NOOP(
"<html>libHSPlasma: Open-source library for Plasma data types<br>"
"Written by Zrax<br>"
"<a href=\"http://code.google.com/p/libhsplasma\">http://code.google.com/p/libhsplasma</a><br>"
"<br>"
"This program is free software: you can redistribute it and/or modify"
"it under the terms of the GNU General Public License as published by"
"the Free Software Foundation, either version 3 of the License, or"
"(at your option) any later version.<br>"
"<br>"
"This program is distributed in the hope that it will be useful,"
"but WITHOUT ANY WARRANTY; without even the implied warranty of"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
"GNU General Public License for more details.<br>"
"<br>"
"You should have received a copy of the GNU General Public License"
"along with this program.  If not, see < a href=http://www.gnu.org/licenses/>http://www.gnu.org/licenses/</a></html>");

static const char *hector_credits = QT_TR_NOOP(
"<html>Hector was created by Aloys of the Guild of Writers<br>"
"<br>"
"Used with permission</html>");

ACMainWindow::ACMainWindow()
  : current_age(NULL)
{
  ui.setupUi(this);
#if QT_VERSION < 0x040601
  setCentralWidget(ui.mainView);
#endif
  ui.actionFileNew->setIcon(ACIcon("document-new"));
  ui.actionFileOpen->setIcon(ACIcon("document-open"));
  ui.actionFileSave->setIcon(ACIcon("document-save"));
  ui.actionFileSaveAs->setIcon(ACIcon("document-save-as"));
  ui.actionFileClose->setIcon(ACIcon("document-close"));
  ui.actionFileQuit->setIcon(ACIcon("application-exit"));

  ui.actionAgeImport->setIcon(ACIcon("document-import"));
  ui.actionAgeExport->setIcon(ACIcon("document-export"));
  ui.actionAgeProperties->setIcon(ACIcon("configure"));

  ui.actionAboutAgeCreator->setIcon(ACIcon("help-about"));
  ui.actionAboutHSPlasma->setIcon(ACIcon("help-about"));
  ui.actionAboutQt->setIcon(ACIcon("help-about"));
  ui.actionAboutHector->setIcon(ACIcon("help-about"));
  
  ui.buttonAddObject->setIcon(ACIcon("list-add"));
  ui.buttonDelObject->setIcon(ACIcon("list-remove"));
  ui.buttonAddLayer->setIcon(ACIcon("folder-new"));

  teardownAgeGui();
  setupAddActions();
}

ACMainWindow::~ACMainWindow()
{
  if(current_age)
    delete current_age;
}

void ACMainWindow::about()
{
  QMessageBox::about(this, tr("About Age Creator"), tr(ac_license));
}

void ACMainWindow::aboutHSPlasma()
{
  QMessageBox::about(this, tr("About libHSPlasma"), tr(hsplasma_license));
}

void ACMainWindow::aboutHector()
{
  QMessageBox::about(this, tr("About Hector"), tr(hector_credits));
}

void ACMainWindow::aboutQt()
{
  QMessageBox::aboutQt(this);
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
  PlasmaVer current_ver = manager->getVer();
  manager->setVer(dialog->plasmaVersion(), true);
  current_age->setPlasmaVersion(dialog->plasmaVersion());
  current_age->exportAge(dialog->exportPath());
  manager->setVer(current_ver);
  current_age->setPlasmaVersion(current_ver);
}

void ACMainWindow::setupAgeGui()
{
  ui.actionFileClose->setEnabled(true);
  ui.actionAgeExport->setEnabled(true);
  ui.actionAgeProperties->setEnabled(true);
  ui.buttonAddObject->setEnabled(true);
  ui.buttonDelObject->setEnabled(true);
  ui.buttonAddLayer->setEnabled(true);
  connect(mapper, SIGNAL(mapped(int)), current_age, SLOT(addObject(int)));
  connect(ui.buttonDelObject, SIGNAL(clicked()), current_age, SLOT(delObject()));
  connect(ui.buttonAddLayer, SIGNAL(clicked()), current_age, SLOT(addLayer()));
  connect(current_age, SIGNAL(rowsInserted(const QModelIndex&, int, int)), ui.preview3d, SLOT(updateGL()));
  connect(current_age, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), ui.preview3d, SLOT(updateGL()));
  connect(current_age, SIGNAL(rowsInserted(const QModelIndex&, int, int)), ui.mainView, SLOT(updateGL()));
  connect(current_age, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), ui.mainView, SLOT(updateGL()));
  ui.sceneTreeView->setModel(current_age);
  connect(ui.sceneTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentObjectChanged(QModelIndex)));
  current_age->setSelectionModel(ui.sceneTreeView->selectionModel());
  current_age->loadTextures(const_cast<QGLContext*>(ui.preview3d->context()));
  ui.preview3d->setAge(current_age);
  ui.mainView->setAge(current_age);
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

void ACMainWindow::setupAddActions()
{
  mapper = new QSignalMapper(this);
  QAction *action;
  QMenu *menu = new QMenu;
  action = menu->addAction(ACIcon("list-add-user"), tr("Spawn Point"));
  connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
  mapper->setMapping(action, ACAge::idSpawnPoint);
  action = menu->addAction(ACIcon("draw-rectangle"), tr("World Object"));
  connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
  mapper->setMapping(action, ACAge::idPhysDrawable);
  action = menu->addAction(ACIcon("draw-triangle"), tr("World Object (no physics)"));
  connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
  mapper->setMapping(action, ACAge::idDrawable);
  ui.buttonAddObject->setMenu(menu);
}
