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


#include <ert/util/test_work_area.h>

#include <ert/util/test_util.hpp>
#include <ert/ecl/EclKW.hpp>
#include <ert/ecl/FortIO.hpp>



void test_fortio() {
    test_work_area_type * work_area = test_work_area_alloc("fortio");

    ERT::FortIO fortio("new_file" , std::fstream::out );
    {
        std::vector<int> data;
        for (size_t i=0; i < 1000; i++)
            data.push_back(i);

        fortio_fwrite_record( fortio.getPointer() , reinterpret_cast<char *>(data.data()) , 1000 * 4 );
    }
    fortio.close();

    fortio = ERT::FortIO("new_file" , std::fstream::in );
    {
        std::vector<int> data;
        for (size_t i=0; i < 1000; i++)
            data.push_back(99);

        test_assert_true( fortio_fread_buffer( fortio.getPointer() , reinterpret_cast<char *>(data.data()) , 1000 * 4 ) );
        for (size_t i =0; i < 1000; i++)
            test_assert_size_t_equal(data[i], i);

    }
    fortio.close();
    test_work_area_free( work_area );

    test_assert_throw( ERT::FortIO fortio("file/does/not/exists" , std::fstream::in) , std::invalid_argument );
}


void test_fortio_kw() {
    test_work_area_type * work_area = test_work_area_alloc("fortio_kw");
    ERT::EclKW<int> kw("XYZ" , 1000);
    for (size_t i =0 ; i < kw.size(); i++)
        kw[i] = i;

    {
        ERT::FortIO fortio("new_file" , std::fstream::out );
        kw.fwrite( fortio );
        fortio.close();
    }

    {
        ERT::FortIO fortio("new_file" , std::fstream::in );
        ERT::EclKW<int> kw2 = ERT::EclKW<int>::load( fortio );
        fortio.close( );
        for (size_t i =0 ; i < kw.size(); i++)
            test_assert_int_equal( kw[i] , kw2[i]);


        fortio = ERT::FortIO("new_file" , std::fstream::in );
	test_assert_throw( ERT::EclKW<float>::load(fortio) , std::invalid_argument );
        fortio.close();
    }

    test_work_area_free( work_area );
}



int main(int argc , char ** argv) {
    test_fortio();
    test_fortio_kw();
}
