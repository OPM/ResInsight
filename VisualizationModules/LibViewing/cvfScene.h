//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#pragma once

#include "cvfObject.h"
#include "cvfCollection.h"
#include "cvfBoundingBox.h"

namespace cvf {

class Model;
class Part;
class Camera;
class PartRenderHintCollection;
class CullSettings;


//==================================================================================================
//
// Scene
// 
//==================================================================================================
class Scene : public Object
{
public:
    void            findVisibleParts(PartRenderHintCollection* visibleParts, const Camera& camera, const CullSettings& cullSettings, unsigned int enableMask);
    void            allParts(Collection<Part>* partCollection);

    void            addModel(Model* model);
    uint		    modelCount() const;
    Model*          model(uint index);
    const Model*    model(uint index) const;
    void            removeAllModels();
    void            removeModel(const Model* model);

    void            updateBoundingBoxesRecursive();
    BoundingBox     boundingBox() const;

protected:
    Collection<Model> m_models;
};
 
}
