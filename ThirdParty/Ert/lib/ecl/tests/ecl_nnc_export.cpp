/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_nnc_export.c' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more detals.
*/
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include <ert/util/test_util.hpp>
#include <ert/util/time_t_vector.hpp>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_nnc_export.hpp>
#include <ert/ecl/ecl_nnc_geometry.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_file_view.hpp>
#include <ert/ecl/ecl_nnc_data.hpp>




int count_kw_data( const ecl_file_type * file , ecl_grid_type * grid , const char * kw1 , const char * kw2) {
  int i,j;
  int count = 0;

  for (i=0; i < ecl_file_get_num_named_kw( file , kw1 ); i++) {
    ecl_kw_type * ecl_kw1 = ecl_file_iget_named_kw( file , kw1 , i );
    ecl_kw_type * ecl_kw2 = NULL;

    if (kw2)
      ecl_kw2 = ecl_file_iget_named_kw(file , kw2 , i);

    {
      if (ecl_kw2) {
        int global_size;
        ecl_grid_type * igrid;
        if (i == 0)
          igrid = grid;
        else
          igrid = ecl_grid_iget_lgr( grid , i - 1);

        global_size = ecl_grid_get_global_size( igrid );

        for (j=0; j < ecl_kw_get_size( ecl_kw1 ); j++) {
          int g1 = ecl_kw_iget_int( ecl_kw1 , j );
          int g2 = ecl_kw_iget_int( ecl_kw2 , j );

          if (g1 <= global_size && g2 <= global_size)
            count +=1;
        }
      } else
        count += ecl_kw_get_size( ecl_kw1 );

    }
  }
  return count;
}



void test_count(const char * name) {
  char * grid_file_name = ecl_util_alloc_filename(NULL , name , ECL_EGRID_FILE , false  , -1);
  ecl_grid_type * grid = ecl_grid_alloc( grid_file_name );
  ecl_file_type * grid_file = ecl_file_open( grid_file_name , 0 );

  int num_nnc = 0;

  num_nnc  = count_kw_data( grid_file , grid , "NNC1" , "NNC2");
  num_nnc += count_kw_data( grid_file , grid , "NNCG" , NULL);
  num_nnc += count_kw_data( grid_file , grid , "NNA1" , NULL);

  test_assert_int_equal( num_nnc , ecl_nnc_export_get_size( grid ));

  free(grid_file_name);
  ecl_grid_free( grid );
  ecl_file_close( grid_file );
}


void test_nnc_export_missing_TRANX(const char * name ) {
  char * grid_file_name = ecl_util_alloc_filename(NULL , name , ECL_EGRID_FILE , false  , -1);
  char * init_file_name = ecl_util_alloc_filename(NULL , name , ECL_INIT_FILE , false  , -1);
  if (util_entry_exists(init_file_name)) {
    ecl_grid_type * grid = ecl_grid_alloc( grid_file_name );
    ecl_file_type * init_file = ecl_file_open( init_file_name , 0);
    ecl_nnc_type  * nnc_data1 = (ecl_nnc_type *) util_calloc( ecl_nnc_export_get_size( grid ) , sizeof * nnc_data1 );
    int count = ecl_nnc_export(grid, init_file, nnc_data1);
    int i;
    test_assert_int_equal( count , 0 );
    for (i=0; i < count; i++)
      test_assert_double_equal( nnc_data1[i].trans , ERT_ECL_DEFAULT_NNC_TRANS );
  }
}

