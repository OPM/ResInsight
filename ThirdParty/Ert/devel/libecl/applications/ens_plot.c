/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ens_plot.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

/*
  The ens_plot program is a very simple program created to plot
  ensembles of many related ECLIPSE simulations. The program is mainly
  designed to be invoked from BASRA, and hence not optimized for
  simple standalone use. However the communication with BASRA is based
  on reading commands from stdin, and those commands could in
  prinicple just as well come from a user generated file. The format
  for the command files is documented here.

  The very first element in the file should be the path to where you
  want the created files to go. After the path name on the first line
  the main loop of the ens_plot.x program evolves around the five
  'commands':

    C         : Create a new ensemble
    P         : Plot 
    A         : Set attributes of an ensemble
    QUANTILES : Plot an ensemble using quantiles instead of many lines
    Q         : Quit

  Each command should be entered on a separate line of the command
  stream. The various main commands then take options and arguments on
  the following lines of the command stream.


  C : CREATE_ENS_CMD
  ------------------
  The CREATE_ENS_CMD command will create a new ensemble. On the first
  line after the 'C' command the program expects the name of the
  ensemble, i.e. for instance 'Prior', and then the ECLIPSE datafiles
  for each simulation case on seperate lines. The CREATE_ENS_CMD
  command should be terminated with a '_stop_' line. For each case
  the program will load the summary data of the case, and the .RFT
  file - if present. In the example below we create an ensemble
  called Prior consisting of three simulations:

C
Prior
/path/to/sim1/ECLIPSE1.DATA
/path/to/sim2/ECLIPSE2.DATA
/path/to/sim3/ECLIPSE3.DATA
_stop_

  P : PLOT_CMD
  ------------
  The plot command will plot the selected variables and ensemble to a
  png file. The 'P' command requires at the minimum the variable to
  plot and ensembles to plot, but can in addition accept several extra
  parameters. Assuming you have used the 'C' command to create two
  ensembles 'Prior' and 'Posterior' you can plot the water cut in well
  XYZ like this:

P
WWCT:XYZ
Prior
Posterior
_stop_

  The variable to plot, i.e. the WWCT:XYZ on the second line, follows
  the format from the summary.x program, alternatively the ens_plot.x
  program can be used to plot pressure-depth relationships from RFT
  surveys. In that case the key should be given as: 
   
           RFT:WELL:10/12/2008

  i.e. the magic string 'RFT', followed by the well name and finally
  the date of the survey, the items separated with ':'. 

  The plot created will be a png file stored in the path given as
  first element in the command file, and with the key as basename.

  The 'P' command is quite flexible, and the following is possible, in
  addition to the minimum behaviour described above:

  - Additional keys: If you e.g. want to plot both the oil production
    rate, and accumulated oil production you can do that with the
    command _newplotvector_ followed by the new key:

      P
      WOPR:XYZ
      Prior
      Posterior
      _newplotvector_
      WOPT:XYZ
      Prior
      Posterior
      _stop_

    Observe that the ensemble names 'Prior' and 'Posterior' must be
    repeated after the second key has been entered. The
    _newplotvector_ can not be used to combine summary and RFT data in
    the same plot.

  - Adding observations: In a history matching context it is typically
    interesting to include observed data. This can be done with the
    subcommand '_meas_points_'. The '_meas_points_' command expects
    each datapoint to come on a separate line, the observed points can
    come in different formats. In the case of summary plots the
    alternatives are:

      xy   date            value
      xyy  date            low_value  high_value
      xxy  days1   days2   value

    The 'xy' format will plot a point, and the 'xyy' and 'xxy'
    alternatives will plot a vertical and horizontal error bar
    respectively. Observe that the '_meas_points_' command must be
    terminated with it's own '_stop_' command; so to add two
    observations of WWCT - with an accomanpying uncertainty - you
    should plot like:
    
      P
      WWCT:XYZ
      Prior
      Posterior
      _meas_points_
      xyy 10/10/2009  0.45  0.55
      xyy 10/01/2010  0.70  0.80
      _stop_
      _stop_
    
    For RFT plots the format for adding observed values is different,
    there each line of observed data should consist of a pressure
    measurement from a cell. So to add two pressure measurements when
    plotting an rft:

      P 
      RFT:XYZ:10/10/2010
      Prior
      Posterior
      _meas_points_
      rft <i> <j> <k> <P_low> <P_high> 
      rft <i> <j> <k> <P_low> <P_high> 
      _stop_
      _stop_
      
    Of course actual values should be used for the cell coordinates
    (i,j,k) and the low and high pressures P_low and P_high.

  - Setting the range: By defualt the ens_plot program will adjust the
    axis to include all the supplied data, but by using the
    '_set_range_' command you can manually set the xmin,xmax,ymin and
    ymax properties of the axes. The range should be set with one line
    formatted like this:

                 VAR  value   VAR  value   VAR   value ...
 
    Where VAR is one of the identifieres 'XMIN', 'XMAX', 'YMIN' and
    'YMAX' and value is the value of the preceeding identifier. In the
    case of summary plots the XMIN and XMAX properties should be
    dates, whereas they are pressures in the case of RFT plots. So; to
    only focus on the the year 2010 in a summary plot you could use
    the following statement:
    
      _set_range_
      XMIN  01/01/2010   XMAX  31/12/2010

    as part of your PLOT command.  

    
  A : ATTRIBUTES_CMD
  ------------------
  The attributes command can be used to set the color of the plotted
  lines on a per-ensemble basis. So to set 'Prior' ensemble to be
  plotted with red lines and the 'Posterior' with blue lines you can
  issue the following command sequence:

    A
    Prior
    1
    A
    Posterior
    9
  
  The colors must be given with the numerical values, the codes are as
  follows:

    WHITE       = 0,
    RED         = 1,
    YELLOW      = 2,
    GREEN       = 3,
    AQUAMARINE  = 4,
    PINK        = 5,
    WHEAT       = 6,
    GRAY        = 7,
    BROWN       = 8,
    BLUE        = 9,
    VIOLET      = 10,
    CYAN        = 11,
    TURQUOISE   = 12,
    MAGENTA     = 13,
    SALMON      = 14,
    BLACK       = 15

  No attributes beyond color ...


  QUANTILES
  ---------
  
  By default the ens_plot program will plot the ensemble by plotting
  all the realisations, by using the 'QUANTILES' command you can
  choose to plot quantiles like P10 and P90 instead of the individual
  realisations. The QUANTILES command takes additional arguments on
  seperate lines like this:

    Turn on quantile plotting:
    
    QUANTILES
    Prior            <- The name of the ensemble
    1                <- Use quantiles 1:True  0:False
    5                <- How many quantiles
    0.10      -----\
    0.25           |
    0.50           +  The quantiles we are interested in, i.e.
    0.75           |  P10, P25, P50, P75 and P90 in this case.
    0.90      -----/
    

    Turn off quantile plotting:
    
    QUANTILES
    Prior
    0              <-- Turn off quantile plotting
  
  Quantiles is only an option for plotting summary vectors, not RFTs.


Complete example file:
-------------------------------------------------------------------------------------------------------------------
/path/to/plots                   | All plots will be created here; the path will be created if it does not exist.
C                                | Create a new ensemble
Prior                            | Call the new ensemble 'Prior'
/path/to/sim1/CASE1.DATA         | Case 1 in prior
/path/to/sim2/CASE2.DATA         | Case 2 in prior
/path/to/sim3/CASE3.DATA         | Case 3 in prior
_stop_                           | All realizations added to 'Prior'
A                                |--\ 
Prior                            |   | Prior should be plotted in red
1                                |--/
C                                | Create a new ensemble
Posterior                        | Call the new ensemble 'Posterior'
/path/to/Post1/CASE1.DATA        | Case 1 in Posterior
/path/to/Post2/CASE2.DATA        | Case 2 in Posterior
/path/to/Post3/CASE3.DATA        | Case 3 in Posterior
_stop_                           | All realizations added to 'Prior'
A                                |--\ 
Posterior                        |   | Posterior should be plotted in blue
9                                |--/
P                                | Create a plot: /path/to/plots/WWCT:XYC.png
WWCT:XYC                         | Of the watercut in well XYZ
Posterior                        | Include the posterior ensemble
Prior                            | Include the posterior ensemble
_stop_                           | All data added to the plot - go and make it.
P                                | Create a plot: /path/to/plots/WOPR:XYC.png
WOPR:XYC                         | Of the oil production rate in well XYC
Posterior                        | Include the posterior ensemble
Prior                            | Include the prior ensemble
_newplotvector_                  | Include a new ...
WOPT:XYC                         | ... summary vector WOPT:XYZ
Posterior                        | Include the posterior ensemble
Prior                            | Include the prior ensemble
_set_range_                      | Manually adjust the plotting range
XMIN 01/01/2009  XMAX 31/12/2009 | Include all of 2009 in the plot; yrange still auto.
_stop_                           | All data added to the plot - go and make it.
Q                                | Quit
-------------------------------------------------------------------------------------------------------------------
*/


