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

#define BOOST_TEST_MODULE BoxManagereTests

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>

BOOST_AUTO_TEST_CASE(CreateBox) {
    BOOST_CHECK_THROW( new Opm::Box(-1,0,0) , std::invalid_argument);
    BOOST_CHECK_THROW( new Opm::Box(10,0,10) , std::invalid_argument);
    BOOST_CHECK_THROW( new Opm::Box(10,10,-1) , std::invalid_argument);

    Opm::Box box(4,3,2);
    BOOST_CHECK_EQUAL( 24U , box.size() );
    BOOST_CHECK( box.isGlobal() );
    BOOST_CHECK_EQUAL( 4 , box.getDim(0) );
    BOOST_CHECK_EQUAL( 3 , box.getDim(1) );
    BOOST_CHECK_EQUAL( 2 , box.getDim(2) );

    BOOST_CHECK_THROW( box.getDim(5) , std::invalid_argument);


    {
        size_t i,j,k;
        const std::vector<size_t>& indexList = box.getIndexList();

        for (k=0; k < box.getDim(2); k++) {
            for (j=0; j < box.getDim(1); j++) {
                for (i=0; i < box.getDim(0); i++) {
                    size_t g = i + j*box.getDim(0) + k*box.getDim(0)*box.getDim(1);
                    BOOST_CHECK_EQUAL( indexList[g] , g);

                }
            }
        }
    }
}



BOOST_AUTO_TEST_CASE(CreateSubBox) {
    Opm::Box globalBox( 10,10,10 );

    BOOST_CHECK_THROW( new Opm::Box( globalBox , -1 , 9 , 1 , 8 , 1, 8)  , std::invalid_argument);   //  Negative throw
    BOOST_CHECK_THROW( new Opm::Box( globalBox ,  1 , 19 , 1 , 8 , 1, 8) , std::invalid_argument);   //  Bigger than global: throw
    BOOST_CHECK_THROW( new Opm::Box( globalBox ,  9 , 1  , 1 , 8 , 1, 8) , std::invalid_argument);   //  Inverted order: throw

    Opm::Box subBox1(globalBox , 0,9,0,9,0,9);
    BOOST_CHECK( subBox1.isGlobal());


    Opm::Box subBox2(globalBox , 1,3,1,4,1,5);
    BOOST_CHECK( !subBox2.isGlobal());
    BOOST_CHECK_EQUAL( 60U , subBox2.size() );

    {
        size_t i,j,k;
        size_t d = 0;
        const std::vector<size_t>& indexList = subBox2.getIndexList();

        for (k=0; k < subBox2.getDim(2); k++) {
            for (j=0; j < subBox2.getDim(1); j++) {
                for (i=0; i < subBox2.getDim(0); i++) {

                    size_t g = (i + 1) + (j + 1)*globalBox.getDim(0) + (k + 1)*globalBox.getDim(0)*globalBox.getDim(1);
                    BOOST_CHECK_EQUAL( indexList[d] , g);
                    d++;
                }
            }
        }
    }
}


BOOST_AUTO_TEST_CASE(BoxEqual) {
    Opm::Box globalBox1( 10,10,10 );
    Opm::Box globalBox2( 10,10,10 );
    Opm::Box globalBox3( 10,10,11 );

    Opm::Box globalBox4( 20,20,20 );
    Opm::Box subBox1( globalBox1 , 0 , 9 , 0 , 9 , 0, 9);
    Opm::Box subBox4( globalBox4 , 0 , 9 , 0 , 9 , 0, 9);
    Opm::Box subBox5( globalBox4 , 10 , 19 , 10 , 19 , 10, 19);

    BOOST_CHECK( globalBox1.equal( globalBox2 ));
    BOOST_CHECK( !globalBox1.equal( globalBox3 ));
    BOOST_CHECK( globalBox1.equal( subBox1 ));

    BOOST_CHECK( !globalBox4.equal( subBox4 ));
    BOOST_CHECK( !subBox4.equal( subBox5 ));
}


