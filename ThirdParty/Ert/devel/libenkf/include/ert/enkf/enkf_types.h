/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_types.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_TYPES_H__
#define __ENKF_TYPES_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/arg_pack.h>




/*
  This enum signals the three different states a "cell" in
  observation/data node can be in:

  ACTIVE: The cell is active and should be used/updated in EnKF
    analysis.

  LOCAL_INACTIVE: The cell is not included in the current local
    analysis ministep

  DEACTIVATED: The cell has been deactivated by the functionality
    deactivating outliers.

*/

typedef enum { ACTIVE         = 1,
               LOCAL_INACTIVE = 2,                   /* Not active in current local update scheme. */
               DEACTIVATED    = 3,                   /* Deactivaed due to to small overlap, or... */ 
               MISSING        = 4} active_type;      /* Set as missing by the forward model. */



/*
  The enkf_var_type enum defines logical groups of variables. All
  variables in the same group, i.e. 'parameter' are typically treated
  in the same manner. So the reason for creating this type is to be
  able to say for instance: "Load all dynamic_state variables".

  Observe that these are used as bitmask's, i.e. the numerical values
  must be a power of 2 series.
*/

typedef enum {INVALID_VAR      =  0  ,    /* */
              PARAMETER        =  1  ,    /* A parameter which is updated with enkf: PORO , MULTFLT , ..*/
              DYNAMIC_STATE    =  2  ,    /* Dynamic data which are needed for a restart - i.e. pressure and saturations.  */
              DYNAMIC_RESULT   =  4  ,    /* Dynamic results which are NOT needed for a restart - i.e. well rates. */
              STATIC_STATE     =  8  ,    /* Keywords like XCON++ from eclipse restart files - which are just dragged along          */ 
              INDEX_STATE      = 16 }     /* Index data - enum value is used for storage classification */ 
enkf_var_type; 
  
  

typedef enum { DEFAULT_KEEP    = 0,    /* Remove for enkf assimilation - keep for ensemble experiments. */
               EXPLICIT_DELETE = 1,    /* Remove unconditionally */
               EXPLICIT_KEEP   = 2}    /* keep unconditionally */
keep_runpath_type;





/* 
   ert_impl_type are the actual node implementation types. Observe
   that one ert_impl_type can be used in several ways as
   enkf_var_type. For instance the pressure is implemented with a
   field, and behaves as a dynamic_state variable, on the other hand
   the permeability is also implemented as a field, but this is a
   parameter.

   These correspond to implementation types. The numbers are on disk,
   and should **NOT BE UPDATED**. The __MIN_TYPE and __MAX_TYPE
   identifiers are needed for the block_fs_driver.
*/


  
typedef enum {INVALID          = 0   , 
              IMPL_TYPE_OFFSET = 100,
              STATIC           = 100 ,       /* MULTZ has been removed & MULTFLT */ 
              FIELD            = 104 ,       /* WELL has been removed  */
              GEN_KW           = 107 ,       /* RELPERM has been removed & HAVANA_FAULT */
              SUMMARY          = 110 ,       /* TPGZONE has been removed */
              GEN_DATA         = 113 ,       /* PILOT_POINT has been removed */
              SURFACE          = 114 ,
              CONTAINER        = 115 } ert_impl_type;
  
  

/* 
   Should update the functions enkf_types_get_impl_name() and
   enkf_types_get_impl_type__() when this enum is updated.
   In addition to enkf_config_add_type().
*/


typedef enum   {UNDEFINED   = 0 ,
                FORECAST    = 2,              /* FORECAST and ANALYZED must be 2^n */
                ANALYZED    = 4,
                BOTH        = 6} state_enum;  /* It is important that both == (forecast + analyzed) */
  /**
     The state == both is used for output purposes (getting both forecast and analyzed).
  */

#define ENKF_STATE_ENUM_DEFS {.value = 0 , .name = "UNDEFINED"}, \
                             {.value = 2 , .name = "FORECAST"} ,\
                             {.value = 4 , .name = "ANALYZED"},\
                             {.value = 6 , .name = "BOTH"}
#define ENKF_STATE_ENUM_SIZE 4





  /** 
      These are 2^n bitmasks.
  */

typedef enum { TRUNCATE_NONE   = 0,
               TRUNCATE_MIN    = 1,
               TRUNCATE_MAX    = 2 } truncation_type;




/**
   This enum is used to differentiate between different types of
   run. The point is that depending on this mode we can be more or
   less restrictive on the amount of input we require from the user. 

   In mode enkf_assimlation ( which is the default ), we require quite
   a lot of info, whereas in the case screening_experiment we require
   less.

   screening_experiment: 
      - SIZE
      - RUNPATH
      - ECLBASE
      - SCHEDULE_FILE
      - DATA_FILE
      - FORWARD_MODEL.

   ensemble_experiment:
      - ENSPATH
      - INIT_FILE (or estimation of EQUIL)

   enkf_assmilation:
      - RESULT_PATH

*/

typedef enum { ENKF_ASSIMILATION       = 1, 
               ENSEMBLE_EXPERIMENT     = 2,
               SMOOTHER_UPDATE         = 4 ,
               INIT_ONLY               = 8 } run_mode_type;
               

#define ENKF_RUN_ENUM_DEFS {.value = 1 , .name = "ENKF_ASSIMILATION"},   \
                           {.value = 2 , .name = "ENSEMBLE_EXPERIMENT"} 

#define ENKF_RUN_ENUM_SIZE 2



/**
   This enum enumerates the different types of inflation which should
   be used. Observe that the actual variable used is not en enum
   instance, but rather an ordinary integer which is in general a sum
   of of the values listed in this enum.
*/


  typedef enum { NO_INFLATION     = 0,
                 SCALAR_INFLATION = 1,
                 LOCAL_INFLATION  = 2} inflation_mode_type;



  typedef enum { JOB_NOT_STARTED  = 0,
                 JOB_RUNNING      = 1,
                 JOB_RUN_FAILURE  = 2,
                 JOB_LOAD_FAILURE = 3,
                 JOB_RUN_OK       = 4  } run_status_type;
  

/*****************************************************************/

/**
   This enum is used when we are setting up the dependencies between
   observations and variables. The modes all_active and inactive are
   sufficient information, for the values partly active we need
   additional information.

   The same type is used both for variables (PRESSURE/PORO/MULTZ/...)
   and for observations.
*/

typedef enum {
  ALL_ACTIVE    = 1,       /* The variable/observation is fully active, i.e. all cells/all faults/all .. */
  INACTIVE      = 2,       /* Fully inactive */
  PARTLY_ACTIVE = 3        /* Partly active - must supply additonal type spesific information on what is active.*/
} active_mode_type; 


  typedef struct {
    int        report_step;
    int        iens;  
    state_enum state;
  } node_id_type;
  
  

/*****************************************************************/
typedef struct enkf_obs_struct enkf_obs_type;

  


const char      * enkf_types_get_var_name(enkf_var_type var_type);
ert_impl_type     enkf_types_get_impl_type(const char * );
const char      * enkf_types_get_impl_name(ert_impl_type );
ert_impl_type     enkf_types_check_impl_type(const char * );

#ifdef __cplusplus
}
#endif
#endif
