/*
  Copyright 2018 Equinor AS

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
#ifndef OPM_COMMON_EDITNNC_HPP
#define OPM_COMMON_EDITNNC_HPP

#include <opm/parser/eclipse/EclipseState/Grid/NNC.hpp>
namespace Opm
{

/// Represents edit information for non-neighboring connections (NNCs, faults, etc.)
class EDITNNC
{
public:
    EDITNNC() = default;

    /// Construct from input deck
    explicit EDITNNC(const Deck& deck);

    /// Returns an instance used for serialization test
    static EDITNNC serializeObject();

    /// \brief Get an ordered set of EDITNNC
    const std::vector<NNCdata>& data() const
    {
        return m_editnnc;
    }
    /// \brief Get the number of entries
    size_t size() const;
    /// \brief Whether EDITNNC was empty.
    bool empty() const;

    bool operator==(const EDITNNC& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(m_editnnc);
    }

private:
    std::vector<NNCdata> m_editnnc;
};
}
#endif // OPM_COMMON_EDITNNC_HPP
