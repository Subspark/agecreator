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

#include <QFile>
#include <QMessageBox>
#include <QMouseEvent>

#include <cmath>

#include <Math/hsGeometry3.h>
#include <PRP/Object/plCoordinateInterface.h>
#include <PRP/Object/plSceneObject.h>
#include <PRP/Surface/plMipmap.h>
#include <ResManager/plResManager.h>

ACGLWidget::ACGLWidget(QWidget *parent)
  : QGLWidget(parent),
    cam_x(0.0f), cam_y(0.0f), cam_z(0.0f), // Start off at origin
    cam_h(0.0f), cam_v(0.0f),
    current_age(0),
    vertex_shader_id(0), shader_program_id(0)
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
  std::vector<plLocation> locs = manager->getLocations();
  for(int i = 0; i < locs.size() && !found; i++) {
    std::vector<plKey> keys = manager->getKeys(locs[i], kSceneObject);
    for(int j = 0; j < keys.size(); j++) {
      if(keys[j]->getName() == plString("LinkInPointDefault")) {
        plSceneObject *obj = static_cast<plSceneObject*>(keys[j]->getObj());
        if(obj->getCoordInterface().Exists()) {
          plCoordinateInterface *coord = static_cast<plCoordinateInterface*>(obj->getCoordInterface()->getObj());
          hsMatrix44 mat = coord->getLocalToWorld();
          cam_x = -mat(0, 3);
          cam_y = -mat(1, 3);
          cam_z = -(mat(2, 3)+6.0f); // offset for the avatar height
          found = true;
          break;
        }
      }
    }
  }
  if(!found) {
    cam_x = cam_y = 0.0f;
    cam_z = -6.0f;
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
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
 glEnableClientState(GL_COLOR_ARRAY);
  if(!glewIsSupported("GL_ARB_vertex_shader")) {
    QMessageBox::warning(this, tr("OpenGL feature missing"), tr("GL_ARB_vertex_shader is not available. This is required for correct rendering in the 3D preview. 3D preview rendering will be incorrect on this computer"));
    return;
  }
  QFile shader_file(ascii(":/data/plasma.vert"));
  shader_file.open(QIODevice::ReadOnly);
  QByteArray shader_string = shader_file.readAll();
  vertex_shader_id = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  const char *shader_data = shader_string.data();
  glShaderSourceARB(vertex_shader_id, 1, &shader_data, NULL);
  glCompileShaderARB(vertex_shader_id);
  int success;
  glGetObjectParameterivARB(vertex_shader_id, GL_OBJECT_COMPILE_STATUS_ARB, &success);
  if(!success) {
    char infoLog[1024];
    glGetInfoLogARB(vertex_shader_id, 1024, NULL, infoLog);
    QMessageBox::warning(this, tr("OpenGL Error"), ascii(infoLog));
    glDeleteObjectARB(vertex_shader_id);
    vertex_shader_id = 0;
    return;
  }
  shader_program_id = glCreateProgramObjectARB();
  glAttachObjectARB(shader_program_id, vertex_shader_id);
  glLinkProgramARB(shader_program_id);
  glGetObjectParameterivARB(shader_program_id, GL_OBJECT_LINK_STATUS_ARB, &success);
  if(!success) {
    char infoLog[1024];
    glGetInfoLogARB(shader_program_id, 1024, NULL, infoLog);
    QMessageBox::warning(this, tr("OpenGL Error"), ascii(infoLog));
    glDeleteObjectARB(shader_program_id);
    glDeleteObjectARB(vertex_shader_id);
    shader_program_id = 0;
    vertex_shader_id = 0;
    return;
  }
  glValidateProgramARB(shader_program_id);
  glGetObjectParameterivARB(shader_program_id, GL_OBJECT_VALIDATE_STATUS_ARB, &success);
  if(!success) {
    char infoLog[1024];
    glGetInfoLogARB(shader_program_id, 1024, NULL, infoLog);
    QMessageBox::warning(this, tr("OpenGL Error"), ascii(infoLog));
    glDeleteObjectARB(shader_program_id);
    glDeleteObjectARB(vertex_shader_id);
    shader_program_id = 0;
    vertex_shader_id = 0;
    return;
  }
  glUseProgramObjectARB(shader_program_id);
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
  

  for(int i = 0; i < current_age->layerCount(); i++)
    for(int j = 0; j < current_age->getLayer(i)->objectCount(); j++)
      current_age->getLayer(i)->getObject(j)->draw(ACObject::Draw3D, shader_program_id);
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
