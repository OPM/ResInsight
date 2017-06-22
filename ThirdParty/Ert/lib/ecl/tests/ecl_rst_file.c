/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'ecl_rst_file.c' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/
#include <stdlib.h>
#include <stdbool.h>

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_rst_file.h>
#include <ert/ecl/ecl_type.h>



void write_keyword( fortio_type * fortio , const char * kw, ecl_data_type data_type ) {
  ecl_kw_type * ecl_kw = ecl_kw_alloc( kw , 1000 , data_type );
  ecl_kw_fwrite( ecl_kw , fortio );
  ecl_kw_free( ecl_kw );
}


void write_seqnum( fortio_type * fortio , int report_step ) {
  ecl_kw_type * ecl_kw = ecl_kw_alloc( SEQNUM_KW , 1 , ECL_INT);
  ecl_kw_iset_int( ecl_kw , 0 , report_step );
  ecl_kw_fwrite( ecl_kw , fortio );
  ecl_kw_free( ecl_kw );
}



void test_empty() {
  ecl_rst_file_type * rst_file = ecl_rst_file_open_write_seek( "EMPTY.UNRST" , 0 );
  test_assert_int_equal( ecl_rst_file_ftell( rst_file ) , 0 );
  ecl_rst_file_close( rst_file );
}


void test_file( const char * src_file , const char * target_file , int report_step , offset_type expected_offset) {
  util_copy_file( src_file , target_file );
  {
    ecl_rst_file_type * rst_file = ecl_rst_file_open_write_seek( target_file , report_step );
    test_assert_true( ecl_rst_file_ftell( rst_file ) == expected_offset );
    ecl_rst_file_close( rst_file );
    test_assert_true( util_file_size( target_file ) == expected_offset );
  }
}



void test_Xfile() {
  test_work_area_type * work_area = test_work_area_alloc("rst-file");
  {
    fortio_type * f = fortio_open_writer( "TEST.X0010" , false , ECL_ENDIAN_FLIP);

    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    fortio_fclose( f );
  }
  test_file( "TEST.X0010" , "FILE.X0010" , 10 , 0 );
  test_work_area_free( work_area );
}



void test_UNRST0() {
  test_work_area_type * work_area = test_work_area_alloc("rst-file");
  offset_type pos10;
  offset_type pos20;
  offset_type pos_end;
  {
    fortio_type * f = fortio_open_writer( "TEST.UNRST" , false , ECL_ENDIAN_FLIP);
    write_seqnum( f , 0 );
    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    pos10 = fortio_ftell( f );
    write_seqnum( f , 10 );
    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    pos20 = fortio_ftell( f );
    write_seqnum( f , 20 );
    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    pos_end = fortio_ftell( f );
    fortio_fclose( f );
  }
  test_file( "TEST.UNRST" , "FILE.UNRST" , 0 , 0 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 5 , pos10 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 10 , pos10 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 15 , pos20 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 20 , pos20 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 25 , pos_end );
  test_work_area_free( work_area );
}


void test_UNRST1() {
  test_work_area_type * work_area = test_work_area_alloc("rst-file");
  offset_type pos5;
  offset_type pos10;
  offset_type pos20;
  offset_type pos_end;
  {
    fortio_type * f = fortio_open_writer( "TEST.UNRST" , false , ECL_ENDIAN_FLIP);
    pos5 = fortio_ftell( f );
    write_seqnum( f , 5 );
    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    pos10 = fortio_ftell( f );
    write_seqnum( f , 10 );
    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    pos20 = fortio_ftell( f );
    write_seqnum( f , 20 );
    write_keyword( f , "INTEHEAD" , ECL_INT);
    write_keyword( f , "PRESSURE" , ECL_FLOAT);
    write_keyword( f , "SWAT" , ECL_FLOAT);

    pos_end = fortio_ftell( f );
    fortio_fclose( f );
  }
  test_file( "TEST.UNRST" , "FILE.UNRST" , 0 , 0 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 1 , 0 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 5 , pos5 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 10 , pos10 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 15 , pos20 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 20 , pos20 );
  test_file( "TEST.UNRST" , "FILE.UNRST" , 25 , pos_end );
  test_work_area_free( work_area );
}







int main(int argc , char ** argv) {
  test_empty();
  test_Xfile();
  test_UNRST0();
  test_UNRST1();
}
