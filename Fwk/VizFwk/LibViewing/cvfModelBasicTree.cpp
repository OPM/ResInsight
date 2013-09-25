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
#include "cvfModelBasicTree.h"
#include "cvfPart.h"
#include "cvfCamera.h"
#include "cvfCullSettings.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfRayIntersectSpec.h"
#include "cvfRay.h"
#include "cvfDrawableGeo.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::ModelBasicTreeNode
/// \ingroup Viewing
///
/// A node in a ModelBasicTree.
/// 
/// Contains a list of part at this level and a list of children.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
ModelBasicTreeNode::ModelBasicTreeNode()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
ModelBasicTreeNode::~ModelBasicTreeNode()
{
    uint numChildren = childCount();
    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);

        delete c;
    }
}


//--------------------------------------------------------------------------------------------------
/// Add a part to this node
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::addPart(Part* part)
{
    CVF_ASSERT(part);

    if (m_partList.isNull())
    {
        m_partList = new ModelBasicList;
    }

    m_partList->addPart(part);
}


//--------------------------------------------------------------------------------------------------
/// Get the part at the given index
//--------------------------------------------------------------------------------------------------
Part* ModelBasicTreeNode::part(uint index)
{
    CVF_ASSERT(m_partList.notNull());
    CVF_ASSERT(index < partCount());

    return m_partList->part(index);
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of parts in this node
//--------------------------------------------------------------------------------------------------
uint ModelBasicTreeNode::partCount() const
{
    if (m_partList.notNull())
    {
        return static_cast<uint>(m_partList->partCount());
    }
    else
    {
        return 0;
}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::removePart(Part* part)
{
    if (m_partList.notNull())
    {
        m_partList->removePart(part);
    }

    uint numChildren = childCount();
    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);
        
        c->removePart(part);
    }
}


//--------------------------------------------------------------------------------------------------
/// Add a child node to this node. The node can have any number of childern.
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::addChild(ModelBasicTreeNode* child)
{
    CVF_ASSERT(child);

    m_children.push_back(child);
}


//--------------------------------------------------------------------------------------------------
/// Returns the child node at the given index
//--------------------------------------------------------------------------------------------------
ModelBasicTreeNode* ModelBasicTreeNode::child(uint index)
{
    CVF_ASSERT(index < childCount());

    return m_children[index];
}


//--------------------------------------------------------------------------------------------------
/// Returns a const ptr to the child node at the given index
//--------------------------------------------------------------------------------------------------
const ModelBasicTreeNode* ModelBasicTreeNode::child(uint index) const
{
    CVF_ASSERT(index < childCount());

    return m_children[index];
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of children in this node
//--------------------------------------------------------------------------------------------------
uint ModelBasicTreeNode::childCount() const
{
    return static_cast<uint>(m_children.size());
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of children in this node
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::removeChild(ModelBasicTreeNode* child)
{
    std::vector<ModelBasicTreeNode*>::iterator it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end())
    {
        m_children.erase(it);
    }
}


//--------------------------------------------------------------------------------------------------
/// Update the bounding box of this node (recursive)
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::updateBoundingBoxesRecursive()
{
    m_boundingBox.reset();

    uint numChildren = childCount();
    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);

        c->updateBoundingBoxesRecursive();
        m_boundingBox.add(c->boundingBox());
    }

    if (m_partList.notNull())
    {
        m_partList->updateBoundingBoxesRecursive();
        m_boundingBox.add(m_partList->boundingBox());
    }
}


//--------------------------------------------------------------------------------------------------
/// Compute and append the visible parts in this node and all child nodes (recursive)
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::findVisibleParts(PartRenderHintCollection* visibleParts, const Camera& camera, const CullSettings& cullSettings, uint enableMask)
{
    if (!m_boundingBox.isValid())
    {
        return;
    }

    const size_t numChildren = m_children.size();
    const size_t numParts = m_partList.notNull() ? m_partList->partCount() : 0;

    if (numChildren > 0 || numParts > 1)
    {
        // View Frustum culling
        if (cullSettings.isViewFrustumCullingEnabled())
        {
            if (camera.frustum().isOutside(m_boundingBox))
            {
                // No parts on this level (or below) are visible. So just return
                return;
            }
        }

        // Pixel size culling
        if (cullSettings.isPixelSizeCullingEnabled())
        {
            double area = camera.computeProjectedBoundingSpherePixelArea(m_boundingBox.center(), m_boundingBox.radius());
            if (area < cullSettings.pixelSizeCullingAreaThreshold())
            {
                return;
            }
        }
    }

    size_t i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = m_children[i];
        CVF_ASSERT(c);

        c->findVisibleParts(visibleParts, camera, cullSettings, enableMask);
    }

    if (m_partList.notNull()) 
    {
        m_partList->findVisibleParts(visibleParts, camera, cullSettings, enableMask);
    }
}


