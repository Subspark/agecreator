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
#include "ACAge.h"
#include "ACLayer.h"
#include "ACObject.h"

#include <QMouseEvent>

#include <cmath>

#include <Math/hsGeometry3.h>

ACGLWidget::ACGLWidget(QWidget *parent)
  : QGLWidget(parent),
    cam_x(0.0f), cam_y(0.0f), cam_z(-6.0f), // Start off at head height
    cam_h(0.0f), cam_v(0.0f),
    current_age(0)
{
  setFocusPolicy(Qt::ClickFocus);
  setMinimumSize(200, 150);
  camera_matrix = hsMatrix44::Identity();
}

void ACGLWidget::setAge(ACAge *age)
{
  current_age = age;
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
  gluPerspective(60.0, double(w)/double(h), 0.1f, 500.0);
  glMatrixMode(GL_MODELVIEW);
}

void ACGLWidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if(!current_age)
    return;
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

  for(int i = 0; i < current_age->layerCount(); i++)
    for(int j = 0; j < current_age->getLayer(i)->objectCount(); j++)
      current_age->getLayer(i)->getObject(j)->draw(ACObject::Draw3D);
}

void ACGLWidget::keyPressEvent(QKeyEvent *event)
{
  hsVector3 vec;
  if(event->key() == Qt::Key_Up)
    vec.Y = -2.0f;
  else if(event->key() == Qt::Key_Down)
    vec.Y = 2.0f;
  else if(event->key() == Qt::Key_Right)
    vec.X = -2.0f;
  else if(event->key() == Qt::Key_Left)
    vec.X = 2.0f;
  else if(event->key() == Qt::Key_PageUp)
    vec.Z = -2.0f;
  else if(event->key() == Qt::Key_PageDown)
    vec.Z = 2.0f;
  else
    QWidget::keyPressEvent(event);
  hsVector3 new_vec;
  new_vec = camera_matrix * vec;
  cam_x += new_vec.X;
  cam_y += new_vec.Y;
  cam_z += new_vec.Z;
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
  
  if(event->buttons() & Qt::MidButton) {
    cam_h += float(dx);
    cam_v += float(dy);
  }
  if(cam_h > 180.0f)
    cam_h -= 360.0f;
  else if(cam_h < -180.0f)
    cam_h += 360.0f;
  if(cam_v > 90.0f)
    cam_v = 90.0f;
  else if(cam_v < -90.0f)
    cam_v = -90.0f;
  event->accept();
  updateGL();
  camera_matrix = hsMatrix44::Identity();
  camera_matrix.rotate(hsMatrix44::kView, cam_h*3.14159/180.0f);
}
