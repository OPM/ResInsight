/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_grid_cell_contains.c' is part of ERT - Ensemble based
   Reservoir Tool.
    
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
#include <ert/ecl/ecl_grid.h>


void test_grid_covering( const ecl_grid_type * grid) {
  const int nx = ecl_grid_get_nx( grid );
  const int ny = ecl_grid_get_ny( grid );
  const int nz = ecl_grid_get_nz( grid );

  for (int k=0; k < nz - 1; k++) {
    for (int j=0; j < ny; j++) {
      for (int i=0; i < nx; i++) {
        int g1 = ecl_grid_get_global_index3(grid, i,j,k);
        int g2 = ecl_grid_get_global_index3(grid, i,j,k + 1);
        double p1[ 3 ];
        double p2[ 3 ];

        for (int l=0; l < 4; l++) {
          ecl_grid_get_cell_corner_xyz1( grid , g1 , l + 4 , &p1[0], &p1[1], &p1[2] );
          ecl_grid_get_cell_corner_xyz1( grid , g2 , l, &p2[0], &p2[1], &p2[2] );
          test_assert_true( p1[ 0 ] == p2[ 0 ] );
          test_assert_true( p1[ 1 ] == p2[ 1 ] );
          test_assert_true( p1[ 2 ] == p2[ 2 ] );
        }
        
        
        for (int l=0; l < 4; l++) {
          ecl_grid_get_cell_corner_xyz1( grid , g1 , l, &p1[0], &p1[1], &p1[2] );
          ecl_grid_get_cell_corner_xyz1( grid , g1 , l + 4 , &p2[0], &p2[1], &p2[2] );
          
          test_assert_true( p2[2] >= p1[2] );
        }
      }
    }
  }
}



void test_contains_count( const ecl_grid_type * grid ) {
  int error_count = 0;
  const int nx = ecl_grid_get_nx( grid );
  const int ny = ecl_grid_get_ny( grid );
  const int nz = ecl_grid_get_nz( grid );

  for (int k=0; k < nz; k++) {
    printf("k: %d/%d \n", k , nz - 1);
    for (int j=0; j < ny; j++) {
      for (int i=0; i < nx; i++) {
        double x,y,z;
        ecl_grid_get_xyz3( grid , i,j,k , &x,&y,&z);
        {
          int i2,j2,k2;
          int contains_count = 0;
          int kmin = util_int_max( 0  , k - 1 );
          int kmax = util_int_min( nz , k + 1 );
          
          int jmin = util_int_max( 0  , j - 1 );
          int jmax = util_int_min( ny , j + 1 );

          int imin = util_int_max( 0  , i - 1 );
          int imax = util_int_min( nx , i + 1 );
          

          for (k2 = kmin; k2 < kmax; k2++) {
            for (j2 = jmin; j2 < jmax; j2++) {
              for (i2 = imin; i2 < imax; i2++) {
                if (ecl_grid_cell_contains_xyz3( grid , i2,j2,k2 , x,y, z )) {
                  contains_count++;
                }
              }
            }
          }
          
          if (contains_count != 1) { 
            if (contains_count > 1)
              error_count += 1;
            else 
              if (ecl_grid_cell_regular3( grid , i,j,k))
                error_count += 1;
          }
        }
      }
    }
  }
  test_assert_int_equal( error_count , 0 );
}



void test_find( ecl_grid_type * grid ) {
  int init_index;
  int find_count = 100;
  int delta = util_int_max(1 , ecl_grid_get_global_size( grid ) / find_count);
  for (init_index = 0; init_index < ecl_grid_get_global_size( grid ); init_index += delta) {
    printf("find index:%d \n",init_index / delta);
    if (!ecl_grid_cell_invalid1(grid , init_index) && ecl_grid_cell_regular1( grid , init_index) ) {
      double x,y,z;
      int find_index;
      int start_index = 0;
      
      ecl_grid_get_xyz1( grid , init_index , &x,&y,&z);
      find_index = ecl_grid_get_global_index_from_xyz(grid , x, y , z , start_index );
      test_assert_int_equal(init_index , find_index ); 
    }
  }
}


