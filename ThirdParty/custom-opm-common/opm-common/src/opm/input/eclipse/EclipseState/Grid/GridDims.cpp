/*
  Copyright 2016  Statoil ASA.

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

#include <array>
#include <stdexcept>
#include <vector>

#include <opm/io/eclipse/EGrid.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/input/eclipse/EclipseState/Grid/GridDims.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp> // DIMENS
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp> // SPECGRID

namespace Opm {
    GridDims::GridDims(std::array<int, 3> xyz) :
                    GridDims(xyz[0], xyz[1], xyz[2])
    {
    }

    GridDims::GridDims(size_t nx, size_t ny, size_t nz) :
                    m_nx(nx), m_ny(ny), m_nz(nz)
    {
    }

    GridDims GridDims::serializeObject()
    {
        GridDims result;
        result.m_nx = 1;
        result.m_ny = 2;
        result.m_nz = 3;

        return result;
    }

    GridDims::GridDims(const Deck& deck) {
        if (deck.hasKeyword("SPECGRID"))
            init(deck["SPECGRID"].back());
        else if (deck.hasKeyword("DIMENS"))
            init(deck["DIMENS"].back());
        else if (deck.hasKeyword("GDFILE"))
            binary_init(deck);
        else
            throw std::invalid_argument("Must have either SPECGRID or DIMENS to indicate grid dimensions");
    }

    size_t GridDims::getNX() const {
        return m_nx;
    }

    size_t GridDims::getNY() const {
        return m_ny;
    }

    size_t GridDims::getNZ() const {
        return m_nz;
    }

    size_t GridDims::operator[](int dim) const {
        switch (dim) {
        case 0:
            return this->m_nx;
            break;
        case 1:
            return this->m_ny;
            break;
        case 2:
            return this->m_nz;
            break;
        default:
            throw std::invalid_argument("Invalid argument dim:" + std::to_string(dim));
        }
    }

    const std::array<int, 3> GridDims::getNXYZ() const {
        return std::array<int, 3> {{int( m_nx ), int( m_ny ), int( m_nz )}};
    }

    size_t GridDims::getGlobalIndex(size_t i, size_t j, size_t k) const {
        return (i + j * getNX() + k * getNX() * getNY());
    }

    const std::array<int, 3> GridDims::getIJK(size_t globalIndex) const {
        std::array<int, 3> r = { { 0, 0, 0 } };
        int k = globalIndex / (getNX() * getNY());
        globalIndex -= k * (getNX() * getNY());
        int j = globalIndex / getNX();
        globalIndex -= j * getNX();
        int i = globalIndex;
        r[0] = i;
        r[1] = j;
        r[2] = k;
        return r;
    }

    size_t GridDims::getCartesianSize() const {
        return m_nx * m_ny * m_nz;
    }

    void GridDims::assertGlobalIndex(size_t globalIndex) const {
        if (globalIndex >= getCartesianSize())
            throw std::invalid_argument("input global index above valid range");
    }

    void GridDims::assertIJK(size_t i, size_t j, size_t k) const {
        if (i >= getNX() || j >= getNY() || k >= getNZ())
            throw std::invalid_argument("input IJK index above valid range");
    }

    GridDims::GridDims() :
                    m_nx(0), m_ny(0), m_nz(0)
    {
    }

    // keyword must be DIMENS or SPECGRID
    inline std::array< int, 3 > readDims(const DeckKeyword& keyword) {
        const auto& record = keyword.getRecord(0);
        return { { record.getItem("NX").get<int>(0),
                   record.getItem("NY").get<int>(0),
                   record.getItem("NZ").get<int>(0) } };
    }

    void GridDims::init(const DeckKeyword& keyword) {
        auto dims = readDims(keyword);
        m_nx = dims[0];
        m_ny = dims[1];
        m_nz = dims[2];
    }

    void GridDims::binary_init(const Deck& deck) {
        const DeckKeyword& gdfile_kw = deck["GDFILE"].back();
        const std::string& gdfile_arg = gdfile_kw.getRecord(0).getItem("filename").get<std::string>(0);
        const EclIO::EGrid egrid( deck.makeDeckPath(gdfile_arg) );
        const auto& dimens = egrid.dimension();
        m_nx = dimens[0];
        m_ny = dimens[1];
        m_nz = dimens[2];
    }

    bool GridDims::operator==(const GridDims& data) const {
        return this->getNXYZ() == data.getNXYZ();
    }

}
