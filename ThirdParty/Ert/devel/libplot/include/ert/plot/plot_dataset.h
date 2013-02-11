/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_dataset.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __PLOT_DATASET_H__
#define __PLOT_DATASET_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/plot/plot_const.h>
#include <ert/plot/plot_range.h>
#include <ert/plot/plot_driver.h>




/**
   A dataset can have an arbitrary number of these elements: x,y,x1,x2,y1,y2.

   Observe the following:

    1. Not all plot will use all the data.
    
    2. Observe that std is implemented with a low value (x1 / y1) and
       a high value (x2 / y2) instead of a std value. If you use the
       xxx_append() functions to add data you can also add std values,
       which will be converted internally. 
*/
    
  
/* 
   The elements in this enum are used as bitmasks - so they must pure
   2^n values (apart from the composite values plot_data_stdy and plot_data_stdx).
*/
typedef enum {
  PLOT_DATA_X    =  1,
  PLOT_DATA_Y    =  2,
  PLOT_DATA_X1   =  4,
  PLOT_DATA_X2   =  8,
  PLOT_DATA_Y1   = 16,
  PLOT_DATA_Y2   = 32,
  PLOT_DATA_HIST = 1     /* == plot_data_x */
} plot_data_types;



/**
 * @addtogroup plot_dataset_type plot_dataset_type: A dataset
 * @brief Defines the plot_dataset_type, this is the "graph data" which should
 * be added to a plot.
 *
 * @remarks For every new dataset, start by allocating a new dataset with plot_dataset_alloc().
 *
 * @{
 */

/*
  This typedef is located in the plot_driver.h file:
  
  typedef struct plot_dataset_struct plot_dataset_type;
*/

void plot_dataset_set_symbol_type(plot_dataset_type * , plot_symbol_type );
void plot_dataset_set_style(plot_dataset_type * dataset , plot_style_type style);
void plot_dataset_set_line_color(plot_dataset_type *, plot_color_type );
void plot_dataset_set_point_color(plot_dataset_type *  , plot_color_type );
void plot_dataset_set_line_style(plot_dataset_type * , plot_line_style_type );
void plot_dataset_set_symbol_size(plot_dataset_type * , double );
void plot_dataset_set_line_width(plot_dataset_type *  , double );


/* Get functions */
int plot_dataset_get_length(plot_dataset_type * d);
plot_color_type plot_dataset_get_color(plot_dataset_type * d);
plot_style_type plot_dataset_get_style(plot_dataset_type * d);
int plot_dataset_get_step(plot_dataset_type * d);

/* Used in canvas tests */
int plot_dataset_step_next(plot_dataset_type * d);
void plot_dataset_finished(plot_dataset_type * d, bool flag);
bool plot_dataset_is_finished(plot_dataset_type * d);

void plot_dataset_update_range(plot_dataset_type * , bool * , plot_range_type * );
void plot_dataset_update_range_histogram(plot_dataset_type * d, plot_range_type * range);

plot_dataset_type *plot_dataset_alloc( plot_data_type , const char * label , bool logx , bool logy);
void plot_dataset_free__(void * d); 
void plot_dataset_free(plot_dataset_type * d);
void plot_dataset_set_data(plot_dataset_type * d, const double * x,
                                  const double * y, int len,
                                  plot_color_type c, plot_style_type s);

void     plot_dataset_draw(plot_dataset_type * d , plot_driver_type * driver, const plot_range_type * range);
double * plot_dataset_get_vector_x(const plot_dataset_type * d);
double * plot_dataset_get_vector_y(const plot_dataset_type * d);
double * plot_dataset_get_vector_x1(const plot_dataset_type * d);
double * plot_dataset_get_vector_y1(const plot_dataset_type * d);
double * plot_dataset_get_vector_x2(const plot_dataset_type * d);
double * plot_dataset_get_vector_y2(const plot_dataset_type * d);

/*****************************************************************/
/* Functions for actually adding data to the dataset.            */

void plot_dataset_append_point_xy(plot_dataset_type * , double  , double  );
void plot_dataset_append_vector_xy(plot_dataset_type * , int , const double *  , const double * );

void plot_dataset_append_point_xy1y2(plot_dataset_type * , double  , double  , double );
void plot_dataset_append_vector_xy1y2(plot_dataset_type * , int , const double *  , const double * , const double *);

void plot_dataset_append_point_x1x2y(plot_dataset_type * , double  , double  , double );
void plot_dataset_append_vector_x1x2y(plot_dataset_type * , int , const double *  , const double * , const double *);

void plot_dataset_set_xline(plot_dataset_type * , double   );
void plot_dataset_set_yline(plot_dataset_type * , double   );

void plot_dataset_append_vector_hist(plot_dataset_type *d , int size, const double * x);


#ifdef __cplusplus
}
#endif
#endif
