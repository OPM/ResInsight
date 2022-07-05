/*
  Copyright 2021 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <opm/input/eclipse/Deck/DeckTree.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace Opm {

DeckTree::TreeNode::TreeNode(const std::string& fn)
    : fname(fn)
{}

DeckTree::TreeNode::TreeNode(const std::string& pn, const std::string& fn)
    : fname(fn)
    , parent(pn)
{}

void DeckTree::TreeNode::add_include(const std::string& include_file) {
    this->include_files.emplace(include_file);
}

bool DeckTree::TreeNode::includes(const std::string& include_file) const {
    return this->include_files.count(include_file) > 0;
}


std::string DeckTree::add_node(const std::string& fname) {
    auto abs_path = fs::canonical(fname).string();
    this->nodes.emplace( abs_path, TreeNode(abs_path) );
    return abs_path;
}


DeckTree::DeckTree(const std::string& fname)
{
    this->add_root(fname);
}

void DeckTree::add_root(const std::string& fname)
{
    if (this->root_file.has_value())
        throw std::logic_error("Root already assigned");

    this->root_file = this->add_node(fname);
}



bool DeckTree::includes(const std::string& parent_file, const std::string& include_file) const {
    if (!this->root_file.has_value())
        return false;

    const auto& parent_node = this->nodes.at(fs::canonical(parent_file).string());
    return parent_node.includes(fs::canonical(include_file).string());
}

const std::string& DeckTree::parent(const std::string& fname) const {
    const auto& node = this->nodes.at(fs::canonical(fname).string());
    const auto& parent_node = this->nodes.at( node.parent.value() );
    return parent_node.fname;
}

const std::string& DeckTree::root() const {
    return this->root_file.value();
}

void DeckTree::add_include(std::string parent_file, std::string include_file) {
    if (!this->root_file.has_value())
        return;

    parent_file = fs::canonical(parent_file).string();
    include_file = fs::canonical(include_file).string();
    this->nodes.emplace(include_file, TreeNode(parent_file, include_file));
    auto& parent_node = this->nodes.at(parent_file);
    parent_node.add_include( include_file );
}

bool DeckTree::has_include(const std::string& fname) const {
    const auto& node = this->nodes.at(fname);
    return !node.include_files.empty();
}

}
