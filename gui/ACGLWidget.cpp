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
#include "glew.h" // included *before* Qt can include the GL headers
#include "ACGLWidget.h"
#include "ACAge.h"
#include "ACLayer.h"
#include "ACObject.h"
#include "ACUtil.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QMouseEvent>

#include <cmath>

#include <Math/hsGeometry3.h>
#include <PRP/Geometry/plDrawableSpans.h>
#include <PRP/Object/plCoordinateInterface.h>
#include <PRP/Object/plSceneObject.h>
#include <PRP/Surface/plMipmap.h>
#include <ResManager/plResManager.h>

ACGLWidget::ACGLWidget(QWidget *parent)
  : QGLWidget(parent),
    cam_x(0.0f), cam_y(0.0f), cam_z(0.0f), // Start off at origin
    cam_h(0.0f), cam_v(0.0f),
    current_age(0),
    vertex_shader_id(0)
{
  setFocusPolicy(Qt::ClickFocus);
  setMinimumSize(200, 150);
  camera_matrix = hsMatrix44::Identity();
  glew_context = new GLEWContext;
}

void ACGLWidget::setAge(ACAge *age)
{
  current_age = age;
  cam_h = cam_v = 0.0f;
  bool found = false;
  plKey fallback_spawn;
  std::vector<plLocation> locs = manager->getLocations();
  for(int i = 0; i < locs.size() && !found; i++) {
    std::vector<plKey> keys = manager->getKeys(locs[i], kSceneObject);
    for(int j = 0; j < keys.size(); j++) {
      if(keys[j]->getName() == plString("LinkInPointDefault")) {
        plSceneObject *obj = plPointer<plSceneObject>(keys[j]);
        if(obj->getCoordInterface().Exists()) {
          plCoordinateInterface *coord = plPointer<plCoordinateInterface>(obj->getCoordInterface());
          hsMatrix44 mat = coord->getLocalToWorld();
          cam_x = -mat(0, 3);
          cam_y = -mat(1, 3);
          cam_z = -(mat(2, 3)+6.0f); // offset for the avatar height
          found = true;
          break;
        }
      } else if(plPointer<plSceneObject>(keys[j])->getModifiers().getSize() && plPointer<plSceneObject>(keys[j])->getModifiers()[0]->getType() == kSpawnModifier)
        fallback_spawn = keys[j];
    }
  }
  if(!found) {
    if(fallback_spawn.Exists()) {
      plSceneObject *obj = plPointer<plSceneObject>(fallback_spawn);
      plCoordinateInterface *coord = plPointer<plCoordinateInterface>(obj->getCoordInterface());
      hsMatrix44 mat = coord->getLocalToWorld();
      cam_x = -mat(0, 3);
      cam_y = -mat(1, 3);
      cam_z = -(mat(2, 3)+6.0f); // offset for the avatar height
    } else {
      cam_x = cam_y = 0.0f;
      cam_z = -6.0f;
    }
  }
}

void ACGLWidget::initializeGL()
{
  current_glew_context = glew_context;
  glewInit();
  glClearColor(0.15f, 0.13f, 0.19f, 1.0f);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  if(!glewIsSupported("GL_ARB_vertex_program")) {
    QMessageBox::critical(this, tr("OpenGL feature missing"), tr("GL_ARB_vertex_program is not available. This is a required feature. Age Creator will now terminate"));
    QApplication::instance()->exit();
    return;
  }
  QFile shader_file(ascii(":/data/plasma.vert"));
  shader_file.open(QIODevice::ReadOnly);
  QByteArray shader_string = shader_file.readAll();
  glGenProgramsARB(1, &vertex_shader_id);
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertex_shader_id);
  const char *shader_data = shader_string.data();
  glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, shader_string.size(), shader_data);
  int error_pos;
  glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
  if(error_pos != -1) {
    QMessageBox::critical(this, tr("ARB Shader Error"), ascii((const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB)));
    QApplication::instance()->exit();
    return;
  }
  glEnable(GL_VERTEX_PROGRAM_ARB);
}

void ACGLWidget::resizeGL(int w, int h)
{
  current_glew_context = glew_context;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //TODO make this match the URU perspective matrix
  gluPerspective(60.0, double(w)/double(h), 0.1f, 500.0);
  glMatrixMode(GL_MODELVIEW);
}

void ACGLWidget::paintGL()
{
  current_glew_context = glew_context;
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
  
  // update all the world transform matrices. This should theoeretically be done only when stuff changes, but for now this is fine
  // TODO: Move this code someplace reasonable
  
  // Get all the different render levels
  QSet<unsigned int> rlevels;
  std::vector<plLocation> locs = manager->getLocations();
  for(int i = 0; i < locs.size(); i++) {
    std::vector<plKey> keys = manager->getKeys(locs[i], kDrawableSpans);
    for(int j = 0; j < keys.size(); j++)
      if(keys[j].Exists()) {
        plDrawableSpans *spans = plPointer<plDrawableSpans>(keys[j]);
        rlevels.insert(spans->getRenderLevel());
      }
  }
    
  
  foreach(unsigned int rl, rlevels)
    for(int i = 0; i < current_age->layerCount(); i++)
      for(int j = 0; j < current_age->getLayer(i)->objectCount(); j++)
        current_age->getLayer(i)->getObject(j)->draw(ACObject::Draw3D, rl);
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
