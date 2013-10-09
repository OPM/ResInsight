/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_dataset.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <string.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/double_vector.h>
#include <ert/util/bool_vector.h>

#include <plplot/plplot.h>
#include <plplot/plplotP.h>

#include <ert/plot/plot_const.h>
#include <ert/plot/plot_range.h>
#include <ert/plot/plot_dataset.h>
#include <ert/plot/plot_driver.h>




#define PLOT_DATASET_TYPE_ID 661409

/**
 * @brief Contains information about a dataset.
 */
struct plot_dataset_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                *label;             /* Label for the dataset - used as hash key in the plot instance. */ 
  double_vector_type  *x;                  /**< Vector containing x-axis data */
  double_vector_type  *y;                  /**< Vector containing y-axis data */
  double_vector_type  *y1;              
  double_vector_type  *y2;
  double_vector_type  *x1;
  double_vector_type  *x2;
  int                 data_mask;       /**< An integer value written as a sum of values from the enum plot_data_types - says which type of data (x/y/...) is supplied. */
  plot_data_type      data_type;       /**< One of the types: plot_xy, plot_xy1y2, plot_x1x2y , plot_xline , plot_yline */      

  plot_style_type       style;         /**< The graph style: line|points|line_points */

  line_attribute_type   line_attr;
  point_attribute_type  point_attr;
  bool                  logx;
  bool                  logy;
};



/*****************************************************************/
/* Set - functions for the style variabels of the dataset.       */

int plot_dataset_get_size( const plot_dataset_type * dataset ) {

  if (dataset->data_mask & PLOT_DATA_X)  return double_vector_size( dataset->x );
  if (dataset->data_mask & PLOT_DATA_Y)  return double_vector_size( dataset->y );
  if (dataset->data_mask & PLOT_DATA_X1) return double_vector_size( dataset->x1 );
  if (dataset->data_mask & PLOT_DATA_Y1) return double_vector_size( dataset->y1 );
  if (dataset->data_mask & PLOT_DATA_X2) return double_vector_size( dataset->x2 );
  if (dataset->data_mask & PLOT_DATA_Y2) return double_vector_size( dataset->y2 );
  
  util_abort("%s: internal error\n",__func__);
  return -1;
}

void plot_dataset_set_style(plot_dataset_type * dataset , plot_style_type style) {
  dataset->style = style;
}


void plot_dataset_set_line_color(plot_dataset_type * dataset , plot_color_type line_color) {
  dataset->line_attr.line_color = line_color;
}


void plot_dataset_set_point_color(plot_dataset_type * dataset , plot_color_type point_color) {
  dataset->point_attr.point_color = point_color;
}


void plot_dataset_set_line_style(plot_dataset_type * dataset , plot_line_style_type line_style) {
  dataset->line_attr.line_style = line_style;
}

void plot_dataset_set_symbol_size(plot_dataset_type * dataset , double symbol_size) {
  dataset->point_attr.symbol_size = symbol_size;
}

void plot_dataset_set_line_width(plot_dataset_type * dataset , double line_width) {
  dataset->line_attr.line_width = line_width;
}

void plot_dataset_set_symbol_type(plot_dataset_type * dataset, plot_symbol_type symbol_type) {
  dataset->point_attr.symbol_type = symbol_type;
}


/*****************************************************************/

//double * plot_dataset_get_vector_x(const plot_dataset_type * d)  { return d->x; }
//double * plot_dataset_get_vector_y(const plot_dataset_type * d)  { return d->y; }
//double * plot_dataset_get_vector_x1(const plot_dataset_type * d) { return d->x1; }
//double * plot_dataset_get_vector_y1(const plot_dataset_type * d) { return d->y1; }
//double * plot_dataset_get_vector_x2(const plot_dataset_type * d) { return d->x2; }
//double * plot_dataset_get_vector_y2(const plot_dataset_type * d) { return d->y2; }


static void plot_dataset_assert_type(const plot_dataset_type * d, plot_data_type type) {
  if (d->data_type != type)
    util_abort("%s: assert failed - wrong plot type \n",__func__);
}



