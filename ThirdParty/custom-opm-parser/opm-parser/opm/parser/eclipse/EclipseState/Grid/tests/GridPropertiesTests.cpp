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
#include <iostream>

#define BOOST_TEST_MODULE EclipseGridTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>


BOOST_AUTO_TEST_CASE(Empty) {
    typedef Opm::GridProperties<int>::SupportedKeywordInfo SupportedKeywordInfo;
    std::vector<SupportedKeywordInfo> supportedKeywords = {
        SupportedKeywordInfo("SATNUM" , 0, "1"),
        SupportedKeywordInfo("FIPNUM" , 2, "1")
    };

    const Opm::EclipseGrid grid(10, 7, 9);
    Opm::GridProperties<int> gridProperties(grid, std::move(supportedKeywords));

    BOOST_CHECK( gridProperties.supportsKeyword("SATNUM") );
    BOOST_CHECK( gridProperties.supportsKeyword("FIPNUM") );
    BOOST_CHECK( !gridProperties.supportsKeyword("FLUXNUM") );
    BOOST_CHECK( !gridProperties.hasKeyword("SATNUM"));
    BOOST_CHECK( !gridProperties.hasKeyword("FLUXNUM"));

    BOOST_CHECK_THROW( gridProperties.getInitializedKeyword("SATNUM") , std::invalid_argument);
    BOOST_CHECK_THROW( gridProperties.getInitializedKeyword("NONONO") , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(addKeyword) {
    typedef Opm::GridProperties<int>::SupportedKeywordInfo SupportedKeywordInfo;
    std::vector<SupportedKeywordInfo> supportedKeywords = {
        SupportedKeywordInfo("SATNUM" , 0, "1")
    };
    Opm::EclipseGrid grid(10,7,9);
    Opm::GridProperties<int> gridProperties(grid, std::move( supportedKeywords ));

    BOOST_CHECK_THROW( gridProperties.addKeyword("NOT-SUPPORTED"), std::invalid_argument);

    BOOST_CHECK(  gridProperties.addKeyword("SATNUM"));
    BOOST_CHECK( !gridProperties.addKeyword("SATNUM"));
    BOOST_CHECK(  gridProperties.hasKeyword("SATNUM"));
}


BOOST_AUTO_TEST_CASE(hasKeyword) {
    typedef Opm::GridProperties<int>::SupportedKeywordInfo SupportedKeywordInfo;
    std::vector<SupportedKeywordInfo> supportedKeywords = {
        SupportedKeywordInfo("SATNUM" , 0, "1")
    };
    const Opm::EclipseGrid grid(10, 7, 9);
    Opm::GridProperties<int> gridProperties( grid, std::move( supportedKeywords ) );

    // calling getKeyword() should not change the semantics of hasKeyword()!
    BOOST_CHECK(!gridProperties.hasKeyword("SATNUM"));
    gridProperties.getKeyword("SATNUM");
    BOOST_CHECK(!gridProperties.hasKeyword("SATNUM"));
}


BOOST_AUTO_TEST_CASE(getKeyword) {
    typedef Opm::GridProperties<int>::SupportedKeywordInfo SupportedKeywordInfo;
    std::vector<SupportedKeywordInfo> supportedKeywords = {
        SupportedKeywordInfo("SATNUM" , 0, "1")
    };
    const Opm::EclipseGrid grid(10,7,9);
    Opm::GridProperties<int> gridProperties( grid, std::move( supportedKeywords ) );
    const Opm::GridProperty<int>& satnum1 = gridProperties.getKeyword( "SATNUM" );
    const Opm::GridProperty<int>& satnum2 = gridProperties.getKeyword( "SATNUM" );
    const Opm::GridProperty<int>& satnum3 = gridProperties.getKeyword( size_t( 0 ) );

    BOOST_CHECK_EQUAL( 1, gridProperties.size() );
    BOOST_CHECK_EQUAL( &satnum1, &satnum2 );
    BOOST_CHECK_EQUAL( &satnum1, &satnum3 );

    BOOST_CHECK_THROW( gridProperties.getKeyword( "NOT-SUPPORTED" ), std::invalid_argument );
    BOOST_CHECK_THROW( gridProperties.getKeyword( 3 ), std::invalid_argument );
}


