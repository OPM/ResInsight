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
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfCamera.h"
#include "cvfCullSettings.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfDrawableGeo.h"
#include "cvfTransform.h"
#include "cvfRayIntersectSpec.h"
#include "cvfRay.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::ModelBasicList
/// \ingroup Viewing
///
/// A simple model implementation using a list of parts.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ModelBasicList::name() const
{
    return m_modelName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicList::setName(const String& name)
{
    m_modelName = name;
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
void ModelBasicList::addPart(Part* part)
{
    CVF_ASSERT(part);
    m_parts.push_back(part);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part* ModelBasicList::part(size_t index)
{
    CVF_ASSERT(index < m_parts.size());
    return m_parts.at(index);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t ModelBasicList::partCount() const
{
    return m_parts.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicList::removePart(Part* part)
{
    CVF_ASSERT(part);
    m_parts.erase(part);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicList::removeAllParts()
{
    m_parts.clear();
    m_boundingBox.reset();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicList::shrinkPartCount(size_t newPartCount)
{
    CVF_ASSERT(newPartCount <= m_parts.size());
    m_parts.resize(newPartCount);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicList::updateBoundingBoxesRecursive()
{
    m_boundingBox.reset();

    size_t i;
    for (i = 0; i < m_parts.size(); i++)
    {
        Part* part = m_parts.at(i);
        part->updateBoundingBox();
        m_boundingBox.add(part->boundingBox());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox ModelBasicList::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicList::findVisibleParts(PartRenderHintCollection* visibleParts, const Camera& camera, const CullSettings& cullSettings, uint enableMask)
{
    // Combination of model's and incoming enable mask
    uint combinedEnableMask = (partEnableMask() & enableMask);

    size_t numParts = m_parts.size();
    size_t i;
    for (i = 0; i < numParts; i++)
    {
        Part* part = m_parts[i].p();
        CVF_ASSERT(part);

        double projectedAreaPixels = -1;
        
        if (partVisible(part, &camera, &cullSettings, combinedEnableMask, &projectedAreaPixels))
        {
            if (projectedAreaPixels >= 0)
            {
                visibleParts->add(part, static_cast<float>(projectedAreaPixels), -1.0f);
            }
            else
            {
                visibleParts->add(part);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ModelBasicList::partVisible(cvf::Part* part, const Camera* camera, const CullSettings* cullSettings, uint enableMask, double* projectedAreaPixels)
{
    if (projectedAreaPixels)
    {
        *projectedAreaPixels = -1.0;
    }

    // Check if part is enabled (visible)
    if (((part->enableMask() & enableMask) > 0) && (part->drawable()))
    {
        // View Frustum Culling
        if (cullSettings && camera && cullSettings->isViewFrustumCullingEnabled())
        {
            const BoundingBox& bb = part->boundingBox();
            CVF_ASSERT(bb.isValid());

            if (camera->frustum().isOutside(bb))
            {
                return false;
            }
        }

        // Pixel size (small feature) culling
        // Cull based on objects projected screen area. Objects with area smaller than specified threshold will be culled
        if (cullSettings && cullSettings->isPixelSizeCullingEnabled())
        {
            const BoundingBox& bb = part->boundingBox();
            CVF_ASSERT(bb.isValid());

            *projectedAreaPixels = camera->computeProjectedBoundingSpherePixelArea(bb.center(), bb.radius());

            if (*projectedAreaPixels < cullSettings->pixelSizeCullingAreaThreshold())
            {
                return false;
            }
        }

        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicList::allParts(Collection<Part>* partCollection)
{
    size_t numParts = m_parts.size();
    size_t i;
    for (i = 0; i < numParts; i++)
    {
        partCollection->push_back(m_parts.at(i));
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ModelBasicList::rayIntersect(const RayIntersectSpec& rayIntersectSpec, HitItemCollection* hitItemCollection)
{
    // Example implementation
    cref<Ray> ray = rayIntersectSpec.ray();
    if (ray.isNull())
    {
        return false;
    }

    // Combination of model's and incoming enable mask
    unsigned int combinedEnableMask = (partEnableMask() & rayIntersectSpec.enableMask());
    bool anyPartsHit = false;

    Collection<Part> allPartsColl;
    allParts(&allPartsColl);
    size_t numParts = allPartsColl.size();
    size_t partIdx;
    for (partIdx = 0; partIdx < numParts; partIdx++)
    {
        Part* part = allPartsColl.at(partIdx);
        CVF_ASSERT(part);

        if (partVisible(part, rayIntersectSpec.camera(), rayIntersectSpec.cullSettings(), combinedEnableMask, NULL))
        {
            anyPartsHit |= part->rayIntersect(*ray, hitItemCollection);
        }
    }

    return anyPartsHit;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part* ModelBasicList::findPartByID(int64 id)
{
    size_t numParts = partCount();
    size_t i;
    for (i = 0; i < numParts; i++)
    {
        Part* p = m_parts.at(i);
        CVF_ASSERT(p);

        if (p->id() == id)
        {
            return p;
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part* ModelBasicList::findPartByName(String name)
{
    size_t numParts = partCount();
    size_t i;
    for (i = 0; i < numParts; i++)
    {
        Part* p = m_parts.at(i);
        CVF_ASSERT(p);

        if (p->name() == name)
        {
            return p;
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// Merge all parts with primitive count below given limit. A new part is created
/// containing merged drawableGeos. Vertices are transformed if a transformation matrix exists.
///
//--------------------------------------------------------------------------------------------------
void ModelBasicList::mergeParts(double maxExtent, uint minimumPrimitiveCount)
{
    // Gather all parts to be merged in a collection
    Collection<Part> candidates;

    size_t numParts = m_parts.size();
    size_t i;
    for (i = 0; i < numParts; i++)
    {
        Part* part = m_parts.at(i);

        // Update BB for all parts, as the BB is used to find parts that are close
        // to each other
        part->updateBoundingBox();

        DrawableGeo* drawable = dynamic_cast<DrawableGeo*>(part->drawable());
        if (drawable)
        {
            size_t primitiveCount = drawable->faceCount();
            if (primitiveCount < minimumPrimitiveCount)
            {
                candidates.push_back(part);
            }
        }
    }

    if (candidates.size() > 1)
    {
        IntArray newPartIDs;
        newPartIDs.resize(candidates.size());
        newPartIDs.setAll(-1);

        int newPartID = 0;

        size_t j;
        for (j = 0; j < candidates.size(); j++)
        {
            BoundingBox bbMerged;

            if (newPartIDs[j] == -1)
            {
                Part* part = candidates.at(j);
                bbMerged = part->boundingBox();

                newPartIDs.set(j, newPartID);
                if (bbMerged.isValid())
                {
                    size_t k;
                    for (k = j + 1; k < candidates.size(); k++)
                    {
                        Part* candidateBB = candidates.at(k);
                        BoundingBox bbCandidate = candidateBB->boundingBox();
                        bbCandidate.add(bbMerged);
                        if (bbCandidate.extent().length() < maxExtent)
                        {
                            bbMerged = bbCandidate;
                            newPartIDs[k] = newPartID;
                        }
                    }
                }

                newPartID++;
            }
        }

        // Gather all parts to be merged in a collection
        Collection<Part> partsToMerge;
        Part* currentCandidate = candidates.at(0);
        partsToMerge.push_back(currentCandidate);

        int currentPartID = 0;
        for (j = 1; j < newPartIDs.size(); j++)
        {
            if (newPartIDs.get(j) == currentPartID)
            {
                Part* candidate = candidates.at(j);
                partsToMerge.push_back(candidate);
            }

            if (newPartIDs.get(j) != currentPartID || j == newPartIDs.size() - 1)
            {
                if (partsToMerge.size() > 1)
                {
                    ref<Part> mergedPart = mergeAndAddPart(partsToMerge);
                    addPart(mergedPart.p());
                
                    // Remove merged parts from part list
                    size_t k;
                    for (k = 0; k < partsToMerge.size(); k++)
                    {
                        Part* part = partsToMerge.at(k);
                        removePart(part);
                    }
                }

                partsToMerge.clear();

                Part* candidate = candidates.at(j);
                partsToMerge.push_back(candidate);

                currentPartID = newPartIDs.get(j);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Merge all parts in given part collection. Create and insert merged part.
/// Incoming parts are removed from list.
//--------------------------------------------------------------------------------------------------
ref<Part> ModelBasicList::mergeAndAddPart(Collection<Part>& partCollection) const
{
    CVF_ASSERT(!partCollection.empty());

    // Build one new drawable from collection of small drawables. Compute normal of new drawable to be able to render.
    ref<DrawableGeo> mergedDrawable = new DrawableGeo;

    // Merge all drawables
    size_t j;
    for (j = 0; j < partCollection.size(); j++)
    {
        Part* part = partCollection.at(j);
        DrawableGeo* drawable = dynamic_cast<DrawableGeo*>(part->drawable());
        if (drawable)
        {
            Transform* transform = part->transform();
            if (transform)
            {
                Mat4d mat = transform->worldTransform();
                mergedDrawable->mergeInto(*drawable, &mat);
            }
            else
            {
                mergedDrawable->mergeInto(*drawable, NULL);
            }
        }
    }

    mergedDrawable->computeNormals();

    // First part defines properties to be used in the new merged part
    Part* firstPart = partCollection.at(0);
    CVF_ASSERT(firstPart);

    Effect* effect = firstPart->effect(0);

    ref<Part> mergedPart = new Part;
    mergedPart->setDrawable(mergedDrawable.p());
    mergedPart->setEffect(0, effect);
    mergedPart->updateBoundingBox();

    return mergedPart;
}


} // namespace cvf

