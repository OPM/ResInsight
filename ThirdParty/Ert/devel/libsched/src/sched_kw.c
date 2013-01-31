/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/sched/sched_kw.h>
#include <ert/sched/sched_util.h>
#include <ert/sched/sched_kw_gruptree.h>
#include <ert/sched/sched_kw_tstep.h>
#include <ert/sched/sched_kw_dates.h>
#include <ert/sched/sched_kw_wconhist.h>
#include <ert/sched/sched_kw_wconinjh.h>
#include <ert/sched/sched_kw_welspecs.h>
#include <ert/sched/sched_kw_wconprod.h>
#include <ert/sched/sched_kw_wconinj.h>
#include <ert/sched/sched_kw_wconinje.h>
#include <ert/sched/sched_kw_compdat.h>
#include <ert/sched/sched_kw_untyped.h>
#include <ert/sched/sched_kw_include.h>
#include <ert/sched/sched_macros.h>


/*
  The structure sched_kw_type is used for internalization
  of arbitrary keywords in an ECLIPSE schedule file/section.

  Two structs are defined in this file:

    1. The sched_kw_type, which can be accessed externaly
       through various interface functions.
    2. The data_handlers_type, which provides an abstraction
       for the data_handling of the various keywords. This
       is for internal use only.

  Keywords from the ECLIPSE schedule are divided into three
  different groups:

    1. Fully internalized keywords, e.g. GRUPTREE.
       Functions implementing the data_handlers and
       more for these keywords are found in separate
       files, e.g. sched_kw_gruptree.c.

    2. Keywords which are known to have a fixed number
       of records, but not having a full internal
       representation. The number of records for these
       keywords are specified in the function
       get_fixed_record_length  in the file
       sched_kw_untyped.c

    3. Keywords which are not implemented and have a
       variable record length. These are handled 
       automatically by sched_kw_untyped.c.

*/

typedef void * (data_token_alloc_proto)  ( const stringlist_type * , int * );
typedef void   (data_free_proto)         ( void *);
typedef void   (data_fprintf_proto)      ( const void *, FILE *);
typedef void * (alloc_copy_proto)        ( const void *);


struct data_handlers_struct {
  data_token_alloc_proto  * token_alloc;
  data_free_proto         * free;
  data_fprintf_proto      * fprintf;
  alloc_copy_proto        * copyc;
};


struct sched_kw_struct {
  char                    * kw_name; 
  sched_kw_type_enum        type;
  int                       restart_nr;  /* The block nr owning this instance. */
  

  /* Function pointers to work on the data pointer. */
  data_token_alloc_proto  * alloc;
  data_free_proto         * free;
  data_fprintf_proto      * fprintf;
  alloc_copy_proto        * copyc;
  
  void                    * data;        /* A void point pointer to a detailed implementation - i.e. sched_kw_wconhist. */
};









/*****************************************************************/

/**
   Nothing like a little manual inheritance....
*/

