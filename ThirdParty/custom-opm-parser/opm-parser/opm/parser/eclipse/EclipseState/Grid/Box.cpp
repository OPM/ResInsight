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

#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>

namespace Opm {

    Box::Box(int nx , int ny , int nz) {
        if (nx <= 0)
            throw std::invalid_argument("The input nx value is invalid");

        if (ny <= 0)
            throw std::invalid_argument("The input ny value is invalid");

        if (nz <= 0)
            throw std::invalid_argument("The input nz value is invalid");

        m_dims[0] = (size_t) nx;
        m_dims[1] = (size_t) ny;
        m_dims[2] = (size_t) nz;

        m_offset[0] = 0;
        m_offset[1] = 0;
        m_offset[2] = 0;

        m_stride[0] = 1;
        m_stride[1] = m_dims[0];
        m_stride[2] = m_dims[0] * m_dims[1];

        m_isGlobal = true;
        initIndexList();
    }


    Box::Box(const Box& globalBox , int i1 , int i2 , int j1 , int j2 , int k1 , int k2) {
        assertDims(globalBox , 0 , i1 , i2);
        assertDims(globalBox , 1 , j1 , j2);
        assertDims(globalBox , 2 , k1 , k2);

        m_dims[0] = (size_t) (i2 - i1 + 1);
        m_dims[1] = (size_t) (j2 - j1 + 1);
        m_dims[2] = (size_t) (k2 - k1 + 1);

        m_offset[0] = (size_t) i1;
        m_offset[1] = (size_t) j1;
        m_offset[2] = (size_t) k1;

        m_stride[0] = 1;
        m_stride[1] = globalBox.getDim(0);
        m_stride[2] = globalBox.getDim(0) * globalBox.getDim(1);

        if (size() == globalBox.size())
            m_isGlobal = true;
        else
            m_isGlobal = false;

        initIndexList();
    }


    void Box::assertDims(const Box& globalBox, size_t idim , int l1 , int l2) {
        if ((l1 < 0) || (l2 < 0) || (l1 > l2))
            throw std::invalid_argument("Invalid index values for sub box");

        if ((size_t) l2 >= globalBox.getDim(idim))
            throw std::invalid_argument("Invalid index values for sub box");
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


    const std::vector<size_t>& Box::getIndexList() const {
        return m_indexList;
    }


    void Box::initIndexList() {
        m_indexList.resize( size() );

        size_t ii,ij,ik;
        size_t l = 0;

        for (ik=0; ik < m_dims[2]; ik++) {
            size_t k = ik + m_offset[2];
            for (ij=0; ij < m_dims[1]; ij++) {
                size_t j = ij + m_offset[1];
                for (ii=0; ii < m_dims[0]; ii++) {
                    size_t i = ii + m_offset[0];
                    size_t g = i * m_stride[0] + j*m_stride[1] + k*m_stride[2];

                    m_indexList[l] = g;
                    l++;
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

}

