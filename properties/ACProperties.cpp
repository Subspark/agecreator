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

#include "ACProperties.h"
#include "ACObject.h"
#include "ACUtil.h"

#include "ACObjectNameProperty.h"
#include "ACObjectName.h"
//#include "AChsVector3Property.h"

void registerTypes()
{
  static bool registered = false;
  if(!registered)
  {
    qRegisterMetaType<hsVector3>("hsVector3");
    qRegisterMetaType<ACObjectName>("ACObjectName");
    registered = true;
  }
}

Property *createCustomProperty(const QString &name, QObject *propertyObject, Property *parent)
{
  int type = propertyObject->property(qPrintable(name)).userType();
  if(type == QMetaType::type("ACObjectName"))
    return new ACObjectNameProperty(name, propertyObject, parent);
  else
    return new Property(name, propertyObject, parent);
}
