/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_nnc_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/util.h>
#include <ert/util/int_vector.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/nnc_info.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_file_view.h>
#include <ert/ecl/ecl_kw_magic.h>


void test_scan( const char * grid_filename) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_filename );
  ecl_file_type * grid_file = ecl_file_open( grid_filename , 0);
  int block_nr;

  for (block_nr = 0; block_nr < ecl_file_get_num_named_kw( grid_file , NNCHEAD_KW ); block_nr++) {
    ecl_grid_type * lgr = ecl_grid;
    int lgr_nr;
    ecl_file_view_type * nnc_view = ecl_file_alloc_global_blockview(grid_file, NNCHEAD_KW, block_nr);
    {
      if (block_nr > 0)
      lgr = ecl_grid_iget_lgr( ecl_grid , block_nr - 1);
      lgr_nr = ecl_grid_get_lgr_nr( lgr );

      /* Internal nnc */
      {
        if (ecl_file_view_has_kw( nnc_view , NNC1_KW)) {
          ecl_kw_type * nnc1_kw = ecl_file_view_iget_named_kw(nnc_view , NNC1_KW , 0 );
          ecl_kw_type * nnc2_kw = ecl_file_view_iget_named_kw(nnc_view , NNC2_KW , 0 );
          int i;
          for (i=0; i < ecl_kw_get_size(nnc1_kw); i++) {
            const int g1 = ecl_kw_iget_int( nnc1_kw , i ) - 1;
            const int g2 = ecl_kw_iget_int( nnc2_kw , i ) - 1;
      
            if (g2 < ecl_grid_get_global_size( lgr )) {  // Skipping matrix <-> fracture link in dual poro.
              const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1( lgr , g1 );
              const int_vector_type * index_list = nnc_info_get_grid_index_list( nnc_info , lgr_nr);
              test_assert_not_NULL( nnc_info );
              test_assert_int_not_equal( -1 , int_vector_index( index_list , g2 ));
            }
          } 
        }
      }
    }

    /* Global -> lgr */
    {
      if (ecl_file_view_has_kw( nnc_view , NNCG_KW)) {
        ecl_kw_type * nnchead_kw = ecl_file_view_iget_named_kw( nnc_view , NNCHEAD_KW , 0);
        ecl_kw_type * nncg_kw    = ecl_file_view_iget_named_kw(nnc_view , NNCG_KW , 0 );
        ecl_kw_type * nncl_kw    = ecl_file_view_iget_named_kw(nnc_view , NNCL_KW , 0 );
        int i;
        int lgr_nr = ecl_kw_iget_int( nnchead_kw , NNCHEAD_LGR_INDEX);
        for (i=0; i < ecl_kw_get_size(nncg_kw); i++) {
          const int g = ecl_kw_iget_int( nncg_kw , i ) - 1;
          const int l = ecl_kw_iget_int( nncl_kw , i ) - 1;
          
          const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1( ecl_grid , g );
          test_assert_not_NULL( nnc_info );
          {
            const int_vector_type * index_list = nnc_info_get_grid_index_list( nnc_info , lgr_nr );
            test_assert_not_NULL( index_list );
            test_assert_int_not_equal( -1 , int_vector_index( index_list , l ));
          }
        } 
      }
    }
    
    /* Amalgamated: LGR -> LGR */
    {
      if (ecl_file_view_has_kw( nnc_view , NNCHEADA_KW)) {
        ecl_kw_type * nncheada_kw = ecl_file_view_iget_named_kw(nnc_view , NNCHEADA_KW , 0);
        ecl_kw_type * nnc1_kw     = ecl_file_view_iget_named_kw(nnc_view , NNA1_KW , 0 );
        ecl_kw_type * nnc2_kw     = ecl_file_view_iget_named_kw(nnc_view , NNA2_KW , 0 );
        int lgr_nr1 = ecl_kw_iget_int( nncheada_kw , NNCHEADA_ILOC1_INDEX); 
        int lgr_nr2 = ecl_kw_iget_int( nncheada_kw , NNCHEADA_ILOC2_INDEX);
        
        ecl_grid_type * lgr1 = ecl_grid_get_lgr_from_lgr_nr( ecl_grid , lgr_nr1);
        for (int i=0; i < ecl_kw_get_size(nnc1_kw); i++) {
          const int g1 = ecl_kw_iget_int( nnc1_kw , i ) - 1;
          const int g2 = ecl_kw_iget_int( nnc2_kw , i ) - 1;
          
          const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1( lgr1 , g1 );
          const int_vector_type * index_list = nnc_info_get_grid_index_list( nnc_info , lgr_nr2);
          test_assert_not_NULL( nnc_info );
          test_assert_int_not_equal( -1 , int_vector_index( index_list , g2 ));
        } 
      }
    }

    ecl_file_view_free( nnc_view );
  }
}

 


int main(int argc , char ** argv) { 
  int iarg;
  for (iarg = 1; iarg < argc; iarg++) {
    printf("Checking file: %s \n",argv[iarg]);
    test_scan( argv[iarg] );
  }
  
  exit(0);
}
