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

#include <PRP/KeyedObject/plKey.h>
#include <PRP/KeyedObject/plLocation.h>
#include <Util/plString.h>
#include <Sys/hsColor.h>
#include <QString>
#include <QColor>
#include <QMap>

#define ascii(str) QString::fromAscii(str)

#if QT_VERSION >= 0x040600
# define ACIcon(str) QIcon::fromTheme(ascii(str))
#else
# include "qticonloader.h"
# define ACIcon(str) QtIconLoader::icon(ascii(str), QIcon())
#endif

class plResManager;
struct GLEWContextStruct;
extern plResManager *manager;
extern plLocation virtual_loc;
extern QMap<plKey, unsigned int> texture_ids;
extern void* current_glew_context;

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

inline hsColorRGBA toPlasma(const QColor &qc)
{
  hsColorRGBA pc;
  pc.r = qc.redF();
  pc.g = qc.greenF();
  pc.b = qc.blueF();
  pc.a = qc.alphaF();
  return pc;
}

inline QColor toQt(const hsColorRGBA &pc)
{
  QColor qc;
  qc.setRedF(pc.r);
  qc.setGreenF(pc.g);
  qc.setBlueF(pc.b);
  qc.setAlphaF(pc.a);
  return qc;
}

template<class PLASMATYPE>
inline PLASMATYPE *plPointer(plKey key)
{
  return static_cast<PLASMATYPE*>(key->getObj());
}

inline GLEWContextStruct *glewGetContext() {
  return static_cast<GLEWContextStruct*>(current_glew_context);
}

#endif
