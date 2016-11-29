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

#include <opm/parser/eclipse/EclipseState/Schedule/GroupTreeNode.hpp>

#define BOOST_TEST_MODULE GroupTreeNodeTests

#include <boost/test/unit_test.hpp>

#include <stdexcept>


using namespace Opm;

BOOST_AUTO_TEST_CASE(CreateFieldNode) {
    GroupTreeNodePtr node = GroupTreeNode::createFieldNode();
    BOOST_CHECK_EQUAL("FIELD", node->name());
}

BOOST_AUTO_TEST_CASE(CreateChild_WithFieldParent_ParentHasChild) {
    GroupTreeNodePtr fieldNode = GroupTreeNode::createFieldNode();
    BOOST_CHECK(!fieldNode->hasChildGroup("Child"));
    GroupTreeNodePtr child = fieldNode->addChildGroup("Child");
    BOOST_REQUIRE(fieldNode->hasChildGroup("Child"));
    BOOST_CHECK_EQUAL(child, fieldNode->getChildGroup("Child"));
}

BOOST_AUTO_TEST_CASE(CreateChildGroup_ChildExists_Throws) {
    GroupTreeNodePtr fieldNode = GroupTreeNode::createFieldNode();
    GroupTreeNodePtr child = fieldNode->addChildGroup("Child");
    BOOST_CHECK_THROW(fieldNode->addChildGroup("Child"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(GetChildGroup_ChildNotExisting_Throws) {
    GroupTreeNodePtr fieldNode = GroupTreeNode::createFieldNode();
    GroupTreeNodePtr child = fieldNode->addChildGroup("Child2");
    BOOST_CHECK_THROW(fieldNode->getChildGroup("Child"), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(RemoveChild_ChildNonExisting_Throws) {
    GroupTreeNodePtr fieldNodeDummy = GroupTreeNode::createFieldNode();
    GroupTreeNodePtr fieldNode = GroupTreeNode::createFieldNode();
    GroupTreeNodePtr child = fieldNode->addChildGroup("Child1");
    BOOST_CHECK_THROW(fieldNode->removeChild(fieldNodeDummy), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(RemoveChild_RemoveTwice_Throws) {
    GroupTreeNodePtr fieldNodeDummy = GroupTreeNode::createFieldNode();
    GroupTreeNodePtr fieldNode = GroupTreeNode::createFieldNode();
    GroupTreeNodePtr child = fieldNode->addChildGroup("Child1");
    BOOST_CHECK_NO_THROW(fieldNode->removeChild(child));;
    BOOST_CHECK_THROW(fieldNode->removeChild(child), std::invalid_argument);

}

BOOST_AUTO_TEST_CASE(RemoveChild_ChildExists_ChildRemoved) {
    GroupTreeNodePtr fieldNode = GroupTreeNode::createFieldNode();
    GroupTreeNodePtr child = fieldNode->addChildGroup("Child1");
    BOOST_CHECK_NO_THROW(fieldNode->removeChild(child));
    BOOST_CHECK(!fieldNode->hasChildGroup("Child1"));
}
