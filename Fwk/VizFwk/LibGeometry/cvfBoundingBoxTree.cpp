//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cvfBoundingBoxTree.h"

#include "cvfLibCore.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfBoundingBox.h"

#include <cmath>
#include <deque>
#include <limits>
#include <thread>

#define ALLOCATION_CHUNK_SIZE 10000

namespace cvf {

//==================================================================================================
///
/// \class cvf::BoundingBoxTree
/// \ingroup Geometry
///
/// An axis-aligned bounding-box search tree
///
/// This class can be used to quickly do an approximate search for geometry using a bounding box. 
/// The geometry entities to search for must be enclosed in bounding boxes which are inserted 
/// together with an ID. 
///
/// When intersecting, the ID's or the indexes of the intersected boundingboxes are returned, 
/// depending on whether explicit id's where supplied.
//==================================================================================================

    enum NodeType
    {
        AB_UNDEFINED,
        AB_LEAF,
        AB_INTERNAL
    };



    //==================================================================================================
    //
    // 
    //
    //==================================================================================================
    class AABBTreeNode
    {
    public:
        AABBTreeNode();
        virtual ~AABBTreeNode() {}

        const cvf::BoundingBox& boundingBox() const;
        void                    setBoundingBox(const cvf::BoundingBox bb);
        const cvf::Vec3d&       boundingBoxCenter() const;

        NodeType                type() const;

    protected:
        NodeType                m_type;

    private:
        cvf::BoundingBox        m_boundingBox;
        cvf::Vec3d              m_boundingBoxCenter;
    };


    //=================================================================================================================================
    /// Internal node in the AABB tree. It have at least two child nodes, but can have many more (grand-children etc.). Both the Left
    /// and right tree node pointes must exist (or else it should be a leaf node).
    //=================================================================================================================================
    class AABBTreeNodeInternal : public AABBTreeNode
    {
    public:
        AABBTreeNodeInternal();
        AABBTreeNodeInternal(AABBTreeNode* left, AABBTreeNode* right);

        void                setLeft(AABBTreeNode* left);
        AABBTreeNode*       left();
        const AABBTreeNode* left() const;

        void                setRight(AABBTreeNode* right);
        AABBTreeNode*       right();
        const AABBTreeNode* right() const;

    private:
        AABBTreeNode* m_pLeft;		///< Left child of this internal node in the binary AABB tree
        AABBTreeNode* m_pRight;		///< Right child of this internal node in the binary AABB tree
    };


    //=================================================================================================================================
    /// Standard leaf node in the AABB tree. The leaf node contains only an index, and the interpretation of this index is depending on
    /// the type of AABB tree using the node.
    //=================================================================================================================================
    class AABBTreeNodeLeaf : public AABBTreeNode
    {
    public:
        AABBTreeNodeLeaf();

        std::vector<int> ids() const;
        void setIds(const std::vector<int>& ids);
        
    private:
        std::vector<int> m_ids; ///< An list of IDs of the leaf node. The interpretation of these values depends on which tree the node is in.
    };

    //=================================================================================================================================
    //
    /// An axis oriented bounding box tree. This is an abstract base class for AABB trees used for searching and intersection testing.
    /// All classes deriving from this must implement the CreateLeaves() method. This method creates all the leaf nodes in the AABB
    /// tree and is called as a part of the BuildTree() process. 
    ///
    /// This base class handles the building of the tree with all the internal nodes. It also handles basic intersection and searching, 
    /// but the IntersectLeafLeaf() and IntersectBoxLeaf() methods should be implemented in any tree classes used for intersection 
    /// testing. 
    ///
    /// The Find() method only searches for matches in bounding boxes, and must be reimplemented in the decendant classes for 
    /// accurate testing of leaf nodes when the leaf node properties are known.
    //=================================================================================================================================
    class AABBTree : public cvf::Object
    {
    public:
        AABBTree();
        virtual ~AABBTree();


        virtual void free();

        bool    buildTree();

        size_t  treeSize() const;
        size_t  leavesCount() const;
        bool    boundingBox(cvf::BoundingBox* pBox) const;

        std::string treeInfo() const;

