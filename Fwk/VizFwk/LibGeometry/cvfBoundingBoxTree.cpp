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
        AB_INTERNAL,
        AB_LEAF_GROUP
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

        NodeType                type() const;

    protected:
        NodeType                m_type;

    private:
        cvf::BoundingBox        m_boundingBox;
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
        AABBTreeNodeLeaf(size_t index);

        size_t index() const;

    private:
        size_t m_index;					///< An index of the leaf node. The interpretation of this index is depending on which tree the node is in.
    };


    //=================================================================================================================================
    /// Group leaf node in the AABB tree. The leaf node contains an array with indices, and the interpretation of these are depending on
    /// the type of AABB tree using the node.
    //=================================================================================================================================
    class AABBTreeNodeLeafGroup : public AABBTreeNode
    {
    public:
        AABBTreeNodeLeafGroup();

        size_t                          addIndex(size_t index);
        const std::vector<size_t>&      indices() const;

        void                            sort();

    private:
        std::vector<size_t> m_indices;		///< The interpretation of these indices is depending on which tree the node is in.
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

        cvf::String treeInfo() const;

    protected:
        virtual bool   createLeaves() = 0;
        virtual size_t treeNodeSize(const AABBTreeNode* pNode) const;

        virtual AABBTreeNodeLeafGroup* createGroupNode(size_t iStartIdx, size_t iEndIdx);

        void   freeThis();
        void   deleteInternalNodes(AABBTreeNode* pNode);

        size_t treeSize(const AABBTreeNode* pNode) const;
        size_t treeHeight(const AABBTreeNode* pNode, size_t iLevel, size_t* piMin, size_t* piMax) const;
        void   leafBoundingBox(cvf::BoundingBox& pBox, size_t iStartIdx, size_t iEndIdx) const;
        bool   buildTree(AABBTreeNodeInternal* pNode, size_t iFromIdx, size_t iToIdx);

        // Queries
        bool intersect(const AABBTreeNode* pA, const AABBTreeNode* pB) const;


    protected:
        std::vector<AABBTreeNodeLeaf*> m_ppLeaves;
        size_t m_iNumLeaves;

        AABBTreeNode* m_pRoot;

        bool m_bUseGroupNodes;
        size_t m_iGroupLimit;
    };

    class BoundingBoxTreeImpl : public AABBTree
    {
        BoundingBoxTreeImpl() {}

    private:
        friend class BoundingBoxTree;

        bool createLeaves();
        void findIntersections(const cvf::BoundingBox& bb, std::vector<size_t>& bbIds) const;

        void findIntersections(const cvf::BoundingBox& bb, const AABBTreeNode* node, std::vector<size_t>& indices) const;

        const std::vector<cvf::BoundingBox>* m_boundingBoxes;
        const std::vector<size_t>* m_optionalBoundingBoxIds;
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
AABBTreeNodeLeaf::AABBTreeNodeLeaf(size_t index)
{
    m_type = AB_LEAF;

    m_index = index;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t AABBTreeNodeLeaf::index() const
{
    return m_index;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t AABBTreeNodeLeafGroup::addIndex(size_t index)
{
    m_indices.push_back(index);

    return m_indices.size() - 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& AABBTreeNodeLeafGroup::indices() const
{
    return m_indices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNodeLeafGroup::AABBTreeNodeLeafGroup()
{
    m_type = AB_LEAF_GROUP;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTreeNodeLeafGroup::sort()
{
    std::sort(m_indices.begin(), m_indices.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTree::AABBTree()
{
    m_pRoot = NULL;
    m_iNumLeaves = 0;

    m_bUseGroupNodes = false;
    m_iGroupLimit = 33;

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
	if (!createLeaves()) return false;

	if (m_iNumLeaves == 0) return true;

	// Then find the bounding box of all items in the tree
	cvf::BoundingBox box;
	leafBoundingBox(box, 0, m_iNumLeaves - 1);

	// Create the root
	if (m_iNumLeaves == 1)
	{
		m_pRoot = m_ppLeaves[0];
		
		return true;
	}

	m_pRoot = new AABBTreeNodeInternal();
	m_pRoot->setBoundingBox(box);
		
	bool bRes = buildTree((AABBTreeNodeInternal*)m_pRoot, 0, m_iNumLeaves - 1);

	return bRes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool AABBTree::buildTree(AABBTreeNodeInternal* pNode, size_t iFromIdx, size_t iToIdx)
{
	int iLongestAxis = largestComponent(pNode->boundingBox().extent());

	double splitValue = pNode->boundingBox().center()[iLongestAxis];
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
        if (m_ppLeaves[i]->boundingBox().center()[iLongestAxis] < splitValue)
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

	// Create the left tree
	if (iMid > iFromIdx)
	{
		if (m_bUseGroupNodes && ((iMid - iFromIdx + 1) < m_iGroupLimit))
		{

			pNode->setLeft(createGroupNode(iFromIdx, iMid));
		}
		else
		{
			cvf::BoundingBox box;
			leafBoundingBox(box, iFromIdx, iMid);

            AABBTreeNodeInternal* newNode = new AABBTreeNodeInternal;
			newNode->setBoundingBox(box);
            pNode->setLeft(newNode);

			if (!buildTree((AABBTreeNodeInternal*)pNode->left(), iFromIdx, iMid)) return false;
		}
	}
	else
	{
		pNode->setLeft(m_ppLeaves[iFromIdx]);
	}

	// Create the right tree
	if (iMid < (iToIdx - 1))
	{
		if (m_bUseGroupNodes && ((iToIdx - (iMid + 1) + 1) < m_iGroupLimit))
		{
			pNode->setRight(createGroupNode(iMid + 1, iToIdx));
		}
		else
		{
            cvf::BoundingBox box;
			leafBoundingBox(box, iMid + 1, iToIdx);

            AABBTreeNodeInternal* newNode = new AABBTreeNodeInternal;
            newNode->setBoundingBox(box);
            pNode->setRight(newNode);

			if (!buildTree((AABBTreeNodeInternal*)pNode->right(), iMid + 1, iToIdx)) return false;
		}
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
        // This also deletes m_pRoot if not a leaf. If it is a leaf, it is deleted below
        deleteInternalNodes(m_pRoot);

        m_pRoot = NULL;
    }

    size_t i;
    for (i = 0; i < m_iNumLeaves; i++)
    {
        // Might be NULL if group nodes are used
        if (m_ppLeaves[i])
        {
            delete(m_ppLeaves[i]);
        }
    }

    m_ppLeaves.clear();

    m_iNumLeaves = 0;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTree::deleteInternalNodes(AABBTreeNode* pNode)
{
    CVF_ASSERT(pNode);

    if (pNode->type() == AB_LEAF) return;
    
    if (pNode->type() == AB_LEAF_GROUP)
    {
        delete(pNode);
        return;
    }

    AABBTreeNodeInternal* pInt = (AABBTreeNodeInternal*)pNode;

    deleteInternalNodes(pInt->left());
    deleteInternalNodes(pInt->right());

    delete(pNode);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AABBTreeNodeLeafGroup* AABBTree::createGroupNode(size_t iStartIdx, size_t iEndIdx)
{
    size_t iNumItems = iEndIdx - iStartIdx + 1;
    CVF_ASSERT(iNumItems > 1);

    AABBTreeNodeLeafGroup* pNode = new AABBTreeNodeLeafGroup;
    if (!pNode) return NULL;

    cvf::BoundingBox bb;
    leafBoundingBox(bb, iStartIdx, iEndIdx);
    pNode->setBoundingBox(bb);

    size_t i;
    for (i = iStartIdx; i <= iEndIdx; i++)
    {
        pNode->addIndex(m_ppLeaves[i]->index());

        // Delete the original leaf
        delete(m_ppLeaves[i]);

        m_ppLeaves[i] = NULL;
    }

    // Sort the element indices (this is not really required)
    // This is done to give then same result as the old implementation that did just a linear search in cases where
    // the points are on the element surfaces and thus will give multiple hits.
    // 
    // The performance hit of this seems very small.
    pNode->sort();

    return pNode;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AABBTree::leafBoundingBox(cvf::BoundingBox& bb, size_t iStartIdx, size_t iEndIdx) const
{
    CVF_ASSERT(iStartIdx <= iEndIdx);

    bb.reset();

    size_t i;
    for (i = iStartIdx; i <= iEndIdx; i++)
    {
        CVF_ASSERT(m_ppLeaves[i]);
        bb.add(m_ppLeaves[i]->boundingBox());
    }
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

    if (pNode->type() == AB_LEAF_GROUP)
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
    if (pNode->type() == AB_LEAF_GROUP)
    {
        const AABBTreeNodeLeafGroup* pLeafGroup = (const AABBTreeNodeLeafGroup*)pNode;

        return static_cast<size_t>(sizeof(AABBTreeNodeLeafGroup) + static_cast<size_t>(pLeafGroup->indices().size()) * sizeof(cvf::uint));
    }

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

    if ((pNode->type() == AB_LEAF) || (pNode->type() == AB_LEAF_GROUP))
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
cvf::String AABBTree::treeInfo() const
{
    cvf::String sInfo;

    /*
    sInfo =  cvf::String("Tree size: %1 \n").arg(static_cast<int>(treeSize()));
    sInfo += cvf::String("Num leaves: %1 \n").arg(static_cast<int>(leavesCount()));

    size_t iMin = cvf::UNDEFINED_UINT;
    size_t iMax = 0;
    size_t iSumHeight = treeHeight(m_pRoot, 1, &iMin, &iMax);
    size_t iAvgHeigth = 0;
    size_t iIdealHeigth = 0;

    if (leavesCount() > 0 ) iAvgHeigth = iSumHeight/leavesCount();

    sInfo += VTString::MakeForm("Tree height: Min: %d - Max: %d - Avg: %d - Ideal: %d\n", iMin, iMax, iAvgHeigth, (VTint)ceil((log((VTfloat)GetNumLeaves())/log(2.0f))));
    iIdealHeigth = (cvf::uint)ceil((log((float)leavesCount())/log(2.0f)));
    sInfo =  cvf::String("Tree height: Min: %1 - Max: %2 - Avg: %3 - Ideal: %4\n").arg(iMin).arg(iMax).arg(iAvgHeigth).arg(iIdealHeigth);

    cvf::BoundingBox bb;
    boundingBox(&bb);
    sInfo += bb.debugString();
    */

    return sInfo;
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
/// Creates leafs for the supplied valid bounding boxes, keeping the original index 
//--------------------------------------------------------------------------------------------------
bool BoundingBoxTreeImpl::createLeaves()
{
    size_t i;
    for (i = 0; i < m_boundingBoxes->size(); i++)
    {
        size_t bbId = i;
        if (m_optionalBoundingBoxIds) bbId = (*m_optionalBoundingBoxIds)[i];

        AABBTreeNodeLeaf* leaf = new AABBTreeNodeLeaf(bbId);

        leaf->setBoundingBox((*m_boundingBoxes)[i]);

        m_ppLeaves.push_back(leaf);
    }

    m_iNumLeaves = m_ppLeaves.size();

    return true;
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
void BoundingBoxTreeImpl::findIntersections(const cvf::BoundingBox& bb, const AABBTreeNode* node, std::vector<size_t>& cvIndices) const
{
    CVF_TIGHT_ASSERT(bb.isValid());
    
    if (node && bb.intersects(node->boundingBox()))
    {
        if (node->type() == AB_LEAF)
        {
            const AABBTreeNodeLeaf* leaf = static_cast<const AABBTreeNodeLeaf*>(node);
            {
                cvIndices.push_back(leaf->index());
                return;
            }
        }
        else if (node->type() == AB_INTERNAL)
        {
            const AABBTreeNodeInternal* internalNode = static_cast<const AABBTreeNodeInternal*>(node);

            findIntersections(bb, internalNode->left(), cvIndices);
            findIntersections(bb, internalNode->right(), cvIndices);
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

    m_implTree->m_boundingBoxes = &boundingBoxes;
    m_implTree->m_optionalBoundingBoxIds = optionalBoundingBoxIds;

    m_implTree->buildTree();
 
}

//--------------------------------------------------------------------------------------------------
/// Find all indices to all bounding boxes intersecting the given bounding box and add them to indices
//--------------------------------------------------------------------------------------------------
void BoundingBoxTree::findIntersections(const cvf::BoundingBox& bb, std::vector<size_t>* bbIdsOrIndices) const
{
    CVF_ASSERT(bbIdsOrIndices);

    m_implTree->findIntersections(bb, *bbIdsOrIndices);
}

} // namespace cvf