#define VIEWER          "/usr/bin/display"

#define CREATE_ENS_CMD  "C"
#define PLOT_CMD        "P"
#define ATTRIBUTES_CMD  "A"
#define QUANTILES_CMD   "QUANTILES" 
#define QUIT_CMD        "Q"

#define XMIN_CMD    "XMIN"
#define XMAX_CMD    "XMAX"
#define YMIN_CMD    "YMIN"
#define YMAX_CMD    "YMAX"

#define STOP_CMD    "_stop_"
#define XY_CMD      "xy"
#define XYY_CMD     "xyy"
#define XXY_CMD     "xxy"

#define RFT_CMD     "rft"

#define MEAS_POINTS_CMD "_meas_points_"
#define SET_RANGE_CMD   "_set_range_"
#define NEW_VECTOR_CMD  "_newplotvector_"


#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/hash.h>
#include <ert/util/menu.h>
#include <ert/util/int_vector.h>
#include <ert/util/arg_pack.h>
#include <ert/util/statistics.h>
#include <ert/util/thread_pool.h>
#include <ert/util/path_fmt.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_rft_file.h>

#include <ert/plot/plot.h>
#include <ert/plot/plot_dataset.h>
#include <ert/plot/plot_const.h>
#include <ert/plot/plot_range.h>





/*******************************************************************/
/**

About plotting quantiles:
-------------------------

The default modus for ens_plot is to plot an ensemble as a collection
of curves, however it is also possible to plot an ensemble as a mean
curve and a collection of quantiles. The implementation works as
follows:

 1. You use the function ens_use_quantile(ens , true) to tell that you
    want to plot the ensemble using quantiles. The ens_alloc()
    function contains the statement:
 
                   ens_use_quantiles(ens , false);

    i.e. the default is to not use quantiles. Observe that the use of
    quantiles versus a collection of lines is on per-ensemble basis.


 2. You use the function ens_add_quantile(ens , q) to add
    quantiles. The quantile values should be q in the interval [0,1].


 3. The function plot_ensemble() has separate code blocks for plotting
    with quantiles and as a collection of lines. When plotting with
    quantiles the algorithm is as follows:

    a) All summary data is resampled to a common time-axis. This time
       axis contains @interp_size elements, this is currently set to
       50. The distribution of interpolation times is currently
       uniform, but that can easily be generalized. 

       The data is resampled using the low-level function
       ecl_sum_data_get_from_sim_time() which tries to differentiates
       between rate and non-rate data. 


    b) For each interpolation time step we use the functions
       statistics_empirical_quantile() to calculate the quantile
       values. This is briefly based on sorting the data, looking up
       the appropriate index and simple linear interpolation.


    c) The quantiles are plotted - along with the mean value.   


*/


/**

Multithreaded loading
---------------------
The loading of ensembles in function create_ensemble_batch() is done
multithreaded. This is implemented with the help of a thread_pool
"class" implemented in libutil/src/thread_pool.c. The maximum number
of concurrent threads is given by MAX_LOAD_THREADS, this is currently
set to 4. Since the loading is mainly I/O bound it might be beneficial
to set it even higher?
*/


bool use_viewer = false ; // Global variable to enable backwords compatible behaviour of batch mode
                          // option -b sets use_viewer = true (will start external viewer to show plots)
                          // option -s sets use_viewer = false (slave mode, returns name of plot file on STDOUT)


#define KEY_JOIN_STRING  ":"    /* The string used when joining strings to form a gen_key lookup key - can be anything (even ""). */
#define PLOT_WIDTH  640
#define PLOT_HEIGHT 480
#define PROMPT_LEN  60
#define MAX_LOAD_THREADS 4






/**
   This is basic datatype to hold the information about one ensemble
   of eclipse simulations. All the simulations in one ensemble should
   share some characteristica, like all beeing from the prior
   distribution. A plotting session can very well be completed with
   only one ensemble.
*/

typedef struct {
  vector_type         * data;               /* This is a vector ecl_sum instances - actually holding the data. */
  pthread_mutex_t       data_mutex;         /* mutex ensuring serial write access to the data vector. */

  plot_style_type       plot_style;         /* LINE | POINTS | LINE_POINTS */ 
  plot_color_type       plot_color;         /* See available colors in libplot/include/plot_const.h */
  plot_line_style_type  plot_line_style;    /* Line style: solid_line | short_dash | long_dash */
  plot_symbol_type      plot_symbol;        /* Integer  - 17 is filled circle ... */
  double                plot_symbol_size;   /* Scale factor for symbol size. */
  double                plot_line_width;    /* Scale factor for line width. */ 
  double                sim_length;         /* The length of the _longest_ simulation in the ensemble. */

  /* Everything below line is related to plotting of quantiles. */
  /*-----------------------------------------------------------------*/  
  bool                  use_quantiles;       /* Should this ensemble be plotted as a mean and quantiles - instead of one line pr. member? */ 
  int                   interp_size;         /* How many interpolation points to use when resampling the summary data.*/
  double_vector_type  * interp_days;         /* The times where we resample the summary data - given in days since simulation start.*/ 
  vector_type         * interp_data;         /* A vector of double_vector instances of the summary data - interpolated to interp_days. */
  double_vector_type  * quantiles;           /* The quantile values we want to plot, i.e [0.10, 0.32, 0.68, 0.90] */
  vector_type         * quantile_data;       /* The quantile data as double_vector instances. */ 
} ens_type;



/** 
    Struct holding basic information used when plotting.
*/

typedef struct plot_info_struct {
  char * plot_path;     /* All the plots will be saved as xxxx files in this directory. */
  char * plot_device;   /* Type of plot file - currently only 'png' is tested. */
  char * viewer;        /* The executable used when displaying the newly created image. */
} plot_info_type;

/*
 * Dialog functions for batch processing (two way communication):
 */

void error_reply(char* message) 
{
  printf("ERROR: %s\n",message) ;
  fflush(stdout) ;
} ;


void warning_reply(char* message) 
{
  printf("WARNING: %s\n",message) ;
  fflush(stdout) ;
};


void info_reply(char* message) 
{
  printf("INFO: %s\n",message) ;
  fflush(stdout) ;
};

void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the program with SIGTERM (the default kill signal) you will get a backtrace. 
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
}



/******************************************************************/
/* Functions manipulating the ensemble type. */

void ens_set_style(ens_type * ens, plot_style_type style) {
  ens->plot_style = style;
}

void ens_set_color(ens_type * ens, plot_color_type color) {
  ens->plot_color = color;
}

void ens_set_line_style(ens_type * ens, plot_line_style_type line_style) {
  ens->plot_line_style = line_style;
}

void ens_set_symbol_type(ens_type * ens, plot_symbol_type symbol_type) {
  ens->plot_symbol = symbol_type;
}

void ens_set_symbol_size(ens_type * ens, double symbol_size) {
  ens->plot_symbol_size = symbol_size;
}

void ens_set_line_width(ens_type * ens, double line_width) {
  ens->plot_line_width = line_width;
}

void ens_set_interp_size( ens_type * ens, int interp_size) {
  ens->interp_size = interp_size;
}

void ens_use_quantiles( ens_type * ens, bool use_quantiles ) {
  ens->use_quantiles = use_quantiles;
}


void ens_add_quantile( ens_type * ens , double quantile ) {
  double_vector_append( ens->quantiles , quantile );
  vector_append_owned_ref( ens->quantile_data , double_vector_alloc(0,0) , double_vector_free__ );
}

void ens_clear_quantiles( ens_type * ens ) {
  double_vector_reset( ens->quantiles );
  vector_clear( ens->quantile_data );
}

void ens_add_data__( ens_type * ens, void * data  , free_ftype * destructor) {
  pthread_mutex_lock( &ens->data_mutex );
  {
    vector_append_owned_ref( ens->data , data , destructor );
  }
  pthread_mutex_unlock( &ens->data_mutex );
}


void ens_add_sum( ens_type * ens, ecl_sum_type * ecl_sum ) {
  ens_add_data__( ens , ecl_sum , ecl_sum_free__);
}