//      /*
//        Will indeed answer yes when asked if it contains it's own center.
//      */
//      test_assert_true( ecl_grid_cell_contains_xyz1( grid , init_index , x,y,z) );
//      
//      if (1)
//      {
//        int start_index = 0;
//        
//        if (find_index != init_index) {
//          int init_ijk[3];
//          int find_ijk[3];
//
//          ecl_grid_get_ijk1( grid , find_index , &find_ijk[0] , &find_ijk[1] , &find_ijk[2]);
//          ecl_grid_get_ijk1( grid , init_index , &init_ijk[0] , &init_ijk[1] , &init_ijk[2]);
//          
//          printf("ijk: %d:(%2d,%2d,%2d) -> %d:(%2d,%2d,%2d) \n",init_index , init_ijk[0] , init_ijk[1] , init_ijk[2] , find_index , find_ijk[0] , find_ijk[1], find_ijk[2]);
//          global_error += 1;
//        }
//
//        if (0) {
//        //if (find_index != init_index) {
//          int init_ijk[3];
//          int find_ijk[3];
//
//          ecl_grid_get_ijk1( grid , find_index , &find_ijk[0] , &find_ijk[1] , &find_ijk[2]);
//          ecl_grid_get_ijk1( grid , init_index , &init_ijk[0] , &init_ijk[1] , &init_ijk[2]);
//          
//          {
//            printf("ijk: (%2d,%2d,%2d) -> (%2d,%2d,%2d) \n",init_ijk[0] , init_ijk[1] , init_ijk[2] , find_ijk[0] , find_ijk[1], find_ijk[2]);
//            
//            if (0) {
//              printf(" ecl_grid_cell_contains_xyz3(%d,%d,%d) : %d Volume:%g \n",init_ijk[0] , init_ijk[1], init_ijk[2] , 
//                     ecl_grid_cell_contains_xyz3( grid , init_ijk[0] , init_ijk[1], init_ijk[2] , x , y , z ),
//                     ecl_grid_get_cell_volume1( grid , init_index));
//              
//              printf(" ecl_grid_cell_contains_xyz3(%d,%d,%d) : %d Volume:%g \n",find_ijk[0] , find_ijk[1], find_ijk[2] , 
//                     ecl_grid_cell_contains_xyz3( grid , find_ijk[0] , find_ijk[1], find_ijk[2] , x , y , z ),
//                     ecl_grid_get_cell_volume1( grid , find_index));
//            }
//            
//            {
//              int find_index2 = ecl_grid_get_global_index_from_xyz(grid , x, y , z , init_index );
//              printf("find_index2:%d \n",find_index2);
//            }
//              
//            printf("init_index:%d  find_index:%d \n",init_index , find_index);
//            if (find_index >= 0)
//              printf("Vrengte: %d\n",ecl_grid_cell_inside_out1( grid , find_index));
//            printf("Vrengte: %d\n",ecl_grid_cell_inside_out1( grid , init_index));
//
//            {
//              bool init_cell_inside_out = ecl_grid_cell_inside_out1( grid , init_index);
//              bool find_cell_inside_out = false;
//              
//              if (find_index >= 0)
//                find_cell_inside_out = ecl_grid_cell_inside_out1( grid , find_index );
//              else
//                test_assert_int_not_equal( -1 , find_index );
//              
//              if (find_cell_inside_out == init_cell_inside_out) {
//                if (init_cell_inside_out == false)
//                  test_assert_int_equal( init_index , find_index );
//              }
//            }
//          }
//        }
//      }
//    }
//  }
//}


void test_corners() {
  ecl_grid_type * grid = ecl_grid_alloc_rectangular(3,3,3,1,1,1,NULL);

  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid , -1,-1,-1 , 0));
  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid , -1, 1, 1 , 0));
  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid ,  1,-1, 1 , 0));
  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid ,  1, 1,-1 , 0));

  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid , 3.5 , 3.5 , 3.5 , 0));
  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid , 3.5 , 1   , 1   , 0));
  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid , 1   , 3.5 , 1   , 0));
  test_assert_int_equal( -1 , ecl_grid_get_global_index_from_xyz( grid , 1   , 1   , 3.5 , 0));

  {
    double x,y,z;
    int i;
    ecl_grid_get_cell_corner_xyz3( grid , 0, 0, 0 , 0 , &x , &y , &z);
    test_assert_int_equal( 0 , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));

    for (i=1; i < 8; i++) {
      ecl_grid_get_cell_corner_xyz3( grid , 0, 0, 0 , i , &x , &y , &z);
      test_assert_int_not_equal( 0 , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));
    }
    
    // Corner 1
    ecl_grid_get_cell_corner_xyz3(grid , 2,0,0,1 , &x,&y,&z);
    test_assert_int_equal( ecl_grid_get_global_index3( grid , 2,0,0 ) , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));

    // Corner 2
    ecl_grid_get_cell_corner_xyz3(grid , 0,2,0,2 , &x,&y,&z);
    test_assert_int_equal( ecl_grid_get_global_index3( grid , 0,2,0 ) , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));

    // Corner 3
    ecl_grid_get_cell_corner_xyz3(grid , 2,2,0,3 , &x,&y,&z);
    test_assert_int_equal( ecl_grid_get_global_index3( grid , 2,2,0 ) , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));

    // Corner 4
    ecl_grid_get_cell_corner_xyz3(grid , 0,0,2,4 , &x,&y,&z);
    test_assert_int_equal( ecl_grid_get_global_index3( grid , 0,0,2 ) , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));

    // Corner 5
    ecl_grid_get_cell_corner_xyz3(grid , 2,0,2,5 , &x,&y,&z);
    test_assert_int_equal( ecl_grid_get_global_index3( grid , 2,0,2 ) , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));

    // Corner 6
    ecl_grid_get_cell_corner_xyz3(grid , 0,2,2,6 , &x,&y,&z);
    test_assert_int_equal( ecl_grid_get_global_index3( grid , 0,2,2 ) , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));

    // Corner 7
    ecl_grid_get_cell_corner_xyz3(grid , 2,2,2,7 , &x,&y,&z);
    test_assert_int_equal( ecl_grid_get_global_index3( grid , 2,2,2 ) , ecl_grid_get_global_index_from_xyz( grid , x,y,z,0));
  }
                         
  ecl_grid_free( grid );
}






int main(int argc , char ** argv) {
  ecl_grid_type * grid;
  int case_nr;

  util_install_signals();
  util_sscanf_int( argv[1] , &case_nr );

  if (argc == 2) {
    grid = ecl_grid_alloc_rectangular(6,6,6,1,2,3,NULL);
  } else
    grid = ecl_grid_alloc( argv[2] );
  
  test_grid_covering( grid );
  test_contains_count( grid );
  

  test_find(grid);
  test_corners();
  ecl_grid_free( grid );
  exit(0);
}
