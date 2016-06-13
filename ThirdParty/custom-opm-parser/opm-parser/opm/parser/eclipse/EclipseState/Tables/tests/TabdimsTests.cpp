/*
  Copyright (C) 2015 Statoil ASA

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
#define BOOST_TEST_MODULE TABDIMS_TESTS
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/EclipseState/Tables/Tabdims.hpp>

BOOST_AUTO_TEST_CASE(TEST_CREATE) {
    Opm::Tabdims tabdims1(1,2,3,4,5,6);
    Opm::Tabdims tabdims2;

    BOOST_CHECK_EQUAL( tabdims1.getNumSatNodes() ,  3U );
    BOOST_CHECK_EQUAL( tabdims2.getNumSatNodes() , 20U );
}
