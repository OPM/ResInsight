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


#ifndef BOXMANAGER_HPP_
#define BOXMANAGER_HPP_

#include <vector>
#include <memory>

#include <opm/input/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

/*
  This class implements a simple book keeping system for the current
  input box. In general there are three different input boxes which
  are relevant:

   1. The global box give by the complete dimensions of the grid.

   2. The input box given explicitly by the BOX keyword. That BOX will
       apply to all following FIELD properties, and it will continue
       to apply until either:

          - ENDBOX
          - A new BOX
          - End of current section

       is encountered.

   3. Some keywords allow for a Box which applies only to the elements
      of that keyword.

*/


namespace Opm {

    class BoxManager {
    public:
        BoxManager(const EclipseGrid& grid);

        void setInputBox( int i1,int i2 , int j1 , int j2 , int k1 , int k2);
        void setKeywordBox( int i1,int i2 , int j1 , int j2 , int k1 , int k2);

        void endSection();
        void endInputBox();
        void endKeyword();

        const Box& getActiveBox() const;
        const std::vector<Box::cell_index>& index_list() const;

    private:
        const EclipseGrid& grid;
        std::unique_ptr<Box> m_globalBox;
        std::unique_ptr<Box> m_inputBox;
        std::unique_ptr<Box> m_keywordBox;
    };
}


#endif
