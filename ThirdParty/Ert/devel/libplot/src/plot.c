/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <string.h>

#include <ert/util/hash.h>
#include <ert/util/vector.h>

#include <ert/plot/plplot_driver.h>
#include <ert/plot/plot.h>
#include <ert/plot/plot_dataset.h>
#include <ert/plot/plot_range.h>
#include <ert/plot/plot_const.h>
#include <ert/plot/plot_driver.h>
#include <ert/plot/text_driver.h>


/**
   The plotting functionality is centered around the plot_type
   struct. The plot_type object contains essentially three things:

   1. It contains all the data which should be plotted; this is
      handled as a vector of plot_dataset instances. 

   2. It contains configurutaion options which are common to 'all'
      plots; i.e. labels colors e.t.c.

   3. It contains a plot_driver instance - this is a struct containing
      function pointers, and optionally state information. The
      functions in plot_driver do the actual plotting. See the
      documentation in plot_driver.c for the various functions and
      how/when they are called.

*/






struct plot_struct {
  char               * timefmt;          /* The format string used to convert from time_t -> strtime on the x-axis; NULL if not used. */
  vector_type        * dataset;          /* Vector of datasets to plot. */ 
  hash_type          * dataset_hash;     /* Hash table of the datasets - indexed by label. */
  bool                 is_histogram;     /* If this is true it can only contain histogram datasets. */
  
  char               * xlabel;           /* Label for the x-axis */
  char               * ylabel;           /* Label for the y-axis */
  char               * title;            /* Plot title */      

  plot_color_type      label_color;      /* Color for the labels */
  plot_color_type      box_color;        /* Color for the axis / box surrounding the plot. */
  double               axis_font_size;   /* Scale factor for the font used on the axes. */
  double               label_font_size;  /* Scale factor for label font size. */ 
  int                  height;           /* The height of your plot window */
  int                  width;            /* The width of your plot window */
  bool                 logy;
  bool                 logx;
  
  plot_range_type    * range;            /* Range instance - keeping control over the min/max values on the x and y axis.*/
  /*****************************************************************/
  plot_driver_type   * driver;           /* Plot driver - mainly list of function pointers to 'actually do it'. */
};




/*
  Internalizes the size of the window; the window size will be
  actually set at a later stage with a call to the driver function
  driver->set_window_size.
*/

void plot_set_window_size(plot_type * plot, int width, int height)
{
  plot->width  = width;
  plot->height = height;
}



/** 
    Observe that this function also tells the plot driver that date
    labels should be used on the x-axis.
*/

void plot_set_timefmt(plot_type * plot , const char * timefmt) {
  plot->timefmt = util_realloc_string_copy( plot->timefmt , timefmt );
}



/**
   This will try to guess a reasonable format string to send to
   plot_set_timefmt() based on the time difference between t1 and
   t2. This will obviously be quite heuristic.

   The selected timefmt is returned for the calling scope to inspect,
   but the calling scope SHOULD NOT touch this return value (and is of
   course free to ignore it completely).
*/

const char * plot_set_default_timefmt(plot_type * plot , time_t t1 , time_t t2) {
  const int minute = 60;
  const int hour   = minute * 60;
  const int day    = hour   * 24;
  const int week   = day    * 7;
  //const int month  = day    * 30;
  const int year   = day    * 365; 
    
  double diff_time = difftime(t2 , t1);

  if (diff_time < day) 
    plot_set_timefmt(plot , "%H:%M");       /* Hour:Minute */ 
  else if (diff_time < week)
    plot_set_timefmt(plot , "%a: %H:%M");   /* Weekday  Hour:Minute */
  else if (diff_time < year)
    plot_set_timefmt(plot , "%d/%m");       /* Monthday/month */
  else
    plot_set_timefmt(plot , "%b %Y");       /* Short month-name Year */

  return plot->timefmt;
}



/**
   Observe that there has been lots of trouble with the PLPLOT driver
   aborting because of two small tick distance when using the log plot
   option. The current implementation (i.e. hack) is just to apply the
   log10 operation directly on the data in plot_dataset scope.  
*/

static void plot_set_log( plot_type * plot , bool logx , bool logy) {
  plot_driver_set_log(plot->driver , logx , logy);  /* The driver can ignore this. */
  plot->logx            = logx;
  plot->logy            = logy;
}


/**
   This function allocates the plot handle. The first argument is a
   string, which identifies the driver, the second argument is passed
   directly on the to driver allocation routine; and you must check
   the documentation of the specific driver allocation routine to see
   what init_arg should be like.
*/

