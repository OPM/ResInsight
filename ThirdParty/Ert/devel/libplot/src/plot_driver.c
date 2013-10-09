/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/plot/plot_driver.h>
#include <ert/plot/plot_const.h>



/**
   This file implements a thin 'class' plot_driver_type which contains
   function pointers for the basic plot operations; in addition it
   contains a void pointer 'state' which can contain arbitrary data.
   
   The driver has/must implement the following functions:

     close_driver( driver ): This function should complete all pending
        plot operations (i.e. close files+++), and then free all the
        resources occupied by the driver. 

     set_labels( driver , title , xlabel , ylabel , label_color ,
         label_font_size ): This function should set the title/labels.

     set_window_size( driver , width , height): Set the size of the
         plot, in pixel units.

     set_axis( driver , range , timefmt , box_color , tick_font_size):
         The range instance contains max/min for the plot (in plot
         coordinates). If timefmt != NULL it is assumed that
         time/dates should be used on the x-axis.

     ----------------------------------------------------------------

     plot_xy( driver , label , x , y , style , line_attr ,
         point_attr): This function should plot an 'ordinary' xy
         plot. Depending in the value of style the function should
         plot either lines, points or both.

     plot_xy1y2( driver , label , x , y1 , y2 , line_attr ): This
         function plots vertical error bars. Observe that the dat
         given is in the form (x,y1,y2), i.e. not (x,y,std).

     plot_x1x2y(): Plots horisontal error bars.

     plot_hist(): Plots a histogram.

     
   The functions listed above the line are not compulsary; the driver
   is free to not implement those. The functions below the line must
   be implemented; otherwise the program will fail hard.

   When the plotting functionality is used to create a plot the driver
   functions are called in this order:

     Function                             Call-stack
     -----------------------------------------------------------------------
     1. driver_alloc()                    plot_alloc()
     2. set_window_size()                 plot_data()
     3. set_labels()                      plot_data()
     4. set_axis()                        plot_data()
     5. plot_xy() / plot_xy1y2() / ...    plot_data() :: plot_dataset_draw()
     6. close_driver()                    plot_free() 
     -----------------------------------------------------------------------
     
*/




plot_driver_type * plot_driver_alloc_empty(const char * driver_name) {
  plot_driver_type * driver = util_malloc( sizeof * driver );
  driver->driver_name       = util_alloc_string_copy( driver_name );

  driver->state             = NULL;            
  driver->set_labels        = NULL;     
  driver->close_driver      = NULL;    
  driver->set_window_size   = NULL;
  driver->set_axis          = NULL;

  driver->text              = NULL;
  driver->plot_xy           = NULL;
  driver->plot_xy1y2        = NULL;
  driver->plot_x1x2y        = NULL;
  driver->plot_hist         = NULL;
  driver->set_log           = NULL;
  
  return driver;
}

/**
   Asserts that all the plot plot_xxx functions are set. 
*/
void plot_driver_assert( const plot_driver_type * driver ){ 
  if (driver->plot_xy    == NULL) util_abort("%s: The driver:%s does not have a plot_xy function - aborting\n",__func__ , driver->driver_name);
  if (driver->plot_xy1y2 == NULL) util_abort("%s: The driver:%s does not have a plot_xy1y2 function - aborting\n",__func__ , driver->driver_name);
  if (driver->plot_x1x2y == NULL) util_abort("%s: The driver:%s does not have a plot_x1x2y function - aborting\n",__func__ , driver->driver_name);
  if (driver->plot_hist  == NULL) util_abort("%s: The driver:%s does not have a plot_hist function - aborting\n",__func__ , driver->driver_name);
}

/**
   If the state pointer contains anything it is the responsability of
   the actual driver implementation to free that first, before calling
   plot_driver_free().
*/

void plot_driver_free( plot_driver_type * driver ) {
  if (driver->close_driver != NULL)
    driver->close_driver( driver );  
  /*
    else: The driver has no internal free/close function. A bit suspicious...
  */
  util_safe_free( driver->driver_name );
  free( driver );
}