void test_export(const char * name, bool have_tran_data) {
  char * grid_file_name = ecl_util_alloc_filename(NULL , name , ECL_EGRID_FILE , false  , -1);
  char * init_file_name = ecl_util_alloc_filename(NULL , name , ECL_INIT_FILE , false  , -1);
  if (util_entry_exists(init_file_name)) {
    ecl_grid_type * grid = ecl_grid_alloc( grid_file_name );
    ecl_file_type * grid_file = ecl_file_open( grid_file_name , 0 );
    ecl_file_type * init_file = ecl_file_open( init_file_name , 0);
    ecl_nnc_type  * nnc_data1 = (ecl_nnc_type *) util_calloc( ecl_nnc_export_get_size( grid ) , sizeof * nnc_data1 );
    ecl_nnc_type  * nnc_data2 = (ecl_nnc_type *) util_calloc( ecl_nnc_export_get_size( grid ) , sizeof * nnc_data2 );


    {
      int nnc_offset = 0;
      int block_nr = 0;
      for (block_nr = 0; block_nr < ecl_file_get_num_named_kw( grid_file , NNCHEAD_KW); block_nr++) {
        ecl_file_view_type * nnc_view = ecl_file_alloc_global_blockview(grid_file, NNCHEAD_KW, block_nr);

        if (ecl_file_view_has_kw( nnc_view , NNC1_KW )) {
          ecl_kw_type * nnc1_kw  = ecl_file_view_iget_named_kw( nnc_view , NNC1_KW , 0 );
          ecl_kw_type * nnc2_kw  = ecl_file_view_iget_named_kw( nnc_view , NNC2_KW , 0 );
          ecl_kw_type * nnchead  = ecl_file_view_iget_named_kw( nnc_view , NNCHEAD_KW , 0);
          int lgr_nr1 = ecl_kw_iget_int( nnchead , NNCHEAD_LGR_INDEX);
          int lgr_nr2 = ecl_kw_iget_int( nnchead , NNCHEAD_LGR_INDEX);
          ecl_kw_type * nnc_tran = ecl_nnc_export_get_tranx_kw( grid , init_file , lgr_nr1 , lgr_nr2);
          if (!have_tran_data) {
            test_assert_NULL(nnc_tran);
            return;
          }

          test_assert_not_NULL( nnc_tran );
          test_assert_int_equal( ecl_kw_get_size( nnc1_kw ) , ecl_kw_get_size( nnc_tran ));
          {
            ecl_grid_type * lgr = grid;
            int num_nnc = 0;
            int i = 0;
            if (lgr_nr1 > 0)
              lgr = ecl_grid_get_lgr_from_lgr_nr( grid , lgr_nr1 );

            while (i < ecl_kw_get_size( nnc1_kw) ) {
              int g1 = ecl_kw_iget_int( nnc1_kw , i) - 1;
              int g2 = ecl_kw_iget_int( nnc2_kw , i) - 1;

              if (g1 < ecl_grid_get_global_size( lgr ) &&
                  g2 < ecl_grid_get_global_size( lgr )) {
                nnc_data1[ i + nnc_offset ].grid_nr1 = lgr_nr1;
                nnc_data1[ i + nnc_offset ].grid_nr2 = lgr_nr2;
                nnc_data1[ i + nnc_offset ].global_index1 = g1;
                nnc_data1[ i + nnc_offset ].global_index2 = g2;
                nnc_data1[ i + nnc_offset ].trans = ecl_kw_iget_as_double( nnc_tran , i );
                nnc_data1[ i + nnc_offset ].input_index = i;
                num_nnc++;
              }
              i++;
            }
            nnc_offset += num_nnc;
          }
        }


        if (ecl_file_view_has_kw( nnc_view , NNCL_KW)) {
          ecl_kw_type * nncl_kw  = ecl_file_view_iget_named_kw( nnc_view , NNCL_KW , 0 );
          ecl_kw_type * nncg_kw  = ecl_file_view_iget_named_kw( nnc_view , NNCG_KW , 0 );
          ecl_kw_type * nnchead  = ecl_file_view_iget_named_kw( nnc_view , NNCHEAD_KW , 0);
          int lgr_nr1 = 0;
          int lgr_nr2 = ecl_kw_iget_int( nnchead , NNCHEAD_LGR_INDEX);
          ecl_kw_type * nnc_tran = ecl_nnc_export_get_tranx_kw( grid , init_file , 0 , lgr_nr2);
          int i;

          test_assert_int_equal( ecl_kw_get_size( nncl_kw ) , ecl_kw_get_size( nnc_tran ));
          for (i=0; i < ecl_kw_get_size( nncl_kw); i++) {
            nnc_data1[ i + nnc_offset ].grid_nr1 = lgr_nr1;
            nnc_data1[ i + nnc_offset ].grid_nr2 = lgr_nr2;
            nnc_data1[ i + nnc_offset ].global_index1 = ecl_kw_iget_int( nncg_kw , i) - 1;
            nnc_data1[ i + nnc_offset ].global_index2 = ecl_kw_iget_int( nncl_kw , i) - 1;
            nnc_data1[ i + nnc_offset ].trans = ecl_kw_iget_as_double( nnc_tran , i );
            nnc_data1[ i + nnc_offset ].input_index = i;
          }
          nnc_offset += ecl_kw_get_size( nncl_kw );
        }

        ecl_file_view_free( nnc_view );
      }

      {
        for (block_nr = 0; block_nr < ecl_file_get_num_named_kw( grid_file , NNCHEADA_KW); block_nr++) {
          ecl_kw_type * nnc1_kw = ecl_file_iget_named_kw( grid_file , NNA1_KW , block_nr);
          ecl_kw_type * nnc2_kw = ecl_file_iget_named_kw( grid_file , NNA2_KW , block_nr);
          ecl_kw_type * nnchead = ecl_file_iget_named_kw( grid_file , NNCHEADA_KW , block_nr);
          int lgr_nr1 = ecl_kw_iget_int( nnchead , NNCHEADA_ILOC1_INDEX );
          int lgr_nr2 = ecl_kw_iget_int( nnchead , NNCHEADA_ILOC2_INDEX );
          ecl_kw_type * nnc_tran;
          int i;

          nnc_tran = ecl_nnc_export_get_tranx_kw( grid , init_file , lgr_nr1 , lgr_nr2);
          test_assert_not_NULL(nnc_tran);
          test_assert_int_equal( ecl_kw_get_size( nnc1_kw ) , ecl_kw_get_size( nnc_tran ) );
          for (i=0; i < ecl_kw_get_size( nnc1_kw); i++) {
            nnc_data1[ i + nnc_offset ].grid_nr1 = lgr_nr1;
            nnc_data1[ i + nnc_offset ].grid_nr2 = lgr_nr2;
            nnc_data1[ i + nnc_offset ].global_index1 = ecl_kw_iget_int( nnc1_kw , i) - 1;
            nnc_data1[ i + nnc_offset ].global_index2 = ecl_kw_iget_int( nnc2_kw , i) - 1;
            nnc_data1[ i + nnc_offset ].trans = ecl_kw_iget_as_double( nnc_tran , i );
            nnc_data1[ i + nnc_offset ].input_index = i;
          }
          nnc_offset += ecl_kw_get_size( nnc1_kw );
        }
      }

      test_assert_int_equal( nnc_offset , ecl_nnc_export_get_size( grid ));
      ecl_nnc_sort( nnc_data1 , nnc_offset );
    }

    {
      int export_size = ecl_nnc_export( grid , init_file , nnc_data2 );
      test_assert_int_equal( export_size , ecl_nnc_export_get_size( grid ));
    }

    {
      int i;
      int size = ecl_nnc_export_get_size( grid );
      for (i=0; i < size; i++)
        test_assert_int_equal( 0 , ecl_nnc_sort_cmp( &nnc_data1[i] , &nnc_data2[i]));
    }

    for (int i =0; i < ecl_nnc_export_get_size( grid ); i++)
      test_assert_true( ecl_nnc_equal( &nnc_data1[i] , &nnc_data2[i] ));

    {
      ecl_nnc_geometry_type * nnc_geo = ecl_nnc_geometry_alloc( grid );
      ecl_file_view_type * view_file = ecl_file_get_global_view( init_file );
      ecl_nnc_data_type * nnc_geo_data = ecl_nnc_data_alloc_tran(grid, nnc_geo, view_file);

      test_assert_int_equal( ecl_nnc_export_get_size( grid ), ecl_nnc_geometry_size( nnc_geo ));
      for (int i=0; i < ecl_nnc_geometry_size( nnc_geo ); i++) {
        const ecl_nnc_pair_type *nnc_pair = ecl_nnc_geometry_iget( nnc_geo , i );
        ecl_nnc_type * nnc1 = &nnc_data1[i];

        test_assert_int_equal( nnc_pair->grid_nr1      , nnc1->grid_nr1 );
        test_assert_int_equal( nnc_pair->grid_nr2      , nnc1->grid_nr2 );
        test_assert_int_equal( nnc_pair->global_index1 , nnc1->global_index1 );
        test_assert_int_equal( nnc_pair->global_index2 , nnc1->global_index2 );
        test_assert_double_equal(ecl_nnc_data_iget_value(nnc_geo_data, i), nnc1->trans);

      }
      ecl_nnc_data_free (nnc_geo_data );
      ecl_nnc_geometry_free( nnc_geo );
    }

    free( nnc_data2 );
    free( nnc_data1 );
    ecl_grid_free( grid );
    ecl_file_close( grid_file );
    ecl_file_close( init_file );
  } else
    fprintf(stderr," Could not find init file:%s - export test skipped \n",init_file_name);

  free(grid_file_name);
  free(init_file_name);
}