void ens_add_rft( ens_type * ens, ecl_rft_file_type * rft ) {
  ens_add_data__( ens , rft , ecl_rft_file_free__);
}


/** 
    Allocating an empty ens_type instance, with all plotting
    attributes initialized to default values.
*/



ens_type * ens_alloc() {
  ens_type * ens   = util_malloc( sizeof * ens );
  ens->data        = vector_alloc_new();
  pthread_mutex_init( &ens->data_mutex , NULL );
  /* Quantyile related stuff. */
  ens->quantiles     = double_vector_alloc(0 , 0);
  ens->interp_days      = double_vector_alloc(0 , 0); 
  ens->interp_data   = vector_alloc_new();
  ens->quantile_data = vector_alloc_new();
  ens->sim_length    = 0;
  
  /* Setting defaults for the plot */
  ens_set_style( ens , LINE );
  ens_set_color( ens , BLUE );
  ens_set_line_style(ens , PLOT_LINESTYLE_SOLID_LINE);
  ens_set_symbol_type(ens , PLOT_SYMBOL_FILLED_CIRCLE); 
  ens_set_symbol_size(ens , 1.0);
  ens_set_line_width(ens , 1.0);
  ens_use_quantiles( ens , false );
  ens_set_interp_size( ens , 50 );
  return ens;
}



void ens_free( ens_type * ens) {
  vector_free( ens->data );
  pthread_mutex_destroy( &ens->data_mutex );
  double_vector_free( ens->quantiles );
  double_vector_free( ens->interp_days  );
  vector_free( ens->interp_data );
  vector_free( ens->quantile_data );
  free(ens);
}


void ens_free__(void * __ens) {
  ens_type * ens = (ens_type *) __ens;
  ens_free (ens );
}


void ens_load_summary(ens_type * ens, const char * data_file) {
  char * base , * path;

  if (util_file_exists(data_file)) {
    util_alloc_file_components( data_file , &path , &base , NULL);
    if (path != NULL)
      printf("Loading case: %s/%s ... ",path , base);
    else
      printf("Loading case: %s .......",base);
    fflush(stdout);
    {
      ecl_sum_type * ecl_sum = ecl_sum_fread_alloc_case( data_file , KEY_JOIN_STRING );
      ens_add_sum( ens , ecl_sum );
      ens->sim_length = util_double_max( ens->sim_length , ecl_sum_get_sim_length( ecl_sum ));
    }
    vector_append_owned_ref( ens->interp_data , double_vector_alloc(0,0) , double_vector_free__ );
    printf("\n");
    free( base );
    util_safe_free( path );
  } else 
    fprintf(stderr,"Sorry: could not locate case:%s \n",data_file);
}


void ens_load_rft(ens_type * ens, const char * data_file) {
  char * base , * path;
  
  
  if (util_file_exists(data_file)) {
    util_alloc_file_components( data_file , &path , &base , NULL);
    char * rft_file = ecl_util_alloc_exfilename( path, base, ECL_RFT_FILE, false, -1 );  
    if(rft_file != NULL){
      printf("Loading case: %s/%s ... ",path , base);
      fflush(stdout);
      ens_add_rft( ens , ecl_rft_file_alloc( rft_file));
      printf("\n");
      free( base );
      util_safe_free( path );
    } else 
      fprintf(stderr,"Sorry: could not locate rft file:%s \n",rft_file);
  } else 
    fprintf(stderr,"Sorry: could not locate case:%s \n",data_file);
}



void ens_load_batch(ens_type* ens, ens_type* ens_rft, const char * data_file) { 
  char* base ;
  char* path ;
  char message[128] ;

  if (util_file_exists(data_file)) {
    util_alloc_file_components( data_file , &path , &base , NULL);
    {
      ecl_sum_type * ecl_sum = ecl_sum_fread_alloc_case( data_file , KEY_JOIN_STRING );
      if (ecl_sum != NULL){
        ens_add_sum( ens , ecl_sum );
        ens->sim_length = util_double_max( ens->sim_length , ecl_sum_get_sim_length( ecl_sum ));
        vector_append_owned_ref( ens->interp_data , double_vector_alloc(0,0) , double_vector_free__ );
      }
      else{
        sprintf(message,"No summary file for case %s loaded",base) ;
        warning_reply(message) ;
      }
    }
    
    
    char * rft_file = ecl_util_alloc_exfilename( path, base, ECL_RFT_FILE, false, -1 ); 
    
    if(rft_file != NULL){
      ens_add_rft( ens_rft , ecl_rft_file_alloc( rft_file));
      sprintf(message,"Case %s loaded",base) ;
      info_reply(message) ;
    } else {
      sprintf(message,"No RFT for case %s loaded",base) ;
      warning_reply(message) ;
    } ;
    
    free( base );
    util_safe_free( path );
  } else {
    sprintf(message,"Case %s not found",base) ;
    error_reply(message) ;
  } 
} 


void * ens_load_batch__(void * arg) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  ens_type * ens         = arg_pack_iget_ptr( arg_pack , 0 );
  ens_type * ens_rft     = arg_pack_iget_ptr( arg_pack , 1 );
  const char * data_file = arg_pack_iget_ptr( arg_pack , 2 );
  
  ens_load_batch( ens, ens_rft , data_file );
  return NULL;
}



void ens_load_many(ens_type * ens, path_fmt_type * data_file_fmt , int iens1, int iens2) {
  int iens;
  for (iens = iens1; iens <= iens2; iens++) {
    char * data_file = path_fmt_alloc_path( data_file_fmt , false , iens , iens , iens); /* Substituting up to three %d with the member number. */
    ens_load_summary(ens , data_file);
    free( data_file );
  }
}


void ens_set_plot_attributes(ens_type * ens) {
  int new_color;
  do {
    new_color = util_scanf_int("Color (integer : 0-15 )", PROMPT_LEN);
  } while ((new_color < 0) || (new_color > 15));
  ens_set_color( ens , new_color);
}


void ens_set_plot_attributes_batch(hash_type * ens_table, hash_type * ens_rft_table) {

  char message[128];
  char ens_name[128];
  scanf("%s" , ens_name);
  int new_color;
  char tmp_col[32];
  scanf("%s" , tmp_col);
  util_sscanf_int(tmp_col, &new_color);
  
  if (hash_has_key( ens_table , ens_name)){
    ens_type  * set_ens    = hash_get( ens_table , ens_name);
    if((new_color > -1) && (new_color < 16)){
      ens_set_color( set_ens , new_color);            
    }    
  } else {
    sprintf(message,"Unknown ensemble %s",ens_name) ;
    error_reply(message) ;
    return ;
  } 
  
  if (hash_has_key( ens_rft_table , ens_name)){
    ens_type  * set_ens    = hash_get( ens_rft_table , ens_name);
    if((new_color > -1) && (new_color < 16)){
      ens_set_color( set_ens , new_color);            
    }    
  } else {
    sprintf(message,"Unknown ensemble %s",ens_name) ;
    error_reply(message) ;
    return ;
  } 

  info_reply("New attributes set") ;
}


/**
   This function will set the quantile properties of an ensemble. The
   main command loop has read a 'Quantiles', and then subsequently gone up
   here. The first argument following the 'Quantiles' should be the name of
   the ensemble, a true or false value (i.e. 1 or 0) as to whether
   quantiles should be used, and if quantiles should be used the
   number of quantiles and their values.
   
   Example1
   --------
   Quantiles <- read in the main loop    
   Prior     <- Name of ensemble this applies to
   1         <- This ensemble should be plotted with quantiles.
   4         <- We want four quantiles
   0.10      <- The four quantile values
   0.32      <- 
   0.68      <-
   0.90      <-

   

   Example2
   --------
   Quantiles <- read in the main loop    
   Prior     <- Name of ensemble this applies to
   0         <- This ensemble should not be plotted with quantiles.

*/

void ens_set_plot_quantile_properties_batch( hash_type * ens_table ) {
  char message[128];
  char ens_name[128];
  scanf("%s" , ens_name);                                             /* Name of ensemble. */
  if (hash_has_key( ens_table , ens_name)){
    ens_type  * ens    = hash_get( ens_table , ens_name);
    int  use_quantiles;
    fscanf(stdin , "%d" , &use_quantiles);                            /* Should quantiles be used? */
    if (use_quantiles == 1) {
      int num_quantiles;
      ens_use_quantiles( ens , true );
      fscanf(stdin , "%d" , &num_quantiles);
      ens_clear_quantiles( ens );
      for (int i = 0; i < num_quantiles; i++) {
        double q;
        fscanf(stdin , "%lg" , &q);
        ens_add_quantile( ens , q );
      }
    } else
      ens_use_quantiles( ens , false);
  } else {
    sprintf(message,"Unknown ensemble %s",ens_name) ;
    error_reply(message) ;
    return;
  } 
  
}