static int  __make_data_mask(plot_data_type data_type) {
  int mask = 0;
  switch (data_type) {
  case(PLOT_XY):
    mask = PLOT_DATA_X + PLOT_DATA_Y;
    break;
  case(PLOT_XY1Y2):
    mask = PLOT_DATA_X + PLOT_DATA_Y1 + PLOT_DATA_Y2;
    break;
  case(PLOT_X1X2Y):
    mask = PLOT_DATA_X1 + PLOT_DATA_X2 + PLOT_DATA_Y;
    break;
  case(PLOT_XLINE):
    mask = PLOT_DATA_X;
    break;
  case(PLOT_YLINE):
    mask = PLOT_DATA_Y;
    break;
  case(PLOT_HIST):
    mask = PLOT_DATA_X;
    break;
  default:
    util_abort("%s: unrecognized value: %d \n",__func__ , data_type);
  }
  return mask;
}



void plot_dataset_fprintf(const plot_dataset_type * dataset , FILE * stream) {
  fprintf(stream , "    x              y            x1            x2            y1           y2 \n");
  fprintf(stream , "----------------------------------------------------------------------------\n");
  for (int i = 0; i < plot_dataset_get_size(dataset); i++) {
    fprintf(stream , "%12.7f  %12.7f  %12.7f  %12.7f  %12.7f  %12.7f\n",
            double_vector_safe_iget(dataset->x , i ),
            double_vector_safe_iget(dataset->y , i ),
            double_vector_safe_iget(dataset->x1 , i),
            double_vector_safe_iget(dataset->x2 , i),
            double_vector_safe_iget(dataset->y1 , i ),
            double_vector_safe_iget(dataset->y2 , i));
  }
  fprintf(stream , "----------------------------------------------------------------------------\n");
}




/**
 * @return Returns a new plot_dataset_type pointer.
 * @brief Create a new plot_dataset_type
 *
 * Create a new dataset - allocates the memory.
 */
plot_dataset_type *plot_dataset_alloc(plot_data_type data_type , const char* label , bool logx , bool logy) {
  plot_dataset_type *d;
  
  d                = util_malloc(sizeof *d );
  UTIL_TYPE_ID_INIT(d , PLOT_DATASET_TYPE_ID);
  d->data_type     = data_type;
  d->data_mask     = __make_data_mask(data_type);
  d->x             = double_vector_alloc( 0 , -1 );
  d->y             = double_vector_alloc( 0 , -1 );
  d->x1            = double_vector_alloc( 0 , -1 );
  d->x2            = double_vector_alloc( 0 , -1 );
  d->y1            = double_vector_alloc( 0 , -1 );
  d->y2            = double_vector_alloc( 0 , -1 );
  d->label         = util_alloc_string_copy( label );
  d->logx          = logx;
  d->logy          = logy;
  /******************************************************************/
  /* Defaults                                                       */
  d->style       = LINE;
  {
    line_attribute_type  line_attr  = {.line_color  = PLOT_DEFAULT_LINE_COLOR  , .line_style  = PLOT_DEFAULT_LINE_STYLE , .line_width  = 1.0};  
    point_attribute_type point_attr = {.point_color = PLOT_DEFAULT_POINT_COLOR , .symbol_type = PLOT_DEFAULT_SYMBOL     , .symbol_size = 1.0};
    
    d->line_attr   = line_attr;
    d->point_attr  = point_attr;
  }
  
  return d;
}

UTIL_SAFE_CAST_FUNCTION(plot_dataset , PLOT_DATASET_TYPE_ID)


/**
   This function frees all the memory related to a dataset - normally
   called automatically from plot_free().
*/
void plot_dataset_free(plot_dataset_type * d)
{
  double_vector_free(d->x);
  double_vector_free(d->x1);
  double_vector_free(d->x2);
  double_vector_free(d->y);
  double_vector_free(d->y1);
  double_vector_free(d->y2);
  free(d->label);
  free(d);
}

