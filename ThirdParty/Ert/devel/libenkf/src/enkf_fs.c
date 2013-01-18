/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_fs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>

#include <util.h>
#include <type_macros.h>
#include <msg.h>
#include <path_fmt.h>
#include <arg_pack.h>
#include <stringlist.h>
#include <arg_pack.h>
#include <block_fs_driver.h>

#include <enkf_fs.h>
#include <enkf_defaults.h>
#include <fs_driver.h>
#include <fs_types.h>
#include <ecl_static_kw.h>
#include <plain_driver.h>
#include <gen_data.h>
#include <time_map.h>
#include <misfit_ensemble.h>


/**

  About storage in the EnKF system
  ================================

  The system for storage in the EnKF system is quite complicated, maybe too
  complicated. The reason the system is so complex is (at least) twofold:

    1. It is a goal that it should be relatively easy to write new
       systems (i.e. drivers) for storage. The current suite of
       drivers (plain_driver_xx) are based on normal (compressed)
       fread() and fwrite() system calls. But with the current
       implementation one could write e.g. a MySQL based driver for
       storage without touching(??) the rest of the EnKF code.

    2. The parameters / static restart data / dynamic restart data
       have very different storage characteristics. By splitting the
       storage up in different drivers we can write drivers which are
       specialized for the different types of data.


  The interface
  -------------     

  The unit of storage in the enkf_fs system is one enkf_node instance. The
  interface between the storage system and the rest of the EnKF system is
  through the enkf_fs functions:

    enkf_fs_fread_node()
    enkf_fs_has_node()
    enkf_fs_fwrite_node()
    enkf_fs_unlink_node()  - not implemented yet.


  So all these functions (partly except enkf_fs_has_node()) work on a enkf_node
  instance, and in addition they take the following input:

    - iens        : ensemble member number
    - report_step : the report_step number we are interested in
    - state       : whether we are considering an analyzed node or a forecast.  

  In addition to the functions enkf_fs_fread_node() and enkf_fs_fwrite_node() there
  are higher level functions enkf_fs_fread_alloc_ensemble() to load an ensemble of
  nodes and enkf_fs_fread_alloc_ts() to load a time series of nodes. The driver can
  implement these functions (to get a performance gain when loading many nodes). It
  is not necceasry to implement these functions, the enkf_fs layer has simple
  functions doing repeated calls to the enkf_fs_fread_node() function.


  The drivers
  -----------

  The enkf_fs layer does not self implement the functions to read and write
  nodes. Instead what happens is:

    1. We determine the type of the node (static/dynamic/parameter), and select
       the appropriate driver.

    2. The appropriate driver is called to implement e.g. the fread_node
       functions.

  The different types of data have different characteristcs, which the driver is
  implemented to support. The characteristics the drivers support are the
  following:


  dynamic driver
  --------------
  This is the simplest driver, all data is stored both in a forecast version and
  an analyzed version.


  parameter driver
  ----------------
  This driver utilizes that parameters do not change during the forward model,
  i.e. (analyzed , t) = (forecast , t - 1). So, only one version of the data is
  actually stored; if you ask for the forecast you just get the data from the
  previous report_step. 
  To support spin-ups and such the driver will actually go backwards in
  report_time all the way until a node is found on disk.


  static driver
  -------------
  Like the parameter driver this also only stores one version od the data,
  however in addition it has to query the node for a ID to support multiply
  occuring keywords in ECLIPSE restart files.

  Currently only the plain_driver_xxx family has been implemented. Observe that
  there is no dependencies between the drivers, it is perfectly possible to
  implement a new driver for storage of static datat only. (There is probably a
  large amount of static data which is common both between members and for
  several consecutive report steps; utilizing that one could write a static
  driver which was admittedly slower, but leaner on the storage.)

  The drivers are allocated prior to allocating the enkf_fs instance, and
  pointers are passed in when allocating the enkf_fs instance.


  Mounting the filesystem
  -----------------------

  Mounting the filesystem - cool ehh?? Anyway, the important point is
  that the moment ensemble information has hit the filesystem later
  versions of the enkf program must support exactly that lay-out,
  those drivers+++. To ensure this I see two possibilities:

    1. We can freeze the filesystem drivers, and the layout on disk
       indefinetly.

    2. We can store the information needed to bootstrap the drivers,
       according to the current layout on disk, in the
       filesystem. I.e. something like a '/etc/fstab' file.

  We have chosen the second alternative. Currently this implemented as
  follows:

    1. In main() we query for the file {root-path}/enkf_mount_info. If
       that file does not exists it is created by calls to the
       selected drivers xxxx_fwrite_mount_info() functions.

    2. enkf_fs_mount() is called with the enkf_mount_info as input.

  The enkf_mount_info file (BINARY) consists of four records (one for
  each driver, including the index). The format of each record is:

     DRIVER_CATEGORY   DRIVER_ID    INFO 
     int               int          void *

  The driver category should be one of the four integer values in
  fs_driver_type (fs_types.h) and DRIVER_ID is one the integer
  values in fs_driver_impl. The last void * data is whatever
  (serialized) info the driver needs to bootstrap. This info is
  written by the drivers xxxx_fwrite_mount_info() function, and it is
  used when the driver is allocated with xxxx_fread_alloc().

  The different drivers can be in arbitrary order in the
  enkf_mount_info file, but when four records are read it checks that
  all drivers have been initialized, and aborts if that is not the
  case.
  
  If the enkf_mount_info file is deleted you (might) be fucked. It
  is currently 'protected' with chomd a-w - but that is of course not
  foolprof.
*/