/*****************************************************************/
/** Functions for 'manipulating' the plot_info type. */

void plot_info_set_path(plot_info_type * plot_info , const char * plot_path) {
  plot_info->plot_path = util_realloc_string_copy(plot_info->plot_path , plot_path);
  util_make_path( plot_path );
}


void plot_info_set_device(plot_info_type * plot_info , const char * plot_device) {
  plot_info->plot_device = util_realloc_string_copy(plot_info->plot_device , plot_device);
}

void plot_info_set_viewer(plot_info_type * plot_info , const char * plot_viewer) {
  plot_info->viewer = util_realloc_string_copy(plot_info->viewer , plot_viewer);
}


void plot_info_free( plot_info_type * plot_info) {
  util_safe_free(plot_info->plot_path);
  util_safe_free(plot_info->viewer);
  util_safe_free(plot_info->plot_device);

}


plot_info_type * plot_info_alloc(const char * plot_path , const char * device , const char * viewer) {
  plot_info_type * info = util_malloc( sizeof * info );
  info->plot_path   = NULL;
  info->plot_device = NULL;
  info->viewer      = NULL;
  
  plot_info_set_path(info , plot_path);
  plot_info_set_device(info , device);
  plot_info_set_viewer(info , viewer);
  
  return info;
}



/*****************************************************************/


void plot_ensemble(const ens_type * ens , plot_type * plot , const char * user_key) {
  const char * label = NULL;
  const int ens_size = vector_get_size( ens->data );
  int iens;


  if (ens->use_quantiles) {
    /* The ensemble is plotted as a mean, and quantiles. */
    
    /* 1: Init simulations days to use for resampling of the summary data. */
    double_vector_reset( ens->interp_days );
    for (int i = 0; i < ens->interp_size; i++) {
      double sim_days = i * ens->sim_length / (ens->interp_size - 1);
      double_vector_iset( ens->interp_days , i , sim_days);
    }
    
    /* 2: resample all the simulation results to the same times. */
    for (iens = 0; iens < ens_size; iens++) {
      const ecl_sum_type * ecl_sum = vector_iget_const( ens->data , iens );
      ecl_sum_resample_from_sim_days( ecl_sum , ens->interp_days , vector_iget( ens->interp_data , iens ) , user_key );
    }
    
    /* 3: Setting up the plot data for the quantiles. */
    {
      /* 3A: Create the plot_dataset instances and set the properties for the plots. */
      vector_type * quantiles  = vector_alloc_new(); 
      plot_dataset_type * mean = plot_alloc_new_dataset( plot , label , PLOT_XY);

      for (int i=0; i < double_vector_size( ens->quantiles ); i++) {
        plot_dataset_type * quantile_dataset = plot_alloc_new_dataset( plot , label , PLOT_XY);
        vector_append_ref( quantiles , quantile_dataset);
        
        /*
          The plotting style of the quantiles is (currently) quite
          hardcoded:
      
          1. The quantiles are plotted as lines, with linestyle LONG_DASH.
          2. The quantiles are plotted with the same color as the "mother curve" (i.e. the mean).
          3. The quantiles are plotted with a linwidth given by 0.75 times the linewidth of the mean.
        */
        
        plot_dataset_set_style      ( quantile_dataset , LINE );                           
        plot_dataset_set_line_color ( quantile_dataset , ens->plot_color);                 
        plot_dataset_set_line_style ( quantile_dataset , PLOT_LINESTYLE_LONG_DASH);        
        plot_dataset_set_line_width ( quantile_dataset , ens->plot_line_width * 0.75);     
      }

      /* Set the style of the mean. */
      plot_dataset_set_style      ( mean , ens->plot_style);
      plot_dataset_set_line_color ( mean , ens->plot_color);
      plot_dataset_set_point_color( mean , ens->plot_color);
      plot_dataset_set_line_style ( mean , ens->plot_line_style);
      plot_dataset_set_symbol_type( mean , ens->plot_symbol);
      plot_dataset_set_symbol_size( mean , ens->plot_symbol_size);
      plot_dataset_set_line_width ( mean , ens->plot_line_width);
      
      /* 3B: Calculate and add the actual data to plot. */
      {
        double_vector_type * tmp = double_vector_alloc( 0,0);
        for (int i =0; i < double_vector_size( ens->interp_days ); i++) {                    /* looping over the time direction */
          double_vector_reset( tmp );
          for (iens=0; iens < ens_size; iens++) {                                         /* Looping over all the realisations. */
            const double_vector_type * interp_data = vector_iget_const( ens->interp_data , iens );
            double_vector_iset( tmp , iens , double_vector_iget( interp_data , i ));
          }

          /* 
             Now tmp is an ensemble of values resampled to the same
             time; this can be used for quantiles. 
          */
          {
            const ecl_sum_type * ecl_sum = vector_iget_const( ens->data , 0);    

            /* Adding the mean value. */
            plot_dataset_append_point_xy( mean , 
                                          ecl_sum_time_from_days( ecl_sum , double_vector_iget( ens->interp_days , i )) ,    /* Time value */
                                          statistics_mean( tmp ));
            
            /* Adding the quantiles. */
            for (int iq =0; iq < double_vector_size( ens->quantiles ); iq++) {                                                      /* Looping over all the quantiles. */
              double qv;
              plot_dataset_type * data_set = vector_iget( quantiles , iq );
              qv                           = statistics_empirical_quantile( tmp , double_vector_iget( ens->quantiles , iq ));
              plot_dataset_append_point_xy( data_set , 
                                            ecl_sum_time_from_days( ecl_sum , double_vector_iget( ens->interp_days , i )) ,         /* Time value */
                                            qv );                                                                                   /* y-value - the interpolated quantile. */
            }
          }
        }
        double_vector_free( tmp );
      }
      vector_free(quantiles);
    } 
  } else {
    /* The ensemble is plotted as a collection of curves. */

    for (iens = 0; iens < ens_size; iens++) {
      plot_dataset_type * plot_dataset = plot_alloc_new_dataset( plot , label , PLOT_XY );
      const ecl_sum_type * ecl_sum = vector_iget_const( ens->data , iens );
      int param_index = ecl_sum_get_general_var_params_index( ecl_sum , user_key );
      int time_index;

      for (time_index = 0; time_index < ecl_sum_get_data_length( ecl_sum ); time_index++) {
        plot_dataset_append_point_xy( plot_dataset , 
                                      //ecl_sum_iget_sim_days( ecl_sum , time_index ),
                                      ecl_sum_iget_sim_time( ecl_sum , time_index ),
                                      ecl_sum_iget( ecl_sum , time_index , param_index ));
      }
      
      plot_dataset_set_style      ( plot_dataset , ens->plot_style);
      plot_dataset_set_line_color ( plot_dataset , ens->plot_color);
      plot_dataset_set_point_color( plot_dataset , ens->plot_color);
      plot_dataset_set_line_style ( plot_dataset , ens->plot_line_style);
      plot_dataset_set_symbol_type( plot_dataset , ens->plot_symbol);
      plot_dataset_set_symbol_size( plot_dataset , ens->plot_symbol_size);
      plot_dataset_set_line_width( plot_dataset , ens->plot_line_width);
    }
  }
}


void plot_rft_ensemble(const ens_type * ens , plot_type * plot , const char * well, time_t survey_time) {
  const char * label = NULL;
  const int ens_size = vector_get_size( ens->data );
  int iens, inode;
  
  const ecl_rft_file_type * ecl_rft = NULL;  
  for (iens = 0; iens < ens_size; iens++) {
    
    plot_dataset_type * plot_dataset = plot_alloc_new_dataset( plot , label , PLOT_XY );
    ecl_rft = vector_iget_const( ens->data , iens );
    const ecl_rft_node_type * ecl_rft_node = ecl_rft_file_get_well_time_rft(ecl_rft, well, survey_time);
    
    
    const int node_size = ecl_rft_node_get_size(ecl_rft_node);
    for (inode =0;inode < node_size;inode++){
      plot_dataset_append_point_xy( plot_dataset , 
                                    ecl_rft_node_iget_pressure(ecl_rft_node,inode) ,
                                    ecl_rft_node_iget_depth(ecl_rft_node,inode) );
    }
    
    plot_dataset_set_style      ( plot_dataset , ens->plot_style);
    plot_dataset_set_line_color ( plot_dataset , ens->plot_color);
    plot_dataset_set_point_color( plot_dataset , ens->plot_color);
    plot_dataset_set_line_style ( plot_dataset , ens->plot_line_style);
    plot_dataset_set_symbol_type( plot_dataset , ens->plot_symbol);
    plot_dataset_set_symbol_size( plot_dataset , ens->plot_symbol_size);
    plot_dataset_set_line_width( plot_dataset , ens->plot_line_width);
  }
}


