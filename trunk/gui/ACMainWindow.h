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

#ifndef ACMAINWINDOW_H
#define ACMAINWINDOW_H

#include <QMainWindow>

#include "ui_ACMainWindow.h"

class ACAge;
class QSignalMapper;

class ACMainWindow : public QMainWindow
{
  Q_OBJECT
public:
  ACMainWindow();
  ~ACMainWindow();

private slots:
  void about();
  void aboutQt();
  void aboutHSPlasma();
  void closeAge();
  void newAge();
  void importAge();
  void exportAge();

private:
  QSignalMapper *mapper;
  ACAge* current_age;
  Ui_ACMainWindow ui;
  void setupAgeGui();
  void teardownAgeGui();
  void setupAddActions();
};

#endif // ACMAINWINDOW_H