/**
   Observe the following convention: the initial ensemble at report
   step 0 is supposed to be analyzed. If we ask for the forecast at
   report_step 0, we should get the analyzed value.
*/


#define ENKF_FS_TYPE_ID       1089763
#define ENKF_MOUNT_MAP        "enkf_mount_info"
#define TIME_MAP_FILE         "time-map"
#define MISFIT_ENSEMBLE_FILE  "misfit-ensemble"

struct enkf_fs_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                   * mount_point;
  
  fs_driver_type         * dynamic_forecast;
  fs_driver_type         * dynamic_analyzed;
  fs_driver_type         * parameter;
  fs_driver_type         * eclipse_static;
  fs_driver_type         * index ;

  bool                     read_only;             /* Whether this filesystem has been mounted read-only. */
  time_map_type          * time_map;
  misfit_ensemble_type   * misfit_ensemble;
  /* 
     The variables below here are for storing arbitrary files within 
     the enkf_fs storage directory, but not as serialized enkf_nodes.
  */
  path_fmt_type             * case_fmt;
  path_fmt_type             * case_member_fmt;
  path_fmt_type             * case_tstep_fmt;
  path_fmt_type             * case_tstep_member_fmt;
};

/*****************************************************************/


UTIL_SAFE_CAST_FUNCTION( enkf_fs , ENKF_FS_TYPE_ID)
UTIL_IS_INSTANCE_FUNCTION( enkf_fs , ENKF_FS_TYPE_ID)

static enkf_fs_type * enkf_fs_alloc_empty( const char * mount_point , bool read_only) {
  enkf_fs_type * fs          = util_malloc(sizeof * fs );
  UTIL_TYPE_ID_INIT( fs , ENKF_FS_TYPE_ID );
  fs->time_map               = time_map_alloc();
  fs->misfit_ensemble        = misfit_ensemble_alloc();
  fs->index                  = NULL;
  fs->eclipse_static         = NULL;
  fs->parameter              = NULL;
  fs->dynamic_forecast       = NULL;
  fs->dynamic_analyzed       = NULL;
  fs->read_only              = read_only;
  fs->mount_point            = util_alloc_string_copy( mount_point );
  if (mount_point == NULL)
    util_abort("%s: fatal internal error: mount_point == NULL \n",__func__);
  return fs;
}




static int enkf_fs_fread_fs_version__(FILE * stream) {
  int version;
  long fs_tag = util_fread_long( stream );
  if (fs_tag == FS_MAGIC_ID) 
    version = util_fread_int(stream);
  else
    version = 0;
  return version;
}


/**
   -1 : No mount map found. 
   0  : Old mount map without version info.
   x  : Actual version info. 
*/

static int enkf_fs_get_fs_version__(const char * config_file) {
  int version = -1;
  if (util_file_exists(config_file)) {
    FILE * stream = util_fopen(config_file , "r");
    version = enkf_fs_fread_fs_version__(stream);
    fclose(stream);
  } 
  return version;
}

