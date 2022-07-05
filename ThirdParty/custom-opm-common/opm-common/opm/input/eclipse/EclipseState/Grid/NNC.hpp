/*
  Copyright 2015 IRIS

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

#ifndef OPM_PARSER_NNC_HPP
#define OPM_PARSER_NNC_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

#include <opm/common/OpmLog/KeywordLocation.hpp>

namespace Opm
{

class GridDims;

struct NNCdata {
    NNCdata(size_t c1, size_t c2, double t)
        : cell1(c1), cell2(c2), trans(t)
    {}
    NNCdata() = default;

    bool operator==(const NNCdata& data) const
    {
        return cell1 == data.cell1 &&
               cell2 == data.cell2 &&
               trans == data.trans;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(cell1);
        serializer(cell2);
        serializer(trans);
    }

    // Observe that the operator< is only for cell ordering and does not consider the
    // trans member
    bool operator<(const NNCdata& other) const
    {
        return std::tie(this->cell1, this->cell2) < std::tie(other.cell1, other.cell2);
    }

    size_t cell1;
    size_t cell2;
    double trans;
};



class Deck;
class EclipseGrid;

/*
  This class is an internalization of the NNC and EDITNNC keywords. Because the
  opm-common codebase does not itself manage the simulation grid the purpose of
  the NNC class is mainly to hold on to the NNC/EDITNNC input and pass it on to
  the grid construction proper.

  The EDITNNC keywords can operate on two different types of NNCs.

    1. NNCs which have been explicitly entered using the NNC keyword.
    2. NNCs which are inderectly inferred from the grid - e.g. due to faults.

  When processing the EDITNNC keyword the class will search through the NNCs
  configured explicitly with the NNC keyword and apply the edit transformation
  on those NNCs, EDITNNCs which affect NNCs which are not configured explicitly
  are stored for later use by the simulator.

  The class guarantees the following ordering:

    1. For all NNC / EDITNNC records we will have cell1 <= cell2
    2. The vectors NNC::input() and NNC::edit() will be ordered in ascending
       order.

  While constructing from a deck NNCs connected to inactive cells will be
  silently ignored. Do observe though that the addNNC() function does not check
  the arguments and alas there is no guarantee that only active cells are
  involved.
*/

class NNC
{
public:
    NNC() = default;
    /// Construct from input deck.
    NNC(const EclipseGrid& grid, const Deck& deck);

    static NNC serializeObject();

    bool addNNC(const size_t cell1, const size_t cell2, const double trans);
    const std::vector<NNCdata>& input() const { return m_input; }
    const std::vector<NNCdata>& edit() const { return m_edit; }
    KeywordLocation input_location(const NNCdata& nnc) const;
    KeywordLocation edit_location(const NNCdata& nnc) const;


    bool operator==(const NNC& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(m_input);
        serializer.vector(m_edit);
        serializer(m_nnc_location);
        serializer(m_edit_location);
    }

private:

    void load_input(const EclipseGrid& grid, const Deck& deck);
    void load_edit(const EclipseGrid& grid, const Deck& deck);
    void add_edit(const NNCdata& edit_node);
    bool update_nnc(std::size_t global_index1, std::size_t global_index2, double tran_mult);

    std::vector<NNCdata> m_input;
    std::vector<NNCdata> m_edit;
    std::optional<KeywordLocation> m_nnc_location;
    std::optional<KeywordLocation> m_edit_location;
};


} // namespace Opm


#endif // OPM_PARSER_NNC_HPP