plot_type * plot_alloc(const char * __driver_type , const void * init_arg , bool logx , bool logy)
{
  plot_type * plot = util_malloc(sizeof *plot );
  {
    /*
      Loading the driver:
    */
    char * driver_type = util_alloc_string_copy( __driver_type );
    util_strupr( driver_type );

    if (util_string_equal( driver_type , "PLPLOT"))
      plot->driver  = plplot_driver_alloc(init_arg);
    else if (util_string_equal( driver_type , "TEXT"))
      plot->driver  = text_driver_alloc(init_arg);
    else
      util_abort("%s: plot driver:%s not implemented ... \n",__func__ , __driver_type);
    
    plot_driver_assert( plot->driver );

    free( driver_type );
  }

  /* Initializing plot data which is common to all drivers. */
  plot->is_histogram    = false;
  plot->dataset         = vector_alloc_new();
  plot->dataset_hash    = hash_alloc();
  plot->range           = plot_range_alloc();
  plot->timefmt         = NULL;
  plot->xlabel          = NULL;
  plot->ylabel          = NULL;
  plot->title           = NULL;
  
  /* 
     These functions only manipulate the internal plot_state
     variables, and do not call the driver functions.
  */
  plot_set_window_size(plot , PLOT_DEFAULT_WIDTH , PLOT_DEFAULT_HEIGHT);
  plot_set_box_color(plot , PLOT_DEFAULT_BOX_COLOR);
  plot_set_label_color(plot , PLOT_DEFAULT_LABEL_COLOR);
  plot_set_label_fontsize(plot , 1.0);
  plot_set_axis_fontsize(plot , 1.0);
  plot_set_labels(plot , "" , "" , ""); /* Initializeing with empty labels. */
  plot_set_log( plot , logx , logy); /* Default - no log on the axis. */
  return plot;
}




/**
   Allocates a new dataset, and attaches it to the plot. When adding
   data to the dataset, setting attributes+++ you should use
   plot_dataset_xxx functions with the return value from this
   function.
*/

plot_dataset_type * plot_alloc_new_dataset(plot_type * plot , const char * __label , plot_data_type data_type) {
  if (data_type == PLOT_HIST) {
    if (vector_get_size( plot->dataset) > 0)
      util_abort("%s: sorry - when using histograms you can *only* have one dataset\n",__func__);
    plot->is_histogram = true;
  } else if (plot->is_histogram)
    util_abort("%s: sorry - when using histograms you can *only* have one dataset\n",__func__);
  
  {
    char * label;
    if (__label == NULL)
      label = util_alloc_sprintf("data_%d" , vector_get_size( plot->dataset ));
    else
      label = (char *) __label;
    
    if (hash_has_key( plot->dataset_hash , label))
      util_abort("%s: sorry - the label %s is already in use - must be unique \n",__func__ , label);
    {
      plot_dataset_type * dataset = plot_dataset_alloc(data_type, label , plot->logx , plot->logy);
      vector_append_owned_ref(plot->dataset , dataset , plot_dataset_free__);
      hash_insert_ref( plot->dataset_hash , label , dataset);
      if (__label == NULL)
        free(label);
      return dataset;
    }
  }
}  


static void plot_free_all_datasets(plot_type * plot) {
  vector_clear( plot->dataset );
  hash_clear( plot->dataset_hash );
}


/**
   This function will close all pending/halfopen plot operations and
   free all resources used by the plot.
*/
void plot_free( plot_type * plot )
{
  plot_driver_free( plot->driver );
  plot_free_all_datasets(plot);
  plot_range_free(plot->range);
  util_safe_free(plot->timefmt);
  util_safe_free(plot);
}



/**
   The function does the following:
   
   Automatic range
   ===============

    1. Looping through all the datasets to find the minimum and
       maximum values of x and y, these are set in the plot_range
       struct.

    2. The plot_range() methods are used to calculate final range
       xmin,xmax,ymin,ymax values based on the extremal values from
       point 1, padding values and invert_axis flags.

    3. The final xmin,xmax,ymin,ymax values are returned by reference.

   Manual range 
   ============
    
    1. The (already manually set) xmin,xmax,ymin,ymax values are
       returned by reference.

*/

static void plot_set_range__(plot_type * plot) {
  plot_update_range(plot , plot->range);
  plot_range_apply( plot->range );
}




