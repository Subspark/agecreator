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

#include <QApplication>
#include <QIcon>
#include "ACMainWindow.h"
#include "ACUtil.h"

#include "ResManager/plResManager.h"

#if QT_VERSION < 0x040600
# include "qticonloader.h"
#endif

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
#if QT_VERSION >= 0x040600
  if(QIcon::themeName().isEmpty())
    QIcon::setThemeName(ascii("oxygen"));
#else
  if(QtIconLoader::iconTheme().isEmpty())
    QtIconLoader::setIconTheme(ascii("oxygen"));
#endif
  manager = new plResManager;
  ACMainWindow win;
  win.show();
  return app.exec();
}

