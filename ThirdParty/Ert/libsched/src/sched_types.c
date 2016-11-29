/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_types.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/sched/sched_types.h>

#define SCHED_KW_DEFAULT_ITEM "*"


#define TYPE_WATER_STRING "WATER"
#define TYPE_GAS_STRING   "GAS"
#define TYPE_OIL_STRING   "OIL"

const char * sched_phase_type_string(sched_phase_enum type) {
  switch (type) {
  case(WATER):
    return TYPE_WATER_STRING;
  case(GAS):
    return TYPE_GAS_STRING;
  case(OIL):
    return TYPE_OIL_STRING;
  default:
    return SCHED_KW_DEFAULT_ITEM;
  }
}

sched_phase_enum sched_phase_type_from_string(const char * type_string) {
  if (strcmp(type_string , TYPE_WATER_STRING) == 0)
    return WATER;
  else if (strcmp(type_string , TYPE_GAS_STRING) == 0)
    return GAS;
  else if (strcmp(type_string , TYPE_OIL_STRING) == 0)
    return OIL;
  else {
    util_abort("%s: Could not recognize:%s as injector phase. Valid values are: [%s, %s, %s] \n",__func__ , type_string , TYPE_WATER_STRING , TYPE_GAS_STRING , TYPE_OIL_STRING);
    return 0;
  }
}


/*****************************************************************/


#define WCONHIST_STRING  "WCONHIST"
#define DATES_STRING     "DATES"
#define COMPDAT_STRING   "COMPDAT"
#define TSTEP_STRING     "TSTEP"
#define TIME_STRING      "TIME"
#define WELSPECS_STRING  "WELSPECS"
#define GRUPTREE_STRING  "GRUPTREE"
#define INCLUDE_STRING   "INCLUDE"
#define WCONINJ_STRING   "WCONINJ"
#define WCONINJE_STRING  "WCONINJE"
#define WCONINJH_STRING  "WCONINJH"
#define WCONPROD_STRING  "WCONPROD"

#define UNTYPED_STRING   "UNTYPED"





/**
   This function does a direct translation of a string name to
   implementation type - i.e. an enum instance. Observe that
   (currently) no case-normalization is performed.
*/

sched_kw_type_enum sched_kw_type_from_string(const char * kw_name)
{
  sched_kw_type_enum kw_type = UNTYPED;

  if     ( strcmp(kw_name, GRUPTREE_STRING ) == 0) kw_type = GRUPTREE ;
  else if( strcmp(kw_name, TSTEP_STRING    ) == 0) kw_type = TSTEP    ;
  else if( strcmp(kw_name, INCLUDE_STRING  ) == 0) kw_type = INCLUDE  ;
  else if( strcmp(kw_name, TIME_STRING     ) == 0) kw_type = TIME     ;
  else if( strcmp(kw_name, DATES_STRING    ) == 0) kw_type = DATES    ;
  else if( strcmp(kw_name, WCONHIST_STRING ) == 0) kw_type = WCONHIST ;
  else if( strcmp(kw_name, WELSPECS_STRING ) == 0) kw_type = WELSPECS ;
  else if( strcmp(kw_name, WCONINJ_STRING  ) == 0) kw_type = WCONINJ  ;
  else if( strcmp(kw_name, WCONINJE_STRING ) == 0) kw_type = WCONINJE ;
  else if( strcmp(kw_name, WCONINJH_STRING ) == 0) kw_type = WCONINJH ;
  else if( strcmp(kw_name, WCONPROD_STRING ) == 0) kw_type = WCONPROD ;
  else if( strcmp(kw_name, COMPDAT_STRING  ) == 0) kw_type = COMPDAT  ;   
  
  return kw_type;
}


const char * sched_kw_type_name(sched_kw_type_enum kw_type) {
  if      ( kw_type == GRUPTREE ) return GRUPTREE_STRING ;
  else if ( kw_type == TSTEP    ) return TSTEP_STRING    ;
  else if ( kw_type == INCLUDE  ) return INCLUDE_STRING  ;
  else if ( kw_type == TIME     ) return TIME_STRING     ;
  else if ( kw_type == DATES    ) return DATES_STRING    ;
  else if ( kw_type == WCONHIST ) return WCONHIST_STRING ;
  else if ( kw_type == WELSPECS ) return WELSPECS_STRING ;
  else if ( kw_type == WCONINJ  ) return WCONINJ_STRING  ;
  else if ( kw_type == WCONINJE ) return WCONINJE_STRING ;
  else if ( kw_type == WCONINJH ) return WCONINJH_STRING ;
  else if ( kw_type == WCONPROD ) return WCONPROD_STRING ;
  else if ( kw_type == COMPDAT  ) return COMPDAT_STRING  ;   

  return UNTYPED_STRING; /* Unknown type */
}


