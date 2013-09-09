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


void test_nnc_global_grid( const char * grid_filename ) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_filename );
  test_assert_not_NULL( ecl_grid );
  
  const int data[] = {2675, 5235, 7795, 10355, 12915, 15475, 18035, 20595, 23155, 25715, 28275, 30835, 33395};
    
  const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(ecl_grid, 114);
  test_assert_not_NULL( nnc_info );
  test_assert_int_equal( ecl_grid_get_lgr_nr( ecl_grid ) , nnc_info_get_lgr_nr( nnc_info ));

  const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0); 
  test_assert_not_NULL(nnc_cell_number_vec);
  test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
  
  int i;
  for (i = 0; i < int_vector_size(nnc_cell_number_vec); i++) 
    test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
  
  ecl_grid_free(ecl_grid);
}

void test_nnc_dual_poro(const char * grid_filename ) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_filename );
  test_assert_not_NULL( ecl_grid );
  ecl_grid_free(ecl_grid);
}

void test_nnc_lgr( const char * grid_filename ) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_filename );
  test_assert_not_NULL( ecl_grid );
  
  //Global grid
  const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(ecl_grid, 125132);
  test_assert_not_NULL( nnc_info );
  test_assert_int_equal( ecl_grid_get_lgr_nr( ecl_grid ) , nnc_info_get_lgr_nr( nnc_info ));
  
  const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0);
  test_assert_not_NULL(nnc_cell_number_vec);
  
  int_vector_fprintf(nnc_cell_number_vec , stdout , "nnc_cell_number" , "%6d");

  test_assert_int_equal(int_vector_size(nnc_cell_number_vec), 1); 
  test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, 0), 151053); 
  
  //LGR
  const int data[] = {126305, 126394};
    
  ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 0);
  test_assert_not_NULL( lgr_grid );
  
  const nnc_info_type * lgr_nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 2017-1);
  test_assert_not_NULL( lgr_nnc_info );
  test_assert_int_equal( ecl_grid_get_lgr_nr( lgr_grid ) , nnc_info_get_lgr_nr( lgr_nnc_info ));
  
  const int_vector_type * lgr_nnc_cell_number_vec = nnc_info_get_index_list(lgr_nnc_info, 0);
  test_assert_not_NULL(lgr_nnc_cell_number_vec);
  
  test_assert_int_equal(int_vector_size(lgr_nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
  
  int i;
  for (i = 0; i < int_vector_size(lgr_nnc_cell_number_vec); i++)
    test_assert_int_equal(data[i], int_vector_iget(lgr_nnc_cell_number_vec, i));
  
  ecl_grid_free(ecl_grid);
}



void test_nnc_multiple_lgr( const char * grid_filename) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_filename );
  test_assert_not_NULL( ecl_grid );
  
  {  
    //Global grid, check NNC for cell with global index 736
    int data[] = {3528, 6321, 9114, 11907, 11957, 20286 , 20336};
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(ecl_grid, 736);
    test_assert_not_NULL(nnc_info); 
    test_assert_int_equal( ecl_grid_get_lgr_nr( ecl_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
    
    int i;
    for (i = 0; i < int_vector_size(nnc_cell_number_vec); i++) {
     test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
    }

    //Check grid name 
    test_assert_string_equal(ecl_grid_get_name(ecl_grid), grid_filename); 
  }
  
  {  
    //Global grid, check NNC for cell with global index 138291
    int data[] = {141035, 141085, 143828, 143878};
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(ecl_grid, 138291);
    test_assert_not_NULL(nnc_info); 
    test_assert_int_equal( ecl_grid_get_lgr_nr( ecl_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    int i;
    for (i = 0; i < int_vector_size(nnc_cell_number_vec); i++) {
      test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
    }
  } 
  
  
  {
    //LGR nr 1, cell global index 0:  check NNCs to main grid
    int data[] = {26220 , 29012};
    
    ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 0); 
    test_assert_not_NULL(lgr_grid);
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 0);
    test_assert_not_NULL(nnc_info);
    test_assert_int_equal( ecl_grid_get_lgr_nr( lgr_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
    
    int i = 0;
    for (; i < int_vector_size(nnc_cell_number_vec); ++i)
      test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
    
    //Check name
   test_assert_string_equal(ecl_grid_get_name(lgr_grid), "LG006023"); 
  }
  
  
  {
    //LGR nr 1, cell global index 0:  check NNCs to grid with lgr nr 20
    int data[] = {12};
    
    ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 0); 
    test_assert_not_NULL(lgr_grid);
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 0);
    test_assert_not_NULL(nnc_info);
    test_assert_int_equal( ecl_grid_get_lgr_nr( lgr_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 20);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
    
    int i = 0;
    for (; i < int_vector_size(nnc_cell_number_vec); i++)
      test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
  }
  
  
  
  
  {
    //LGR nr 3, check NNC for cell with global index 8
    int data[] = {20681, 23474, 26267, 37440}; 
    
    ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 2); 
    test_assert_not_NULL(lgr_grid);
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 8);
    test_assert_not_NULL(nnc_info);
    test_assert_int_equal( ecl_grid_get_lgr_nr( lgr_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
    
    int i;
    for (i = 0; i < int_vector_size(nnc_cell_number_vec); i++)
      test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
    
    //Check LGR name 
    test_assert_string_equal(ecl_grid_get_name(lgr_grid), "LG005024"); 
  }
  
  {
    //LGR nr 99, check NNC for cell with global index 736
    int data[] = {79142 ,126671};
    
    ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 98-1); //Subtract 1: LGR nr 98 is not present in the test file. LGRs are numbered 1-97 and 99-110 in test file.
    test_assert_not_NULL(lgr_grid);
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 736);
    test_assert_not_NULL(nnc_info);
    test_assert_int_equal( ecl_grid_get_lgr_nr( lgr_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
    
    int i;
    for (i = 0; i < int_vector_size(nnc_cell_number_vec); i++)
      test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
    
    //Check LGR name 
    test_assert_string_equal(ecl_grid_get_name(lgr_grid), "LG008021"); 
  }
  
  {
    //LGR nr 110, cell with global index 271: Check NNCs to global grid
    int data[] = {20191, 22983};
        
    ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 109-1); //Subtract 1: LGR nr 98 is not present in the test file. LGRs are numbered 1-97 and 99-110 in test file.
    test_assert_not_NULL(lgr_grid);
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 271);
    test_assert_not_NULL(nnc_info);
    test_assert_int_equal( ecl_grid_get_lgr_nr( lgr_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 0);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
    
    int i;
    for (i = 0; i < int_vector_size(nnc_cell_number_vec); i++)
      test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
    
    //Check LGR name 
    test_assert_string_equal(ecl_grid_get_name(lgr_grid), "LG003014"); 
  }
  
   {
  //LGR nr 110, cell with global index 271: Check NNCs to lgr nr 109
    int data[] = {275};
        
    ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 109-1); //Subtract 1: LGR nr 98 is not present in the test file. LGRs are numbered 1-97 and 99-110 in test file.
    test_assert_not_NULL(lgr_grid);
    
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 271);
    test_assert_not_NULL(nnc_info);
    test_assert_int_equal( ecl_grid_get_lgr_nr( lgr_grid ) , nnc_info_get_lgr_nr( nnc_info ));
    
    const int_vector_type * nnc_cell_number_vec = nnc_info_get_index_list(nnc_info, 109);
    test_assert_not_NULL(nnc_cell_number_vec);
    
    test_assert_int_equal(int_vector_size(nnc_cell_number_vec), (sizeof(data)/sizeof(data[0])));
    
    int i;
    for (i = 0; i < int_vector_size(nnc_cell_number_vec); i++)
      test_assert_int_equal(int_vector_iget(nnc_cell_number_vec, i), data[i]); 
  }
  
  
  {
    //Test global versus ijk indexing
    const nnc_info_type * nnc_info1 = ecl_grid_get_cell_nnc_info1(ecl_grid, 736);
    test_assert_not_NULL(nnc_info1); 
    
    int i = 0;
    int j = 0;
    int k = 0; 
    
    ecl_grid_get_ijk1(ecl_grid, 736, &i, &j, &k); 
    
    const nnc_info_type * nnc_info2 = ecl_grid_get_cell_nnc_info3(ecl_grid, i, j, k); 
    test_assert_not_NULL(nnc_info2); 
    
    test_assert_ptr_equal(nnc_info1, nnc_info2); 
  }
  
  ecl_grid_free(ecl_grid);
}