void set_range(plot_type * plot, time_t start_time){
  int     num_tokens;
  char ** token_list;
  char  * line;
  
  line = util_blocking_alloc_stdin_line(100);
  util_split_string(line , " " , &num_tokens , &token_list);
  
  int i;
  for (i=0;i<num_tokens-1;i+=2){
    if(strcmp(token_list[i], XMIN_CMD) == 0){
      time_t time = start_time;  
      util_sscanf_date(token_list[i+1] , &time);
      plot_set_xmin(plot , time);
    }
    else if(strcmp(token_list[i], XMAX_CMD) == 0){
      time_t time = start_time;  
      util_sscanf_date(token_list[i+1] , &time);
      plot_set_xmax(plot , time);
    }
    else if(strcmp(token_list[i], YMIN_CMD) == 0){
      double  ymin = 0.00;       
      util_sscanf_double(token_list[i+1] , &ymin);
      plot_set_ymin(plot , ymin);
    }
    else if(strcmp(token_list[i], YMAX_CMD) == 0){
      double  ymax = 0.00;       
      util_sscanf_double(token_list[i+1] , &ymax);
      plot_set_ymax(plot , ymax);
    }
  }
  
  /** The ymin/ymax values are calculated automatically. */
  
}

void set_range_rft(plot_type * plot){
  int     num_tokens;
  char ** token_list;
  char  * line;
  
  line = util_blocking_alloc_stdin_line(100);
  util_split_string(line , " " , &num_tokens , &token_list);
  
  int i;
  for (i=0;i<num_tokens-1;i+=2){
    if(strcmp(token_list[i], XMIN_CMD) == 0){
      double xmin = 0.00;
      util_sscanf_double(token_list[i+1] , &xmin);
      plot_set_xmin(plot , xmin);
    }
    else if(strcmp(token_list[i], XMAX_CMD) == 0){
      double xmax  = 0.00;
      util_sscanf_double(token_list[i+1] , &xmax);
      plot_set_xmax(plot , xmax);
    }
    else if(strcmp(token_list[i], YMIN_CMD) == 0){
      double  ymin = 0.00;       
      util_sscanf_double(token_list[i+1] , &ymin);
      plot_set_ymin(plot , ymin);
    }
    else if(strcmp(token_list[i], YMAX_CMD) == 0){
      double  ymax = 0.00;       
      util_sscanf_double(token_list[i+1] , &ymax);
      plot_set_ymax(plot , ymax);
    }
  }
  
  /** The ymin/ymax values are calculated automatically. */
  
}

double get_rft_depth (hash_type * ens_table, char * well, int i, int j, int k) {

  ens_type * ens = NULL;
  
  {
    hash_iter_type * ens_iter = hash_iter_alloc( ens_table );
    while (!hash_iter_is_complete( ens_iter )) {
      ens = hash_iter_get_next_value( ens_iter );
      if (ens !=NULL && ens->data && vector_get_size(ens->data) > 0){
        hash_iter_free( ens_iter );
        break;
      }
    }
  }
  
  const ecl_rft_file_type * ecl_rft = vector_iget_const( ens->data , 0 );
  if (ecl_rft_file_has_well( ecl_rft , well )) {
    const ecl_rft_node_type * ecl_rft_node = ecl_rft_file_iget_well_rft(ecl_rft, well, 0);
    const int node_size = ecl_rft_node_get_size(ecl_rft_node);
    int inode;
    int ni,nj,nk;
    for (inode =0;inode < node_size;inode++){
      /* 
         The ecl_rft structure has zero offset values for the
         coordinates, whereas the (i,j,k) input to this function is
         assumed offset 1.
      */
      ecl_rft_node_iget_ijk( ecl_rft_node , inode , &ni , &nj , &nk);
      
      if( ( i == (ni + 1)) && 
          ( j == (nj + 1)) &&
          ( k == (nk + 1))) {
        //double depth = ecl_rft_node_iget_depth(ecl_rft_node,inode);
        return ecl_rft_node_iget_depth(ecl_rft_node,inode);
      }
    }
  }
  
  return 0;
}


void plot_meas_file(plot_type * plot, time_t start_time){
  bool done = 0;
  double x,x1,x2,y,y1,y2;
  char * error_ptr;
  plot_dataset_type * plot_dataset; 
  time_t time;
  int days;
  while (!done) {
    int     num_tokens;
    char ** token_list;
    char  * line;
    
    line = util_blocking_alloc_stdin_line(10);
    util_split_string(line , " " , &num_tokens , &token_list);
    
    /*
      Tips:

      
      1. Free token_list: util_free_stringlist( token_list , num_tokens);
      2. Parse int/double/...

         if (util_sscanf_double(token_list[??] , &double_value)) 
            prinftf("Parsed %s -> %g \n",token_list[?+] , double_value);
         else
            printf("Could not interpret %s as double \n",token_list[??]);
            
    */      
    
    if (token_list[0] != NULL) {
      if(strcmp(token_list[0], STOP_CMD) == 0){
        done = 1;
      }
      
      if(strcmp(token_list[0], XY_CMD) == 0){
        util_sscanf_date(token_list[1] , &time);
        util_difftime(start_time, time, &days, NULL, NULL, NULL);
        x = time;
        //x = days;
        
        y = strtod(token_list[2], &error_ptr);
        plot_dataset = plot_alloc_new_dataset( plot , NULL , PLOT_XY );
        plot_dataset_set_style      (plot_dataset , POINTS);
        plot_dataset_append_point_xy(plot_dataset , x , y);
        plot_dataset_set_line_width( plot_dataset , 1.5);
        plot_dataset_set_line_color( plot_dataset , 15);
      }
      
      if(strcmp(token_list[0], XYY_CMD) == 0){
        util_sscanf_date(token_list[1] , &time);
        util_difftime(start_time, time, &days, NULL, NULL, NULL);
        //x = days;
        x = time;
        y1 = strtod(token_list[2], &error_ptr);
        y2 = strtod(token_list[3], &error_ptr);
        
        plot_dataset = plot_alloc_new_dataset( plot , NULL , PLOT_XY1Y2 );
        plot_dataset_append_point_xy1y2(plot_dataset , x , y1, y2);
        plot_dataset_set_line_width( plot_dataset , 1.5);
        plot_dataset_set_line_color( plot_dataset , 15);
      }
      
      if(strcmp(token_list[0], XXY_CMD) == 0){
        x1 = strtod(token_list[1], &error_ptr);
        x2 = strtod(token_list[2], &error_ptr);
        time_t time1 = start_time;       
        time_t time2 = start_time;      
        util_inplace_forward_days(&time1 , x1);
        util_inplace_forward_days(&time2 , x2);
        
        y = strtod(token_list[3], &error_ptr);
        plot_dataset = plot_alloc_new_dataset( plot , NULL  , PLOT_X1X2Y );
        plot_dataset_append_point_x1x2y(plot_dataset , time1 , time2, y);
        plot_dataset_set_line_width( plot_dataset , 1.5);
        plot_dataset_set_line_color( plot_dataset , 15);
      }
      
    } else {
      util_forward_line(stdin , &done);
    }
    util_free_stringlist( token_list , num_tokens);
  }
}

void plot_meas_rft_file(plot_type * plot, char * well, hash_type * ens_table){
  bool done = 0;
  int i, j, k;
  double x1, x2;
  plot_dataset_type * plot_dataset; 
  //time_t time;
  //int days;
  while (!done) {
    int     num_tokens;
    char ** token_list;
    char  * line;
    
    line = util_blocking_alloc_stdin_line(10);
    util_split_string(line , " " , &num_tokens , &token_list);
    
    /*
      Tips:

      
      1. Free token_list: util_free_stringlist( token_list , num_tokens);
      2. Parse int/double/...

         if (util_sscanf_double(token_list[??] , &double_value)) 
            prinftf("Parsed %s -> %g \n",token_list[?+] , double_value);
         else
            printf("Could not interpret %s as double \n",token_list[??]);
            
    */      
    
    if (token_list[0] != NULL) {
      if(strcmp(token_list[0], STOP_CMD ) == 0){
        done = 1;
      }
      
      if(strcmp(token_list[0], RFT_CMD) == 0){
        util_sscanf_int(token_list[1] , &i);
        util_sscanf_int(token_list[2] , &j);
        util_sscanf_int(token_list[3] , &k);
        util_sscanf_double(token_list[4] , &x1);
        util_sscanf_double(token_list[5] , &x2);
        
        double depth = get_rft_depth(ens_table, well, i, j, k);
        
        if(depth != 0){
          plot_dataset = plot_alloc_new_dataset( plot , NULL  , PLOT_X1X2Y );
          plot_dataset_append_point_x1x2y(plot_dataset , x1 , x2, depth);
          plot_dataset_set_line_width( plot_dataset , 1.5);
          plot_dataset_set_line_color( plot_dataset , 15);
        }
        else{
          printf ("The block %i %i %i does not exist in well %s\n", i, j, k, well);
        }
      }
      
    } else {
      util_forward_line(stdin , &done);
    }
    util_free_stringlist( token_list , num_tokens);
  }
}