void plot_dataset_free__(void * d) {
  plot_dataset_type * pd = plot_dataset_safe_cast ( d );
  plot_dataset_free( pd );
}


/*****************************************************************/
/** Here comes functions for setting the data for the dataset. */




static void __append_vector(double_vector_type * target, const double * src , const bool_vector_type * mask , bool log_data) {
  if (src == NULL)
    util_abort("%s: trying to extract data from NULL pointer\n",__func__);
  
  
  for (int index = 0; index < bool_vector_size( mask ); index++) {
    if (bool_vector_iget(mask , index)) {
      if (log_data)
        double_vector_append( target , log10( src[index] ) );
      else
        double_vector_append( target , src[index] );
    }
  }
}



/**
   Before a tuple, i.e. (x,y), (x,y1,y2) , (x1,x2,y), ... is added to
   the dataset we verify that the test isfinite(·) returns true for
   all the elements in the tuple.  

   This functionality is implemented with the boolean vector 'mask'.
*/

void __update_mask( bool_vector_type * mask , const double * data , bool log_data) {
  if (data != NULL) {
    for (int index = 0; index < bool_vector_size( mask ); index++) {
      bool value = data[index];
      if (log_data)
        value = log( value );

      if (!isfinite( value )) 
        /* This datapoint is marked as invalid - and not added to the proper datavectors. */
        bool_vector_iset( mask , index , false );  
    }
  }
}


static void plot_dataset_append_vector__(plot_dataset_type * d , int size , const double * x , const double * y , const double * y1 , const double * y2 , const double *x1 , const double *x2) {
  bool_vector_type * mask = bool_vector_alloc( size , true );   /* Initialize to all true */
  
  __update_mask(mask , x  , d->logx);
  __update_mask(mask , y  , d->logy);
  __update_mask(mask , y1 , d->logy);
  __update_mask(mask , y2 , d->logy);
  __update_mask(mask , x1 , d->logx);
  __update_mask(mask , x2 , d->logx);
    
  if (d->data_mask & PLOT_DATA_X)  __append_vector(d->x  , x  ,  mask , d->logx);
  if (d->data_mask & PLOT_DATA_X1) __append_vector(d->x1 , x1 ,  mask , d->logx);
  if (d->data_mask & PLOT_DATA_X2) __append_vector(d->x2 , x2 ,  mask , d->logx);
  if (d->data_mask & PLOT_DATA_Y)  __append_vector(d->y  , y  ,  mask , d->logy);
  if (d->data_mask & PLOT_DATA_Y1) __append_vector(d->y1 , y1 ,  mask , d->logy);
  if (d->data_mask & PLOT_DATA_Y2) __append_vector(d->y2 , y2 ,  mask , d->logy);

  bool_vector_free(mask);
}



/*****************************************************************/
/* Here comes the exported functions - they all have _xy, _xy1y2,
   _xline ... suffix. */


void plot_dataset_append_vector_xy(plot_dataset_type *d , int size, const double * x , const double *y) {
  plot_dataset_assert_type(d , PLOT_XY);
  plot_dataset_append_vector__(d , size , x , y , NULL , NULL , NULL , NULL);
}


void plot_dataset_append_point_xy(plot_dataset_type *d , double x , double y) {
  plot_dataset_append_vector_xy(d , 1 , &x , &y);
}


/*----*/


void plot_dataset_append_vector_xy1y2(plot_dataset_type *d , int size, const double * x , const double *y1 , const double *y2) {
  plot_dataset_assert_type(d , PLOT_XY1Y2);
  plot_dataset_append_vector__(d , size , x , NULL , y1 , y2 , NULL , NULL);
}


void plot_dataset_append_point_xy1y2(plot_dataset_type *d , double x , double y1 , double y2) {
  plot_dataset_append_vector_xy1y2(d , 1 , &x , &y1, &y2);
}



/*----*/

