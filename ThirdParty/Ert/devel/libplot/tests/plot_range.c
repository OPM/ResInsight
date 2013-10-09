/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'plot_range.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <math.h>

#include <ert/util/test_util.h>
#include <ert/util/double_vector.h>

#include <ert/plot/plot_range.h>

void test_create_range() {
  plot_range_type * range = plot_range_alloc();
  test_assert_not_NULL( range );
  plot_range_free( range );
}


void test_default() {
  plot_range_type * range = plot_range_alloc();
  
  test_assert_double_equal( PLOT_RANGE_DEFAULT_PADDING , plot_range_get_left_padding( range ));
  test_assert_double_equal( PLOT_RANGE_DEFAULT_PADDING , plot_range_get_right_padding( range ));
  test_assert_double_equal( PLOT_RANGE_DEFAULT_PADDING , plot_range_get_top_padding( range ));
  test_assert_double_equal( PLOT_RANGE_DEFAULT_PADDING , plot_range_get_bottom_padding( range ));

  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_xmin( range ));
  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_xmax( range ));
  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_ymin( range ));
  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_ymax( range ));

  test_assert_false( plot_range_get_invert_x_axis( range ));
  test_assert_false( plot_range_get_invert_y_axis( range ));
  plot_range_set_invert_x_axis( range , true );
  plot_range_set_invert_y_axis( range , true );
  test_assert_true( plot_range_get_invert_x_axis( range ));
  test_assert_true( plot_range_get_invert_y_axis( range ));

  plot_range_free( range );
}


void test_empty_range() {
  plot_range_type * range = plot_range_alloc();
  
  {
    double x = 10;
    double y = 78;

    plot_range_update( range , x , y );
    test_assert_double_equal( plot_range_get_current_xmin( range ) , x );
    test_assert_double_equal( plot_range_get_current_xmax( range ) , x );
    test_assert_double_equal( plot_range_get_current_ymin( range ) , y );
    test_assert_double_equal( plot_range_get_current_ymax( range ) , y );
  }
  plot_range_free( range );
}


void test_empty_range_xy() {
  plot_range_type * range = plot_range_alloc();
  
  {
    double x = 10;
    double y = 78;

    plot_range_update_x( range , x );
    plot_range_update_y( range , y );

    test_assert_double_equal( plot_range_get_current_xmin( range ) , x );
    test_assert_double_equal( plot_range_get_current_xmax( range ) , x );
    test_assert_double_equal( plot_range_get_current_ymin( range ) , y );
    test_assert_double_equal( plot_range_get_current_ymax( range ) , y );
  }
  plot_range_free( range );
}


void test_update_vector() {
  plot_range_type * range = plot_range_alloc();
  double_vector_type * xl = double_vector_alloc(0,0);
  double_vector_type * yl = double_vector_alloc(0,0);
  const int N = 100;
  int i;

  for (i=0; i < N; i++) {
    double x = 2*3.14159265 * i / (N - 1);
    
    double_vector_append( xl , sin(x));
    double_vector_append( yl , cos(x));
  }
  plot_range_update_vector( range , xl , yl );

  test_assert_double_equal( plot_range_get_current_xmin( range ) , -1 );
  test_assert_double_equal( plot_range_get_current_xmax( range ) ,  1 );
  test_assert_double_equal( plot_range_get_current_ymin( range ) , -1 );
  test_assert_double_equal( plot_range_get_current_ymax( range ) ,  1 );
  
  for (i=0; i < N; i++) {
    double x = 2*3.14159265 * i / (N - 1);
    
    double_vector_append( xl , 2*sin(x));
    double_vector_append( yl , 2*cos(x));
  }
  plot_range_update_vector_x( range , xl );
  plot_range_update_vector_y( range , yl );

  test_assert_double_equal( plot_range_get_current_xmin( range ) , -2 );
  test_assert_double_equal( plot_range_get_current_xmax( range ) ,  2 );
  test_assert_double_equal( plot_range_get_current_ymin( range ) , -2 );
  test_assert_double_equal( plot_range_get_current_ymax( range ) ,  2 );

  plot_range_free( range );
  double_vector_free( xl );
  double_vector_free( yl );
}


void test_update_range() {
  plot_range_type * range = plot_range_alloc();

  double xmin = -1;
  double xmax =  1;
  double ymin = -1;
  double ymax =  1;

  plot_range_update( range , 0 , 0 );
  plot_range_update( range , 0.5 , 0.5 );
  plot_range_update( range , 1.0 , 0.5 );
  plot_range_update( range , 0.0 , -1.0 );
  plot_range_update( range , -0.5 , -1.0 );
  plot_range_update( range , -1.0 ,  1.0 );

  test_assert_double_equal( xmin , plot_range_get_current_xmin( range ) );
  test_assert_double_equal( xmax , plot_range_get_current_xmax( range ) );
  test_assert_double_equal( ymin , plot_range_get_current_ymin( range ) );
  test_assert_double_equal( ymax , plot_range_get_current_ymax( range ) );

  plot_range_free( range );
}



