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
#include <memory>
#include <boost/filesystem.hpp>

#define BOOST_TEST_MODULE BoxManagerTests
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/BoxManager.hpp>


BOOST_AUTO_TEST_CASE(CreateBoxManager) {
    Opm::BoxManager boxManager(10,10,10);
    Opm::Box box(10,10,10);

    BOOST_CHECK( box.equal( *boxManager.getGlobalBox()) );
    BOOST_CHECK( box.equal( *boxManager.getActiveBox()) );
    BOOST_CHECK( !boxManager.getInputBox() );
    BOOST_CHECK( !boxManager.getKeywordBox() );
}




BOOST_AUTO_TEST_CASE(TestInputBox) {
    Opm::BoxManager boxManager(10,10,10);
    Opm::Box inputBox( *boxManager.getGlobalBox() , 0,4,0,4,0,4);

    boxManager.setInputBox( 0,4,0,4,0,4 );
    BOOST_CHECK( inputBox.equal( *boxManager.getInputBox()) );
    BOOST_CHECK( inputBox.equal( *boxManager.getActiveBox()) );


    boxManager.endSection();
    BOOST_CHECK( !boxManager.getInputBox() );
    BOOST_CHECK( boxManager.getActiveBox()->equal( *boxManager.getGlobalBox()));
}




BOOST_AUTO_TEST_CASE(TestKeywordBox) {
    Opm::BoxManager boxManager(10,10,10);
    Opm::Box inputBox( *boxManager.getGlobalBox() , 0,4,0,4,0,4);
    Opm::Box keywordBox( *boxManager.getGlobalBox() , 0,2,0,2,0,2);


    boxManager.setInputBox( 0,4,0,4,0,4 );
    boxManager.setKeywordBox( 0,2,0,2,0,2 );
    BOOST_CHECK( inputBox.equal( *boxManager.getInputBox()) );
    BOOST_CHECK( keywordBox.equal( *boxManager.getKeywordBox()) );
    BOOST_CHECK( keywordBox.equal( *boxManager.getActiveBox()) );

    // Must end keyword first
    BOOST_CHECK_THROW( boxManager.endSection() , std::invalid_argument );

    boxManager.endKeyword();
    BOOST_CHECK( inputBox.equal( *boxManager.getActiveBox()) );
    BOOST_CHECK( !boxManager.getKeywordBox() );

    boxManager.endSection();
    BOOST_CHECK( !boxManager.getInputBox() );
    BOOST_CHECK( boxManager.getActiveBox()->equal( *boxManager.getGlobalBox()));
}