void plot_finalize(plot_type * plot , plot_info_type * info , const char * plot_file) {
  plot_data(plot);
  plot_free(plot);
  util_fork_exec(info->viewer , 1 , (const char *[1]) { plot_file } , false , NULL , NULL , NULL , NULL , NULL);
}


void plot_all(void * arg) {
  arg_pack_type * arg_pack   = arg_pack_safe_cast( arg );
  hash_type  * ens_table     = arg_pack_iget_ptr( arg_pack , 0);
  plot_info_type * plot_info = arg_pack_iget_ptr( arg_pack , 1);
  {
    plot_type * plot;
    char key[32];
    char * plot_file;
    
    util_printf_prompt("Give key to plot" , PROMPT_LEN , '=' , "=> ");
    scanf("%s" , key);

    plot_file = util_alloc_sprintf("%s/%s.%s" , plot_info->plot_path , key , plot_info->plot_device);
    {
      arg_pack_type * arg_pack = arg_pack_alloc();
      arg_pack_append_ptr( arg_pack , plot_file );
      arg_pack_append_ptr( arg_pack , plot_info->plot_device);

      plot = plot_alloc("PLPLOT" , arg_pack , false , false);  /* The two last arguments are for log - works by taking log on the __the_data__. */

      arg_pack_free( arg_pack );
    }
    plot_set_window_size(plot , PLOT_WIDTH , PLOT_HEIGHT);
    plot_set_labels(plot , "Time (days)" , key , key);

    {
      bool complete;
      hash_iter_type * ens_iter = hash_iter_alloc( ens_table );
      do {
        complete = hash_iter_is_complete( ens_iter );
        if (!complete) {
          const ens_type * ens = hash_iter_get_next_value( ens_iter );
          plot_ensemble( ens , plot , key);
        }
      } while (!complete);
      hash_iter_free( ens_iter );
      plot_finalize(plot , plot_info , plot_file);
    }
    printf("Plot saved in: %s \n",plot_file);
    free( plot_file );
  }
}


void _plot_batch_rft(arg_pack_type* arg_pack, char* inkey){

  // subroutine used in batch mode to plot an rft vector for a list of ensembles given at stdin

  char message[128] ;

  hash_type*      ens_rft_table = arg_pack_iget_ptr( arg_pack , 1);
  plot_info_type* plot_info     = arg_pack_iget_ptr( arg_pack , 2);
  
  plot_type* plot = NULL ;
  
  char * key = inkey;
  
  // split the key in to head, well, date
  int     num_tokens;
  char ** token_list;
  util_split_string(key , ":" , &num_tokens , &token_list);  
  
  if(num_tokens != 3){
    sprintf(message,"The key %s does not exist", key);
    error_reply(message) ;
    return;
  }
 
  char * well = token_list[1];
  char * date = token_list[2];
  time_t survey_time ;
  util_sscanf_date(date , &survey_time) ;  
  
  char * plot_file;
  {
    char * plot_name = util_alloc_sprintf("%s:%s" , token_list[0] , token_list[1]);
    plot_file = util_alloc_filename(plot_info->plot_path , plot_name  , plot_info->plot_device);
    free( plot_name );
  }
  
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , plot_file );
    arg_pack_append_ptr( arg_pack , plot_info->plot_device);
    plot = plot_alloc("PLPLOT" , arg_pack , false , false );
    arg_pack_free( arg_pack );
  }
  plot_set_window_size(plot , PLOT_WIDTH , PLOT_HEIGHT);
  plot_set_labels(plot , "Pressure" , "Depth" , key);
  

  char ens_name[32];    
  int iens;
  int ens_size;
  bool ens_ok = false;
  
  // Check if there is anything to plot
  ens_type * ens = NULL;
  {
    hash_iter_type * ens_iter = hash_iter_alloc( ens_rft_table );
    while (!hash_iter_is_complete( ens_iter )) {
      ens = hash_iter_get_next_value( ens_iter );
      if (ens != NULL && ens->data && vector_get_size(ens->data) > 0){
        hash_iter_free( ens_iter );
        ens_ok = true;
        break;
      }
    }
  }
  
  if (!ens_ok) { 
    error_reply("No ensembles or RFT files to plot\n");
    return;
  }
  

  const ecl_rft_file_type * ecl_rft = vector_iget_const( ens->data , 0 );
  
  sprintf(message,"Will plot %s",key) ;
  info_reply(message) ;
  
  bool plotempty = true ;
  bool complete = false;
  bool failed = false ;

  while (!complete) {
    scanf("%s" , ens_name);
    
    if(strcmp(ens_name, MEAS_POINTS_CMD ) == 0){
      plot_meas_rft_file(plot, well, ens_rft_table);
      plotempty = false ;
      info_reply("Measured values plotted") ;
    } else if(strcmp(ens_name, SET_RANGE_CMD) == 0){
      set_range_rft(plot);
      info_reply("Range set") ;
    } else if(strcmp(ens_name, NEW_VECTOR_CMD ) == 0){
      scanf("%s" , key);
      util_split_string(key , ":" , &num_tokens , &token_list);  

      if(num_tokens != 3){
        sprintf(message,"The key %s does not exist", key);
        error_reply(message) ;
        return;
      }
      
      if(strcmp(token_list[0],"RFT") != 0){
        sprintf(message,"The key %s does not exist", key);
        error_reply(message) ;
        failed = true ;
        char tmp[32];    
        scanf("%s" , tmp);
      }
      else{
        well = token_list[1];
        date = token_list[2];
        util_sscanf_date(date , &survey_time) ;  
        sprintf(message,"Will plot %s",key) ;
        info_reply(message) ;
      }
    } else if (strcmp(ens_name, STOP_CMD) == 0) {
      complete = true ;
    } else if (hash_has_key(ens_rft_table , ens_name)){
      ens = hash_get(ens_rft_table , ens_name);
      
      // Check if there is anything to plot
      if (ens == NULL  || !(ens->data) || vector_get_size(ens->data) <= 0) { // Denne satt langt inne !!!!
        sprintf(message,"No RFT files to plot in ensemble %s\n", ens_name);
        error_reply(message);
        return;
      }

      ens_size = vector_get_size( ens->data );

      
      // Check if the rft file has the requested well and date
      for (iens = 0; iens < ens_size && !failed; iens++) {
        ecl_rft = vector_iget_const( ens->data , iens );
        if(!ecl_rft_file_has_well(ecl_rft , well)){
          sprintf(message,"The well %s does not exist\n", well);
          error_reply(message) ;
          failed = true ;
        } else {
          // Check if the rft file has the requested survey time
          const ecl_rft_node_type * ecl_rft_node = ecl_rft_file_get_well_time_rft(ecl_rft, well, survey_time);
          if(ecl_rft_node == NULL){
            sprintf(message,"The survey %s in %s does not exist", well, date);
            error_reply(message) ;
            failed = true ;
          }
        }
      }
        
      if (!failed) {
        plot_rft_ensemble( ens , plot , well, survey_time);
        plotempty = false ;
        sprintf(message,"%s plotted",ens_name) ;
        info_reply(message) ;
      } ;
      
    } else {
      sprintf(message,"unknown ensemble %s",ens_name) ;
      error_reply(message) ;
      return ;
    }
  
  } // End while

  if (plot && !plotempty) {
    plot_data(plot);
    plot_free(plot);
    if (use_viewer) {
      util_fork_exec(plot_info->viewer , 1 , (const char *[1]) { plot_file } , false , NULL , NULL , NULL , NULL , NULL);
    } ;
    sprintf(message,"Plot file %s",plot_file) ;
    info_reply(message) ;
  } else {
    error_reply("No data plotted") ;
  } ;

  free( plot_file );
}

 

