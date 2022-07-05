/*
  Copyright 2014 Statoil ASA.

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

#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

#include <opm/input/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

namespace {

    void assert_dims(int len,  int l1 , int l2) {
        if (len <= 0)
            throw std::invalid_argument("Box must have finite size in all directions");

        if ((l1 < 0) || (l2 < 0) || (l1 > l2))
            throw std::invalid_argument("Invalid index values for sub box");

        if (l2 >= len)
            throw std::invalid_argument("Invalid index values for sub box");
    }


    void update_default(int &value, std::size_t& default_count, const Opm::DeckItem& item) {
        if (item.defaultApplied(0))
            default_count += 1;
        else
            value = item.get<int>(0) - 1;
    }
}

namespace Opm {

    Box::Box(const EclipseGrid& grid_arg) :
        grid(grid_arg)
    {
        this->reset();
    }


    Box::Box(const EclipseGrid& grid_arg, int i1 , int i2 , int j1 , int j2 , int k1 , int k2) :
        grid(grid_arg)
    {
        this->init(i1,i2,j1,j2,k1,k2);
    }


    void Box::update(const DeckRecord& deckRecord) {
        const auto& I1Item = deckRecord.getItem("I1");
        const auto& I2Item = deckRecord.getItem("I2");
        const auto& J1Item = deckRecord.getItem("J1");
        const auto& J2Item = deckRecord.getItem("J2");
        const auto& K1Item = deckRecord.getItem("K1");
        const auto& K2Item = deckRecord.getItem("K2");

        std::size_t default_count = 0;
        int i1 = 0;
        int i2 = this->grid.getNX() - 1;
        int j1 = 0;
        int j2 = this->grid.getNY() - 1;
        int k1 = 0;
        int k2 = this->grid.getNZ() - 1;

        update_default(i1, default_count, I1Item);
        update_default(i2, default_count, I2Item);
        update_default(j1, default_count, J1Item);
        update_default(j2, default_count, J2Item);
        update_default(k1, default_count, K1Item);
        update_default(k2, default_count, K2Item);

        if (default_count != 6)
            this->init(i1,i2,j1,j2,k1,k2);
    }


    void Box::reset()
    {
        this->init(0, this->grid.getNX() - 1 , 0, this->grid.getNY() - 1, 0, this->grid.getNZ() - 1);
    }


    void Box::init(int i1, int i2, int j1, int j2, int k1, int k2) {
        assert_dims(this->grid.getNX(), i1 , i2);
        assert_dims(this->grid.getNY(), j1 , j2);
        assert_dims(this->grid.getNZ(), k1 , k2);
        m_stride[0] = 1;
        m_stride[1] = this->grid.getNX();
        m_stride[2] = this->grid.getNX() * this->grid.getNY();

        m_dims[0] = (size_t) (i2 - i1 + 1);
        m_dims[1] = (size_t) (j2 - j1 + 1);
        m_dims[2] = (size_t) (k2 - k1 + 1);

        m_offset[0] = (size_t) i1;
        m_offset[1] = (size_t) j1;
        m_offset[2] = (size_t) k1;

        if (size() == this->grid.getCartesianSize())
            m_isGlobal = true;
        else
            m_isGlobal = false;

        initIndexList();
    }



    size_t Box::size() const {
        return m_dims[0] * m_dims[1] * m_dims[2];
    }


    bool Box::isGlobal() const {
        return m_isGlobal;
    }


    size_t Box::getDim(size_t idim) const {
        if (idim >= 3)
            throw std::invalid_argument("The input dimension value is invalid");

        return m_dims[idim];
    }



    const std::vector<Box::cell_index>& Box::index_list() const {
        return this->m_active_index_list;
    }

    const std::vector<Box::cell_index>& Box::global_index_list() const {
        return this->m_global_index_list;
    }



    void Box::initIndexList() {
        m_active_index_list.clear();
        m_global_index_list.clear();

        size_t ii,ij,ik;
        for (ik=0; ik < m_dims[2]; ik++) {
            size_t k = ik + m_offset[2];
            for (ij=0; ij < m_dims[1]; ij++) {
                size_t j = ij + m_offset[1];
                for (ii=0; ii < m_dims[0]; ii++) {
                    std::size_t i = ii + m_offset[0];
                    std::size_t global_index = i * m_stride[0] + j*m_stride[1] + k*m_stride[2];
                    std::size_t data_index = ii + ij*this->m_dims[0] + ik*this->m_dims[0]*this->m_dims[1];

                    if (this->grid.cellActive(global_index)) {
                        std::size_t active_index = this->grid.activeIndex(global_index);
                        m_active_index_list.emplace_back(global_index, active_index, data_index);
                    }

                    this->m_global_index_list.emplace_back( global_index, data_index );
                }
            }
        }
    }

    bool Box::equal(const Box& other) const {

        if (size() != other.size())
            return false;

        {
            for (size_t idim = 0; idim < 3; idim++) {
                if (m_dims[idim] != other.m_dims[idim])
                    return false;

                if (m_stride[idim] != other.m_stride[idim])
                    return false;

                if (m_offset[idim] != other.m_offset[idim])
                    return false;
            }
        }

        return true;
    }


    int Box::lower(int dim) const {
        return m_offset[dim];
    }

    int Box::upper(int dim) const {
        return m_offset[dim] + m_dims[dim] - 1;
    }

    int Box::I1() const {
        return lower(0);
    }

    int Box::I2() const {
        return upper(0);
    }

    int Box::J1() const {
        return lower(1);
    }

    int Box::J2() const {
        return upper(1);
    }

    int Box::K1() const {
        return lower(2);
    }

    int Box::K2() const {
        return upper(2);
    }

}
