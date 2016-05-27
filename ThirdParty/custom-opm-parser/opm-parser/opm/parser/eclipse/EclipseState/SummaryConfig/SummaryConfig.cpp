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


#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Completion.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/CompletionSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/parser/eclipse/Utility/Functional.hpp>

#include <ert/ecl/ecl_smspec.h>

#include <algorithm>
#include <array>

namespace Opm {

    static std::string wellName( const std::shared_ptr< const Well >& well ) {
        return well->name();
    }

    static std::string groupName( const Group* group ) {
        return group->name();
    }

    static inline std::vector< ERT::smspec_node > keywordWG(
            ecl_smspec_var_type var_type,
            const DeckKeyword& keyword,
            const EclipseState& es ) {

        const auto mknode = [&keyword,var_type]( const std::string& name ) {
            return ERT::smspec_node( var_type, name, keyword.name() );
        };

        const auto find = []( ecl_smspec_var_type v, const EclipseState& est ) {
            if( v == ECL_SMSPEC_WELL_VAR )
                return fun::map( wellName, est.getSchedule()->getWells() );
            else
                return fun::map( groupName, est.getSchedule()->getGroups() );
        };

        const auto& item = keyword.getDataRecord().getDataItem();
        const auto wgnames = item.size() > 0 && item.hasValue( 0 )
            ? item.getData< std::string >()
            : find( var_type, es );

        return fun::map( mknode, wgnames );
    }

    static inline std::vector< ERT::smspec_node > keywordF(
            const DeckKeyword& keyword,
            const EclipseState& /* es */ ) {

        std::vector< ERT::smspec_node > res;
        res.push_back( ERT::smspec_node( keyword.name() ) );
        return res;
    }

    static inline std::array< int, 3 > dimensions( const EclipseGrid& grid ) {
        return {{
            int( grid.getNX() ),
            int( grid.getNY() ),
            int( grid.getNZ() )
        }};
    }

    static inline std::array< int, 3 > getijk( const DeckRecord& record,
                                               int offset = 0 )
    {
        return {{
            record.getItem( offset + 0 ).get< int >( 0 ) - 1,
            record.getItem( offset + 1 ).get< int >( 0 ) - 1,
            record.getItem( offset + 2 ).get< int >( 0 ) - 1
        }};
    }

    static inline std::array< int, 3 > getijk( const Completion& completion ) {
        return {{ completion.getI(), completion.getJ(), completion.getK() }};
    }

    static inline std::vector< ERT::smspec_node > keywordB(
            const DeckKeyword& keyword,
            const EclipseState& es ) {

        auto dims = dimensions( *es.getInputGrid() );

        const auto mkrecord = [&dims,&keyword]( const DeckRecord& record ) {
            auto ijk = getijk( record );
            return ERT::smspec_node( keyword.name(), dims.data(), ijk.data() );
        };

        return fun::map( mkrecord, keyword );
    }

    static inline std::vector< ERT::smspec_node > keywordR(
            const DeckKeyword& keyword,
            const EclipseState& es ) {

        auto dims = dimensions( *es.getInputGrid() );

        const auto mknode = [&dims,&keyword]( int region ) {
            return ERT::smspec_node( keyword.name(), dims.data(), region );
        };

        const auto& item = keyword.getDataRecord().getDataItem();
        const auto regions = item.size() > 0 && item.hasValue( 0 )
            ? item.getData< int >()
            : es.getRegions( "FIPNUM" );

        return fun::map( mknode, regions );
    }

   static inline std::vector< ERT::smspec_node > keywordC(
           const DeckKeyword& keyword,
           const EclipseState& es ) {

       std::vector< ERT::smspec_node > nodes;
       const auto& keywordstring = keyword.name();
       const auto& schedule = es.getSchedule();
       const auto last_timestep = schedule->getTimeMap()->last();
       auto dims = dimensions( *es.getInputGrid() );

       for( const auto& record : keyword ) {

           if( record.getItem( 0 ).defaultApplied( 0 ) ) {
               for( const auto& well : schedule->getWells() ) {

                   const auto& name = wellName( well );

                   for( const auto& completion : *well->getCompletions( last_timestep ) ) {
                       auto cijk = getijk( *completion );

                       /* well defaulted, block coordinates defaulted */
                       if( record.getItem( 1 ).defaultApplied( 0 ) ) {
                           nodes.emplace_back( keywordstring, name, dims.data(), cijk.data() );
                       }
                       /* well defaulted, block coordinates specified */
                       else {
                           auto recijk = getijk( record, 1 );
                           if( std::equal( recijk.begin(), recijk.end(), cijk.begin() ) )
                               nodes.emplace_back( keywordstring, name, dims.data(), cijk.data() );
                       }
                   }
               }

           } else {
                const auto& name = record.getItem( 0 ).get< std::string >( 0 );
               /* all specified */
               if( !record.getItem( 1 ).defaultApplied( 0 ) ) {
                   auto ijk = getijk( record, 1 );
                   nodes.emplace_back( keywordstring, name, dims.data(), ijk.data() );
               }
               else {
                   /* well specified, block coordinates defaulted */
                   for( const auto& completion : *schedule->getWell( name ).getCompletions( last_timestep ) ) {
                       auto ijk = getijk( *completion );
                       nodes.emplace_back( keywordstring, name, dims.data(), ijk.data() );
                   }
               }
           }
       }

       return nodes;
   }

    std::vector< ERT::smspec_node > handleKW( const DeckKeyword& keyword, const EclipseState& es ) {
        const auto var_type = ecl_smspec_identify_var_type( keyword.name().c_str() );

        switch( var_type ) {
            case ECL_SMSPEC_WELL_VAR: /* intentional fall-through */
            case ECL_SMSPEC_GROUP_VAR: return keywordWG( var_type, keyword, es );
            case ECL_SMSPEC_FIELD_VAR: return keywordF( keyword, es );
            case ECL_SMSPEC_BLOCK_VAR: return keywordB( keyword, es );
            case ECL_SMSPEC_REGION_VAR: return keywordR( keyword, es );
            case ECL_SMSPEC_COMPLETION_VAR: return keywordC( keyword, es );

            default: return {};
        }
    }

    SummaryConfig::SummaryConfig( const Deck& deck, const EclipseState& es ) {

        SUMMARYSection section( deck );
        const auto handler = [&es]( const DeckKeyword& kw ) {
            return handleKW( kw, es );
        };

#ifdef _MSC_VER
        for (auto& x : section)
        {
            auto keywords = handler(x);
            for (auto& keyword : keywords)
            {
                this->keywords.push_back(keyword);
            }
        }
#else
        this->keywords = fun::concat( fun::map( handler, section ) );
#endif
    }

    SummaryConfig::const_iterator SummaryConfig::begin() const {
        return this->keywords.cbegin();
    }

    SummaryConfig::const_iterator SummaryConfig::end() const {
        return this->keywords.cend();
    }

}