void test_nnc_amalgamated_lgrs(const char * grid_filename) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_filename );
  test_assert_not_NULL( ecl_grid );
  
  
  //Get the NNC info for cell with global index 0 in LGR nr 1
  ecl_grid_type * lgr_grid = ecl_grid_iget_lgr(ecl_grid, 0); 
  test_assert_not_NULL(lgr_grid);

  const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1(lgr_grid, 0);
  test_assert_not_NULL(nnc_info);

  //Get the connections that this cell has to LGR 20
  const int_vector_type * nnc_lgr_20 = nnc_info_get_index_list(nnc_info, 20); 
  test_assert_not_NULL(nnc_lgr_20);
  
  test_assert_int_equal(int_vector_size(nnc_lgr_20), 1);
  test_assert_int_equal(int_vector_iget(nnc_lgr_20, 0), 12); 
    
  ecl_grid_free(ecl_grid);
}

 


int main(int argc , char ** argv) {
  const char * EGRID_file1 = argv[1];
  const char * EGRID_file2 = argv[2];
  const char * EGRID_file3 = argv[3];
  const char * EGRID_file4 = argv[4];
  
  test_nnc_global_grid( EGRID_file1 );
  test_nnc_lgr( EGRID_file2 );
  test_nnc_multiple_lgr( EGRID_file3 );
  test_nnc_amalgamated_lgrs(EGRID_file3);
  test_nnc_dual_poro( EGRID_file4 );
  
  exit(0);
}
