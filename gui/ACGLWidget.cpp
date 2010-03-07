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

#include "ACGLWidget.h"

#include <QMouseEvent>

#include <cmath>

ACGLWidget::ACGLWidget(QWidget *parent)
  : QGLWidget(parent),
    cam_x(0.0f), cam_y(0.0f), cam_z(-6.0f),
    cam_h(0.0f), cam_v(0.0f)
{
  setFocusPolicy(Qt::ClickFocus);
  camera_matrix.setToIdentity();
}

void ACGLWidget::initializeGL()
{
  glClearColor(0.15f, 0.13f, 0.19f, 1.0f);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

void ACGLWidget::resizeGL(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //TODO make this match the URU perspective matrix
  gluPerspective(60.0, double(w)/double(h), 5.0, 500.0);
  glMatrixMode(GL_MODELVIEW);
}

void ACGLWidget::paintGL()
{
  glLoadIdentity();
  // setup base coordinate system
  //TODO: figure out Uru's coordinate system
  // For now: Z up, X right, Y forward
  glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
  
  // Adjust the camera angle
  glRotatef(cam_v, 1.0f, 0.0f, 0.0f);
  glRotatef(cam_h, 0.0f, 0.0f, 1.0f);

  // Position the camera
  glTranslatef(cam_x, cam_y, cam_z);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //TODO: call draw function for all world objects
}

void ACGLWidget::keyPressEvent(QKeyEvent *event)
{
  //TODO properly translate these based on the current angle
  QVector3D vec;
  if(event->key() == Qt::Key_Up)
    vec.setY(-2.0f);
  else if(event->key() == Qt::Key_Down)
    vec.setY(2.0f);
  else if(event->key() == Qt::Key_Right)
    vec.setX(-2.0f);
  else if(event->key() == Qt::Key_Left)
    vec.setX(2.0f);
  QWidget::keyPressEvent(event);
  QVector3D new_vec;
  new_vec = vec * camera_matrix;
  cam_x += new_vec.x();
  cam_y += new_vec.y();
  cam_z += new_vec.z();
  updateGL();
}

void ACGLWidget::mousePressEvent(QMouseEvent *event)
{
  last_pos = event->pos();
  event->accept();
}

void ACGLWidget::mouseMoveEvent(QMouseEvent *event)
{
  int dx = event->pos().x() - last_pos.x();
  int dy = event->pos().y() - last_pos.y();
  last_pos = event->pos();
  
  if(event->buttons() & Qt::LeftButton) {
    cam_h += float(dx);
    cam_v += float(dy);
  }
  event->accept();
  updateGL();
  camera_matrix.setToIdentity();
  camera_matrix.rotate(cam_v, 1.0f, 0.0f, 0.0f);
  camera_matrix.rotate(cam_h, 0.0f, 0.0f, 1.0f);
}