//--------------------------------------------------------------------------------------------------
/// Get all parts in this node and all child nodes (recursive)
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::allParts(Collection<Part>* partCollection)
{
    uint numChildren = childCount();
    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);

        c->allParts(partCollection);
    }

    if (m_partList.notNull())
    {
        m_partList->allParts(partCollection);
    }
}


//--------------------------------------------------------------------------------------------------
/// Compute a ray intersection with all parts in this node and all child nodes (recursive)
//--------------------------------------------------------------------------------------------------
bool ModelBasicTreeNode::rayIntersect(const RayIntersectSpec& rayIntersectSpec, HitItemCollection* hitItemCollection) 
{
    bool hit = false;

    if (!m_boundingBox.isValid())
    {
        return false;
    }

    // Early reject: Check if bounding box is hit
    if (!rayIntersectSpec.ray()->boxIntersect(m_boundingBox))
    {
        return false;
    }

    uint numChildren = childCount();
    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);

        hit |= c->rayIntersect(rayIntersectSpec, hitItemCollection);
    }

    if (m_partList.notNull())
    {
        hit |= m_partList->rayIntersect(rayIntersectSpec, hitItemCollection);
    }

    return hit;
}


//--------------------------------------------------------------------------------------------------
/// Get the bounding box of this node
//--------------------------------------------------------------------------------------------------
const BoundingBox& ModelBasicTreeNode::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
/// Returns the first part found with the given ID. NULL if not found.
//--------------------------------------------------------------------------------------------------
Part* ModelBasicTreeNode::findPartByID(int id)
{
    if (m_partList.notNull())
    {
        uint numParts = partCount();
        uint i;
        for (i = 0; i < numParts; i++)
        {
            Part* p = m_partList->part(i);
            CVF_ASSERT(p);

            if (p->id() == id)
            {
                return p;
            }
        }
    }

    uint numChildren = childCount();
    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);

        Part* p = c->findPartByID(id);
        
        if (p)
        {
            return p;
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// Returns the first part found with the given name. NULL if not found.
//--------------------------------------------------------------------------------------------------
Part* ModelBasicTreeNode::findPartByName(String name)
{
    if (m_partList.notNull())
    {
        uint numParts = partCount();
        uint i;
        for (i = 0; i < numParts; i++)
        {
                Part* p = m_partList->part(i);
            CVF_ASSERT(p);

            if (p->name() == name)
            {
                return p;
            }
        }
    }

    uint numChildren = childCount();
    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);

        Part* p = c->findPartByName(name);

        if (p)
        {
            return p;
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicTreeNode::mergeParts(double maxExtent, uint minimumPrimitiveCount)
{
    uint numChildren = childCount();
    
    std::vector<ModelBasicTreeNode*> childrenToBeRemoved;

    uint i;
    for (i = 0; i < numChildren; i++)
    {
        ModelBasicTreeNode* c = child(i);
        CVF_ASSERT(c);

        c->mergeParts(maxExtent, minimumPrimitiveCount);

        // Devour any child node being too small. 
        // Any candidate child node for being devoured is a leaf node because we have already recursed downwards.
        // Check if this is a leaf node
        if (c->childCount() == 0)
        {
            size_t primCount = c->primitiveCount();
            if (primCount < minimumPrimitiveCount)
            {
                size_t numChildParts = 0;
                if (c->m_partList.notNull())
                {
                    numChildParts = c->m_partList->partCount();
                }
                
                size_t j;
                for (j = 0; j < numChildParts; j++)
                {
                    addPart(c->m_partList->part(j));
                }

                childrenToBeRemoved.push_back(c);
            }
        }
    }

    if (childrenToBeRemoved.size() > 0)
    {
        // Remove children from last to first index to make sure the indices are valid
        std::vector<ModelBasicTreeNode*>::iterator it;
        for (it = childrenToBeRemoved.begin(); it != childrenToBeRemoved.end(); it++)
        {
            removeChild(*it);
        }
    }


    // Merge too small parts in own list of parts. Any child node being devoured by the above code is 
    // in own list of parts. They are too small, but will be merged also by the statement below. 
    m_partList->mergeParts(maxExtent, minimumPrimitiveCount);
}


//--------------------------------------------------------------------------------------------------
/// Count primitives of part in this node - do not recurse
//--------------------------------------------------------------------------------------------------
size_t ModelBasicTreeNode::primitiveCount()
{
    size_t totalPrimitiveCount = 0;

    size_t j;
    for (j = 0; j < m_partList->partCount(); j++)
    {
        Part* part = m_partList->part(j);
        DrawableGeo* drawable = dynamic_cast<DrawableGeo*>(part->drawable());
        if (drawable)
        {
            totalPrimitiveCount += drawable->faceCount();
        }
    }

    return totalPrimitiveCount;
}



//==================================================================================================
///
/// \class cvf::ModelBasicTree
/// \ingroup Viewing
///
/// A simple model implementation using a tree.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
ModelBasicTree::ModelBasicTree()
{
    m_root = NULL;
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
ModelBasicTree::~ModelBasicTree()
{
    delete m_root;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ModelBasicTree::name() const
{
    return m_modelName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicTree::setName(const String& name)
{
    m_modelName = name;
}


//--------------------------------------------------------------------------------------------------
/// Set the root node of the tree.
///
/// If set to NULL, all current nodes will be deleted.
//--------------------------------------------------------------------------------------------------
void ModelBasicTree::setRoot(ModelBasicTreeNode* root)
{
    if (m_root != root)
    {
        delete m_root;
        m_root = root;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns a pointer to the root node.
//--------------------------------------------------------------------------------------------------
ModelBasicTreeNode* ModelBasicTree::root()
{
    return m_root;
}


//--------------------------------------------------------------------------------------------------
/// Returns a const pointer to the root node
//--------------------------------------------------------------------------------------------------
const ModelBasicTreeNode* ModelBasicTree::root() const
{
    return m_root;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicTree::removePart(Part* part)
{
    if (m_root)    
    {
        m_root->removePart(part);
    }
}


//--------------------------------------------------------------------------------------------------
/// Update the bounding boxes in the entire tree
//--------------------------------------------------------------------------------------------------
void ModelBasicTree::updateBoundingBoxesRecursive()
{
    m_boundingBox.reset();

    if (m_root)
    {
        m_root->updateBoundingBoxesRecursive();
        m_boundingBox.add(m_root->boundingBox());
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the bounding box of all parts in the tree
//--------------------------------------------------------------------------------------------------
BoundingBox ModelBasicTree::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicTree::findVisibleParts(PartRenderHintCollection* visibleParts, const Camera& camera, const CullSettings& cullSettings, uint enableMask)
{
    // Add in the model's enable mask before delegating to root node
    uint combinedEnableMask = (partEnableMask() & enableMask);

    if (m_root)
    {
        m_root->findVisibleParts(visibleParts, camera, cullSettings, combinedEnableMask);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicTree::allParts(Collection<Part>* partCollection)
{
    if (m_root)
    {
        m_root->allParts(partCollection);
    }
}


//--------------------------------------------------------------------------------------------------
/// Intersect all parts in the model with the given ray. 
//--------------------------------------------------------------------------------------------------
bool ModelBasicTree::rayIntersect(const RayIntersectSpec& rayIntersectSpec, HitItemCollection* hitItemCollection) 
{
    if (!m_root)
    {
        return false;
    }

    return m_root->rayIntersect(rayIntersectSpec, hitItemCollection);
}


//--------------------------------------------------------------------------------------------------
/// Returns the first part found with the given ID. NULL if not found.
//--------------------------------------------------------------------------------------------------
Part* ModelBasicTree::findPartByID(int id)
{
    if (!m_root)
    {
        return NULL;
    }

    return m_root->findPartByID(id);
}


//--------------------------------------------------------------------------------------------------
/// Returns the first part found with the given name. NULL if not found.
//--------------------------------------------------------------------------------------------------
Part* ModelBasicTree::findPartByName(String name)
{
    if (!m_root)
    {
        return NULL;
    }

    return m_root->findPartByName(name);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ModelBasicTree::mergeParts(double maxExtent, uint minimumPrimitiveCount)
{
    if (m_root)
    {
        m_root->mergeParts(maxExtent, minimumPrimitiveCount);
    }
}


} // namespace cvf