/**
   Function written to look for old (version <= 104) mount info maps. 
*/

int enkf_fs_get_version104( const char * path ) {
  char * config_file = util_alloc_filename( path , ENKF_MOUNT_MAP, NULL);  
  int version = enkf_fs_get_fs_version__( config_file );
  free( config_file );
  return version;
}







/*****************************************************************/


static void enkf_fs_init_path_fmt( enkf_fs_type * fs) {
  /*
    Installing the path_fmt instances for the storage of arbitrary files.
  */
  fs->case_fmt              = path_fmt_alloc_directory_fmt( DEFAULT_CASE_PATH );
  fs->case_member_fmt       = path_fmt_alloc_directory_fmt( DEFAULT_CASE_MEMBER_PATH );
  fs->case_tstep_fmt        = path_fmt_alloc_directory_fmt( DEFAULT_CASE_TSTEP_PATH );
  fs->case_tstep_member_fmt = path_fmt_alloc_directory_fmt( DEFAULT_CASE_TSTEP_MEMBER_PATH );

}


static void enkf_fs_create_plain_fs( FILE * stream , void * arg) {
  
  plain_driver_create_fs( stream , DRIVER_PARAMETER        , DEFAULT_PLAIN_NODE_PARAMETER_PATH        , DEFAULT_PLAIN_VECTOR_PARAMETER_PATH);
  plain_driver_create_fs( stream , DRIVER_STATIC           , DEFAULT_PLAIN_NODE_STATIC_PATH           , DEFAULT_PLAIN_VECTOR_STATIC_PATH);
  plain_driver_create_fs( stream , DRIVER_DYNAMIC_FORECAST , DEFAULT_PLAIN_NODE_DYNAMIC_FORECAST_PATH , DEFAULT_PLAIN_VECTOR_DYNAMIC_FORECAST_PATH);
  plain_driver_create_fs( stream , DRIVER_DYNAMIC_ANALYZED , DEFAULT_PLAIN_NODE_DYNAMIC_ANALYZED_PATH , DEFAULT_PLAIN_VECTOR_DYNAMIC_ANALYZED_PATH);
  plain_driver_create_fs( stream , DRIVER_INDEX            , DEFAULT_PLAIN_NODE_INDEX_PATH            , DEFAULT_PLAIN_VECTOR_INDEX_PATH );
  
}



static void enkf_fs_create_block_fs( FILE * stream , int num_drivers , const char * mount_point , void * arg) {
  
  block_fs_driver_create_fs( stream , mount_point , DRIVER_PARAMETER        , num_drivers , "Ensemble/mod_%d" , "PARAMETER");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_STATIC           , num_drivers , "Ensemble/mod_%d" , "STATIC");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_DYNAMIC_FORECAST , num_drivers , "Ensemble/mod_%d" , "FORECAST");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_DYNAMIC_ANALYZED , num_drivers , "Ensemble/mod_%d" , "ANALYZED");
  block_fs_driver_create_fs( stream , mount_point , DRIVER_INDEX            , 1           , "Index"           , "INDEX");
  
}


static void enkf_fs_assign_driver( enkf_fs_type * fs , fs_driver_type * driver , fs_driver_enum driver_type ) {
  switch(driver_type) {
  case(DRIVER_PARAMETER):
    fs->parameter = driver;
    break;
  case(DRIVER_STATIC):
    fs->eclipse_static = driver;
    break;
  case(DRIVER_DYNAMIC_FORECAST):
    fs->dynamic_forecast = driver;
    break;
  case(DRIVER_DYNAMIC_ANALYZED):
    fs->dynamic_analyzed = driver;
    break;
  case(DRIVER_INDEX):
    fs->index = driver;
    break;
  }
}


static enkf_fs_type *  enkf_fs_mount_block_fs( FILE * fstab_stream , const char * mount_point , bool read_only ) {
  enkf_fs_type * fs = enkf_fs_alloc_empty( mount_point , read_only );
  {
    int driver_nr;
    for (driver_nr = 0; driver_nr < 5; driver_nr++) {
      fs_driver_enum driver_type = util_fread_int( fstab_stream );
      fs_driver_type * driver = block_fs_driver_open( fstab_stream , mount_point , driver_type , read_only);
      
      enkf_fs_assign_driver( fs , driver , driver_type );
    }
  }
  return fs;
}


