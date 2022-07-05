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

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DECK_TREE_HPP
#define DECK_TREE_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>


namespace Opm {


/*
  The purpose of the DeckTree class is to maintain a minimal relationship
  between the include files in the deck; the sole purpose of this class is to
  support writing of decks with the keywords in the correct files.
*/

class DeckTree {
public:
    DeckTree() = default;
    DeckTree(const std::string&);

    const std::string& parent(const std::string& fname) const;
    bool includes(const std::string& parent_file, const std::string& include_file) const;
    void add_include(std::string parent_file, std::string include_file);
    void add_root(const std::string& fname);
    bool has_include(const std::string& fname) const;
    const std::string& root() const;

private:
    class TreeNode {
    public:
        explicit TreeNode(const std::string& fn);
        TreeNode(const std::string& pn, const std::string& fn);
        void add_include(const std::string& include_file);
        bool includes(const std::string& include_file) const;

        std::string fname;
        std::optional<std::string> parent;
        std::unordered_set<std::string> include_files;
    };

    std::string add_node(const std::string& fname);

    std::optional<std::string> root_file;
    std::unordered_map<std::string, TreeNode> nodes;
};


}
#endif  /* DECKRECORD_HPP */

