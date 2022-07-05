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


#include <array>
#include <vector>
#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>

namespace Opm {

    class DeckItem;
    class DeckKeyword;
    class UnitSystem;

/**
 * Class for reading data from a VFPPROD (vertical flow performance production) table
 */
class VFPProdTable {
public:

    enum class FLO_TYPE {
        FLO_OIL=1,
        FLO_LIQ,
        FLO_GAS
    };

    enum class WFR_TYPE {
        WFR_WOR=11,
        WFR_WCT,
        WFR_WGR
    };

    enum class GFR_TYPE {
        GFR_GOR=21,
        GFR_GLR,
        GFR_OGR
    };

    enum class ALQ_TYPE {
        ALQ_GRAT=31,
        ALQ_IGLR,
        ALQ_TGLR,
        ALQ_PUMP,
        ALQ_COMP,
        ALQ_BEAN,
        ALQ_UNDEF
    };

    VFPProdTable();
    VFPProdTable( const DeckKeyword& table, bool gaslift_opt_active, const UnitSystem& deck_unit_system);
    VFPProdTable(int table_num,
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
                 const std::vector<double>& data);
    static Dimension ALQDimension(const ALQ_TYPE& alq_type, const UnitSystem& unit_system);

    static VFPProdTable serializeObject();

    inline int getTableNum() const {
        return m_table_num;
    }

    inline const KeywordLocation& location() const {
        return this->m_location;
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

    inline WFR_TYPE getWFRType() const {
        return m_wfr_type;
    }

    inline GFR_TYPE getGFRType() const {
        return m_gfr_type;
    }

    inline ALQ_TYPE getALQType() const {
        return m_alq_type;
    }

    inline const std::vector<double>& getFloAxis() const {
        return m_flo_data;
    }

    inline const std::vector<double>& getTHPAxis() const {
        return m_thp_data;
    }

    inline const std::vector<double>& getWFRAxis() const {
        return m_wfr_data;
    }

    inline const std::vector<double>& getGFRAxis() const {
        return m_gfr_data;
    }

    inline const std::vector<double>& getALQAxis() const {
        return m_alq_data;
    }

    /**
     * Returns the data of the table itself. For ordered access
     * use operator()(thp_idx, wfr_idx, gfr_idx, alq_idx, flo_idx)
     *
     * This gives the bottom hole pressure value in the table for the coordinate
     * given by
     * flo_axis = getFloAxis();
     * thp_axis = getTHPAxis();
     * ...
     *
     * flo_coord = flo_axis(flo_idx);
     * thp_coord = thp_axis(thp_idx);
     * ...
     */
    inline const std::vector<double>& getTable() const {
        return m_data;
    }

    bool operator==(const VFPProdTable& data) const;

    std::array<size_t,5> shape() const;

    double operator()(size_t thp_idx, size_t wfr_idx, size_t gfr_idx, size_t alq_idx, size_t flo_idx) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_table_num);
        serializer(m_datum_depth);
        serializer(m_flo_type);
        serializer(m_wfr_type);
        serializer(m_gfr_type);
        serializer(m_alq_type);
        serializer(m_flo_data);
        serializer(m_thp_data);
        serializer(m_wfr_data);
        serializer(m_gfr_data);
        serializer(m_alq_data);
        serializer(m_data);
        m_location.serializeOp(serializer);
    }

private:
    int m_table_num;
    double m_datum_depth;
    FLO_TYPE m_flo_type;
    WFR_TYPE m_wfr_type;
    GFR_TYPE m_gfr_type;
    ALQ_TYPE m_alq_type;

    std::vector<double> m_flo_data;
    std::vector<double> m_thp_data;
    std::vector<double> m_wfr_data;
    std::vector<double> m_gfr_data;
    std::vector<double> m_alq_data;

    std::vector<double> m_data;
    KeywordLocation m_location;

    void check();

    double& operator()(size_t thp_idx, size_t wfr_idx, size_t gfr_idx, size_t alq_idx, size_t flo_idx);

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
    static void convertAlqToSI(const ALQ_TYPE& type,
                               std::vector<double>& values,
                               const UnitSystem& unit_system);
};



}


#endif