static enkf_fs_type *  enkf_fs_mount_plain( FILE * fstab_stream , const char * mount_point , bool read_only ) {
  enkf_fs_type * fs = enkf_fs_alloc_empty( mount_point , read_only);
  {
    int driver_nr;
    for (driver_nr = 0; driver_nr < 5; driver_nr++) {
      fs_driver_enum driver_type = util_fread_int( fstab_stream );
      fs_driver_type * driver = plain_driver_open( fstab_stream , mount_point );
      
      enkf_fs_assign_driver( fs , driver , driver_type );
    }
  }
  return fs;
}



void enkf_fs_create_fs( const char * mount_point, fs_driver_impl driver_id , void * arg) {
  const int num_drivers = 32;
  FILE * stream = fs_driver_open_fstab( mount_point , true );
  if (stream != NULL) {
    fs_driver_init_fstab( stream, driver_id);
    {
      switch( driver_id ) {
      case( BLOCK_FS_DRIVER_ID ):
        enkf_fs_create_block_fs( stream , num_drivers , mount_point , arg );
        break;
      case( PLAIN_DRIVER_ID ):
        enkf_fs_create_plain_fs( stream , arg );
        break;
      default:
        util_abort("%s: Invalid driver_id value:%d \n",__func__ , driver_id );
      }
    }
    fclose( stream );
  }
}


static void enkf_fs_fsync_time_map( enkf_fs_type * fs ) {
  char * filename = enkf_fs_alloc_case_filename( fs , TIME_MAP_FILE );
  time_map_fwrite( fs->time_map , filename );
  free( filename );
}


static void enkf_fs_fread_time_map( enkf_fs_type * fs ) {
  char * filename = enkf_fs_alloc_case_filename( fs , TIME_MAP_FILE );
  time_map_fread( fs->time_map , filename );
  free( filename );
}


static void enkf_fs_fread_misfit( enkf_fs_type * fs ) {
  FILE * stream = enkf_fs_open_excase_file( fs , MISFIT_ENSEMBLE_FILE );
  if (stream != NULL) {
    misfit_ensemble_fread( fs->misfit_ensemble , stream );
    fclose( stream );
  }
}


static void enkf_fs_fwrite_misfit( enkf_fs_type * fs ) {
  if (misfit_ensemble_initialized( fs->misfit_ensemble )) {
    FILE * stream = enkf_fs_open_case_file( fs , MISFIT_ENSEMBLE_FILE , "w");
    misfit_ensemble_fwrite( fs->misfit_ensemble , stream );
    fclose( stream );
  }
}


enkf_fs_type * enkf_fs_open( const char * mount_point , bool read_only) {
  enkf_fs_type * fs = NULL;
  FILE * stream = fs_driver_open_fstab( mount_point , false );

  if (stream != NULL) {
    fs_driver_assert_magic( stream );
    fs_driver_assert_version( stream , mount_point );
    {
      fs_driver_impl driver_id = util_fread_int( stream );
    
      switch( driver_id ) {
      case( BLOCK_FS_DRIVER_ID ):
        fs = enkf_fs_mount_block_fs( stream , mount_point , read_only );
        break;
      case( PLAIN_DRIVER_ID ):
        fs = enkf_fs_mount_plain( stream , mount_point , read_only );
        break;
      default:
        util_abort("%s: unrecognized driver_id:%d \n",__func__ , driver_id );
      }
    }
    fclose( stream );
    enkf_fs_init_path_fmt( fs );
    enkf_fs_fread_time_map( fs );
    enkf_fs_fread_misfit( fs );
  }
  return fs;
}


bool enkf_fs_exists( const char * path ) {
  bool exists   = false;
  
  FILE * stream = fs_driver_open_fstab( path , false );
  if (stream != NULL) {
    exists = true;
    fclose( stream );
  }

  return exists;
}




/*****************************************************************/



static void enkf_fs_free_driver(fs_driver_type * driver) {
  driver->free_driver(driver);
}