/*****************************************************************/

#define STATUS_OPEN_STRING "OPEN"
#define STATUS_STOP_STRING "STOP"
#define STATUS_SHUT_STRING "SHUT"
#define STATUS_AUTO_STRING "AUTO"


const char * sched_types_get_status_string(well_status_enum status) 
{
  switch(status) {
  case(OPEN):
    return STATUS_OPEN_STRING; 
  case(STOP):
    return STATUS_STOP_STRING;
  case(SHUT):
    return STATUS_SHUT_STRING;
  case(AUTO):
    return STATUS_AUTO_STRING;
  case(DEFAULT):
    return SCHED_KW_DEFAULT_ITEM;
  default:
    util_abort("%s: invalid status:%d \n",__func__ , status );
    return 0;
  }
}



well_status_enum sched_types_get_status_from_string(const char * st_string)
{
  if (strcmp( st_string , SCHED_KW_DEFAULT_ITEM ) == 0) 
    return DEFAULT;   
  /* 
     Must be checked by calling scope whether DEFAULT is a valid
     return - and then subsequently apply the correct value.
  */
  else if( strcmp(st_string, STATUS_OPEN_STRING) == 0)
    return OPEN; 
  else if( strcmp(st_string, STATUS_STOP_STRING) == 0)
    return STOP; 
  else if( strcmp(st_string, STATUS_SHUT_STRING) == 0)
    return SHUT; 
  else if( strcmp(st_string, STATUS_AUTO_STRING) == 0)
    return AUTO; 
  else
  {
    util_abort("%s: Could not recognize %s as a well status.\n", __func__, st_string);
    return 0;
  }
}


/*****************************************************************/


const char * sched_types_get_cm_string( well_cm_enum cm ) {
  switch(cm) {
  case( RESV ):
    return CM_RESV_STRING;
    break;
  case( RATE ):
    return CM_RATE_STRING;
    break;
  case( BHP  ):
    return CM_BHP_STRING;
    break;
  case( THP  ):
    return CM_THP_STRING;
    break;
  case( GRUP ):
    return CM_GRUP_STRING;
    break;
  case( ORAT ):
    return CM_ORAT_STRING;
    break;
  case( WRAT ):
    return CM_WRAT_STRING;
    break;
  case( GRAT ):
    return CM_GRAT_STRING;
    break;
  case( LRAT ):
    return CM_LRAT_STRING;
    break;
  default:
    util_abort("%s: invalid value: %s \n", cm );
    return 0;
  }
}




/**
   Must use the strncmp(x,x,4) function for comparison, because
   suddenly files with control mode 'GRUP ' appear; and ECLIPSE
   appearantly eats that nicely.
*/


well_cm_enum sched_types_get_cm_from_string(const char * cm_string , bool wconhist)
{
  if (wconhist) {
    if(     strcmp(cm_string, CM_ORAT_STRING) == 0)
      return ORAT;
    else if(strcmp(cm_string, CM_WRAT_STRING) == 0)
      return WRAT;
    else if(strcmp(cm_string, CM_GRAT_STRING) == 0)
      return GRAT;
    else if(strcmp(cm_string, CM_LRAT_STRING) == 0)
      return LRAT;
    else if(strcmp(cm_string, CM_RESV_STRING) == 0)
      return RESV;
    else {
      util_abort("%s: Could not recognize %s as a control mode.\n", __func__, cm_string);
      return 0;
    }
  } else {
    if(     strncmp(cm_string, CM_RATE_STRING , 4) == 0)
      return RATE;
    else if(strncmp(cm_string, CM_RESV_STRING , 4) == 0)
      return RESV;
    else if(strncmp(cm_string, CM_BHP_STRING , 4) == 0)
      return BHP;
    else if(strncmp(cm_string, CM_THP_STRING, 4) == 0)
      return THP;
    else if(strncmp(cm_string, CM_GRUP_STRING , 4) == 0)
      return GRUP;
    else {
      util_abort("%s: Could not recognize \'%s\' as a control mode. Valid values are: [%s, %s, %s, %s, %s] \n", __func__, cm_string, 
                 CM_RATE_STRING , CM_RESV_STRING , CM_BHP_STRING, CM_THP_STRING, CM_GRUP_STRING);
      return 0;
    }
  }
}
