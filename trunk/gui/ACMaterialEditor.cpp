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

#include "ACMaterialEditor.h"
#include "ACUtil.h"

#include <PRP/Surface/hsGMaterial.h>
#include <PRP/Surface/plLayer.h>
#include <ResManager/plResManager.h>

#include <QColorDialog>
#include <QInputDialog>
#include <QPointer>

const char *newmat = QT_TR_NOOP("New Material...");

ACMaterialEditor::ACMaterialEditor(plKey current, QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);
  connect(ui.materialCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMaterialLayers()));
  connect(ui.changeColor, SIGNAL(clicked(bool)), this, SLOT(updateLayerColor()));
  ui.addLayerButton->setIcon(ACIcon("list-add"));
  ui.delLayerButton->setIcon(ACIcon("list-remove"));
  texture_page = current->getLocation();
  materials = manager->getKeys(texture_page, kGMaterial);
  for(unsigned int i = 0; i < materials.size(); i++) {
    ui.materialCombo->addItem(toQt(materials[i]->getName()));
    if(materials[i] == current)
      ui.materialCombo->setCurrentIndex(i);
  }
  ui.materialCombo->addItem(tr(newmat));
}

plKey ACMaterialEditor::selectedMaterial()
{
  return materials[ui.materialCombo->currentIndex()];
}

void ACMaterialEditor::updateMaterialLayers()
{
  if(ui.materialCombo->currentIndex() == materials.size()) {
    QString matname = QInputDialog::getText(this, tr("Material Name"), tr("Enter new material name"));
    int idx = ui.materialCombo->currentIndex();
    hsGMaterial *mat = new hsGMaterial;
    mat->init(toPlasma(matname));
    manager->AddObject(texture_page, mat);
    plLayer *l = new plLayer;
    l->init(toPlasma(matname + ascii("layer0")));
    manager->AddObject(texture_page, l);
    l->setAmbient(hsColorRGBA());
    l->setPreshade(hsColorRGBA(1.0f, 1.0f, 1.0f));
    l->setRuntime(hsColorRGBA(1.0f, 1.0f, 1.0f));
    l->setSpecular(hsColorRGBA(1.0f, 1.0f, 1.0f));
    l->getState().fShadeFlags = hsGMatState::kShadeSpecular | hsGMatState::kShadeSpecularAlpha | hsGMatState::kShadeSpecularColor
                              | hsGMatState::kShadeSpecularHighlight;
    l->setLODBias(-1);
    mat->addLayer(l->getKey());
    materials.push_back(mat->getKey());
    ui.materialCombo->removeItem(idx);
    ui.materialCombo->addItem(toQt(materials[idx]->getName()));
    ui.materialCombo->addItem(tr(newmat));
    ui.materialCombo->setCurrentIndex(idx);
    return;
  }
  ui.layerList->clear();
  hsGMaterial *mat = static_cast<hsGMaterial*>(selectedMaterial()->getObj());
  int mat_name_size = (toQt(selectedMaterial()->getName()) + QChar::fromAscii('_')).length();
  for(size_t i = 0; i < mat->getNumLayers(); i++) {
    ui.layerList->addItem(toQt(mat->getLayer(i)->getName()).remove(0, mat_name_size));
  }
  ui.layerList->setCurrentRow(0);
}

void ACMaterialEditor::updateLayerColor()
{
  QPointer<QColorDialog> dialog = new QColorDialog(this);
  hsGMaterial *mat = static_cast<hsGMaterial*>(selectedMaterial()->getObj());
  plLayer *layer = static_cast<plLayer*>(mat->getLayer(ui.layerList->currentRow())->getObj());
  dialog->setCurrentColor(toQt(layer->getPreshade()));
  dialog->setOptions(QColorDialog::ShowAlphaChannel);
  dialog->exec();
  layer->setPreshade(toPlasma(dialog->currentColor()));
  layer->setRuntime(toPlasma(dialog->currentColor()));
  layer->setSpecular(toPlasma(dialog->currentColor()));
}