void enkf_fs_close( enkf_fs_type * fs ) {
  enkf_fs_fsync( fs );
  enkf_fs_fwrite_misfit( fs );

  enkf_fs_free_driver( fs->dynamic_forecast );
  enkf_fs_free_driver( fs->dynamic_analyzed );
  enkf_fs_free_driver( fs->parameter );
  enkf_fs_free_driver( fs->eclipse_static );
  enkf_fs_free_driver( fs->index );

  util_safe_free( fs->mount_point );
  path_fmt_free( fs->case_fmt );
  path_fmt_free( fs->case_member_fmt );
  path_fmt_free( fs->case_tstep_fmt );
  path_fmt_free( fs->case_tstep_member_fmt );

  time_map_free( fs->time_map );
  free( fs );
}



static void * select_dynamic_driver(enkf_fs_type * fs , state_enum state ) {
  void * driver = NULL;

  if (state == ANALYZED) 
    driver = fs->dynamic_analyzed;
  else if (state == FORECAST) 
    driver = fs->dynamic_forecast;
  else 
    util_abort("%s: tried to select dynamic driver according to ID:%d - invalid \n",__func__ , state);
  
  return driver;
}


static void * enkf_fs_select_driver(enkf_fs_type * fs , enkf_var_type var_type, state_enum state , const char * key) {
  void * driver = NULL;
  switch (var_type) {
  case(PARAMETER):
    driver = fs->parameter;
    break;
  case(DYNAMIC_RESULT):
    driver = select_dynamic_driver( fs , state );
    break;
  case(DYNAMIC_STATE):
    driver = select_dynamic_driver( fs , state );
    break;
  case(STATIC_STATE):
    driver = fs->eclipse_static;
    break;
  default:
    util_abort("%s: fatal internal error - could not determine enkf_fs driver for object:%s[integer type:%d] - aborting.\n",__func__, key , var_type);
  }
  return driver;
}



/*****************************************************************/
/* Exported functions for enkf_node instances . */


static void enkf_fs_fsync_driver( fs_driver_type * driver ) {
  if (driver->fsync_driver != NULL)
    driver->fsync_driver( driver );
}



void enkf_fs_fsync( enkf_fs_type * fs ) {
  enkf_fs_fsync_driver( fs->parameter );
  enkf_fs_fsync_driver( fs->eclipse_static );
  enkf_fs_fsync_driver( fs->dynamic_forecast );
  enkf_fs_fsync_driver( fs->dynamic_analyzed );
  enkf_fs_fsync_driver( fs->index );

  enkf_fs_fsync_time_map( fs );
}


/**
  For parameters the state is uniquely identified by the report step,
  corresponding to the __analyzed__ state. If you really want the
  forecast that is achieved by subtracting one.
*/

static int __get_parameter_report_step( fs_driver_type * driver , const char * node_key , int report_step , int iens , state_enum state) {
  if (state == FORECAST) {
    if (report_step > 0) /* Time step zero is special - we do not differentiate between forecast and analyzed. */
      report_step--;
  } else if (state != ANALYZED)
    util_abort("%s: asked for state:%d - internal error \n",__func__ , state);
  {
    /*
      Observe that if we do not find the filename we are looking for, we
      seek backwards through the report numbers, all the way back to
      report_nr 0. The direct motivation for this functionality is the
      following situation:
      
        1. We do a spin-up from report 0 to report R1.
      
        2. We start the assimulation from R1, then we have to go all the
           way back to report 0 to get hold of the parameter.

    */
    while (!driver->has_node( driver , node_key , report_step , iens )) {
      report_step--;
      if (report_step < 0)
        util_abort("%s: can not find any stored item for key:%s(%d). Forgot to initialize ensemble ??? \n",__func__ , node_key, iens);
    }
  }
  return report_step;
}


void enkf_fs_fread_node(enkf_fs_type * enkf_fs , buffer_type * buffer , 
                        const char * node_key , 
                        enkf_var_type var_type , 
                        int report_step, 
                        int iens , 
                        state_enum state) {
  
  fs_driver_type * driver = enkf_fs_select_driver(enkf_fs , var_type , state , node_key );
  if (var_type == PARAMETER)
    report_step = __get_parameter_report_step(driver , node_key , report_step , iens , state);
  
  buffer_rewind( buffer );
  driver->load_node(driver , node_key ,  report_step , iens , buffer);
}


