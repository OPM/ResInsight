/*
  Copyright 2015 SINTEF ICT, Applied Mathematics.

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


#ifndef OPM_PARSER_ECLIPSE_ECLIPSESTATE_TABLES_VFPINJTABLE_HPP_
#define OPM_PARSER_ECLIPSE_ECLIPSESTATE_TABLES_VFPINJTABLE_HPP_

#include <opm/parser/eclipse/Deck/Deck.hpp>


#include <boost/multi_array.hpp>


namespace Opm {

    class DeckKeyword;

/**
 * Class for reading data from a VFPINJ (vertical flow performance injection) table
 */
class VFPInjTable {
public:
    typedef boost::multi_array<double, 2> array_type;
    typedef boost::array<array_type::index, 2> extents;

    ///Rate type
    enum FLO_TYPE {
        FLO_OIL=1, //< Oil rate
        FLO_WAT, //< Water rate
        FLO_GAS, //< Gas rate
        FLO_INVALID
    };

    /**
     * Constructor
     */
    inline VFPInjTable() : m_table_num(-1),
            m_datum_depth(-1),
            m_flo_type(FLO_INVALID) {

    }

    /**
         * Initializes objects from raw data. NOTE: All raw data assumed to be in SI units
         * @param table_num VFP table number
         * @param datum_depth Reference depth for BHP
         * @param flo_type Specifies what flo_data represents
         * @param flo_data Axis for flo_type
         * @param thp_data Axis for thp_type
         * @param data BHP to be interpolated. Given as a 2D array so that
         *        BHP = data[thp][flo] for the indices thp, flo.
         */
        void init(int table_num,
                double datum_depth,
                FLO_TYPE flo_type,
                const std::vector<double>& flo_data,
                const std::vector<double>& thp_data,
                const array_type& data);

        /**
         * Constructor which parses a deck keyword and retrieves the relevant parts for a
         * VFP table.
         */
        void init(const DeckKeyword& table, const UnitSystem& deck_unit_system);

        /**
         * Returns the table number
         * @return table number
         */
        inline int getTableNum() const {
            return m_table_num;
        }

        /**
         * Returns the datum depth for the table data
         * @return datum depth
         */
        inline double getDatumDepth() const {
            return m_datum_depth;
        }

        /**
         * Returns the rate/flo type for the flo axis
         * @return flo type
         */
        inline FLO_TYPE getFloType() const {
            return m_flo_type;
        }

        /**
         * Returns the coordinates of the FLO sample points in the table
         * @return Flo sample coordinates
         */
        inline const std::vector<double>& getFloAxis() const {
            return m_flo_data;
        }

        /**
         * Returns the coordinates for the tubing head pressure sample points in the table
         * @return Tubing head pressure coordinates
         */
        inline const std::vector<double>& getTHPAxis() const {
            return m_thp_data;
        }

        /**
         * Returns the data of the table itself. The data is ordered so that
         *
         * table = getTable();
         * bhp = table[thp_idx][flo_idx];
         *
         * gives the bottom hole pressure value in the table for the coordinate
         * given by
         * flo_axis = getFloAxis();
         * thp_axis = getTHPAxis();
         *
         * flo_coord = flo_axis(flo_idx);
         * thp_coord = thp_axis(thp_idx);
         */
        inline const array_type& getTable() const {
            return m_data;
        }
private:

    //"Header" variables
    int m_table_num;
    double m_datum_depth;
    FLO_TYPE m_flo_type;

    //The actual table axes
    std::vector<double> m_flo_data;
    std::vector<double> m_thp_data;

    //The data itself, using the data ordering m_data[thp][flo]
    array_type m_data;

    /**
     * Debug function that runs a series of asserts to check for sanity of inputs.
     * Called after init to check that everything looks ok.
     */
    void check();

    static FLO_TYPE getFloType(std::string flo_string);

    static void scaleValues(std::vector<double>& values,
                            const double& scaling_factor);

    static void convertFloToSI(const FLO_TYPE& type,
                            std::vector<double>& values,
                            const UnitSystem& unit_system);
    static void convertTHPToSI(std::vector<double>& values,
                            const UnitSystem& unit_system);
};


} //Namespace Opm


#endif /* OPM_PARSER_ECLIPSE_ECLIPSESTATE_TABLES_VFPINJTABLE_HPP_ */