    protected:
        virtual cvf::BoundingBox createLeaves() = 0;
        virtual size_t treeNodeSize(const AABBTreeNode* pNode) const;

        void   freeThis();

        size_t treeSize(const AABBTreeNode* pNode) const;
        size_t treeHeight(const AABBTreeNode* pNode, size_t iLevel, size_t* piMin, size_t* piMax) const;
        cvf::BoundingBox leafBoundingBox(size_t iStartIdx, size_t iEndIdx) const;
        bool buildTree(AABBTreeNodeInternal* pNode, size_t iFromIdx, size_t iToIdx, int currentLevel, int maxLevel = -1);

        // Queries
        bool intersect(const AABBTreeNode* pA, const AABBTreeNode* pB) const;

        AABBTreeNodeInternal*  createNode();
        AABBTreeNodeLeaf*      createOrAssignLeaf(size_t leafIndex, const std::vector<int>& bbIds);

    private:
        static void deleteInternalNodesBottomUp(AABBTreeNode* node);

    protected:
        struct InternalNodeAndRange
        {
            AABBTreeNodeInternal* node;
            size_t fromIdx;
            size_t toIdx;

            InternalNodeAndRange(AABBTreeNodeInternal* node, size_t fromIdx, size_t toIdx)
                : node(node)
                , fromIdx(fromIdx)
                , toIdx(toIdx)
            {}
        };

        std::vector<AABBTreeNodeLeaf*> m_ppLeaves;
        size_t m_iNumLeaves;

        AABBTreeNode* m_pRoot;

        std::vector<AABBTreeNodeLeaf>      m_leafPool;
        size_t                             m_nextNodeIndex;

        std::vector<InternalNodeAndRange>  m_previousLevelNodes;
    };

    class BoundingBoxTreeImpl : public AABBTree
    {
        BoundingBoxTreeImpl() {}
        
        void buildTree(const std::vector<cvf::BoundingBox>& boundingBoxes, const std::vector<std::vector<int>>& ids);

    private:
        friend class BoundingBoxTree;

        cvf::BoundingBox createLeaves();
        void findIntersections(const cvf::BoundingBox& bb, std::vector<size_t>& bbIds) const;

        void findIntersections(const cvf::BoundingBox& bb, const AABBTreeNode* node, std::vector<size_t>& ids) const;

        std::vector<cvf::BoundingBox> m_validBoundingBoxes;
        std::vector<std::vector<int>> m_validOptionalBoundingBoxIds;
    };
}



