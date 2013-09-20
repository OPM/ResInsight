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
#include "cvfPart.h"
#include "cvfDrawable.h"
#include "cvfTransform.h"
#include "cvfEffect.h"
#include "cvfRay.h"
#include "cvfHitDetail.h"
#include "cvfHitItemCollection.h"


namespace cvf {



//==================================================================================================
///
/// \class cvf::Part
/// \ingroup Viewing
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part::Part()
:   m_name(),
    m_id(-1),
    m_enableMask(0xffffffff),
    m_priority(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part::Part(int64 id, String name)
:   m_name(name),
    m_id(id),
    m_enableMask(0xffffffff),
    m_priority(0)
{
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part::~Part()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> Part::shallowCopy() const
{
    ref<Part> newPart = new Part;

    newPart->m_name         = m_name;
    newPart->m_id           = m_id;
    newPart->m_enableMask   = m_enableMask;
    newPart->m_priority     = m_priority;
    newPart->m_transform    = m_transform;
    newPart->m_boundingBox  = m_boundingBox;
    newPart->m_sourceInfo   = m_sourceInfo;

    uint i;
    for (i = 0; i < MAX_NUM_LOD_LEVELS; i++)
    {
        newPart->m_drawables[i] = m_drawables[i];
        newPart->m_effects[i] = m_effects[i];
    }

    return newPart;
}


//--------------------------------------------------------------------------------------------------
/// Set a name to identify the part. 
///
/// It's up to the developer to decide if the name should be unique or not.
///
/// \sa name
//--------------------------------------------------------------------------------------------------
void Part::setName(const String& name)
{
    m_name = name;
}


//--------------------------------------------------------------------------------------------------
/// Set an ID to identify the part
///
/// It's up to the developer to decide if the value should be unique or not.
///
/// \sa id()
//--------------------------------------------------------------------------------------------------
void Part::setId(int64 id)
{
    m_id = id;
}


//--------------------------------------------------------------------------------------------------
/// Get the name of the part
///
/// Default is an empty String.
///
/// \sa setName
//--------------------------------------------------------------------------------------------------
const String& Part::name() const
{
    return m_name;
}


//--------------------------------------------------------------------------------------------------
/// Get the ID of the part
///
/// Default is -1.
///
/// \sa setId
//--------------------------------------------------------------------------------------------------
int64 Part::id() const
{
    return m_id;
}


//--------------------------------------------------------------------------------------------------
/// Sets the enable mask for this part
/// 
/// The enable mask determines if the part should be visible and/or rendered depending on the 
/// masks set in its containing Model and/or the Rendering where the part participates.
/// The default enable mask if 0xffffffff
/// 
/// \sa Model::setPartEnableMask()
//--------------------------------------------------------------------------------------------------
void Part::setEnableMask(uint mask)
{
    m_enableMask = mask;
}


//--------------------------------------------------------------------------------------------------
/// Gets the enable mask for this part
/// 
/// \sa setEnableMask()
//--------------------------------------------------------------------------------------------------
uint Part::enableMask() const
{
    return m_enableMask;
}


//--------------------------------------------------------------------------------------------------
/// Set render priority for this part
/// 
/// The render priority determines the order in which parts get rendered. Parts with lower priorities
/// get rendered first. The default priority is 0.
//--------------------------------------------------------------------------------------------------
void Part::setPriority(int priority)
{
    m_priority = priority;
}


//--------------------------------------------------------------------------------------------------
/// Get render priority
//--------------------------------------------------------------------------------------------------
int Part::priority() const
{
    return m_priority;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Part::setDrawable(Drawable* drawable)
{
    m_drawables[0] = drawable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Part::setDrawable(uint lodLevel, Drawable* drawable)
{
    CVF_ASSERT(lodLevel < MAX_NUM_LOD_LEVELS);
    m_drawables[lodLevel] = drawable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Drawable* Part::drawable()
{
    return m_drawables[0].p();   
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Drawable* Part::drawable() const
{
    return m_drawables[0].p();   
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Drawable* Part::drawable(uint lodLevel)
{
    CVF_ASSERT(lodLevel < MAX_NUM_LOD_LEVELS);
    return m_drawables[lodLevel].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Part::setEffect(Effect* effect)
{
    m_effects[0] = effect;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Part::setEffect(uint lodLevel, Effect* effect)
{
    CVF_ASSERT(lodLevel < MAX_NUM_LOD_LEVELS);
    m_effects[lodLevel] = effect;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Effect* Part::effect()
{
    return m_effects[0].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Effect* Part::effect(uint lodLevel)
{
    CVF_ASSERT(lodLevel < MAX_NUM_LOD_LEVELS);
    return m_effects[lodLevel].p();
}


//--------------------------------------------------------------------------------------------------
/// Returns the current transform (matrix) of the part. NULL if none.
//--------------------------------------------------------------------------------------------------
Transform* Part::transform() 
{
    return m_transform.p();
}


//--------------------------------------------------------------------------------------------------
/// Returns the current transform (matrix) of the part. NULL if none.
//--------------------------------------------------------------------------------------------------
const Transform* Part::transform() const
{
    return m_transform.p();
}


//--------------------------------------------------------------------------------------------------
/// Set the transform (matrix) to use for this part. NULL for none.
/// 
/// Note that you need to call updateBoundingBox() after setting the transform and after any changes
/// to the transform object.
//--------------------------------------------------------------------------------------------------
void Part::setTransform(Transform* transform)
{
    m_transform = transform;
}


//--------------------------------------------------------------------------------------------------
/// Updates Part's bounding box using LOD level 0. Will result in an invalid bounding box if no drawable exists.
//--------------------------------------------------------------------------------------------------
void Part::updateBoundingBox() 
{
    Drawable* drawable = m_drawables[0].p();
    if (drawable)
    {
        m_boundingBox = drawable->boundingBox();

        if (m_transform.notNull())
        {
            m_boundingBox.transform(m_transform->worldTransform());
        }
    }
    else
    {
        m_boundingBox.reset();
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the currently computed bounding box. May be invalid.
//--------------------------------------------------------------------------------------------------
const BoundingBox& Part::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
/// Intersect the drawable (at lod level 0) in the part with the specified ray
/// 
/// Returns true if any intersections were added to the hit item collection. 
/// Checks first against the bounding box of the part, so it must be up to date.
//--------------------------------------------------------------------------------------------------
bool Part::rayIntersect(const Ray& ray, HitItemCollection* hitItemCollection) const
{
    const Drawable* myDrawable = m_drawables[0].p();
    if (!myDrawable) 
    {
        return false;
    }

    // Early reject: Check if bounding box is hit
    if (!ray.boxIntersect(m_boundingBox))
    {
        return false;
    }

    // Inverse transform ray to be able to do picking in local coordinates
    Ray tranformedRay(ray);
    if (m_transform.notNull())
    {
        tranformedRay.transform(m_transform->worldTransform().getInverted());
    }

    Vec3d intersectionPoint;
    ref<HitDetail> hitDetail;
    if (myDrawable->rayIntersectCreateDetail(tranformedRay, &intersectionPoint, &hitDetail))
    {
        if (m_transform.notNull())
        {
            intersectionPoint.transformPoint(m_transform->worldTransform());
        }

        double dist = (intersectionPoint - ray.origin()).length();

        ref<HitItem> hitItem = new HitItem(dist, intersectionPoint);
        hitItem->setPart(this);
        hitItem->setDetail(hitDetail.p());

        hitItemCollection->add(hitItem.p());

        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Delete or release all OpenGL resources held by the part
/// 
/// \warning The OpenGL context in which the resources were created or a context that is being
///          shared must be current in the calling thread.
/// \warning Some resources are just released (by unreferencing the object) so in order to assure 
///          that the actual OpenGL resources get deleted, you may have to do cleanup through the  
///          OpenGLResourceManager as afterwards (eg. deleteOrphanedManagedBufferObjects())
//--------------------------------------------------------------------------------------------------
void Part::deleteOrReleaseOpenGLResources(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    uint lod;
    for (lod = 0; lod < MAX_NUM_LOD_LEVELS; lod++)
    {
        Effect* theEffect = effect(lod);
        if (theEffect)
        {
            theEffect->deleteOrReleaseOpenGLResources(oglContext);
        }

        Drawable* theDrawable = drawable(lod);
        if (theDrawable)
        {
            theDrawable->releaseBufferObjectsGPU();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Part::setSourceInfo(Object* sourceInfo)
{
    m_sourceInfo = sourceInfo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Object* Part::sourceInfo()
{
    return m_sourceInfo.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Object* Part::sourceInfo() const
{
    return m_sourceInfo.p();
}


} // namespace cvf