void _plot_batch_summary(arg_pack_type* arg_pack, char * inkey){

  // subroutine used in batch mode to plot a summary vector for a list of ensembles given at stdin

  char message[128] ;

  hash_type*      ens_table = arg_pack_iget_ptr( arg_pack , 0);
  plot_info_type* plot_info = arg_pack_iget_ptr( arg_pack , 2);
  
  plot_type* plot = NULL ;
  
  char* key = inkey;
  char* plot_file ;
  
  plot_file = util_alloc_sprintf("%s/%s.%s" , plot_info->plot_path , key , plot_info->plot_device);
  
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , plot_file );
    arg_pack_append_ptr( arg_pack , plot_info->plot_device);
    plot = plot_alloc("PLPLOT" , arg_pack , false , false );
    arg_pack_free( arg_pack );
  }
  plot_set_window_size(plot , PLOT_WIDTH , PLOT_HEIGHT);
  plot_set_labels(plot , "Date" , key , key);
  
  // get the simulation start time, to be used in plot_meas_file
  ens_type * ens = NULL;
  bool ens_ok = false;
  {
    hash_iter_type * ens_iter = hash_iter_alloc( ens_table );
    while (!hash_iter_is_complete( ens_iter )) {
      ens = hash_iter_get_next_value( ens_iter );
      if (ens != NULL && ens->data && vector_get_size(ens->data) > 0){
        hash_iter_free( ens_iter );
        ens_ok = true;
        break;
      }
    }
  }
  
  if ( !ens_ok ) { 
    error_reply("No ensembles or summary files to plot\n");
    return ;
  } ;
  
  const ecl_sum_type * ecl_sum = vector_iget_const( ens->data , 0 );
  
  time_t start_time       = ecl_sum_get_start_time( ecl_sum );
  time_t end_time         = ecl_sum_get_end_time( ecl_sum );

  sprintf(message,"Will plot %s",key) ;
  info_reply(message) ;

  char ens_name[32];    
  bool complete = false;
  bool plotempty = true ;
  bool failed = false ;

  int iens;
  int ens_size;
  
  while (!complete) {
    scanf("%s" , ens_name);

    if(strcmp(ens_name, MEAS_POINTS_CMD) == 0){
      plot_meas_file(plot, start_time);
      plotempty = false ;
      info_reply("Measured values plotted") ;
    } else if(strcmp(ens_name, SET_RANGE_CMD) == 0){
      set_range(plot, start_time);
      info_reply("Range set") ;
    } else if(strcmp(ens_name, NEW_VECTOR_CMD) == 0){// ??????????
      scanf("%s" , key);
    } else if (strcmp(ens_name, STOP_CMD) == 0) {
      complete = true ;
    } else  if (hash_has_key( ens_table , ens_name)){
      ens = hash_get(ens_table , ens_name);
      
      // Check if there is anything to plot
      if (ens == NULL || !(ens->data) || vector_get_size(ens->data) <= 0) { // Denne satt langt inne !!!!
        sprintf(message,"No files to plot in ensemble %s\n", ens_name);
        error_reply(message);
        return;
      }

      ens_size = vector_get_size( ens->data );
      // Check if the summary file has the requested key
      for (iens = 0; iens < ens_size && !failed; iens++) {
        ecl_sum = vector_iget_const( ens->data , iens );
        if(!ecl_sum_has_general_var(ecl_sum , key)){
          sprintf(message,"The key %s does not exist in case %i", key,iens); // How to get name
          error_reply(message) ;
          failed = true ;
        }
      } ;

      if (!failed) {
        plotempty = false ;
        plot_ensemble( ens , plot , key);
        sprintf(message,"%s plotted",ens_name) ;
        info_reply(message) ;
      } ;

    } else {
      sprintf(message,"unknown ensemble %s",ens_name) ;
      error_reply(message) ;
      return ;
    }
  
  } // End while
  
  if (plot && !plotempty) {
    plot_set_default_timefmt(plot , start_time , end_time);
    plot_data(plot);
    plot_free(plot);
    if (use_viewer) {
      util_fork_exec(plot_info->viewer , 1 , (const char *[1]) { plot_file } , false , NULL , NULL , NULL , NULL , NULL);
    } ;
    sprintf(message,"Plot file %s",plot_file) ;
    info_reply(message) ;
  } else {
    error_reply("No data plotted") ;
  } ;

  free( plot_file );
}


/******************************************************************/

ens_type * select_ensemble(hash_type * ens_table, const char * prompt) {
  char ens_name[32];
  
  util_printf_prompt(prompt, PROMPT_LEN , '=' , "=> ");
  scanf("%s" , ens_name);
  if (hash_has_key( ens_table , ens_name))
    return hash_get( ens_table , ens_name);
  else {
    fprintf(stderr,"Do not have ensemble: \'%s\' \n", ens_name);
    return NULL;
  }
}

void plot_batch(arg_pack_type* arg) {
  char *  key = util_blocking_alloc_stdin_line(10);
  int     num_tokens;
  char ** token_list;
  
  // scan stdin for vector
  util_split_string(key , ":" , &num_tokens , &token_list);  
  if(strcmp(token_list[0],"RFT") == 0){
    _plot_batch_rft(arg, key);
  } else{
    _plot_batch_summary(arg, key);
  }
}


void add_simulation(void * arg) {
  hash_type * ens_table = (hash_type *) arg;
  ens_type  * ens       = select_ensemble( ens_table , "Which ensemble");
  if (ens != NULL) {
    char data_file[512];
    util_printf_prompt("Give datafile", PROMPT_LEN , '=' , "=> ");
    scanf("%s" , data_file);
    ens_load_summary( ens , data_file );
  } 
}


void scanf_ensemble_numbers(int * iens1 , int * iens2) {
  bool OK = false;

  util_printf_prompt("Ensemble members (xx yy)" , PROMPT_LEN , '=' , "=> ");
  
  while (!OK) {
    char * input = util_alloc_stdin_line();
    const char * current_ptr = input;
    OK = true;

    current_ptr = util_parse_int(current_ptr , iens1 , &OK);
    current_ptr = util_skip_sep(current_ptr , " ,-:" , &OK);
    current_ptr = util_parse_int(current_ptr , iens2 , &OK);
    
    if (!OK) 
      printf("Failed to parse two integers from: \"%s\". Example: \"0 - 19\" to get the 20 first members.\n",input);
    free(input);
  }
}
  


void add_many_simulations(void * arg) {
  hash_type * ens_table = (hash_type *) arg;
  ens_type  * ens       = select_ensemble( ens_table , "Which ensemble");
  if (ens != NULL) {
    path_fmt_type * data_file_fmt;
    int iens1 , iens2;
    char data_file[512];
    util_printf_prompt("Give datafile format (with up to three %d) ", PROMPT_LEN , '=' , "=> ");
    scanf("%s" , data_file);
    data_file_fmt = path_fmt_alloc_path_fmt( data_file );
    
    scanf_ensemble_numbers(&iens1 , &iens2); /* iens1 and iens2 are inclusive */

    ens_load_many( ens , data_file_fmt , iens1 , iens2 );
    path_fmt_free( data_file_fmt );
  } 
}




void create_ensemble(void * arg) {
  hash_type * ens_table = (hash_type *) arg;
  char ens_name[32];
  
  util_printf_prompt("Name of new ensemble", PROMPT_LEN , '=' , "=> ");
  scanf("%s" , ens_name);
  if (!hash_has_key( ens_table , ens_name)) {
    ens_type * ens = ens_alloc();
    hash_insert_hash_owned_ref( ens_table , ens_name , ens , ens_free__);
  }
}


void create_named_ensembles(ens_type** ens, ens_type** ens_rft, hash_type* ens_table, hash_type* ens_rft_table, char* ens_name) {
  char message[128] ;
  if (!hash_has_key( ens_table , ens_name) && !hash_has_key( ens_rft_table , ens_name)) {
    *ens = ens_alloc();
    ens_set_line_width(*ens, 1.5);
    hash_insert_hash_owned_ref( ens_table , ens_name , *ens , ens_free__);

    *ens_rft = ens_alloc();
    ens_set_line_width(*ens_rft, 1.5);    
    ens_set_style(*ens_rft, POINTS);    
    ens_set_symbol_type(*ens_rft , PLOT_SYMBOL_FILLED_CIRCLE); 
    hash_insert_hash_owned_ref( ens_rft_table , ens_name , *ens_rft , ens_free__);

    sprintf(message,"Ensemble %s created", ens_name);
    info_reply(message) ;
  } else {
    sprintf(message,"Ensemble %s already exist",ens_name) ;
    error_reply(message) ;
  } ;
  return ;
} ;