namespace cvf {

using cvf::ref;





int largestComponent(const cvf::Vec3d v)
{
    double maxLength = v.x();
    int idx = 0;

    if (v.y() > maxLength)
    {
        maxLength = v.y();
        idx = 1;
    }

    if (v.z() > maxLength)
    {
        maxLength = v.z();
        idx = 2;
    }

    return idx;
}


double largestComponent(const cvf::Vec3d v, cvf::uint* largestIndex)
{
    double length = v.x();
    cvf::uint idx = 0;

    if (v.y() > length)
    {
        length = v.y();
        idx = 1;
    }

    if (v.z() > length)
    {
        length = v.z();
        idx = 2;
    }

    if (largestIndex) *largestIndex = idx;

    return length;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNode::AABBTreeNode()
{
    m_type = AB_UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::BoundingBox& AABBTreeNode::boundingBox() const
{
    return m_boundingBox;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
NodeType AABBTreeNode::type() const
{
    return m_type;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTreeNode::setBoundingBox(const cvf::BoundingBox bb)
{
    m_boundingBox = bb;
    m_boundingBoxCenter = m_boundingBox.center();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d& AABBTreeNode::boundingBoxCenter() const
{
    return m_boundingBoxCenter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNodeInternal::AABBTreeNodeInternal(AABBTreeNode* left, AABBTreeNode* right)
{
    m_type = AB_INTERNAL;

    CVF_ASSERT(left);
    CVF_ASSERT(right);

    m_pLeft = left;
    m_pRight = right;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNodeInternal::AABBTreeNodeInternal()
{
    m_type = AB_INTERNAL;

    m_pLeft = NULL;
    m_pRight = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const AABBTreeNode* AABBTreeNodeInternal::left() const
{
    return m_pLeft;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNode* AABBTreeNodeInternal::left()
{
    return m_pLeft;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const AABBTreeNode* AABBTreeNodeInternal::right() const
{
    return m_pRight;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNode* AABBTreeNodeInternal::right()
{
    return m_pRight;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTreeNodeInternal::setLeft(AABBTreeNode* left)
{
    CVF_ASSERT(left);
    m_pLeft = left;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTreeNodeInternal::setRight(AABBTreeNode* right)
{
    CVF_ASSERT(right);
    m_pRight = right;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNodeLeaf::AABBTreeNodeLeaf()
{
    m_type = AB_LEAF;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> AABBTreeNodeLeaf::ids() const
{
    return m_ids;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AABBTreeNodeLeaf::setIds(const std::vector<int>& ids)
{
    m_ids = ids;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTree::AABBTree()
{
    m_pRoot = NULL;
    m_iNumLeaves = 0;

    m_nextNodeIndex      = 0u;
//    ResetStatistics();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTree::~AABBTree()
{
    freeThis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTree::free()
{
    freeThis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool AABBTree::buildTree()
{

    // Possibly delete the tree before building it
    freeThis();

    // First, create all the leaves
    cvf::BoundingBox box = createLeaves();

    if (m_iNumLeaves == 0) return true;

    // Create the root
    if (m_iNumLeaves == 1)
    {
        m_pRoot = m_ppLeaves[0];
        
        return true;
    }

    m_pRoot = createNode();
    m_pRoot->setBoundingBox(box);
        
    bool bRes = buildTree((AABBTreeNodeInternal*)m_pRoot, 0, m_iNumLeaves - 1, 0, 4);

#pragma omp parallel
    {
        bool bThreadRes = bRes;
#pragma omp for
        for (int i = 0; i < static_cast<int>(m_previousLevelNodes.size()); ++i)
        {
            bThreadRes = bThreadRes && buildTree(m_previousLevelNodes[i].node, m_previousLevelNodes[i].fromIdx, m_previousLevelNodes[i].toIdx, 4);
        }
#pragma omp critical
        {
            bRes = bRes && bThreadRes;
        }
    }
    m_previousLevelNodes.clear();

    return bRes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool AABBTree::buildTree(AABBTreeNodeInternal* pNode, size_t iFromIdx, size_t iToIdx, int currentLevel, int maxLevel)
{
    if (currentLevel == maxLevel)
    {
        m_previousLevelNodes.emplace_back(pNode, iFromIdx, iToIdx);
        return true;
    }

    int iLongestAxis = largestComponent(pNode->boundingBox().extent());

    double splitValue = pNode->boundingBoxCenter()[iLongestAxis];
    size_t i = iFromIdx;
    size_t iMid = iToIdx;

    //=================================================================================================================================
    //
    // Geometric split tree - best!
    //
    //=================================================================================================================================
    // Order the leaves according to the position of the center of each BB in comparison with longest axis of the BB
    while (i < iMid)
    {
        if (m_ppLeaves[i]->boundingBoxCenter()[iLongestAxis] < splitValue)
        {
            // Ok, move on
            i++;
        }
        else
        {
            // Swap, and move iMid back
            AABBTreeNodeLeaf* pTemp = m_ppLeaves[i];
            m_ppLeaves[i] = m_ppLeaves[iMid];
            m_ppLeaves[iMid] = pTemp;
            
            iMid--;
        }
    }

    if ((iMid == iFromIdx) || (iMid == iToIdx))
    {
        iMid = (iToIdx + iFromIdx)/2;
    }
    else
    {
        const size_t maxInbalance = 1000u;
        size_t rangeA = iMid - iFromIdx;
        size_t rangeB = iToIdx - iMid;
        if (rangeA > maxInbalance * rangeB || rangeB > maxInbalance * rangeA)
        {
            // Extremely imbalanced tree. Sort leaves and chose iMid as the median of the centres.
            std::sort(m_ppLeaves.begin() + iFromIdx, m_ppLeaves.begin() + iToIdx, [=](const AABBTreeNodeLeaf* lhs, const AABBTreeNodeLeaf* rhs)
            {
                return lhs->boundingBoxCenter()[iLongestAxis] < rhs->boundingBoxCenter()[iLongestAxis];
            });
            iMid = (iToIdx + iFromIdx)/2;
        }
    }

    // Create the left tree
    if (iMid > iFromIdx)
    {
        cvf::BoundingBox box = leafBoundingBox(iFromIdx, iMid);

        AABBTreeNodeInternal* newNode = createNode();
        newNode->setBoundingBox(box);
        pNode->setLeft(newNode);

        if (!buildTree((AABBTreeNodeInternal*)pNode->left(), iFromIdx, iMid, currentLevel + 1, maxLevel)) return false;
    }
    else
    {
        pNode->setLeft(m_ppLeaves[iFromIdx]);
    }

    // Create the right tree
    if (iMid < (iToIdx - 1))
    {
        cvf::BoundingBox box = leafBoundingBox(iMid + 1, iToIdx);

        AABBTreeNodeInternal* newNode = createNode();
        newNode->setBoundingBox(box);
        pNode->setRight(newNode);

        if (!buildTree((AABBTreeNodeInternal*)pNode->right(), iMid + 1, iToIdx, currentLevel + 1, maxLevel)) return false;
    }
    else
    {
        pNode->setRight(m_ppLeaves[iToIdx]);
    }

    return true;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTree::freeThis()
{
    // Delete all the internal nodes
    if (m_pRoot)
    {
        AABBTree::deleteInternalNodesBottomUp(m_pRoot);

        m_pRoot = NULL;
    }

    m_ppLeaves.clear();
    m_leafPool.clear();

    m_previousLevelNodes.clear();

    m_iNumLeaves    = 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox AABBTree::leafBoundingBox(size_t iStartIdx, size_t iEndIdx) const
{
    CVF_ASSERT(iStartIdx <= iEndIdx);

    cvf::BoundingBox bb;

    size_t i;
    for (i = iStartIdx; i <= iEndIdx; i++)
    {
        CVF_ASSERT(m_ppLeaves[i]);
        bb.addValid(m_ppLeaves[i]->boundingBox());
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t AABBTree::treeSize(const AABBTreeNode* pNode) const
{
    CVF_ASSERT(pNode);

    if (pNode->type() == AB_LEAF)
    {
        return treeNodeSize(pNode);
    }

    const AABBTreeNodeInternal* pInt = (const AABBTreeNodeInternal*)pNode;

    return treeNodeSize(pInt) + treeSize(pInt->left()) + treeSize(pInt->right());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t AABBTree::treeSize() const
{
    if (m_pRoot) return treeSize(m_pRoot);

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t AABBTree::treeNodeSize(const AABBTreeNode* pNode) const
{
    CVF_ASSERT(pNode);

    if (pNode->type() == AB_INTERNAL)	return sizeof(AABBTreeNodeInternal);
    if (pNode->type() == AB_LEAF)		return sizeof(AABBTreeNodeLeaf);

    // Should not get here...
    CVF_ASSERT(0);
    return 0;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t AABBTree::treeHeight(const AABBTreeNode* pNode, size_t iLevel, size_t* piMin, size_t* piMax) const
{
    CVF_ASSERT(pNode);

    if (pNode->type() == AB_LEAF)
    {
        if (iLevel < *piMin) *piMin = iLevel;
        if (iLevel > *piMax) *piMax = iLevel;

        return iLevel;
    }

    const AABBTreeNodeInternal* pInt = (const AABBTreeNodeInternal*)pNode;

    iLevel++;

    return treeHeight(pInt->left(), iLevel, piMin, piMax) + treeHeight(pInt->right(), iLevel, piMin, piMax);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string AABBTree::treeInfo() const
{
    size_t treeSizeInMB = treeSize() / (1024u *1024u);
    auto text = "Tree size : " + std::to_string(treeSizeInMB) + "[MB] \n";
    if (treeSize() == 0) return text;
    
    text += "Num leaves: " + std::to_string(leavesCount()) + "\n";

    size_t iMin = cvf::UNDEFINED_UINT;
    size_t iMax = 0;
    size_t iSumHeight = treeHeight(m_pRoot, 1, &iMin, &iMax);
    size_t iAvgHeigth = 0;

    if (leavesCount() > 0 ) iAvgHeigth = iSumHeight/leavesCount();
    
    auto iIdealHeigth = (cvf::uint)ceil((log((float)leavesCount())/log(2.0f)));

    text += "Tree height: \n";
    text += "  Min   : " + std::to_string(iMin) + "\n";
    text += "  Max   : " + std::to_string(iMax) + "\n";
    text += "  Avg   : " + std::to_string(iAvgHeigth) + "\n";
    text += "  Ideal : " + std::to_string(iIdealHeigth) + "\n";
   
    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool AABBTree::boundingBox(cvf::BoundingBox* pBox) const
{
    CVF_ASSERT(pBox);

    if (!m_pRoot) return false;
    
    *pBox = m_pRoot->boundingBox();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t AABBTree::leavesCount() const
{
    return m_iNumLeaves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool AABBTree::intersect(const AABBTreeNode* pA, const AABBTreeNode* pB) const
{
    return pA->boundingBox().intersects(pB->boundingBox());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::AABBTreeNodeInternal* AABBTree::createNode()
{
    return new AABBTreeNodeInternal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::AABBTreeNodeLeaf* AABBTree::createOrAssignLeaf(size_t leafIndex, const std::vector<int>& bbIds)
{
    cvf::AABBTreeNodeLeaf* leaf = &m_leafPool[leafIndex];
    leaf->setIds(bbIds);
    return leaf;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AABBTree::deleteInternalNodesBottomUp(AABBTreeNode* node)
{
    // All leaf nodes (AABBTreeNodeLeaf) are allocated in m_leafPool and does not require a delete
    if (node->type() == AB_LEAF) return;

    auto internalNode = dynamic_cast<AABBTreeNodeInternal*>(node);
    CVF_ASSERT(internalNode);

    AABBTree::deleteInternalNodesBottomUp(internalNode->left());
    AABBTree::deleteInternalNodesBottomUp(internalNode->right());

    delete internalNode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void BoundingBoxTreeImpl::buildTree(const std::vector<cvf::BoundingBox>& boundingBoxes, const std::vector<std::vector<int>>& ids)
{
    // Assign data used in the tree construction
    m_validBoundingBoxes = boundingBoxes;
    m_validOptionalBoundingBoxIds = ids;
    
    AABBTree::buildTree();

    // Release the memory used by the bounding boxes and ids, as this information is now distributed in the tree
    m_validBoundingBoxes.clear();
    m_validOptionalBoundingBoxIds.clear();
}

//--------------------------------------------------------------------------------------------------
/// Creates leafs for the supplied valid bounding boxes, keeping the original index 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox BoundingBoxTreeImpl::createLeaves()
{
    cvf::BoundingBox box;

    m_leafPool.resize(m_validBoundingBoxes.size());
    m_ppLeaves.resize(m_validBoundingBoxes.size());

#pragma omp parallel
    {
        cvf::BoundingBox threadBox;
#pragma omp for
        for (int i = 0; i < (int)m_validBoundingBoxes.size(); i++)
        {
            std::vector<int> bbIds = {i};
            if (!m_validOptionalBoundingBoxIds.empty()) bbIds = m_validOptionalBoundingBoxIds[i];

            AABBTreeNodeLeaf* leaf = createOrAssignLeaf(i, bbIds);

            leaf->setBoundingBox(m_validBoundingBoxes[i]);

            m_ppLeaves[i] = leaf;
            threadBox.addValid(m_validBoundingBoxes[i]);
        }
#pragma omp critical
        {
            box.addValid(threadBox);
        }
    }
    m_iNumLeaves = m_ppLeaves.size();

    return box;
}

//--------------------------------------------------------------------------------------------------
/// Find all indices to all bounding boxes intersecting the given bounding box and add them to indices
//--------------------------------------------------------------------------------------------------
void BoundingBoxTreeImpl::findIntersections(const cvf::BoundingBox& bb, std::vector<size_t>& indices) const
{
    if (bb.isValid())
    {
        findIntersections(bb, m_pRoot, indices);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BoundingBoxTreeImpl::findIntersections(const cvf::BoundingBox& bb, const AABBTreeNode* node, std::vector<size_t>& ids) const
{
    CVF_TIGHT_ASSERT(bb.isValid());
    
    if (node && bb.intersects(node->boundingBox()))
    {
        if (node->type() == AB_LEAF)
        {
            const AABBTreeNodeLeaf* leaf = static_cast<const AABBTreeNodeLeaf*>(node);
            {
                auto leafIds = leaf->ids();

                ids.insert(ids.end(), leafIds.begin(), leafIds.end());
                return;
            }
        }
        else if (node->type() == AB_INTERNAL)
        {
            const AABBTreeNodeInternal* internalNode = static_cast<const AABBTreeNodeInternal*>(node);

            findIntersections(bb, internalNode->left(), ids);
            findIntersections(bb, internalNode->right(), ids);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBoxTree::BoundingBoxTree()
{
    m_implTree = new BoundingBoxTreeImpl;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBoxTree::~BoundingBoxTree()
{
    delete m_implTree;
}

//--------------------------------------------------------------------------------------------------
/// Build a tree representation of valid bounding boxes. Invalid bounding boxes are ignored
/// The supplied ID array is the ID's returned in the intersection method.
/// If the ID array is omitted, the index of the bounding boxes are returned.
//--------------------------------------------------------------------------------------------------
void BoundingBoxTree::buildTreeFromBoundingBoxes(const std::vector<cvf::BoundingBox>& boundingBoxes,
                                                 const std::vector<size_t>* optionalBoundingBoxIds)
{
    if (optionalBoundingBoxIds) CVF_ASSERT(boundingBoxes.size() == optionalBoundingBoxIds->size());

    std::vector<cvf::BoundingBox> validBoundingBoxes;
    std::vector<std::vector<int>> validOptionalBoundingBoxIds;

    validBoundingBoxes.reserve(boundingBoxes.size());
    if (optionalBoundingBoxIds)
        validOptionalBoundingBoxIds.reserve(optionalBoundingBoxIds->size());

    for (size_t i = 0; i < boundingBoxes.size(); ++i)
    {
        if (boundingBoxes[i].isValid())
        {
            validBoundingBoxes.push_back(boundingBoxes[i]);
            if (optionalBoundingBoxIds)
            {
                const auto& id = (*optionalBoundingBoxIds)[i];
                
                std::vector<int> ids = {static_cast<int>(id)};
                validOptionalBoundingBoxIds.push_back(ids);
            }
        }
    }
    
    m_implTree->buildTree(validBoundingBoxes, validOptionalBoundingBoxIds);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void BoundingBoxTree::buildTreeFromBoundingBoxesOptimized(const std::vector<cvf::BoundingBox>& boundingBoxes,
                                                          const std::vector<std::vector<int>>& optionalBoundingBoxIds)
{
    std::vector<cvf::BoundingBox> validBoundingBoxes;
    std::vector<std::vector<int>> validOptionalBoundingBoxIds;
    
    for (int i = 0; i < (int)boundingBoxes.size(); ++i)
    {
        if (boundingBoxes[i].isValid())
        {
            validBoundingBoxes.push_back(boundingBoxes[i]);
            validOptionalBoundingBoxIds.push_back(optionalBoundingBoxIds[i]);
        }
    }
    
    m_implTree->buildTree(validBoundingBoxes, validOptionalBoundingBoxIds);
}

//--------------------------------------------------------------------------------------------------
/// Find all indices to all bounding boxes intersecting the given bounding box and add them to indices
//--------------------------------------------------------------------------------------------------
void BoundingBoxTree::findIntersections(const cvf::BoundingBox& bb, std::vector<size_t>* bbIdsOrIndices) const
{
    CVF_ASSERT(bbIdsOrIndices);

    m_implTree->findIntersections(bb, *bbIdsOrIndices);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string BoundingBoxTree::info() const
{
    return m_implTree->treeInfo();
}

} // namespace cvf

