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

#include "ACTexture.h"
#include "ACUtil.h"

#include <PRP/Surface/plLayer.h>

ACTexture::ACTexture(const QString &name)
{
  layer = new plLayer;
  layer->init(toPlasma(name));
  manager->AddObject(plLocation(), layer);
  layer->setAmbient(hsColorRGBA());
  layer->setPreshade(hsColorRGBA(1.0f, 1.0f, 1.0f));
  layer->setRuntime(hsColorRGBA(1.0f, 1.0f, 1.0f));
  layer->setSpecular(hsColorRGBA(1.0f, 1.0f, 1.0f));
  layer->getState().fShadeFlags = hsGMatState::kShadeSpecular | hsGMatState::kShadeSpecularAlpha | hsGMatState::kShadeSpecularColor
                                | hsGMatState::kShadeSpecularHighlight;
  layer->setLODBias(-1);
}

plKey ACTexture::key() const
{
  return layer->getKey();
}

void ACTexture::setLocation(plLocation loc)
{
  manager->MoveKey(key(), loc);
}