static sched_kw_type * sched_kw_alloc_empty( const char * kw_name ) {
  sched_kw_type * kw = util_malloc(sizeof * kw);
  kw->kw_name = util_alloc_string_copy( kw_name );
  kw->type    = sched_kw_type_from_string( kw_name );
  
  switch( kw->type ) {
  case(WCONHIST):
    kw->alloc   = sched_kw_wconhist_alloc__;
    kw->free    = sched_kw_wconhist_free__;
    kw->fprintf = sched_kw_wconhist_fprintf__;
    kw->copyc   = sched_kw_wconhist_copyc__;
    break;
  case(DATES):
    kw->alloc   = sched_kw_dates_alloc__;
    kw->free    = sched_kw_dates_free__;
    kw->fprintf = sched_kw_dates_fprintf__;
    kw->copyc   = sched_kw_dates_copyc__;
    break;
  case(TSTEP):
    kw->alloc   = sched_kw_tstep_alloc__;
    kw->free    = sched_kw_tstep_free__;
    kw->fprintf = sched_kw_tstep_fprintf__;
    kw->copyc   = sched_kw_tstep_copyc__;
    break;
  case(COMPDAT):
    kw->alloc   = sched_kw_compdat_alloc__;
    kw->free    = sched_kw_compdat_free__;
    kw->fprintf = sched_kw_compdat_fprintf__;
    kw->copyc   = sched_kw_compdat_copyc__;
    break;
  case(WELSPECS):
    kw->alloc   = sched_kw_welspecs_alloc__;
    kw->free    = sched_kw_welspecs_free__;
    kw->fprintf = sched_kw_welspecs_fprintf__;
    kw->copyc   = sched_kw_welspecs_copyc__;
    break;
  case(GRUPTREE):
    kw->alloc   = sched_kw_gruptree_alloc__;
    kw->free    = sched_kw_gruptree_free__;
    kw->fprintf = sched_kw_gruptree_fprintf__;
    kw->copyc   = sched_kw_gruptree_copyc__;
    break;
  case(INCLUDE):
    kw->alloc   = sched_kw_include_alloc__;
    kw->free    = sched_kw_include_free__;
    kw->fprintf = sched_kw_include_fprintf__;
    kw->copyc   = sched_kw_include_copyc__;
    break;
  case(UNTYPED):
    /** 
        Observe that the untyped keyword uses a custom allocator
        function, because it needs to get the keyword length as extra
        input. 
    */
    kw->alloc   = NULL;    
    kw->free    = sched_kw_untyped_free__;
    kw->fprintf = sched_kw_untyped_fprintf__;
    kw->copyc   = sched_kw_untyped_copyc__;
    break;
  case(WCONINJ):
    kw->alloc   = sched_kw_wconinj_alloc__;
    kw->free    = sched_kw_wconinj_free__;
    kw->fprintf = sched_kw_wconinj_fprintf__;
    kw->copyc   = sched_kw_wconinj_copyc__;
    break;
  case(WCONINJE):
    kw->alloc   = sched_kw_wconinje_alloc__;
    kw->free    = sched_kw_wconinje_free__;
    kw->fprintf = sched_kw_wconinje_fprintf__;
    kw->copyc   = sched_kw_wconinje_copyc__;
    break;
  case(WCONINJH):
    kw->alloc   = sched_kw_wconinjh_alloc__;
    kw->free    = sched_kw_wconinjh_free__;
    kw->fprintf = sched_kw_wconinjh_fprintf__;
    kw->copyc   = sched_kw_wconinjh_copyc__;
    break;
  case(WCONPROD):
    kw->alloc   = sched_kw_wconprod_alloc__;
    kw->free    = sched_kw_wconprod_free__;
    kw->fprintf = sched_kw_wconprod_fprintf__;
    kw->copyc   = sched_kw_wconprod_copyc__;
    break;
  default:
    util_abort("%s: unrecognized type:%d \n",__func__ , kw->type );
  }
  return kw;
}




/*
  This tries to check if kw_name is a valid keyword in an ECLIPSE
  schedule file. It is essentially based on checking that there are
  not more argeuments on the line:

   OK:
   -------------------
   RPTSCHED
    arg1 arg2 arg2 ....

   
   Invalid:
   ------------------- 
   RPTSCHED arg1 arg2 arg3 ...

   Quite naive .... 
*/
static void sched_kw_name_assert(const char * kw_name , FILE * stream)
{
  if(kw_name == NULL) {
    fprintf(stderr,"** Parsing SCHEDULE file line-nr: %d \n",util_get_current_linenr(stream));
    util_abort("%s: Internal error - trying to dereference NULL pointer.\n",__func__);
  }
  
  {
    bool valid_kw = true;
    for (int i = 0; i < strlen(kw_name); i++)
      if (isspace(kw_name[i])) 
        valid_kw = false;

    if (!valid_kw) {
      if (stream != NULL)
        fprintf(stderr,"** Parsing SCHEDULE file line-nr: %d \n",util_get_current_linenr(stream));
      util_abort("%s: \"%s\" is not a valid schedule kw - aborting.\n",__func__ , kw_name);
    }
  }
}




static sched_kw_type ** sched_kw_tstep_split_alloc(const sched_kw_type * sched_kw, int * num_steps)
{
  *num_steps = sched_kw_tstep_get_size(sched_kw->data);
  sched_kw_type ** sched_kw_tsteps = util_malloc(*num_steps * sizeof * sched_kw_tsteps);
  
  for(int i=0; i<*num_steps; i++) {
    sched_kw_tsteps[i] = sched_kw_alloc_empty( "TSTEP" );
    double step = sched_kw_tstep_iget_step((const sched_kw_tstep_type *) sched_kw->data, i);
    sched_kw_tsteps[i]->data = sched_kw_tstep_alloc_from_double(step);
  }
  
  return sched_kw_tsteps;
}



