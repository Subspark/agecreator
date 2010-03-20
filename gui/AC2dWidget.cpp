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

#include "AC2dWidget.h"
#include "ACObject.h"
#include "ACAge.h"

AC2dWidget::AC2dWidget(QWidget *parent)
  : QGLWidget(parent), current_age(0)
{}

void AC2dWidget::setAge(ACAge *age)
{
  current_age = age;
}

void AC2dWidget::initializeGL()
{
  qglClearColor(palette().color(QPalette::Base));
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void AC2dWidget::resizeGL(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if(w > h) {
    size_w = 50.0*(double(w)/double(h));
    size_h = 50.0;
  } else {
    size_w = 50.0;
    size_h = 50.0*(double(h)/double(w));
  }
  glOrtho(-size_w, size_w, -size_h, size_h, -10000.0, 10000.0);
  glMatrixMode(GL_MODELVIEW);
}

void AC2dWidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT);
  int w = size_w;
  int h = size_h;
  glBegin(GL_LINES);
  qglColor(palette().color(QPalette::Shadow));
  glVertex2f(-size_w, 0.0f);
  glVertex2f(size_w, 0.0f);
  glVertex2f(0.0f, -size_h);
  glVertex2f(0.0f, size_h);
  qglColor(palette().color(QPalette::Mid));
  for(int i = 5; i < h; i+=5) {
    glVertex2f(-size_w, float(i));
    glVertex2f(size_w, float(i));
    glVertex2f(-size_w, float(-i));
    glVertex2f(size_w, float(-i));
  }
  for(int i = 5; i < w; i+=5) {
    glVertex2f(float(i), -size_h);
    glVertex2f(float(i), size_h);
    glVertex2f(float(-i), -size_h);
    glVertex2f(float(-i), size_h);
  }
  glEnd();

  if(!current_age)
    return;

  glColor3f(1.0f, 1.0f, 1.0f);
  for(int i = 0; i < current_age->layerCount(); i++)
    for(int j = 0; j < current_age->getLayer(i)->objectCount(); j++)
      current_age->getLayer(i)->getObject(j)->draw();
}

void AC2dWidget::contextMenuEvent(QContextMenuEvent* event)
{
  // Find the object under the cursor and popup the correct menu
}

