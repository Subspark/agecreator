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

#ifndef ACAGEWIZARD_H
#define ACAGEWIZARD_H

#include <QWizard>

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;

class ACAge;

class ACAgeWizard : public QWizard
{
  Q_OBJECT;
public:
  ACAgeWizard(QWidget *parent = 0);
  void accept();
  ACAge *getAge();
private:
  ACAge *age;
};

class IntroPage : public QWizardPage
{
  Q_OBJECT;
public:
  IntroPage(QWidget *parent = 0);
private:
  QLabel *label;
};

class AgeInfoPage : public QWizardPage
{
  Q_OBJECT;
public:
  AgeInfoPage(QWidget *parent = 0);
private:
  QLineEdit *age_name;
  QSpinBox *sequence_prefix;
  QDoubleSpinBox *day_length;
  QSpinBox *maximum_capacity;
};

class FogInfoPage : public QWizardPage
{
  Q_OBJECT;
public:
  FogInfoPage(QWidget *parent = 0);
private:
  QDoubleSpinBox *yon;
  QRadioButton *linear_fog;
  QRadioButton *exp_fog;
  QGroupBox *fog_type;
  QDoubleSpinBox *fog_start;
  QDoubleSpinBox *fog_end;
  QDoubleSpinBox *fog_density;
};

#endif // ACAGEWIZARD_H