static sched_kw_type ** sched_kw_dates_split_alloc(const sched_kw_type * sched_kw, int * num_steps) 
{
  *num_steps = sched_kw_dates_get_size(sched_kw->data);
  sched_kw_type ** sched_kw_dates = util_malloc(*num_steps * sizeof * sched_kw_dates);
  
  for(int i=0; i<*num_steps; i++) {
    sched_kw_dates[i] = sched_kw_alloc_empty( "DATES" );
    time_t date = sched_kw_dates_iget_date((const sched_kw_dates_type *) sched_kw->data, i);
    sched_kw_dates[i]->data = sched_kw_dates_alloc_from_time_t(date);
  }
  return sched_kw_dates;
}
/*****************************************************************/





const char * sched_kw_get_type_name( const sched_kw_type * sched_kw ) {
  return sched_kw_type_name( sched_kw->type );
}


sched_kw_type_enum sched_kw_get_type(const sched_kw_type * sched_kw)
{
  return sched_kw->type;  
}


static void sched_kw_alloc_data( sched_kw_type * kw , const stringlist_type * token_list , int * token_index , hash_type * fixed_length_table) {
  if (kw->type == UNTYPED) {
    int rec_len = -1;
    if (hash_has_key( fixed_length_table , kw->kw_name ))
      rec_len = hash_get_int( fixed_length_table , kw->kw_name );
    kw->data = sched_kw_untyped_alloc( token_list , token_index , rec_len );
  } else
    kw->data = kw->alloc( token_list , token_index );
}


sched_kw_type * sched_kw_token_alloc(const stringlist_type * token_list, int * token_index, hash_type * fixed_length_table, bool * foundEND) {
  if (*token_index >= stringlist_get_size( token_list ))
    return NULL;
  else {
    const char * kw_name  = stringlist_iget( token_list , *token_index );
    (*token_index) += 1;
    sched_kw_name_assert(kw_name , NULL);
    if (strcmp(kw_name,"END") == 0) {
      if (foundEND != NULL)
        *foundEND = true;
      
      return NULL;
    } else {
      sched_kw_type * sched_kw = sched_kw_alloc_empty( kw_name );
      sched_kw->restart_nr     = -1;
      
      sched_util_skip_newline( token_list , token_index );
      sched_kw_alloc_data( sched_kw , token_list , token_index , fixed_length_table);
      
      return sched_kw;
    }
  }
}


const char * sched_kw_get_name( const sched_kw_type * kw) { return kw->kw_name; }



void sched_kw_set_restart_nr( sched_kw_type * kw , int restart_nr) {
  kw->restart_nr = restart_nr;
}



void sched_kw_free(sched_kw_type * sched_kw)
{
  sched_kw->free(sched_kw->data);
  free(sched_kw->kw_name);
  free(sched_kw);
}



void sched_kw_free__(void * sched_kw_void)
{
  sched_kw_type * sched_kw = (sched_kw_type *) sched_kw_void;
  sched_kw_free(sched_kw);
}



/*
  This will print the kw in ECLIPSE style formating.
*/
void sched_kw_fprintf(const sched_kw_type * sched_kw, FILE * stream)
{
  sched_kw->fprintf(sched_kw->data, stream);
}







/*
  This function takes a kw related to timing, such as DATES or TSTEP
  and converts it into a series of kw's with one timing event in each
  kw. Note that TIME (ECL300 only) is not supported.
*/
sched_kw_type ** sched_kw_split_alloc_DATES(const sched_kw_type * sched_kw,  int * num_steps)
{
  switch(sched_kw_get_type(sched_kw))
  {
  case(TSTEP):
    return sched_kw_tstep_split_alloc(sched_kw, num_steps);
    break;
  case(DATES):
    return sched_kw_dates_split_alloc(sched_kw, num_steps);
    break;
  case(TIME):
    util_abort("%s: Sorry - no support for TIME kw yet. Please use TSTEP.\n", __func__);
    return NULL;
    break;
  default:
    util_abort("%s: Internal error - aborting.\n", __func__);
    return NULL;
  }
}



time_t sched_kw_get_new_time(const sched_kw_type * sched_kw, time_t curr_time)
{
  time_t new_time = -1;
  switch(sched_kw_get_type(sched_kw))
  {
    case(TSTEP):
      new_time = sched_kw_tstep_get_new_time((const sched_kw_tstep_type *) sched_kw->data, curr_time);
      break;
    case(DATES):
      new_time = sched_kw_dates_iget_date((const sched_kw_dates_type *) sched_kw->data , 0);
      break;
    case(TIME):
      util_abort("%s: Sorry - no support for TIME kw. Please use TSTEP.\n", __func__);
      break;
    default:
      util_abort("%s: Internal error - trying to get time from non-timing kw - aborting.\n", __func__);
      break;
  }

  return new_time;
}



