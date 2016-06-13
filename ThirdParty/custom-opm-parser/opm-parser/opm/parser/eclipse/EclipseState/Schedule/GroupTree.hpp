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

#ifndef GROUPTREE_HPP
#define	GROUPTREE_HPP

#include <opm/parser/eclipse/EclipseState/Schedule/GroupTreeNode.hpp>

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace Opm {

    class GroupTree {
    public:
        GroupTree();
        void updateTree(const std::string& childName);
        void updateTree(const std::string& childName, const std::string& parentName);

        GroupTreeNodePtr getNode(const std::string& nodeName) const;
        std::vector<GroupTreeNodeConstPtr> getNodes() const;
        GroupTreeNodePtr getParent(const std::string& childName) const;

        std::shared_ptr<GroupTree> deepCopy() const;
        void printTree(std::ostream &os) const;


    private:
        GroupTreeNodePtr m_root;
        GroupTreeNodePtr getNode(const std::string& nodeName, GroupTreeNodePtr current) const;
        GroupTreeNodePtr getParent(const std::string& childName, GroupTreeNodePtr currentChild, GroupTreeNodePtr parent) const;

        void getNodes(GroupTreeNodePtr fromNode, std::vector<GroupTreeNodeConstPtr>& nodes) const;
        void deepCopy(GroupTreeNodePtr origin, GroupTreeNodePtr copy) const;
        void printTree(std::ostream &os , GroupTreeNodePtr fromNode) const;
    };

    typedef std::shared_ptr<GroupTree> GroupTreePtr;
    typedef std::shared_ptr<const GroupTree> GroupTreeConstPtr;
}

#endif	/* GROUPTREE_HPP */