void enkf_fs_fread_vector(enkf_fs_type * enkf_fs , buffer_type * buffer , 
                          const char * node_key , 
                          enkf_var_type var_type , 
                          int iens , 
                          state_enum state) {
  
  fs_driver_type * driver = enkf_fs_select_driver(enkf_fs , var_type , state , node_key );

  buffer_rewind( buffer );
  driver->load_vector(driver , node_key ,  iens , buffer);
}



bool enkf_fs_has_node(enkf_fs_type * enkf_fs , const char * node_key , enkf_var_type var_type , int report_step , int iens , state_enum state) {
  fs_driver_type * driver = fs_driver_safe_cast(enkf_fs_select_driver(enkf_fs , var_type , state , node_key));
  return driver->has_node(driver , node_key , report_step , iens ); 
}


bool enkf_fs_has_vector(enkf_fs_type * enkf_fs , const char * node_key , enkf_var_type var_type , int iens , state_enum state) {
  fs_driver_type * driver = fs_driver_safe_cast(enkf_fs_select_driver(enkf_fs , var_type , state , node_key));
  return driver->has_vector(driver , node_key , iens ); 
}

void enkf_fs_fwrite_node(enkf_fs_type * enkf_fs , buffer_type * buffer , const char * node_key, enkf_var_type var_type,  
                         int report_step , int iens , state_enum state) {
  if (enkf_fs->read_only)
    util_abort("%s: attempt to write to read_only filesystem mounted at:%s - aborting. \n",__func__ , enkf_fs->mount_point);
  {
    void * _driver = enkf_fs_select_driver(enkf_fs , var_type , state , node_key);
    {
      fs_driver_type * driver = fs_driver_safe_cast(_driver);
      driver->save_node(driver , node_key , report_step , iens , buffer);
    }
  }
}


void enkf_fs_fwrite_vector(enkf_fs_type * enkf_fs , buffer_type * buffer , const char * node_key, enkf_var_type var_type,  
                           int iens , state_enum state) {
  if (enkf_fs->read_only)
    util_abort("%s: attempt to write to read_only filesystem mounted at:%s - aborting. \n",__func__ , enkf_fs->mount_point);
  {
    void * _driver = enkf_fs_select_driver(enkf_fs , var_type , state , node_key);
    {
      fs_driver_type * driver = fs_driver_safe_cast(_driver);
      driver->save_vector(driver , node_key  , iens , buffer);
    }
  }
}




/*****************************************************************/





/*****************************************************************/

const char * enkf_fs_get_mount_point( const enkf_fs_type * fs ) {
  return fs->mount_point;
}

void enkf_fs_debug_fprintf( const enkf_fs_type * fs) {
  printf("-----------------------------------------------------------------\n");
  printf("fs...................: %p \n",fs );
  printf("Mount point..........: %s \n",fs->mount_point );
  printf("Dynamic forecast.....: %p \n",fs->dynamic_forecast );
  printf("Dynamic analyzed.....: %p \n",fs->dynamic_analyzed );
  printf("Parameter............: %p \n",fs->parameter );
  printf("Index................: %p \n",fs->index );
  printf("Static...............: %p \n",fs->eclipse_static );
  printf("-----------------------------------------------------------------\n");
}


/*****************************************************************/
/* Index related functions  . */


void enkf_fs_fwrite_restart_kw_list(enkf_fs_type * enkf_fs , int report_step , int iens, const stringlist_type * kw_list) {
  buffer_type * buffer = buffer_alloc(1024);
  stringlist_buffer_fwrite( kw_list , buffer );
  {
    fs_driver_type * driver = enkf_fs->index;
    driver->save_node(driver , "kw_list" , report_step , iens , buffer);
  }
  buffer_free( buffer );
}



void enkf_fs_fread_restart_kw_list(enkf_fs_type * enkf_fs , int report_step , int iens, stringlist_type * kw_list) {
  buffer_type * buffer = buffer_alloc(1024);
  {
    fs_driver_type * driver = enkf_fs->index;
    driver->load_node(driver , "kw_list" , report_step , iens , buffer);
  }
  stringlist_buffer_fread( kw_list , buffer );
  buffer_free( buffer );
}

/*****************************************************************/
/* write_dir / read_dir confusion. */

