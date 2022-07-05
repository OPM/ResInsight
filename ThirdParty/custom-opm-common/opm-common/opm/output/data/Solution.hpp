/*
  Copyright 2016 Statoil ASA.

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

#ifndef OPM_OUTPUT_DATA_SOLUTION_HPP
#define OPM_OUTPUT_DATA_SOLUTION_HPP

#include <string>
#include <map>

#include <opm/output/data/Cells.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

namespace Opm {
namespace data {

class Solution : public std::map< std::string, data::CellData > {
    using Base = std::map< std::string, data::CellData >;

    public:
        Solution() = default;
        explicit Solution( bool si );
        using Base::map;
        using Base::insert;

        bool has( const std::string& ) const;

        /*
         * Get the data field of the struct matching the requested key. Will
         * throw std::out_of_range if they key does not exist.
         */
        std::vector< double >& data(const std::string& );
        const std::vector< double >& data(const std::string& ) const;

        std::pair< iterator, bool > insert( std::string name,
                                            UnitSystem::measure,
                                            std::vector< double >,
                                            TargetType );

        void convertToSI( const UnitSystem& );
        void convertFromSI( const UnitSystem& );

    private:
        bool si = true;
};

}
}

#endif