void test_cmp() {

  ecl_nnc_type nnc1 = {1,1,1,1};
  ecl_nnc_type nnc2 = {1,1,1,1};
  ecl_nnc_type nnc3 = {4,1,1,1};
  ecl_nnc_type nnc4 = {4,2,1,1};
  ecl_nnc_type nnc5 = {4,2,3,1};
  ecl_nnc_type nnc6 = {4,2,3,5};

  test_assert_int_equal( 0 , ecl_nnc_sort_cmp( &nnc1 , &nnc2 ));
  test_assert_int_equal( 1 , ecl_nnc_sort_cmp( &nnc3 , &nnc1 ));
  test_assert_int_equal( 1 , ecl_nnc_sort_cmp( &nnc4 , &nnc1 ));
  test_assert_int_equal( 1 , ecl_nnc_sort_cmp( &nnc5 , &nnc4 ));
  test_assert_int_equal(-1 , ecl_nnc_sort_cmp( &nnc4 , &nnc5 ));
  test_assert_int_equal(-1 , ecl_nnc_sort_cmp( &nnc5 , &nnc6 ));
}


void test_sort() {
  const int N = 1000;
  ecl_nnc_type * nnc_list = (ecl_nnc_type *) util_calloc(N , sizeof * nnc_list );
  int i;
  for (i=0; i < N; i++) {
    ecl_nnc_type nnc;

    nnc.grid_nr1 = (i % 19);
    nnc.grid_nr2 = (i % 3);
    nnc.global_index1 = (i % 7);
    nnc.global_index2 = (i % 13);

    nnc_list[i] = nnc;
  }
  ecl_nnc_sort( nnc_list , N );
  for (i=0; i < (N - 1); i++)
    test_assert_int_equal( -1 , ecl_nnc_sort_cmp(&nnc_list[i] , &nnc_list[i+1]));

  free( nnc_list );

}



void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the program with SIGTERM (the default kill signal) you will get a backtrace.
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
}


int main(int argc, char ** argv) {

  const char * base = argv[1];
  bool have_tran_data;
  install_SIGNALS();

  if (util_sscanf_bool( argv[2] , &have_tran_data)) {
    test_cmp( );
    test_sort();
    test_count( base );
    test_export( base, have_tran_data );
    if (!have_tran_data)
      test_nnc_export_missing_TRANX(base);

    exit(0);
  } else
    test_error_exit("Failed to parse input:%s as bool" , argv[2]);
}