/**
   This function has been slightly rewritten to support multithreaded
   loading of realizations. The most important changes is that the
   error and complete conditions are handled with a break statement,
   and not an immediate return. This is essential to ensure that
   thread_pool_join() call at the end of the function is issued.
*/

void create_ensemble_batch(hash_type* ens_table, hash_type* ens_rft_table) {
  thread_pool_type * tp = thread_pool_alloc( MAX_LOAD_THREADS , true );
  vector_type * arglist = vector_alloc_new();   /* Container to discard the arg_pack instances used for the threaded loading. */
  char message[128] ;
  char * line;  
  // scan stdin for ensemble name
  char * ens_name  = util_alloc_stdin_line();
  
  ens_type* ens     = NULL ;
  ens_type* ens_rft = NULL ;
  create_named_ensembles(&ens, &ens_rft, ens_table, ens_rft_table, ens_name);

  // scan stdin for valid simulation directory, or a valid eclipse data filename
  
  char * base;
  
  while (1) {
    char * data_file = NULL;
    
    line = util_alloc_stdin_line();
    
    if(strcmp(line, "_stop_") == 0){
      sprintf(message,"Ensemble %s created",ens_name) ;
      info_reply(message) ;
      break;  
    }

    // Check if this is a directory or a file
    if(util_is_directory(line)) {
      base      = ecl_util_alloc_base_guess(line);
      data_file = ecl_util_alloc_filename(line, base, ECL_DATA_FILE , true , 0);
    } else if (util_is_file(line))       
      data_file = util_alloc_string_copy( line );

    if (data_file != NULL) {
      arg_pack_type * arg_pack = arg_pack_alloc();
      vector_append_owned_ref( arglist , arg_pack , arg_pack_free__);
      arg_pack_append_ptr( arg_pack , ens );
      arg_pack_append_ptr( arg_pack , ens_rft );
      arg_pack_append_owned_ptr( arg_pack , data_file , free);   /* The arg pack takes ownership of the data_file pointer. */

      thread_pool_add_job(tp , ens_load_batch__ , arg_pack );
      //ens_load_batch(ens , ens_rft , data_file) ;
    } else {
      sprintf(message,"%s is not a valid eclipse summary file or directory", line);
      error_reply(message) ;
      free(line) ;
      break;
    };
    free(line);
  };
  
  thread_pool_join( tp );
  thread_pool_free( tp );
  vector_free( arglist );
  sprintf(message,"Ensemble %s created",ens_name) ;
  info_reply(message) ;
}


void set_plot_attributes(void * arg) {
  hash_type * ens_table = (hash_type *) arg;
  ens_type  * ens       = select_ensemble( ens_table , "Which ensemble");
  if (ens != NULL) 
    ens_set_plot_attributes( ens );
  
}





/*****************************************************************/

void print_usage() {
  printf("***************************************************************************\n");
  printf("  This program is used to plot the summary results from many ECLIPSE\n");
  printf("  simulations in one plot.\n");
  printf("\n");
  printf("  1. When starting the program you can give the path to ECLIPSE data\n");
  printf("     files on the command line - the corresponding summary results will\n");
  printf("     be loaded.\n");
  printf("\n");
  printf("  2. In the menu:\n");
  printf("\n");
  printf("     p: Plot summary ensemble(s): This will ask you for a key to plot,\n");
  printf("      i.e. RPR:2 for a region, WWCT:OP_3 for the watercut in well OP_3\n");
  printf("      and FOPR for the field oil production rate - and so on. It will\n");
  printf("      plot all the ensembles you have loaded.\n");
  printf("\n");
  printf("     c: Create a new ensemble: The simulation results are grouped\n");
  printf("      together in ensembles. With this command you can make a new\n");
  printf("      (empty) ensemble.\n");
  printf("\n");
  printf("     n: New simulation case: You can load a new simulation; first you\n");
  printf("      are prompted for the name of an ensemble, and then afterwards\n");
  printf("      an ECLIPSE data file which is then loaded.\n");
  printf("\n");
  printf("     m: Add many simulations: This is similar to 'n' - but instead of\n");
  printf("      giving a data-file you give a (C-based) format string containing\n");
  printf("      up to three %%d format specifiers - these are replaced with\n");
  printf("      simulation number, and the corresponding cases are loaded.\n");
  printf("\n");
  printf("     a: Set plot attributes: This gives you the possibility to set plot\n");
  printf("      attributes for an ensemble. Observe that all the members in the\n");
  printf("      ensemble are plotted with the same attributes. (Currently only\n");
  printf("      attribute is color).\n");
  printf("\n");
  printf("***************************************************************************\n");
}


/*****************************************************************/

int main(int argc , char ** argv) {
  install_SIGNALS();
  
  //setvbuf(stdout, NULL, _IOFBF, 0);
  setenv( "PLPLOT_LIB" , "/project/res/x86_64_RH_5/plplot/plplot-5.10.0/share/plplot5.10.0" , 1);

  if(argc > 1){
    if(strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "-s") == 0) {
      if (strcmp(argv[1], "-b") == 0) {
        use_viewer = true ;
      } ;
      char * path = util_blocking_alloc_stdin_line(10);
      
      hash_type * ens_table = hash_alloc();
      hash_type * ens_rft_table = hash_alloc();
      plot_info_type * info = plot_info_alloc( path , "png" , VIEWER);
      arg_pack_type * arg_pack = arg_pack_alloc();
      arg_pack_append_ptr( arg_pack , ens_table );
      arg_pack_append_ptr( arg_pack , ens_rft_table );
      arg_pack_append_ptr( arg_pack , info );
      char * line;
      free(path);
      
      while (1){
        line = util_blocking_alloc_stdin_line(10);
        util_strupr(line);
        
        //if(strcmp(line, "Q") == 0 || strcmp(line, "STOP") == 0 ){
        if(strcmp(line, QUIT_CMD) == 0){

          plot_info_free( info );
          hash_free( ens_table );
          hash_free( ens_rft_table );
          return 0 ;

        } else if(strcmp(line, CREATE_ENS_CMD) == 0){
          
          create_ensemble_batch(ens_table, ens_rft_table);

        } else if (strcmp(line, PLOT_CMD) == 0){
          
          plot_batch(arg_pack);

        } else if (strcmp(line, ATTRIBUTES_CMD) == 0){
          ens_set_plot_attributes_batch(ens_table, ens_rft_table);
        } else if (strcmp(line, QUANTILES_CMD) == 0){
          ens_set_plot_quantile_properties_batch( ens_table );
        } else {
          
          char message[128] ;
          sprintf(message,"Unknown command %s",line) ;
          error_reply(message) ;
          
        } ;
        
        free(line);
      }
      plot_info_free( info );
      hash_free( ens_table );
      hash_free( ens_rft_table );
      return 1 ;
      
    }
  }
  
  hash_type * ens_table = hash_alloc();
  plot_info_type * info = plot_info_alloc( "Plot" , "png" , VIEWER);
  
  print_usage();  

  
  {
    int iarg;
    if (argc > 1) {
      printf("Loading realizations in ensemble: \"Ensemble1\"\n");
      ens_type  * ens  = ens_alloc();
      for (iarg = 1; iarg < argc; iarg++)
        ens_load_summary(ens , argv[iarg]);
      hash_insert_hash_owned_ref( ens_table , "Ensemble1" , ens , ens_free__);
    }
  }
  
  {
    menu_type * menu = menu_alloc("E C L I P S E Ensemble plot" , "Quit" , "q");
    arg_pack_type * arg_pack = arg_pack_alloc();
    
    arg_pack_append_ptr( arg_pack , ens_table );
    arg_pack_append_ptr( arg_pack , info );
    menu_add_item(menu , "Plot summary ensemble(s)" , "p" , plot_all , arg_pack , NULL);
    menu_add_separator( menu );
    menu_add_item(menu , "Create new ensemble" , "cC" , create_ensemble , ens_table , NULL);
    menu_add_item(menu , "New simulation case" , "nN" , add_simulation , ens_table , NULL);
    menu_add_item(menu , "Add many simulations" , "mM" , add_many_simulations , ens_table , NULL);
    menu_add_separator( menu );
    menu_add_item(menu , "Set plot attributes (color)" , "aA" , set_plot_attributes , ens_table , NULL);
    menu_run(menu);
    menu_free( menu );
    arg_pack_free( arg_pack );
  }
  
  plot_info_free( info );
  hash_free( ens_table );
  exit(1);
}
