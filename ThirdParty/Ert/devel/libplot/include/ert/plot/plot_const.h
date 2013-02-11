/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_const.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __PLOT_CONST_H__
#define __PLOT_CONST_H__

#ifdef __cplusplus 
extern "C" {
#endif


typedef enum {
  PLOT_XY    = 1,     /* Normal xy plot. */
  PLOT_XY1Y2 = 2,     /* x and y-error bars, from y1 - y2. */
  PLOT_X1X2Y = 3,     /* x error bars (from x1 to x2) and y. */
  PLOT_XLINE = 4,     /* Vertical lines with fixed x */
  PLOT_YLINE = 5,     /* Horizontal lines with fixed y. */
  PLOT_HIST  = 6      /* A list of values - which are binned, and plotted in a histogram. */    
} plot_data_type;



/**
 * @brief: Plot style for one single graph/dataset.
 * 
 * When adding a new dataset to your plot item you can define what plot style
 * you want for that one graph.
 */
typedef enum plot_style_enum {
    BLANK       = 0,
    LINE        = 1,
    POINTS      = 2,  
    LINE_POINTS = 3     /* It is essential that LINE_POINTS == LINE + POINTS */
} plot_style_type;


/**
 * @brief: Plot color for one single graph/dataset.
 * 
 * When adding a new dataset to your plot item you can define what plot color
 * you want for that one graph. This color is also used to define color on labels
 * and titles.
 */
typedef enum plot_color_enum {
    WHITE      	= 0,
    RED        	= 1,
    YELLOW     	= 2,
    GREEN      	= 3,
    AQUAMARINE 	= 4,
    PINK    	= 5,
    WHEAT  	= 6,
    GRAY   	= 7,
    BROWN  	= 8,
    BLUE   	= 9,
    VIOLET 	= 10,
    CYAN   	= 11,
    TURQUOISE   = 12,
    MAGENTA 	= 13,
    SALMON  	= 14,
    BLACK   	= 15
} plot_color_type;
#define PLOT_NUM_COLORS 16   /* The number of colors - used for alternating colors. */


/**
   pllsty uses predefined line styles.
   plsty  defines linestyle with pen up/down.
*/

typedef enum  {
  PLOT_LINESTYLE_SOLID_LINE  = 1,
  PLOT_LINESTYLE_SHORT_DASH  = 2,
  PLOT_LINESTYLE_LONG_DASH   = 3
} plot_line_style_type;
     

/* The set of symbols is seemingly extremely limited. */
typedef enum {
  PLOT_SYMBOL_X     	     = 5,
  PLOT_SYMBOL_HDASH 	     = 45,
  PLOT_SYMBOL_FILLED_CIRCLE  = 17 } plot_symbol_type;
  



/* Here comes defaults which apply to the plot as a whole */
#define PLOT_DEFAULT_LABEL_FONTSIZE  0.60    /* Scaled */
#define PLOT_DEFAULT_BOX_COLOR       BLACK
#define PLOT_DEFAULT_LABEL_COLOR     BLACK
#define PLOT_DEFAULT_WIDTH           1024
#define PLOT_DEFAULT_HEIGHT          768



/* Here comes defaults which apply to one dataset. */
#define PLOT_DEFAULT_SYMBOL_SIZE     1.10   /* Scaled */
#define PLOT_DEFAULT_LINE_WIDTH      1.50   /* Scaled */
#define PLOT_DEFAULT_SYMBOL            17   /* PLOT_SYMBOL_FILLED_CIRCLE */
#define PLOT_DEFAULT_LINE_COLOR        BLUE
#define PLOT_DEFAULT_POINT_COLOR       BLUE
#define PLOT_DEAFULT_STYLE             LINE
#define PLOT_DEFAULT_LINE_STYLE        PLOT_LINESTYLE_SOLID_LINE

/* For the variables marked with 'scaled', the API is based on scale
   factors. I.e. to double the symbol size the user would call

     plot_dataset_set_symbol_size(2.0);
     
   Which will multiply PLOT_DEFAULT_SYMBOL_SIZE with 2.0
*/ 


#define PLOT_DEFAULT_PLBOX_XOPT      "bcnst"
#define PLOT_DEFAULT_PLBOX_YOPT      "bcnstv"
#define PLOT_DEFAULT_LOG_PLBOX_XOPT  "bclnst"
#define PLOT_DEFAULT_LOG_PLBOX_YOPT  "bcnlstv"


#ifdef __cplusplus 
}
#endif
#endif
