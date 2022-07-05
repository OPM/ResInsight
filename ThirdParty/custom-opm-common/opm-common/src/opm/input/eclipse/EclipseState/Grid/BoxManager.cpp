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

#include <opm/input/eclipse/EclipseState/Grid/BoxManager.hpp>


namespace Opm {

    BoxManager::BoxManager(const EclipseGrid& grid_arg) :
        grid( grid_arg ),
        m_globalBox( std::unique_ptr<Box>( new Box(grid_arg) ))
    {}


    const Box& BoxManager::getActiveBox() const {
        if (m_keywordBox)
            return *m_keywordBox;

        if (m_inputBox)
            return *m_inputBox;

        return *m_globalBox;
    }


    void BoxManager::setInputBox( int i1,int i2 , int j1 , int j2 , int k1 , int k2) {
        this->m_inputBox.reset(new Box( this->grid, i1, i2, j1, j2, k1, k2 ));
    }

    void BoxManager::endInputBox() {
        if(m_keywordBox)
            throw std::invalid_argument("Hmmm - this seems like an internal error - the SECTION is terminated with an active keyword box");

        m_inputBox.reset( 0 );
    }

    void BoxManager::endSection() {
        endInputBox();
    }

    void BoxManager::setKeywordBox( int i1,int i2 , int j1 , int j2 , int k1 , int k2) {
        this->m_keywordBox.reset( new Box( this->grid, i1, i2, j1, j2, k1, k2 ));
    }

    void BoxManager::endKeyword() {
        this->m_keywordBox.reset( 0 );
    }

    const std::vector<Box::cell_index>& BoxManager::index_list() const {
        return this->getActiveBox().index_list();
    }
}
