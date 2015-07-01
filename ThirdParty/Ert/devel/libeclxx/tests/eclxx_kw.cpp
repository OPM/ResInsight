/*
  Copyright 2015 Statoil ASA.

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
#include <fstream>

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/ecl/EclKW.hpp>
#include <ert/ecl/FortIO.hpp>


void test_kw() {
    ERT::EclKW<int> kw("XYZ" , 1000);
    test_assert_size_t_equal( kw.size() , 1000 );

    kw[0] = 1;
    kw[10] = 77;

    test_assert_int_equal( kw[0] , 1 );
    test_assert_int_equal( kw[10] , 77 );
}






int main (int argc, char **argv) {
    test_kw();
}
