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

#ifndef OPM_PARSER_ECLIPSE_ECLIPSESTATE_TABLES_VFPPRODTABLE_HPP_
#define OPM_PARSER_ECLIPSE_ECLIPSESTATE_TABLES_VFPPRODTABLE_HPP_


#include <boost/multi_array.hpp>

#include <opm/parser/eclipse/Parser/MessageContainer.hpp>

namespace Opm {

    class DeckItem;
    class DeckKeyword;
    class UnitSystem;

/**
 * Class for reading data from a VFPPROD (vertical flow performance production) table
 */
class VFPProdTable {
public:
    typedef boost::multi_array<double, 5> array_type;
    typedef boost::array<array_type::index, 5> extents;

    ///Rate type
    enum FLO_TYPE {
        FLO_OIL=1, //< Oil rate
        FLO_LIQ, //< Liquid rate
        FLO_GAS, //< Gas rate
        FLO_INVALID
    };

    ///Water fraction variable
    enum WFR_TYPE {
        WFR_WOR=11, //< Water-oil ratio
        WFR_WCT, //< Water cut
        WFR_WGR, //< Water-gas ratio
        WFR_INVALID
    };

    ///Gas fraction variable
    enum GFR_TYPE {
        GFR_GOR=21, //< Gas-oil ratio
        GFR_GLR, //< Gas-liquid ratio
        GFR_OGR, //< Oil-gas ratio
        GFR_INVALID
    };

    ///Artificial lift quantity
    enum ALQ_TYPE {
        ALQ_GRAT=31, //< Lift as injection rate
        ALQ_IGLR, //< Injection gas-liquid ratio
        ALQ_TGLR, //< Total gas-liquid ratio
        ALQ_PUMP, //< Pump rating
        ALQ_COMP, //< Compressor power
        ALQ_BEAN, //< Choke diameter
        ALQ_UNDEF, //< Undefined
        ALQ_INVALID
    };

    /**
     * Constructor
     */
    inline VFPProdTable() : m_table_num(-1),
            m_datum_depth(-1),
            m_flo_type(FLO_INVALID),
            m_wfr_type(WFR_INVALID),
            m_gfr_type(GFR_INVALID),
            m_alq_type(ALQ_INVALID) {

    }

    /**
     * Initializes objects from raw data. NOTE: All raw data assumed to be in SI units
     * @param table_num VFP table number
     * @param datum_depth Reference depth for BHP
     * @param flo_type Specifies what flo_data represents
     * @param wfr_type Specifies what wfr_data represents
     * @param gfr_type Specifies what gfr_data represents
     * @param alq_type Specifies what alq_data represents
     * @param flo_data Axis for flo_type
     * @param thp_data Axis for tubing head pressure
     * @param wfr_data Axis for wfr_type
     * @param gfr_data Axis for gfr_type
     * @param alq_data Axis for alq_type
     * @param data BHP to be interpolated. Given as a 5D array so that
     *        BHP = data[thp][wfr][gfr][alq][flo] for the indices thp, wfr, etc.
     */
    void init(int table_num,
            double datum_depth,
            FLO_TYPE flo_type,
            WFR_TYPE wfr_type,
            GFR_TYPE gfr_type,
            ALQ_TYPE alq_type,
            const std::vector<double>& flo_data,
            const std::vector<double>& thp_data,
            const std::vector<double>& wfr_data,
            const std::vector<double>& gfr_data,
            const std::vector<double>& alq_data,
            const array_type& data);

    /**
     * Constructor which parses a deck keyword and retrieves the relevant parts for a
     * VFP table.
     */
    void init( const DeckKeyword& table, const UnitSystem& deck_unit_system);

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
     * Returns the water fraction type for the WFR axis
     * @return water fraction type
     */
    inline WFR_TYPE getWFRType() const {
        return m_wfr_type;
    }

    /**
     * Returns the gas fraction type for the GFR axis
     * @return gas fraction type
     */
    inline GFR_TYPE getGFRType() const {
        return m_gfr_type;
    }

    /**
     * Returns the artificial lift quantity type for the ALQ axis
     * @return artificial lift quantity type
     */
    inline ALQ_TYPE getALQType() const {
        return m_alq_type;
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
     * Returns the coordinates for the water fraction sample points in the table
     * @return Water fraction coordinates
     */
    inline const std::vector<double>& getWFRAxis() const {
        return m_wfr_data;
    }

    /**
     * Returns the coordinates for the gas fraction sample points in the table
     * @return Gas fraction coordinates
     */
    inline const std::vector<double>& getGFRAxis() const {
        return m_gfr_data;
    }

    /**
     * Returns the coordinates for the artificial lift quantity points in the table
     * @return Artificial lift quantity coordinates
     */
    inline const std::vector<double>& getALQAxis() const {
        return m_alq_data;
    }

    /**
     * Returns the data of the table itself. The data is ordered so that
     *
     * table = getTable();
     * bhp = table[thp_idx][wfr_idx][gfr_idx][alq_idx][flo_idx];
     *
     * gives the bottom hole pressure value in the table for the coordinate
     * given by
     * flo_axis = getFloAxis();
     * thp_axis = getTHPAxis();
     * ...
     *
     * flo_coord = flo_axis(flo_idx);
     * thp_coord = thp_axis(thp_idx);
     * ...
     */
    inline const array_type& getTable() const {
        return m_data;
    }

    const MessageContainer& getMessageContainer() const;

private:

    //"Header" variables
    int m_table_num;
    double m_datum_depth;
    FLO_TYPE m_flo_type;
    WFR_TYPE m_wfr_type;
    GFR_TYPE m_gfr_type;
    ALQ_TYPE m_alq_type;

    //The actual table axes
    std::vector<double> m_flo_data;
    std::vector<double> m_thp_data;
    std::vector<double> m_wfr_data;
    std::vector<double> m_gfr_data;
    std::vector<double> m_alq_data;

    //The data itself, using the data ordering m_data[thp][wfr][gfr][alq][flo]
    array_type m_data;

    MessageContainer m_messages;
    /**
     * Debug function that runs a series of asserts to check for sanity of inputs.
     * Called after init to check that everything looks ok.
     */
    void check(const DeckKeyword& table, const double factor);

    static void scaleValues(std::vector<double>& values,
                            const double& scaling_factor);

    static void convertFloToSI(const FLO_TYPE& type,
                            std::vector<double>& values,
                            const UnitSystem& unit_system);
    static void convertTHPToSI(std::vector<double>& values,
                               const UnitSystem& unit_system);
    static void convertWFRToSI(const WFR_TYPE& type,
                               std::vector<double>& values,
                               const UnitSystem& unit_system);
    static void convertGFRToSI(const GFR_TYPE& type,
                               std::vector<double>& values,
                               const UnitSystem& unit_system);
    static void convertALQToSI(const ALQ_TYPE& type,
                               std::vector<double>& values,
                               const UnitSystem& unit_system);
};



} //Namespace opm


#endif /* OPM_PARSER_ECLIPSE_ECLIPSESTATE_TABLES_VFPPRODTABLE_HPP_ */
