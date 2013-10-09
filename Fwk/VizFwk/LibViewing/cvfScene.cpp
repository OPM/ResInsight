//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
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
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfScene.h"
#include "cvfModel.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfTransform.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Scene
/// \ingroup Viewing
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Scene::Scene()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Scene::~Scene()
{
}


//--------------------------------------------------------------------------------------------------
/// Compute the visible parts of all models in the scene
//--------------------------------------------------------------------------------------------------
void Scene::findVisibleParts(PartRenderHintCollection* visibleParts, const Camera& camera, const CullSettings& cullSettings, uint enableMask)
{
    visibleParts->setCountZero();

    uint numModels = modelCount();
    for (uint i = 0; i < numModels; i++)
    {
        Model* model = m_models[i].p();
        if (model) 
        {
            if ((enableMask & model->partEnableMask()) != 0)
            {
                model->findVisibleParts(visibleParts, camera, cullSettings, enableMask);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Get all pars in all models in the scene
//--------------------------------------------------------------------------------------------------
void Scene::allParts(Collection<Part>* partCollection)
{
    uint numModels = modelCount();
    uint i;
    for (i = 0; i < numModels; i++)
    {
        Model* model = m_models.at(i);
        CVF_ASSERT(model);

        model->allParts(partCollection);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Scene::addModel(Model* model)
{
    CVF_ASSERT(model);
    m_models.push_back(model);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint Scene::modelCount() const
{   
    return static_cast<uint>(m_models.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Model* Scene::model(uint index)
{
    return m_models[index].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Model* Scene::model(uint index) const
{
    return m_models[index].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Scene::removeAllModels()
{
    m_models.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Scene::removeModel(const Model* model)
{
    CVF_ASSERT(model);
    m_models.erase(model);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Scene::updateBoundingBoxesRecursive()
{
    size_t numModels = m_models.size();
    size_t i;
    for (i = 0; i < numModels; i++)
    {
        Model* model = m_models.at(i);
        CVF_ASSERT(model);
        model->updateBoundingBoxesRecursive();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox Scene::boundingBox() const
{
    BoundingBox bb;

    size_t numModels = m_models.size();
    size_t i;
    for (i = 0; i < numModels; i++)
    {
        const Model* model = m_models.at(i);
        CVF_ASSERT(model);

        bb.add(model->boundingBox());
    }

    return bb;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Scene::setTransformTree(Transform* transform)
{
    m_tranformTree = transform;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Transform* Scene::transformTree()
{
    return m_tranformTree.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Scene::updateTransformTree(const Camera* camera)
{
    bool boundingBoxesNeedUpdate = false;

    if (m_tranformTree.notNull())
    {
        m_tranformTree->updateWorldTransform(camera);
        boundingBoxesNeedUpdate = true;
    }

    size_t numModels = m_models.size();
    for (size_t i = 0; i < numModels; i++)
    {
        Model* model = m_models.at(i);
        CVF_ASSERT(model);

        if (model->transformTree())
        {
            model->transformTree()->updateWorldTransform(camera);
        }

        if (model->transformTree() || boundingBoxesNeedUpdate)
        {
            model->updateBoundingBoxesRecursive();
        }
    }
}

} // namespace cvf
