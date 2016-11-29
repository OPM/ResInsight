/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>
#include <iostream>


#include <opm/parser/eclipse/EclipseState/Schedule/GroupTree.hpp>


namespace Opm {

    GroupTree::GroupTree() {
        m_root = GroupTreeNode::createFieldNode();
    }

    GroupTree::GroupTree( const GroupTree& x ) :
        m_root( GroupTreeNode::createFieldNode() ) {
        deepCopy( x.m_root, this->m_root );
    }

    void GroupTree::updateTree(const std::string& childName) {
        updateTree(childName, m_root->name());
    }

    void GroupTree::updateTree(const std::string& childName, const std::string& parentName) {
        if (childName == m_root->name()) {
            throw std::domain_error("Error, trying to add node with the same name as the root, offending name: " + childName);
        }

        GroupTreeNodePtr newParentNode = getNode(parentName);
        if (!newParentNode) {
            newParentNode = m_root->addChildGroup(parentName);
        }

        GroupTreeNodePtr childNodeInTree = getNode(childName);
        if (childNodeInTree) {
            GroupTreeNodePtr currentParent = getParent(childName);
            currentParent->removeChild(childNodeInTree);
            newParentNode->addChildGroup(childNodeInTree);
        } else {
            newParentNode->addChildGroup(childName);
        }
    }

    GroupTreeNodePtr GroupTree::getNode(const std::string& nodeName) const {
        GroupTreeNodePtr current = m_root;
        return getNode(nodeName, current);
    }

    GroupTreeNodePtr GroupTree::getNode(const std::string& nodeName, GroupTreeNodePtr current) const {
        if (current->name() == nodeName) {
            return current;
        } else {
            std::map<std::string, GroupTreeNodePtr>::const_iterator iter = current->begin();
            while (iter != current->end()) {
                GroupTreeNodePtr result = getNode(nodeName, (*iter).second);
                if (result) {
                    return result;
                }
                ++iter;
            }
            return GroupTreeNodePtr();
        }
    }

    std::vector<GroupTreeNodeConstPtr> GroupTree::getNodes() const {
        std::vector<GroupTreeNodeConstPtr> nodes;
        nodes.push_back(m_root);
        getNodes(m_root, nodes);
        return nodes;
    }

    void GroupTree::getNodes(GroupTreeNodePtr fromNode, std::vector<GroupTreeNodeConstPtr>& nodes) const {
        std::map<std::string, GroupTreeNodePtr >::const_iterator iter = fromNode->begin();
        while (iter != fromNode->end()) {
            GroupTreeNodePtr child = (*iter).second;
            nodes.push_back(child);
            getNodes(child, nodes);
            ++iter;
        }
    }

    GroupTreeNodePtr GroupTree::getParent(const std::string& childName) const {
        GroupTreeNodePtr currentChild = m_root;
        return getParent(childName, currentChild, GroupTreeNodePtr());
    }

    GroupTreeNodePtr GroupTree::getParent(const std::string& childName, GroupTreeNodePtr currentChild, GroupTreeNodePtr parent) const {
        if (currentChild->name() == childName) {
            return parent;
        } else {
            std::map<std::string, GroupTreeNodePtr>::const_iterator iter = currentChild->begin();
            while (iter != currentChild->end()) {
                GroupTreeNodePtr result = getParent(childName, (*iter).second, currentChild);
                if (result) {
                    return result;
                }
                ++iter;
            }
            return GroupTreeNodePtr();
        }
    }

    GroupTreePtr GroupTree::deepCopy() const {
        GroupTreePtr newTree(new GroupTree());
        GroupTreeNodePtr currentOriginNode = m_root;
        GroupTreeNodePtr currentNewNode = newTree->getNode("FIELD");

        deepCopy(currentOriginNode, currentNewNode);
        return newTree;

    }

    void GroupTree::deepCopy(GroupTreeNodePtr origin, GroupTreeNodePtr copy) const {
        std::map<std::string, GroupTreeNodePtr >::const_iterator iter = origin->begin();
        while (iter != origin->end()) {
            GroupTreeNodePtr originChild = (*iter).second;
            GroupTreeNodePtr copyChild = copy->addChildGroup(originChild->name());
            deepCopy(originChild, copyChild);
            ++iter;
        }
    }

    void GroupTree::printTree(std::ostream &os) const {
        printTree(os , m_root);
        os << std::endl;
        os << "End of tree" << std::endl;
    }

    void GroupTree::printTree(std::ostream &os , GroupTreeNodePtr fromNode) const {
        os << fromNode->name() << "(" << fromNode.get() << ")";
        std::map<std::string, GroupTreeNodePtr >::const_iterator iter = fromNode->begin();
        while (iter != fromNode->end()) {
            GroupTreeNodePtr child = (*iter).second;
            os << "<-" << child->name() << "(" << child.get() << ")" << std::endl;
            printTree(os , child);
            ++iter;
        }
    }

}
