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

#ifndef AC2DWIDGET_H
#define AC2DWIDGET_H

#include <QGLWidget>

#include <Math/hsMatrix44.h>

class ACAge;
struct GLEWContextStruct;

class AC2dWidget : public QGLWidget
{
  Q_OBJECT
public:
  AC2dWidget(QWidget *parent);
  
  void setAge(ACAge *age);

protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  
//  void keyPressEvent(QKeyEvent *event);
//  void mousePressEvent(QMouseEvent *event);
//  void mouseMoveEvent(QMouseEvent *event);
  void contextMenuEvent(QContextMenuEvent *event);

private:
  double size_w, size_h;
  ACAge *current_age;
  GLEWContextStruct *glew_context;
};

#endif // AC2DWIDGET_H