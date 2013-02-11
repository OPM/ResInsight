/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_types.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_TYPES_H__
#define __SCHED_TYPES_H__
#ifdef __cplusplus 
extern "C" {
#endif

#include <stdbool.h>

/**
   Contains numerous typedefs for the types used in the sched_kw keywords. 
*/


/*
  The two (supported) keywords used for time stepping.
*/

typedef enum { DATES_TIME = 1,
               TSTEP_TIME = 2 } sched_time_enum;



typedef double (sched_history_callback_ftype)    (const void * , int);  
  



typedef enum { INJECTOR = 1,
               PRODUCER = 2 } well_state_well_type;



/**
   This enum is used for internalizing both the sched_kw_wconhist
   keyword, and the sched_kw_wconinje keyword. Observe that the 'AUTO'
   mode is only applicable when used in injector context.
*/
  

typedef enum { DEFAULT = 0,
               OPEN    = 1, 
               STOP    = 2, 
               SHUT    = 3, 
               AUTO    = 4 }   well_status_enum;

  
/**
   There is no default injector type.
*/
typedef enum {WATER = 0 , 
              GAS   = 1 , 
              OIL   = 2 } sched_phase_enum;

  
/**
   This enum is used to enumerate the different control modes. Observe
   that the enum is used BOTH by the WCONHIST and the WCONINJE
   keywords (only the enum value 'RESV' is actually shared between the
   two keywords.

   The final element in the enum 'CM_SHUT' is only used internally; that
   will never be in the schedule file.
*/
   
typedef enum {RESV     = 0, /* Injector || Producer */
              RATE     = 1, /* Injector */
              BHP      = 2, /* Injector  */
              THP      = 3, /* Injector  */ 
              GRUP     = 4, /* Injector  */
              ORAT     = 5, /* Producer  */ 
              WRAT     = 6, /* Producrer */ 
              GRAT     = 7, /* Producer  */
              LRAT     = 8, /* Producer  */ 
              CM_SHUT  = 100 } well_cm_enum;
  

#define CM_RATE_STRING "RATE"
#define CM_RESV_STRING "RESV"
#define CM_BHP_STRING  "BHP"
#define CM_THP_STRING  "THP"
#define CM_GRUP_STRING "GRUP"
#define CM_ORAT_STRING "ORAT"
#define CM_WRAT_STRING "WRAT"
#define CM_GRAT_STRING "GRAT"
#define CM_LRAT_STRING "LRAT"


/*****************************************************************/

#define FIELD_GROUP "FIELD"

/*****************************************************************/



typedef enum {NONE               =     0,
              WCONHIST           =     1,
              DATES              =     2, 
              COMPDAT            =     4, 
              TSTEP              =     8, 
              TIME               =    16,   /* Not implemented support */
              WELSPECS           =    32, 
              GRUPTREE           =    64,
              INCLUDE            =   128,   
              UNTYPED            =   256,
              WCONINJ            =   512,
              WCONINJE           =  1024, 
              WCONINJH           =  2048,
              WCONPROD           =  4096,  
              NUM_SCHED_KW_TYPES =  8192} sched_kw_type_enum;
              

sched_kw_type_enum sched_kw_type_from_string(const char * kw_name);
const char *       sched_kw_type_name(sched_kw_type_enum kw_type);

/*****************************************************************/  
sched_phase_enum       sched_phase_type_from_string(const char * type_string);
const char *           sched_phase_type_string(sched_phase_enum type);
const char           * sched_types_get_status_string(well_status_enum status);
well_status_enum       sched_types_get_status_from_string(const char * st_string);

well_cm_enum           sched_types_get_cm_from_string(const char * cm_string , bool wconhist);
const char *           sched_types_get_cm_string( well_cm_enum cm );

#ifdef __cplusplus 
}
#endif
#endif
