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
#include "cvfModel.h"
#include "cvfOpenGLContext.h"
#include "cvfPart.h"
#include "cvfRayIntersectSpec.h"
#include "cvfRay.h"
#include "cvfTransform.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Model
/// \ingroup Viewing
///
/// Model is an abstract class that can deliver Part objects to a Scene.
/// 
/// The model does not enforce any internal structure. This is left to derived classes. The only 
/// requirement for a derived model is to be able to respond to the abstract interface.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
Model::Model()
:   m_partEnableMask(0xffffffff)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Model::~Model()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Model::setPartEnableMask(uint partEnableMask)
{
    m_partEnableMask = partEnableMask;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint Model::partEnableMask() const
{
    return m_partEnableMask;
}


//--------------------------------------------------------------------------------------------------
/// Delete or release all OpenGL resources held by the contained parts
/// 
/// \warning The OpenGL context in which the resources were created or a context that is being
///          shared must be current in the calling thread.
/// \warning Some resources are merely released (by unreferencing the object) so in order to assure 
///          that the actual OpenGL resources get deleted, you may have to do cleanup through the  
///          OpenGLResourceManager as afterwards (eg. deleteOrphanedManagedBufferObjects())
//--------------------------------------------------------------------------------------------------
void Model::deleteOrReleaseOpenGLResources(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    Collection<Part> allPartsColl;
    allParts(&allPartsColl);

    size_t numParts = allPartsColl.size();
    size_t partIdx;
    for (partIdx = 0; partIdx < numParts; partIdx++)
    {
        Part* part = allPartsColl.at(partIdx);
        CVF_ASSERT(part);

        part->deleteOrReleaseOpenGLResources(oglContext);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Model::setTransformTree(Transform* transform)
{
    m_tranformTree = transform;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Transform* Model::transformTree()
{
    return m_tranformTree.p();
}

} // namespace cvf

