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

#include "ACAgeWizard.h"
#include "ACAge.h"
#include "ACUtil.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QRegExpValidator>
#include <QSpinBox>
#include <QVBoxLayout>

ACAgeWizard::ACAgeWizard(QWidget *parent)
  : QWizard(parent), age(0)
{
  addPage(new IntroPage);
  addPage(new AgeInfoPage);
//  addPage(new FogInfoPage);
  setWindowTitle(tr("Age Wizard"));
}

void ACAgeWizard::accept()
{
  age = new ACAge(field(ascii("name")).toString());
  age->setSequencePrefix(field(ascii("prefix")).toInt());
  // set age properties here
  QDialog::accept();
}

ACAge *ACAgeWizard::getAge()
{
  return age;
}

IntroPage::IntroPage(QWidget *parent)
  : QWizardPage(parent)
{
  setTitle(tr("Introduction"));
  label = new QLabel(tr("This wizard will guide you through the process of creating a new age.\n"
                        "Currently, not all of the settings are used properly, and none of them can "
                        "be reset. This will be fixed quickly"));
  label->setWordWrap(true);
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(label);
  setLayout(layout);
}

AgeInfoPage::AgeInfoPage(QWidget *parent)
  : QWizardPage(parent)
{
  setTitle(tr("Age Settings"));
  setSubTitle(tr("Specify basic information about the age"));
  age_name = new QLineEdit;
  QRegExpValidator *validator = new QRegExpValidator(this);
  validator->setRegExp(QRegExp(ascii("[a-zA-Z0-9]+")));
  age_name->setValidator(validator);
  sequence_prefix = new QSpinBox;
  sequence_prefix->setMinimum(0);
  sequence_prefix->setMaximum(65535);
  sequence_prefix->setValue(50000);
  day_length = new QDoubleSpinBox;
  day_length->setMinimum(0.0);
  // No idea what the maximum is. This is fairly arbitrary
  day_length->setMaximum(100.0);
  day_length->setValue(24.0);
  day_length->setSingleStep(0.5);
  maximum_capacity = new QSpinBox;
  maximum_capacity->setMinimum(1);
  maximum_capacity->setMaximum(100);
  maximum_capacity->setValue(10);
  registerField(ascii("name*"), age_name);
  registerField(ascii("prefix"), sequence_prefix);
  registerField(ascii("length"), day_length);
  registerField(ascii("capacity"), maximum_capacity);
  QFormLayout *layout = new QFormLayout;
  layout->addRow(tr("Age Name"), age_name);
  layout->addRow(tr("Sequence Prefix"), sequence_prefix);
  layout->addRow(tr("Day Length"), day_length);
  layout->addRow(tr("Maximum Capacity"), maximum_capacity);
  setLayout(layout);
}