void plot_dataset_append_vector_x1x2y(plot_dataset_type *d , int size, const double * x1 , const double *x2 , const double *y) {
  plot_dataset_assert_type(d , PLOT_X1X2Y);
  plot_dataset_append_vector__(d , size , NULL , y , NULL , NULL , x1 , x2);
}


void plot_dataset_append_point_x1x2y(plot_dataset_type *d , double x1 , double x2 , double y) {
  plot_dataset_append_vector_x1x2y(d , 1 , &x1 , &x2 , &y);
}


/*----*/


/**
   The xline and yline datasets can only be used to plot
   _one_single_line_. If you want more than one vertical/horizontal
   line you must instantiate one plot_dataset for each line.
*/


void plot_dataset_set_xline(plot_dataset_type *d , double x) {
  if (isfinite(x))
    double_vector_iset( d->x , 0 , x );
}


/*----*/


void plot_dataset_set_yline(plot_dataset_type *d , double y) {
  if (isfinite(y))
    double_vector_iset( d->y , 0 , y );
}



/*----*/

/*----*/

void plot_dataset_append_vector_hist(plot_dataset_type *d , int size, const double * x) {
  plot_dataset_assert_type(d , PLOT_HIST);
  plot_dataset_append_vector__(d , size , x , NULL , NULL , NULL , NULL , NULL );
}




void plot_dataset_append_point_hist(plot_dataset_type *d , double x) {
  plot_dataset_append_vector_hist(d , 1 , &x);
}


/*****************************************************************/





void plot_dataset_draw(plot_dataset_type * d , plot_driver_type * driver, const plot_range_type * range) {

  switch (d->data_type) {
  case(PLOT_XY):
    plot_driver_plot_xy( driver , d->label , d->x , d->y , d->style , d->line_attr , d->point_attr);
    break;
  case(PLOT_HIST):
    plot_driver_plot_hist( driver , d->label , d->x , d->line_attr );
    break;
  case(PLOT_XY1Y2):
    plot_driver_plot_xy1y2( driver , d->label , d->x , d->y1 , d->y2 , d->line_attr);
    break;
  case(PLOT_X1X2Y):
    plot_driver_plot_x1x2y( driver , d->label , d->x1 , d->x2 , d->y , d->line_attr);
    break;
  case(PLOT_YLINE):
    plot_driver_plot_yline( driver , 
                            d->label , 
                            plot_range_get_current_xmin(range) , 
                            plot_range_get_current_xmax(range) , 
                            double_vector_iget( d->y , 0) , 
                            d->line_attr);
    break;
  case(PLOT_XLINE):
    plot_driver_plot_xline( driver , 
                            d->label , 
                            double_vector_iget( d->x , 0) , 
                            plot_range_get_current_ymin(range) , 
                            plot_range_get_current_ymax(range) , 
                            d->line_attr);
    break;
  default:
    util_abort("%s: internal error ... \n",__func__);
    break;
  }
}


void plot_dataset_update_range_histogram(const plot_dataset_type * d, plot_range_type * range) {
  plot_range_update_vector_x( range , d->x );
  plot_range_update_y( range , 0 );
  plot_range_update_y( range , double_vector_size( d->x ) );  /* Pure heuristics. */
}





void plot_dataset_update_range(const plot_dataset_type * d, plot_range_type * range) {
  
  const int size = plot_dataset_get_size( d );
  if (size > 0) {
    if (d->data_mask & PLOT_DATA_X)  plot_range_update_vector_x( range , d->x );
    if (d->data_mask & PLOT_DATA_X1) plot_range_update_vector_x( range , d->x1 );
    if (d->data_mask & PLOT_DATA_X2) plot_range_update_vector_x( range , d->x2 );
    
    if (d->data_mask & PLOT_DATA_Y)  plot_range_update_vector_y( range , d->y );
    if (d->data_mask & PLOT_DATA_Y1) plot_range_update_vector_y( range , d->y1 );
    if (d->data_mask & PLOT_DATA_Y2) plot_range_update_vector_y( range , d->y2 );
  }
}