void test_limits_default() {
  plot_range_type * range = plot_range_alloc();
  double xmin = -1;
  double xmax =  1;
  double ymin = -2;
  double ymax =  2;

 
  plot_range_update( range , 0 , 0 );
  plot_range_update( range , 0.5 , 0.5 );
  plot_range_update( range , xmax , 0.5 );
  plot_range_update( range , 0.0 , ymin );
  plot_range_update( range , -0.5 , -1.0 );
  plot_range_update( range , xmin ,  ymax );

  plot_range_set_invert_x_axis( range , false );
  plot_range_set_invert_y_axis( range , false );
  {
    double x1 , x2 , y1 , y2;
    double w = fabs(xmax - xmin);
    double h = fabs(ymax - ymin);
    
    plot_range_get_limits( range , &x1 , &x2 , &y1 , &y2 );
    test_assert_double_equal(  xmin  -  w*0.025 , x1 );
    test_assert_double_equal(  xmax  +  w*0.025 , x2 );
    test_assert_double_equal(  ymin  -  h*0.025 , y1 );
    test_assert_double_equal(  ymax  +  h*0.025 , y2 );
  }
  plot_range_free( range );
}


void test_limits_manual() {
  plot_range_type * range = plot_range_alloc();
  double xmin = -1;
  double xmax =  1;
  double ymin = -2;
  double ymax =  2;

 
  plot_range_update( range , 0 , 0 );
  plot_range_update( range , 0.5 , 0.5 );
  plot_range_update( range , xmax , 0.5 );
  plot_range_update( range , 0.0 , ymin );
  plot_range_update( range , -0.5 , -1.0 );
  plot_range_update( range , xmin ,  ymax );

  plot_range_set_manual_xmax( range , 2*xmax );
  plot_range_set_manual_ymin( range , 2*ymin );

  
  {
    double x1 , x2 , y1 , y2;
    double w = fabs(2*xmax - xmin);
    double h = fabs(ymax - 2*ymin);
    
    plot_range_get_limits( range , &x1 , &x2 , &y1 , &y2 );

    test_assert_double_equal(  xmin  -  w*0.025 , x1 );
    test_assert_double_equal(  2*xmax           , x2 );
    test_assert_double_equal(  2*ymin           , y1 );
    test_assert_double_equal(  ymax  +  h*0.025 , y2 );
  }
  plot_range_free( range );
}


void test_limits_inverted() {
  plot_range_type * range = plot_range_alloc();
  double xmin = 0;
  double xmax = 100;
  double ymin = 0;
  double ymax = 1000;

 
  plot_range_update( range , xmin , ymin );
  plot_range_update( range , xmax , ymin );
  plot_range_update( range , xmin , ymax );
  plot_range_update( range , xmax , ymax );
  
  plot_range_set_left_padding( range , 0.50 );
  plot_range_set_right_padding( range , 0.10 );
  plot_range_set_top_padding( range , 0.50 );
  plot_range_set_bottom_padding( range , 0.10 );
  
  plot_range_set_invert_x_axis( range , true );
  plot_range_set_invert_y_axis( range , true );
  {
    double x1 , x2 , y1 , y2;
    
    plot_range_get_limits( range , &x1 , &x2 , &y1 , &y2 );
    
    test_assert_double_equal(150 , x1 );
    test_assert_double_equal(-10 , x2 );

    test_assert_double_equal(1100 , y1 );
    test_assert_double_equal(-500 , y2 );
  }
  plot_range_free( range );
}






void test_set_manual1() {
  plot_range_type * range = plot_range_alloc();
  double xmin = -1;
  double xmax =  1;
  double ymin = -2;
  double ymax =  2;

  
  plot_range_set_manual_xmin( range , xmin );
  test_assert_double_equal( xmin , plot_range_get_manual_xmin( range ));
  plot_range_unset_manual_xmin( range );
  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_xmin( range ));


  plot_range_set_manual_xmax( range , xmax );
  test_assert_double_equal( xmax , plot_range_get_manual_xmax( range ));
  plot_range_unset_manual_xmax( range );
  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_xmax( range ));


  plot_range_set_manual_ymin( range , ymin );
  test_assert_double_equal( ymin , plot_range_get_manual_ymin( range ));
  plot_range_unset_manual_ymin( range );
  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_ymin( range ));


  plot_range_set_manual_ymax( range , ymax );
  test_assert_double_equal( ymax , plot_range_get_manual_ymax( range ));
  plot_range_unset_manual_ymax( range );
  test_assert_double_equal( PLOT_RANGE_DEFAULT_MANUAL_LIMIT , plot_range_get_manual_ymax( range ));

  plot_range_free( range );
}



int main(int argc , char ** argv) {

  test_create_range();
  test_default();
  test_empty_range( );
  test_update_range();
  test_set_manual1( );
  
  test_limits_default( );
  test_limits_manual( );
  test_limits_inverted( );
  
  exit(0);
}
