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

#ifndef ACPAGE_H
#define ACPAGE_H

#include <QObject>

#include <PRP/KeyedObject/plKey.h>

class ACAge;
class plLocation;
class plPageInfo;

class ACPage : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(bool dirty READ isDirty)
public:
  ACPage(const QString &name, int page, ACAge *age=0);
  ACPage(const plLocation &loc, ACAge *age=0);

  int suffix() const;
  ACAge *age() const;
  plPageInfo *page() const;
  const plLocation &location() const;
  bool isDirty() const;
  void makeDirty();

  QString name() const;

public slots:
  virtual void setName(const QString &name);

private:
  plPageInfo *info;
  bool dirty;
};

#endif // ACPAGE_H