/*****************************************************************/
/* 
   Here comes a series of very thin wrapper functions, which
   immediately call the the underlying driver function. Some of the
   functions will abort (i.e. the plot_xxx functions) if the driver
   does not have a function, whereas others will just print a warning.

   The plot / plot_dataset layer should call these functions, instead
   of using the driver functions directly.
*/


void plot_driver_plot_xy( plot_driver_type * driver , const char * label , 
                          double_vector_type * x  , 
                          double_vector_type * y  , 
                          plot_style_type style         , 
                          line_attribute_type line_attr , 
                          point_attribute_type point_attr) {
  
  driver->plot_xy( driver  , label , x , y , style , line_attr , point_attr);
}

 

void plot_driver_plot_xy1y2(plot_driver_type * driver     , 
                            const char * label , 
                            double_vector_type * x  , 
                            double_vector_type * y1  , 
                            double_vector_type * y2  , 
                            line_attribute_type line_attr) {
  
  driver->plot_xy1y2( driver , label , x , y1 , y2 , line_attr);

}



void plot_driver_plot_x1x2y(plot_driver_type * driver      , 
                            const char * label             , 
                            double_vector_type * x1  , 
                            double_vector_type * x2  , 
                            double_vector_type * y   , 
                            line_attribute_type line_attr) {
  driver->plot_x1x2y( driver , label , x1 , x2  , y , line_attr);
}


void plot_driver_text( plot_driver_type * driver , const plot_text_type * plot_text) {
  if (driver->text)
    driver->text( driver , plot_text);
  else
    fprintf(stderr,"** Sorry: this plot driver does not support the text() function\n");
}



void plot_driver_plot_yline( plot_driver_type * driver , const char * label , double xmin , double xmax , double y0 , line_attribute_type line_attr) {
  double_vector_type * x = double_vector_alloc(2 , 0);
  double_vector_type * y = double_vector_alloc(2 , 0);
  
  double_vector_iset(x , 0 , xmin );
  double_vector_iset(x , 1 , xmax );
  double_vector_iset(y , 0 , y0 );
  double_vector_iset(y , 1 , y0 );
  
  {
    point_attribute_type point_attr /* Complete dummy */;
    plot_driver_plot_xy( driver , label , x , y , LINE , line_attr , point_attr);
  }
  
  double_vector_free( x );
  double_vector_free( y );
}




void plot_driver_plot_xline( plot_driver_type * driver , const char * label , double x0 , double ymin , double ymax , line_attribute_type line_attr) {
  double_vector_type * x = double_vector_alloc(2 , 0);
  double_vector_type * y = double_vector_alloc(2 , 0);
  
  double_vector_iset(x , 0 , x0 );
  double_vector_iset(x , 1 , x0 );
  double_vector_iset(y , 0 , ymin );
  double_vector_iset(y , 1 , ymax );
  
  {
    point_attribute_type point_attr /* Complete dummy */;
    plot_driver_plot_xy( driver , label , x , y , LINE , line_attr , point_attr);
  }
  
  double_vector_free( x );
  double_vector_free( y );
}



void plot_driver_plot_hist( plot_driver_type * driver, const char * label , double_vector_type * x , line_attribute_type line_attr) {
  driver->plot_hist( driver , label , x , line_attr );
}


/*****************************************************************/


void plot_driver_set_axis( plot_driver_type * driver , plot_range_type * range , const char * timefmt , plot_color_type box_color , double tick_font_size) {
  if (driver->set_axis != NULL)
    driver->set_axis(driver , range , timefmt , box_color , tick_font_size);
  /*
    else: The driver does not have a setup_plot function; that is fair enough.
  */
}


void plot_driver_set_labels(plot_driver_type * driver , const char *title , const char * xlabel , const char * ylabel, plot_color_type label_color , double label_font_size) {
  if (driver->set_labels != NULL)
    driver->set_labels(driver , title , xlabel  , ylabel , label_color , label_font_size);
  /*
    else: no function to set labels - fair enough.
  */
}



void plot_driver_set_window_size(plot_driver_type * driver , int width , int height) {
  if (driver->set_window_size != NULL)
    driver->set_window_size(driver , width , height );
}


void plot_driver_set_log(plot_driver_type * driver , bool logx , bool logy) {
  if (driver->set_log != NULL)
    driver->set_log(driver , logx , logy);
}

