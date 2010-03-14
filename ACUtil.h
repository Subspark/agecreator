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

#ifndef ACUTIL_H
#define ACUTIL_H

#include <Util/plString.h>
#include <QString>

#define ascii(str) QString::fromAscii(str)

#if QT_VERSION >= 0x040600
# define ACIcon(str) QIcon::fromTheme(ascii(str))
#else
# include "qticonloader.h"
# define ACIcon(str) QtIconLoader::icon(ascii(str), QIcon())
#endif

class plResManager;
extern plResManager *manager;

inline plString toPlasma(const QString &qs)
{
  plString ps;
  ps = qs.toAscii().data();
  return ps;
}

inline QString toQt(const plString &ps)
{
  return QString::fromAscii(ps.cstr());
}

#endif