/**
   This were the plot is finally made.
*/
void plot_data(plot_type * plot)
{
  int iplot;
  plot_driver_type * driver = plot->driver;
  
  plot_set_range__(plot);
  
  plot_driver_set_window_size( driver , plot->width , plot->height );
  plot_driver_set_labels( driver , plot->title , plot->xlabel  , plot->ylabel , plot->label_color , plot->label_font_size);
  plot_driver_set_axis( driver , plot->range , plot->timefmt , plot->box_color , plot->axis_font_size );
  
  for (iplot = 0; iplot < vector_get_size( plot->dataset ); iplot++) 
    plot_dataset_draw(vector_iget(plot->dataset , iplot) , driver , plot->range);
}






/* 
   This is the low-level function setting the range of the plot.
*/
   
/*****************************************************************/


void plot_set_range(plot_type * plot , double xmin , double xmax , double ymin , double ymax) {
  plot_range_set_range(plot->range , xmin , xmax , ymin , ymax);
}

void plot_set_xmin(plot_type * plot , double xmin) {
  plot_range_set_xmin( plot->range , xmin );
}

void plot_set_xmax(plot_type * plot , double xmax) {
  plot_range_set_xmax( plot->range , xmax );
}

void plot_set_ymin(plot_type * plot , double ymin) {
  plot_range_set_ymin( plot->range , ymin );
}

void plot_set_ymax(plot_type * plot , double ymax) {
  plot_range_set_ymax( plot->range , ymax );
}


void plot_set_left_padding(plot_type * plot , double value) {
  plot_range_set_left_padding(plot->range, value);
}

void plot_set_right_padding(plot_type * plot , double value) {
  plot_range_set_right_padding(plot->range , value);
}

void plot_set_top_padding(plot_type * plot , double value) {
  plot_range_set_top_padding(plot->range , value);
}

void plot_set_bottom_padding(plot_type * plot , double value) {
  plot_range_set_bottom_padding(plot->range , value);
}

void plot_set_padding(plot_type * plot , double padding) {
  plot_set_left_padding  ( plot , padding );
  plot_set_right_padding ( plot , padding );
  plot_set_top_padding   ( plot , padding );
  plot_set_bottom_padding( plot , padding );
}


/*****************************************************************/

void plot_invert_x_axis(plot_type * plot) {
  plot_range_invert_x_axis(plot->range , true);
}

void plot_invert_y_axis(plot_type * plot) {
  plot_range_invert_y_axis(plot->range , true);
}

/*****************************************************************/

void plot_set_label_color(plot_type * plot , plot_color_type label_color) {
  plot->label_color = label_color;
}


void plot_set_box_color(plot_type * plot , plot_color_type box_color) {
  plot->box_color = box_color;
}

void plot_set_label_fontsize(plot_type * plot , double label_font_size_scale) {
  plot->label_font_size = label_font_size_scale;
}

void plot_set_axis_fontsize(plot_type * plot , double axis_font_size_scale) {
  plot->axis_font_size = axis_font_size_scale;
}

void plot_set_xlabel(plot_type * plot , const char * xlabel) {
  plot->xlabel = util_realloc_string_copy(plot->xlabel , xlabel);
}

void plot_set_ylabel(plot_type * plot , const char * ylabel) {
  plot->ylabel = util_realloc_string_copy(plot->ylabel , ylabel);
}

void plot_set_title(plot_type * plot , const char * title) {
  plot->title = util_realloc_string_copy(plot->title , title);
}


void plot_set_labels(plot_type * plot, const char *xlabel, const char *ylabel, const char *title)
{
  plot_set_xlabel(plot , xlabel);
  plot_set_ylabel(plot , ylabel);
  plot_set_title(plot , title);
}




/**
 * @brief Get extrema values
 * @param plot your current plot
 * @param x_max pointer to the new x maximum
 * @param y_max pointer to the new y maximum
 * @param x_min pointer to the new x minimum
 * @param y_min pointer to the new y minimum
 * 
 * Find the extrema values in the plot plot, checks all added datasets.
 */

void plot_update_range(plot_type * plot, plot_range_type * range) {
  if (plot->is_histogram) 
    plot_dataset_update_range_histogram( vector_iget(plot->dataset , 0) , range);
  else {
    bool first_pass = true;
    int iplot;
    for (iplot = 0; iplot < vector_get_size( plot->dataset  ); iplot++) 
      plot_dataset_update_range(vector_iget(plot->dataset , iplot) , &first_pass , range);
  }
}