char ** sched_kw_alloc_well_list(const sched_kw_type * sched_kw, int * num_wells)
{
  switch(sched_kw_get_type(sched_kw))
  {
  case(WCONPROD):
    return sched_kw_wconprod_alloc_wells_copy( (const sched_kw_wconprod_type *) sched_kw->data , num_wells);
    break;
  case(WCONINJE):
    return sched_kw_wconinje_alloc_wells_copy( (const sched_kw_wconinje_type *) sched_kw->data , num_wells);
    break;
  case(WCONINJ):
    return sched_kw_wconinj_alloc_wells_copy( (const sched_kw_wconinj_type *) sched_kw->data , num_wells);
    break;
  case(WCONHIST):
    {
      hash_type * well_obs = sched_kw_wconhist_alloc_well_obs_hash( (sched_kw_wconhist_type *) sched_kw->data);
      *num_wells = hash_get_size(well_obs);
      char ** well_list = hash_alloc_keylist(well_obs);
      hash_free(well_obs);
      return well_list;
    }
    break;
  case(WCONINJH):
    {
      hash_type * well_obs = sched_kw_wconinjh_alloc_well_obs_hash( (sched_kw_wconinjh_type *) sched_kw->data);
      *num_wells = hash_get_size(well_obs);
      char ** well_list = hash_alloc_keylist(well_obs);
      hash_free(well_obs);
      return well_list;
    }
    break;
  default:
       util_abort("%s: Internal error - trying to get well list from non-well kw - aborting.\n", __func__);
       return NULL;
  }
}



hash_type * sched_kw_alloc_well_obs_hash(const sched_kw_type * sched_kw)
{
  switch(sched_kw_get_type(sched_kw))
  {
    case(WCONHIST):
    {
      return sched_kw_wconhist_alloc_well_obs_hash( (sched_kw_wconhist_type *) sched_kw->data);
    }
    case(WCONINJH):
    {
      return sched_kw_wconinjh_alloc_well_obs_hash( (sched_kw_wconinjh_type *) sched_kw->data);
    }
    default:
    {
       util_abort("%s: Internal error - trying to get well observations from non-history kw - aborting.\n", __func__);
       return NULL;
    }
  }
}



void sched_kw_alloc_child_parent_list(const sched_kw_type * sched_kw, char *** children, char *** parents, int * num_pairs)
{
  switch(sched_kw_get_type(sched_kw))
  {
    case(GRUPTREE):
    {
      sched_kw_gruptree_alloc_child_parent_list((sched_kw_gruptree_type *) sched_kw->data, children, parents, num_pairs);
      break;
    }
    case(WELSPECS):
    {
      sched_kw_welspecs_alloc_child_parent_list((sched_kw_welspecs_type *) sched_kw->data, children, parents, num_pairs);
      break;
    }
    default:
    {
       util_abort("%s: Internal error - trying to get GRUPTREE from non-gruptre kw - aborting.\n", __func__);
    }
  }
}


/** 
    Only WCONHIST 
*/

bool sched_kw_has_well( const sched_kw_type * sched_kw , const char * well ) {
  sched_kw_type_enum type = sched_kw_get_type( sched_kw );
  if (type == WCONHIST)
    return sched_kw_wconhist_has_well( sched_kw->data , well);
  else if (type == WCONINJE)
    return sched_kw_wconinje_has_well( sched_kw->data , well);
  else
    return false;
}



/** 
    Only WCONHIST & WCONINJE
*/

bool sched_kw_well_open( const sched_kw_type * sched_kw , const char * well ) {
  sched_kw_type_enum type = sched_kw_get_type( sched_kw );
  if (type == WCONHIST)
    return sched_kw_wconhist_well_open( sched_kw->data , well);
  else if (type == WCONINJE)
    return sched_kw_wconinje_well_open( sched_kw->data , well);
  else
    return false;
}



sched_kw_type * sched_kw_alloc_copy(const sched_kw_type * src) {
  sched_kw_type * target = NULL;
  
  return target;
}


/*
  Returns an untyped poiniter to the spesific implementation. Used by
  the sched_file_update system. A bit careful with this one...
*/

void * sched_kw_get_data( sched_kw_type * kw) {
  return kw->data;
}

const void * sched_kw_get_const_data( const sched_kw_type * kw) {
  return kw->data;
}


