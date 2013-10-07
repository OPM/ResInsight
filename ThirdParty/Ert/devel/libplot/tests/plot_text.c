/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'plot_text.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>

#include <ert/plot/plot_text.h>
#include <ert/plot/plot_range.h>

void test_create_normal( ) {
  double xpos = 0.0;
  double ypos = 1.0;
  double font_scale = 0.07;
  
  const char * text = "Bjarne";
  plot_text_type * plot_text = plot_text_alloc( xpos , ypos , font_scale , text );

  test_assert_true( plot_text_is_instance( plot_text ));
  test_assert_string_equal( text , plot_text_get_text( plot_text ));
  test_assert_double_equal( xpos , plot_text_get_x( plot_text ));
  test_assert_double_equal( ypos , plot_text_get_y( plot_text ));
  test_assert_double_equal( font_scale , plot_text_get_font_scale( plot_text ));
  
  plot_text_free( plot_text );
}


void test_create_NULL_content( ) {
  plot_text_type * plot_text = plot_text_alloc( 1.0 , 1.0 , 1.0 , NULL );

  test_assert_NULL( plot_text );
}


void test_update_range() {
  double xpos = 1.56;
  double ypos = 1.0;
  double font_scale = 0.07;
  
  const char * text = "Bjarne";
  plot_text_type * plot_text = plot_text_alloc( xpos , ypos , font_scale , text );
  plot_range_type * plot_range = plot_range_alloc();

  plot_text_update_range( plot_text , plot_range );
  
  test_assert_double_equal( xpos , plot_range_get_current_xmin( plot_range ));
  test_assert_double_equal( xpos , plot_range_get_current_xmax( plot_range ));
  test_assert_double_equal( ypos , plot_range_get_current_ymin( plot_range ));
  test_assert_double_equal( ypos , plot_range_get_current_ymax( plot_range ));  
}





int main(int argc , char ** argv) {
  
  test_create_normal( );
  test_create_NULL_content( );
  test_update_range( );
  
  exit(0);
}
