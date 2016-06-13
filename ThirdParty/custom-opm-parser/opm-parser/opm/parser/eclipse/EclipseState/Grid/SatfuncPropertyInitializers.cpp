/*
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <array>

#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/SatfuncPropertyInitializers.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SlgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Sof3Table.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Tabdims.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableContainer.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/Utility/Functional.hpp>

namespace Opm {

    /*
     * See the "Saturation Functions" chapter in the Eclipse Technical
     * Description; there are several alternative families of keywords which
     * can be used to enter relperm and capillary pressure tables.
     *
     * If SWOF and SGOF are specified in the deck it return I
     * If SWFN, SGFN and SOF3 are specified in the deck it return II
     * If keywords are missing or mixed, an error is given.
     */
    enum class SatfuncFamily { none = 0, I = 1, II = 2 };

    static SatfuncFamily getSaturationFunctionFamily( const TableManager* tm ) {
        const TableContainer& swofTables = tm->getSwofTables();
        const TableContainer& sgofTables = tm->getSgofTables();
        const TableContainer& slgofTables = tm->getSlgofTables();
        const TableContainer& sof3Tables = tm->getSof3Tables();
        const TableContainer& swfnTables = tm->getSwfnTables();
        const TableContainer& sgfnTables = tm->getSgfnTables();


        bool family1 = (!sgofTables.empty() || !slgofTables.empty()) && !swofTables.empty();
        bool family2 = !swfnTables.empty() && !sgfnTables.empty() && !sof3Tables.empty();

        if (family1 && family2) {
            throw std::invalid_argument("Saturation families should not be mixed \n"
                                        "Use either SGOF (or SLGOF) and SWOF or SGFN, SWFN and SOF3");
        }

        if (!family1 && !family2) {
            throw std::invalid_argument("Saturations function must be specified using either "
                                        "family 1 or family 2 keywords \n"
                                        "Use either SGOF (or SLGOF) and SWOF or SGFN, SWFN and SOF3" );
        }

        if( family1 ) return SatfuncFamily::I;
        if( family2 ) return SatfuncFamily::II;
        return SatfuncFamily::none;
    }

    enum class limit { min, max };

    static std::vector< double > findMinWaterSaturation( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& swfnTables = tm->getSwfnTables();

        const auto famI = [&swofTables]( int i ) {
            return swofTables.getTable< SwofTable >( i ).getSwColumn().front();
        };

        const auto famII = [&swfnTables]( int i ) {
            return swfnTables.getTable< SwfnTable >( i ).getSwColumn().front();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I: return map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II: return map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findMaxWaterSaturation( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& swfnTables = tm->getSwfnTables();

        const auto famI = [&swofTables]( int i ) {
            return swofTables.getTable< SwofTable >( i ).getSwColumn().back();
        };

        const auto famII = [&swfnTables]( int i ) {
            return swfnTables.getTable< SwfnTable >( i ).getSwColumn().back();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I: return map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II: return map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findMinGasSaturation( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgofTables  = tm->getSgofTables();
        const auto& slgofTables = tm->getSlgofTables();
        const auto& sgfnTables = tm->getSgfnTables();

        const auto famI_sgof = [&sgofTables]( int i ) {
            return sgofTables.getTable< SgofTable >( i ).getSgColumn().front();
        };

        const auto famI_slgof = [&slgofTables]( int i ) {
            return 1.0 - slgofTables.getTable< SlgofTable >( i ).getSlColumn().back();
        };

        const auto famII = [&sgfnTables]( int i ) {
            return sgfnTables.getTable< SgfnTable >( i ).getSgColumn().front();
        };


        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:

                if( sgofTables.empty() && slgofTables.empty() )
                    throw std::runtime_error( "Saturation keyword family I requires either sgof or slgof non-empty" );

                if( !sgofTables.empty() )
                    return fun::map( famI_sgof, fun::iota( num_tables ) );
                else
                    return fun::map( famI_slgof, fun::iota( num_tables ) );

            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );

            default:
                throw std::domain_error("No valid saturation keyword family specified");

        }

    }

    static std::vector< double > findMaxGasSaturation( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgofTables  = tm->getSgofTables();
        const auto& slgofTables = tm->getSlgofTables();
        const auto& sgfnTables = tm->getSgfnTables();

        const auto famI_sgof = [&sgofTables]( int i ) {
            return sgofTables.getTable< SgofTable >( i ).getSgColumn().back();
        };

        const auto famI_slgof = [&slgofTables]( int i ) {
            return 1.0 - slgofTables.getTable< SlgofTable >( i ).getSlColumn().front();
        };

        const auto famII = [&sgfnTables]( int i ) {
            return sgfnTables.getTable< SgfnTable >( i ).getSgColumn().back();
        };


        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                if( sgofTables.empty() && slgofTables.empty() )
                    throw std::runtime_error( "Saturation keyword family I requires either sgof or slgof non-empty" );

                if( !sgofTables.empty() )
                    return fun::map( famI_sgof, fun::iota( num_tables ) );
                else
                    return fun::map( famI_slgof, fun::iota( num_tables ) );

            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );

            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    /*
     * These functions have been ported from an older implementation to instead
     * use std::upper_bound and more from <algorithm> to make code -intent-
     * clearer. This also made some (maybe intentional) details easier to spot.
     * A short discussion:
     *
     * I don't know if not finding any element larger than 0.0 in the tables
     * was ever supposed to happen (or even possible), in which case the vector
     * elements remained at their initial value of 0.0. This behaviour has been
     * preserved, but is now explicit. The original code was also not clear if
     * it was possible to look up columns at index -1 (see critical_water for
     * an example), but the new version is explicit about this. Unfortuately
     * I'm not familiar enough with the maths or internal structure to make
     * more than a guess here, but most of this behaviour should be preserved.
     *
     */

    template< typename T >
    static inline double critical_water( const T& table ) {

        const auto& col = table.getKrwColumn();
        const auto end = col.begin() + table.numRows();
        const auto critical = std::upper_bound( col.begin(), end, 0.0 );
        const auto index = std::distance( col.begin(), critical );

        if( index == 0 || critical == end ) return 0.0;

        return table.getSwColumn()[ index - 1 ];
    }

    static std::vector< double > findCriticalWater( const TableManager* tm ) {

        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& swfnTables = tm->getSwfnTables();

        const auto famI = [&swofTables]( int i ) {
            return critical_water( swofTables.getTable< SwofTable >( i ) );
        };

        const auto famII = [&swfnTables]( int i ) {
            return critical_water( swfnTables.getTable< SwfnTable >( i ) );
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I: return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II: return fun::map( famII, fun::iota( num_tables ) );
            default: throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    template< typename T >
    static inline double critical_gas( const T& table ) {
        const auto& col = table.getKrgColumn();
        const auto end = col.begin() + table.numRows();
        const auto critical = std::upper_bound( col.begin(), end, 0.0 );
        const auto index = std::distance( col.begin(), critical );

        if( index == 0 || critical == end ) return 0.0;

        return table.getSgColumn()[ index - 1 ];
    }

    static inline double critical_gas( const SlgofTable& slgofTable ) {
        const auto& col = slgofTable.getKrgColumn();
        const auto critical = std::upper_bound( col.begin(), col.end(), 0.0 );
        const auto index = std::distance( col.begin(), critical );

        if( index == 0 || critical == col.end() ) return 0.0;

        return slgofTable.getSlColumn()[ index - 1 ];
    }

    static std::vector< double > findCriticalGas( const TableManager* tm ) {

        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgfnTables = tm->getSgfnTables();
        const auto& sgofTables = tm->getSgofTables();
        const auto& slgofTables = tm->getSlgofTables();

        const auto famI_sgof = [&sgofTables]( int i ) {
            return critical_gas( sgofTables.getTable< SgofTable >( i ) );
        };

        const auto famI_slgof = [&slgofTables]( int i ) {
            return critical_gas( slgofTables.getTable< SlgofTable >( i ) );
        };

        const auto famII = [&sgfnTables]( int i ) {
            return critical_gas( sgfnTables.getTable< SgfnTable >( i ) );
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                if( sgofTables.empty() && slgofTables.empty() )
                    throw std::runtime_error( "Saturation keyword family I requires either sgof or slgof non-empty" );

                if( !sgofTables.empty() )
                    return fun::map( famI_sgof, fun::iota( num_tables ) );
                else
                    return fun::map( famI_slgof, fun::iota( num_tables ) );

            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );

            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static inline double critical_oil_water( const SwofTable& swofTable ) {
        const auto& col = swofTable.getKrowColumn();

        using reverse = std::reverse_iterator< decltype( col.begin() ) >;
        auto rbegin = reverse( col.begin() + swofTable.numRows() );
        auto rend = reverse( col.begin() );
        const auto critical = std::upper_bound( rbegin, rend, 0.0 );
        const auto index = std::distance( col.begin(), critical.base() - 1 );

        if( critical == rend ) return 0.0;

        return 1 - swofTable.getSwColumn()[ index + 1 ];
    }

    static inline double critical_oil( const Sof3Table& sof3Table, const TableColumn& col ) {
        const auto critical = std::upper_bound( col.begin(), col.end(), 0.0 );
        const auto index = std::distance( col.begin(), critical );

        if( index == 0 || critical == col.end() ) return 0.0;

        return sof3Table.getSoColumn()[ index - 1 ];
    }

    static std::vector< double > findCriticalOilWater( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& sof3Tables= tm->getSof3Tables();

        const auto famI = [&swofTables]( int i ) {
            return critical_oil_water( swofTables.getTable< SwofTable >( i ) );
        };

        const auto famII = [&sof3Tables]( int i ) {
            const auto& tb = sof3Tables.getTable< Sof3Table >( i );
            return critical_oil( tb, tb.getKrowColumn() );
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I: return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II: return fun::map( famII, fun::iota( num_tables ) );
            default: throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static inline double critical_oil_gas( const SgofTable& sgofTable ) {
        const auto& col = sgofTable.getKrogColumn();

        using reverse = std::reverse_iterator< decltype( col.begin() ) >;
        auto rbegin = reverse( col.begin() + sgofTable.numRows() );
        auto rend = reverse( col.begin() );
        const auto critical = std::upper_bound( rbegin, rend, 0.0 );
        const auto index = std::distance( col.begin(), critical.base() - 1 );

        if( critical == rend ) return 0.0;

        return 1 - sgofTable.getSgColumn()[ index + 1 ];
    }

    static inline double critical_oil_gas( const SlgofTable& sgofTable ) {

        const auto& col = sgofTable.getKrogColumn();
        using reverse = std::reverse_iterator< decltype( col.begin() ) >;
        auto rbegin = reverse( col.begin() + sgofTable.numRows() );
        auto rend = reverse( col.begin() );
        const auto critical = std::upper_bound( rbegin, rend, 0.0 );
        const auto index = std::distance( col.begin(), critical.base() - 1 );

        if( critical == rend ) return 0.0;

        return 1 - sgofTable.getSlColumn()[ index + 1 ];
    }


    static std::vector< double > findCriticalOilGas( const TableManager* tm ) {

        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgofTables = tm->getSgofTables();
        const auto& slgofTables = tm->getSlgofTables();
        const auto& sof3Tables = tm->getSof3Tables();

        const auto famI_sgof = [&sgofTables]( int i ) {
            return critical_oil_gas( sgofTables.getTable< SgofTable >( i ) );
        };

        const auto famI_slgof = [&slgofTables]( int i ) {
            return critical_oil_gas( slgofTables.getTable< SlgofTable >( i ) );
        };

        const auto famII = [&sof3Tables]( int i ) {
            const auto& tb = sof3Tables.getTable< Sof3Table >( i );
            return critical_oil( tb, tb.getKrogColumn() );
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                if( sgofTables.empty() && slgofTables.empty() )
                    throw std::runtime_error( "Saturation keyword family I requires either sgof or slgof non-empty" );

                if( !sgofTables.empty() )
                    return fun::map( famI_sgof, fun::iota( num_tables ) );
                else
                    return fun::map( famI_slgof, fun::iota( num_tables ) );

            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );

            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findMaxKrg( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgofTables = tm->getSgofTables();
        const auto& sgfnTables = tm->getSgfnTables();

        const auto& famI = [&sgofTables]( int i ) {
            return sgofTables.getTable< SgofTable >( i ).getKrgColumn().back();
        };

        const auto& famII = [&sgfnTables]( int i ) {
            return sgfnTables.getTable< SgfnTable >( i ).getKrgColumn().back();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findKrgr( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgofTables = tm->getSgofTables();
        const auto& sgfnTables = tm->getSgfnTables();

        const auto& famI = [&sgofTables]( int i ) {
            return sgofTables.getTable< SgofTable >( i ).getKrgColumn().front();
        };

        const auto& famII = [&sgfnTables]( int i ) {
            return sgfnTables.getTable< SgfnTable >( i ).getKrgColumn().back();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findKrwr( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& swfnTables = tm->getSwfnTables();

        const auto& famI = [&swofTables]( int i ) {
            return swofTables.getTable< SwofTable >( i ).getKrwColumn().front();
        };

        const auto& famII = [&swfnTables]( int i ) {
            return swfnTables.getTable< SwfnTable >( i ).getKrwColumn().front();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findKrorw( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& sof3Tables = tm->getSof3Tables();

        const auto& famI = [&swofTables]( int i ) {
            const auto& swofTable = swofTables.getTable< SwofTable >( i );
            const auto& krwCol = swofTable.getKrwColumn();
            const auto crit = std::upper_bound( krwCol.begin(), krwCol.end(), 0.0 );
            const auto index = std::distance( krwCol.begin(), crit );

            if( crit == krwCol.end() ) return 0.0;

            return swofTable.getKrowColumn()[ index - 1 ];
        };

        const auto crit_water = findCriticalWater( tm );
        const auto min_gas = findMinGasSaturation( tm );
        const auto& famII = [&sof3Tables,&crit_water,&min_gas]( int i ) {
            const double OilSatAtcritialWaterSat = 1.0 - crit_water[ i ] - min_gas[ i ];
            return sof3Tables.getTable< Sof3Table >( i )
                .evaluate("KROW", OilSatAtcritialWaterSat);
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findKrorg( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgofTables = tm->getSgofTables();
        const auto& sof3Tables = tm->getSof3Tables();

        const auto& famI = [&sgofTables]( int i ) {
            const auto& sgofTable = sgofTables.getTable< SgofTable >( i );
            const auto& krgCol = sgofTable.getKrgColumn();
            const auto crit = std::upper_bound( krgCol.begin(), krgCol.end(), 0.0 );
            const auto index = std::distance( krgCol.begin(), crit );

            if( crit == krgCol.end() ) return 0.0;

            return sgofTable.getKrogColumn()[ index - 1 ];
        };

        const auto crit_gas = findCriticalGas( tm );
        const auto min_water = findMinWaterSaturation( tm );
        const auto& famII = [&sof3Tables,&crit_gas,&min_water]( int i ) {
            const double OilSatAtcritialGasSat = 1.0 - crit_gas[ i ] - min_water[ i ];
            return sof3Tables.getTable< Sof3Table >( i )
                .evaluate("KROG", OilSatAtcritialGasSat);
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    /* find the maximum output values of the water-oil system. the maximum oil
     * relperm is possibly wrong because we have two oil relperms in a threephase
     * system. the documentation is very ambiguos here, though: it says that the
     * oil relperm at the maximum oil saturation is scaled according to maximum
     * specified the KRO keyword. the first part of the statement points at
     * scaling the resultant threephase oil relperm, but then the gas saturation
     * is not taken into account which means that some twophase quantity must be
     * scaled.
     */
    static std::vector< double > findMaxPcog( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& sgofTables = tm->getSgofTables();
        const auto& sgfnTables = tm->getSgfnTables();

        const auto& famI = [&sgofTables]( int i ) {
            return sgofTables.getTable< SgofTable >( i ).getPcogColumn().front();
        };

        const auto& famII = [&sgfnTables]( int i ) {
            return sgfnTables.getTable< SgfnTable >( i ).getPcogColumn().back();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findMaxPcow( const TableManager* tm ) {
        const auto num_tables  = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& swfnTables = tm->getSwfnTables();

        const auto& famI = [&swofTables]( int i ) {
            return swofTables.getTable< SwofTable >( i ).getPcowColumn().front();
        };

        const auto& famII = [&swfnTables]( int i ) {
            return swfnTables.getTable< SwfnTable >( i ).getPcowColumn().front();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findMaxKro( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& sof3Tables = tm->getSof3Tables();

        const auto& famI = [&swofTables]( int i ) {
            return swofTables.getTable< SwofTable >( i ).getKrowColumn().front();
        };

        const auto& famII = [&sof3Tables]( int i ) {
            return sof3Tables.getTable< Sof3Table >( i ).getKrowColumn().back();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static std::vector< double > findMaxKrw( const TableManager* tm ) {
        const auto num_tables = tm->getTabdims()->getNumSatTables();
        const auto& swofTables = tm->getSwofTables();
        const auto& swfnTables = tm->getSwfnTables();

        const auto& famI = [&swofTables]( int i ) {
            return swofTables.getTable< SwofTable >( i ).getKrwColumn().back();
        };

        const auto& famII = [&swfnTables]( int i ) {
            return swfnTables.getTable< SwfnTable >( i ).getKrwColumn().back();
        };

        switch( getSaturationFunctionFamily( tm ) ) {
            case SatfuncFamily::I:
                return fun::map( famI, fun::iota( num_tables ) );
            case SatfuncFamily::II:
                return fun::map( famII, fun::iota( num_tables ) );
            default:
                throw std::domain_error("No valid saturation keyword family specified");
        }
    }

    static double selectValue( const TableContainer& depthTables,
                               int tableIdx,
                               const std::string& columnName,
                               double cellDepth,
                               double fallbackValue,
                               bool useOneMinusTableValue) {

        if( tableIdx < 0 ) return fallbackValue;

        const auto& table = depthTables.getTable( tableIdx );

        if( tableIdx >= int( depthTables.size() ) )
            throw std::invalid_argument("Not enough tables!");

        // evaluate the table at the cell depth
        const double value = table.evaluate( columnName, cellDepth );

        // a column can be fully defaulted. In this case, eval() returns a NaN
        // and we have to use the data from saturation tables
        if( !std::isfinite( value ) ) return fallbackValue;
        if( useOneMinusTableValue ) return 1 - value;
        return value;
    }

    static std::vector< double > satnumApply( size_t size,
                                              const std::string& columnName,
                                              const std::vector< double >& fallbackValues,
                                              const TableManager* tableManager,
                                              const EclipseGrid* eclipseGrid,
                                              GridProperties<int>* intGridProperties,
                                              bool useOneMinusTableValue ) {


        std::vector< double > values( size, 0 );
        auto tabdims = tableManager->getTabdims();

        auto& satnum = intGridProperties->getKeyword("SATNUM");
        auto& endnum = intGridProperties->getKeyword("ENDNUM");
        int numSatTables = tabdims->getNumSatTables();

        satnum.checkLimits( 1 , numSatTables );

        // All table lookup assumes three-phase model
        assert( tableManager->getNumPhases() == 3 );

        // acctually assign the defaults. if the ENPVD keyword was specified in the deck,
        // this currently cannot be done because we would need the Z-coordinate of the
        // cell and we would need to know how the simulator wants to interpolate between
        // sampling points. Both of these are outside the scope of opm-parser, so we just
        // assign a NaN in this case...
        const bool useEnptvd = tableManager->useEnptvd();
        const auto& enptvdTables = tableManager->getEnptvdTables();

        for( size_t cellIdx = 0; cellIdx < eclipseGrid->getCartesianSize(); cellIdx++ ) {
            int satTableIdx = satnum.iget( cellIdx ) - 1;
            int endNum = endnum.iget( cellIdx ) - 1;
            double cellDepth = std::get< 2 >( eclipseGrid->getCellCenter( cellIdx ) );


            values[cellIdx] = selectValue(enptvdTables,
                                        (useEnptvd && endNum >= 0) ? endNum : -1,
                                        columnName,
                                        cellDepth,
                                        fallbackValues[ satTableIdx ],
                                        useOneMinusTableValue);
        }

        return values;
    }

    static std::vector< double > imbnumApply( size_t size,
                                              const std::string& columnName,
                                              const std::vector< double >& fallBackValues,
                                              const TableManager* tableManager,
                                              const EclipseGrid* eclipseGrid,
                                              GridProperties<int>* intGridProperties,
                                              bool useOneMinusTableValue ) {

        std::vector< double > values( size, 0 );

        auto& imbnum = intGridProperties->getKeyword("IMBNUM");
        auto& endnum = intGridProperties->getKeyword("ENDNUM");

        auto tabdims = tableManager->getTabdims();
        const int numSatTables = tabdims->getNumSatTables();

        imbnum.checkLimits( 1 , numSatTables );
        // acctually assign the defaults. if the ENPVD keyword was specified in the deck,
        // this currently cannot be done because we would need the Z-coordinate of the
        // cell and we would need to know how the simulator wants to interpolate between
        // sampling points. Both of these are outside the scope of opm-parser, so we just
        // assign a NaN in this case...
        const bool useImptvd = tableManager->useImptvd();
        const TableContainer& imptvdTables = tableManager->getImptvdTables();
        for( size_t cellIdx = 0; cellIdx < eclipseGrid->getCartesianSize(); cellIdx++ ) {
            int imbTableIdx = imbnum.iget( cellIdx ) - 1;
            int endNum = endnum.iget( cellIdx ) - 1;
            double cellDepth = std::get< 2 >( eclipseGrid->getCellCenter( cellIdx ) );

            values[cellIdx] = selectValue(imptvdTables,
                                                (useImptvd && endNum >= 0) ? endNum : -1,
                                                columnName,
                                                cellDepth,
                                                fallBackValues[imbTableIdx],
                                                useOneMinusTableValue);
        }

        return values;
    }

    std::vector< double > SGLEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid* eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto min_gas = findMinGasSaturation( tableManager );
        return satnumApply( size, "SGCO", min_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > ISGLEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid* eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto min_gas = findMinGasSaturation( tableManager );
        return imbnumApply( size, "SGCO", min_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > SGUEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid* eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto max_gas = findMaxGasSaturation( tableManager );
        return satnumApply( size, "SGMAX", max_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > ISGUEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid* eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto max_gas = findMaxGasSaturation( tableManager );
        return imbnumApply( size, "SGMAX", max_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > SWLEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid* eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto min_water = findMinWaterSaturation( tableManager );
        return satnumApply( size, "SWCO", min_water, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > ISWLEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto min_water = findMinWaterSaturation( tableManager );
        return imbnumApply( size, "SWCO", min_water, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > SWUEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid  * eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto max_water = findMaxWaterSaturation( tableManager );
        return satnumApply( size, "SWMAX", max_water, tableManager, eclipseGrid,
                            intGridProperties, true );
    }

    std::vector< double > ISWUEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto max_water = findMaxWaterSaturation( tableManager );
        return imbnumApply( size, "SWMAX", max_water, tableManager, eclipseGrid,
                            intGridProperties, true);
    }

    std::vector< double > SGCREndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto crit_gas = findCriticalGas( tableManager );
        return satnumApply( size, "SGCRIT", crit_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > ISGCREndpoint( size_t size,
                                         const TableManager * tableManager,
                                         const EclipseGrid  * eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto crit_gas = findCriticalGas( tableManager );
        return imbnumApply( size, "SGCRIT", crit_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > SOWCREndpoint( size_t size,
                                         const TableManager * tableManager,
                                         const EclipseGrid  * eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto oil_water = findCriticalOilWater( tableManager );
        return satnumApply( size, "SOWCRIT", oil_water, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > ISOWCREndpoint( size_t size,
                                          const TableManager * tableManager,
                                          const EclipseGrid  * eclipseGrid,
                                          GridProperties<int>* intGridProperties )
    {
        const auto oil_water = findCriticalOilWater( tableManager );
        return imbnumApply( size, "SOWCRIT", oil_water, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > SOGCREndpoint( size_t size,
                                         const TableManager * tableManager,
                                         const EclipseGrid  * eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto crit_oil_gas = findCriticalOilGas( tableManager );
        return satnumApply( size, "SOGCRIT", crit_oil_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > ISOGCREndpoint( size_t size,
                                          const TableManager * tableManager,
                                          const EclipseGrid  * eclipseGrid,
                                          GridProperties<int>* intGridProperties )
    {
        const auto crit_oil_gas = findCriticalOilGas( tableManager );
        return imbnumApply( size, "SOGCRIT", crit_oil_gas, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > SWCREndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto crit_water = findCriticalWater( tableManager );
        return satnumApply( size, "SWCRIT", crit_water, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > ISWCREndpoint( size_t size,
                                         const TableManager * tableManager,
                                         const EclipseGrid  * eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto crit_water = findCriticalWater( tableManager );
        return imbnumApply( size, "SWCRIT", crit_water, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > PCWEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid  * eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto max_pcow = findMaxPcow( tableManager );
        return satnumApply( size, "PCW", max_pcow, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IPCWEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto max_pcow = findMaxPcow( tableManager );
        return imbnumApply( size, "IPCW", max_pcow, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > PCGEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid  * eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto max_pcog = findMaxPcog( tableManager );
        return satnumApply( size, "PCG", max_pcog, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IPCGEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto max_pcog = findMaxPcog( tableManager );
        return imbnumApply( size, "IPCG", max_pcog, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > KRWEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid  * eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto max_krw = findMaxKrw( tableManager );
        return satnumApply( size, "KRW", max_krw, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IKRWEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto krwr = findKrwr( tableManager );
        return imbnumApply( size, "IKRW", krwr, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > KRWREndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto krwr = findKrwr( tableManager );
        return satnumApply( size, "KRWR", krwr, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IKRWREndpoint( size_t size,
                                         const TableManager * tableManager,
                                         const EclipseGrid  * eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto krwr = findKrwr( tableManager );
        return imbnumApply( size, "IKRWR", krwr, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > KROEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid  * eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto max_kro = findMaxKro( tableManager );
        return satnumApply( size, "KRO", max_kro, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IKROEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto max_kro = findMaxKro( tableManager );
        return imbnumApply( size, "IKRO", max_kro, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > KRORWEndpoint( size_t size,
                                         const TableManager * tableManager,
                                         const EclipseGrid  * eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto krorw = findKrorw( tableManager );
        return satnumApply( size, "KRORW", krorw, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IKRORWEndpoint( size_t size,
                                          const TableManager * tableManager,
                                          const EclipseGrid  * eclipseGrid,
                                          GridProperties<int>* intGridProperties )
    {
        const auto krorw = findKrorw( tableManager );
        return imbnumApply( size, "IKRORW", krorw, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > KRORGEndpoint( size_t size,
                                         const TableManager * tableManager,
                                         const EclipseGrid  * eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto krorg = findKrorg( tableManager );
        return satnumApply( size, "KRORG", krorg, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IKRORGEndpoint( size_t size,
                                          const TableManager * tableManager,
                                          const EclipseGrid  * eclipseGrid,
                                          GridProperties<int>* intGridProperties )
    {
        const auto krorg = findKrorg( tableManager );
        return imbnumApply( size, "IKRORG", krorg, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > KRGEndpoint( size_t size,
                                       const TableManager * tableManager,
                                       const EclipseGrid  * eclipseGrid,
                                       GridProperties<int>* intGridProperties )
    {
        const auto max_krg = findMaxKrg( tableManager );
        return satnumApply( size, "KRG", max_krg, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IKRGEndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto max_krg = findMaxKrg( tableManager );
        return imbnumApply( size, "IKRG", max_krg, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > KRGREndpoint( size_t size,
                                        const TableManager * tableManager,
                                        const EclipseGrid  * eclipseGrid,
                                        GridProperties<int>* intGridProperties )
    {
        const auto krgr = findKrgr( tableManager );
        return satnumApply( size, "KRGR", krgr, tableManager, eclipseGrid,
                            intGridProperties, false );
    }

    std::vector< double > IKRGREndpoint( size_t size,
                                        const TableManager * tableManager,
                                         const EclipseGrid* eclipseGrid,
                                         GridProperties<int>* intGridProperties )
    {
        const auto krgr = findKrgr( tableManager );
        return imbnumApply( size, "IKRGR", krgr, tableManager, eclipseGrid,
                            intGridProperties, false );
    }
}
