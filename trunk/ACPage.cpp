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

#include "ACPage.h"
#include "ACAge.h"
#include "ACUtil.h"

#include <PRP/KeyedObject/plLocation.h>
#include <PRP/plPageInfo.h>
#include <ResManager/plResManager.h>

ACPage::ACPage(const QString &name, int page, ACAge *age)
  : QObject(age)
{
  info = new plPageInfo;
  plLocation l;
  l.setPageNum(page);
  l.setSeqPrefix(age->sequencePrefix());
  l.setVer(pvUniversal);
  info->setAge(toPlasma(age->name()));
  info->setPage(toPlasma(name));
  info->setLocation(l);
  manager->AddPage(info);
  // A Layer created in-memory is always dirty
  dirty = true;
}

ACPage::ACPage(const plLocation &loc, ACAge *age)
  : QObject(age)
{
  info = manager->FindPage(loc);
  dirty = false;
}

int ACPage::suffix() const
{
  return info->getLocation().getPageNum();
}

ACAge *ACPage::age() const
{
  return qobject_cast<ACAge*>(parent());
}

plPageInfo *ACPage::page() const
{
  return info;
}

const plLocation &ACPage::location() const
{
  return info->getLocation();
}

bool ACPage::isDirty() const
{
  return dirty;
}

void ACPage::makeDirty()
{
  dirty = true;
}

QString ACPage::name() const
{
  return QString(toQt(info->getPage()));
}

void ACPage::setName(const QString &name)
{
  info->setPage(toPlasma(name));
  age()->updatePageName(location().getPageNum(), name);
}