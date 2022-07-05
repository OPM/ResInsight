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


#include <array>
#include <vector>
#include <string>

#include <opm/common/OpmLog/KeywordLocation.hpp>

namespace Opm {

class DeckKeyword;
class UnitSystem;

class VFPInjTable {
public:

    enum class FLO_TYPE {
        FLO_OIL=1,
        FLO_WAT,
        FLO_GAS,
    };


    VFPInjTable();
    VFPInjTable(const DeckKeyword& table, const UnitSystem& deck_unit_system);

    static VFPInjTable serializeObject();

    inline const KeywordLocation& location() const {
        return this->m_location;
    }

    inline int getTableNum() const {
        return m_table_num;
    }

    // The name() method is added to simplify serialization.
    inline int name() const {
        return m_table_num;
    }

    inline double getDatumDepth() const {
        return m_datum_depth;
    }

    inline FLO_TYPE getFloType() const {
        return m_flo_type;
    }

    inline const std::vector<double>& getFloAxis() const {
        return m_flo_data;
    }

    inline const std::vector<double>& getTHPAxis() const {
        return m_thp_data;
    }

    /**
     * Returns the data of the table itself. For ordered access
     * use operator()(thp_idx, flo_idx)
     *
     * This gives the bottom hole pressure value in the table for the coordinate
     * given by
     * flo_axis = getFloAxis();
     * thp_axis = getTHPAxis();
     *
     * flo_coord = flo_axis(flo_idx);
     * thp_coord = thp_axis(thp_idx);
     */
    inline const std::vector<double>& getTable() const {
        return m_data;
    }

    bool operator==(const VFPInjTable& data) const;

    std::array<size_t,2> shape() const;

    double operator()(size_t thp_idx, size_t flo_idx) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_table_num);
        serializer(m_datum_depth);
        serializer(m_flo_type);
        serializer(m_flo_data);
        serializer(m_thp_data);
        serializer(m_data);
        m_location.serializeOp(serializer);
    }

private:
    int m_table_num;
    double m_datum_depth;
    FLO_TYPE m_flo_type;

    std::vector<double> m_flo_data;
    std::vector<double> m_thp_data;


    std::vector<double> m_data;
    KeywordLocation m_location;

    void check();

    double& operator()(size_t thp_idx, size_t flo_idx);

    static FLO_TYPE getFloType(std::string flo_string);

    static void scaleValues(std::vector<double>& values,
                            const double& scaling_factor);

    static void convertFloToSI(const FLO_TYPE& type,
                            std::vector<double>& values,
                            const UnitSystem& unit_system);
    static void convertTHPToSI(std::vector<double>& values,
                            const UnitSystem& unit_system);
};


}


#endif
