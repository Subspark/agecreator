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
#include "AC2dWidget.h"
#include "ACObject.h"
#include "ACAge.h"
#include "ACUtil.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>

AC2dWidget::AC2dWidget(QWidget *parent)
  : QGLWidget(parent), current_age(0)
{
  glew_context = new GLEWContext;
}

void AC2dWidget::setAge(ACAge *age)
{
  current_age = age;
}

void AC2dWidget::initializeGL()
{
  current_glew_context = glew_context;
  glewInit();
  qglClearColor(palette().color(QPalette::Base));
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  if(!glewIsSupported("GL_ARB_vertex_shader")) {
    QMessageBox::critical(this, tr("OpenGL feature missing"), tr("GL_ARB_vertex_shader is not available. This is a required feature. Age Creator will now terminate"));
    QApplication::instance()->exit();
    return;
  }
  QFile shader_file(ascii(":/data/2Dplasma.vert"));
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

void AC2dWidget::resizeGL(int w, int h)
{
  current_glew_context = glew_context;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if(w > h) {
    size_w = 200.0*(double(w)/double(h));
    size_h = 200.0;
  } else {
    size_w = 200.0;
    size_h = 200.0*(double(h)/double(w));
  }
  glOrtho(-size_w, size_w, -size_h, size_h, -10000.0, 10000.0);
  glMatrixMode(GL_MODELVIEW);
}

void AC2dWidget::paintGL()
{
  current_glew_context = glew_context;
  glClear(GL_COLOR_BUFFER_BIT);
  int w = size_w;
  int h = size_h;
  unsigned int matrix_id = glGetUniformLocation(shader_program_id, "plasma_matrix");
  if(matrix_id != -1) {
    glUniformMatrix4fvARB(matrix_id, 1, GL_FALSE, hsMatrix44::Identity().glMatrix());
  }
  glBegin(GL_LINES);
  qglColor(palette().color(QPalette::Shadow));
  glVertex2f(-size_w, 0.0f);
  glVertex2f(size_w, 0.0f);
  glVertex2f(0.0f, -size_h);
  glVertex2f(0.0f, size_h);
  qglColor(palette().color(QPalette::Mid));
  for(int i = 10; i < h; i+=10) {
    glVertex2f(-size_w, float(i));
    glVertex2f(size_w, float(i));
    glVertex2f(-size_w, float(-i));
    glVertex2f(size_w, float(-i));
  }
  for(int i = 10; i < w; i+=10) {
    glVertex2f(float(i), -size_h);
    glVertex2f(float(i), size_h);
    glVertex2f(float(-i), -size_h);
    glVertex2f(float(-i), size_h);
  }
  glEnd();

  if(!current_age)
    return;

  qglColor(palette().color(QPalette::Text));
  for(int i = 0; i < current_age->layerCount(); i++)
    for(int j = 0; j < current_age->getLayer(i)->objectCount(); j++)
      current_age->getLayer(i)->getObject(j)->draw(ACObject::Draw2D, shader_program_id);
}

void AC2dWidget::contextMenuEvent(QContextMenuEvent* event)
{
  // Find the object under the cursor and popup the correct menu
}

