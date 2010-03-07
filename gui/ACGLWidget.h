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

#ifndef ACGLWIDGET_H
#define ACGLWIDGET_H

#include <QGLWidget>
#include <QMatrix4x4>

class ACGLWidget : public QGLWidget
{
  Q_OBJECT
public:
  ACGLWidget(QWidget *parent);

protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  
  void keyPressEvent(QKeyEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);

private:
  float cam_x, cam_y, cam_z; // position
  float cam_h, cam_v; // rotation
  
  QPoint last_pos;
  QMatrix4x4 camera_matrix;
};

#endif // ACGLWIDGET_H