char * enkf_fs_alloc_case_filename( const enkf_fs_type * fs , const char * input_name) {
  char * filename = path_fmt_alloc_file( fs->case_fmt , false , fs->mount_point , input_name);
  return filename;
}


char * enkf_fs_alloc_case_member_filename( const enkf_fs_type * fs , int iens , const char * input_name) {
  char * filename = path_fmt_alloc_file( fs->case_member_fmt , false , fs->mount_point , iens , input_name);
  return filename;
}


char * enkf_fs_alloc_case_tstep_filename( const enkf_fs_type * fs , int tstep , const char * input_name) {
  char * filename    = path_fmt_alloc_file( fs->case_tstep_fmt , false , fs->mount_point , tstep , input_name);
  return filename;
}

char * enkf_fs_alloc_case_tstep_member_filename( const enkf_fs_type * fs , int tstep , int iens , const char * input_name) {
  char * filename = path_fmt_alloc_file( fs->case_tstep_member_fmt , false , fs->mount_point , tstep , iens , input_name);
  return filename;
}



FILE * enkf_fs_open_case_file( const enkf_fs_type * fs , const char * input_name , const char * mode) {
  char * filename = enkf_fs_alloc_case_filename( fs , input_name );
  FILE * stream   = util_mkdir_fopen( filename , mode );
  free( filename );
  return stream;
}


FILE * enkf_fs_open_case_member_file( const enkf_fs_type * fs , const char * input_name , int iens , const char * mode) {
  char * filename = enkf_fs_alloc_case_member_filename( fs , iens , input_name );
  FILE * stream   = util_mkdir_fopen( filename , mode );
  free( filename );
  return stream;
}
  

FILE * enkf_fs_open_case_tstep_file( const enkf_fs_type * fs , const char * input_name , int tstep , const char * mode) {
  char * filename = enkf_fs_alloc_case_tstep_filename( fs , tstep , input_name );
  FILE * stream   = util_mkdir_fopen( filename , mode );
  free( filename );
  return stream;
}


FILE * enkf_fs_open_case_tstep_member_file( const enkf_fs_type * fs , const char * input_name , int tstep , int iens , const char * mode) {
  char * filename = enkf_fs_alloc_case_tstep_member_filename( fs , tstep , iens , input_name );
  FILE * stream   = util_mkdir_fopen( filename , mode );
  free( filename );
  return stream;
}

/*****************************************************************/
/* 
   The open_exXXX functions will return NULL if the file does not
   already exist. These functions can only be used to open with 'r'
   mode.
*/



static FILE * enkf_fs_open_exfile( const char * filename ) {
  if (util_file_exists(filename))
    return util_fopen( filename , "r");
  else
    return NULL;
}

FILE * enkf_fs_open_excase_file( const enkf_fs_type * fs , const char * input_name ) {
  char * filename = enkf_fs_alloc_case_filename( fs , input_name );
  FILE * stream   = enkf_fs_open_exfile( filename );
  free( filename );
  return stream;
}


FILE * enkf_fs_open_excase_member_file( const enkf_fs_type * fs , const char * input_name , int iens ) {
  char * filename = enkf_fs_alloc_case_member_filename( fs , iens , input_name );
  FILE * stream   = enkf_fs_open_exfile( filename );
  free( filename );
  return stream;
}
  

FILE * enkf_fs_open_excase_tstep_file( const enkf_fs_type * fs , const char * input_name , int tstep ) {
  char * filename = enkf_fs_alloc_case_tstep_filename( fs , tstep , input_name );
  FILE * stream   = enkf_fs_open_exfile( filename );
  free( filename );
  return stream;
}


FILE * enkf_fs_open_excase_tstep_member_file( const enkf_fs_type * fs , const char * input_name , int tstep , int iens ) {
  char * filename = enkf_fs_alloc_case_tstep_member_filename( fs , tstep , iens , input_name );
  FILE * stream   = enkf_fs_open_exfile( filename );
  free( filename );
  return stream;
}
  
/*****************************************************************/

time_map_type * enkf_fs_get_time_map( const enkf_fs_type * fs ) {
  return fs->time_map;
}


misfit_ensemble_type * enkf_fs_get_misfit_ensemble( const enkf_fs_type * fs ) {
  return fs->misfit_ensemble;
}

