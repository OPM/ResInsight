/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_tetrahedron_contains.c' is part of ERT - Ensemble based
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

#include <ert/ecl/point.h>
#include <ert/ecl/tetrahedron.h>


void init_tet(tetrahedron_type * tet , point_type * point_list) {
  point_set(&point_list[0] , 0 , 0 , 0);
  point_set(&point_list[1] , 1 , 0 , 0);
  point_set(&point_list[2] , 0 , 1 , 0);
  point_set(&point_list[3] , 0 , 0 , 1);

  tetrahedron_init(tet , &point_list[0] , &point_list[1] , &point_list[2] , &point_list[3]);
}





void test_inside(const tetrahedron_type * tet ) {
  point_type p;
  point_set(&p , 0.10 , 0.10 , 0.10);
  test_assert_true( tetrahedron_contains( tet , &p ));
}


void test_i(const tetrahedron_type * tet ) {
  point_type p;
  point_set(&p , -10 , 0.10 , 0.10);
  test_assert_false( tetrahedron_contains( tet , &p ));
  
  point_set(&p , 10 , 0.10 , 0.10);
  test_assert_false( tetrahedron_contains( tet , &p ));
}


void test_j(const tetrahedron_type * tet ) {
  point_type p;
  point_set(&p , 0.10 , -10 , 0.10);
  test_assert_false( tetrahedron_contains( tet , &p ));
  
  point_set(&p , 0.10 , 10 , 0.10);
  test_assert_false( tetrahedron_contains( tet , &p ));
}


void test_k(const tetrahedron_type * tet ) {
  point_type p;
  point_set(&p , 0.25 , 0.15 , -10);
  test_assert_false( tetrahedron_contains( tet , &p ));

  point_set(&p , 0.25 , 0.15 , 10);
  test_assert_false( tetrahedron_contains( tet , &p ));
}



int main(int argc , char ** argv) {
  tetrahedron_type tet;
  point_type point_list[4];

  init_tet( &tet , point_list );
  
  test_inside(&tet);
  test_j(&tet);
  test_i(&tet);
  test_k(&tet);
  
}
