/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'member_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <ert/util/util.h>
#include <ert/util/path_fmt.h>
#include <ert/util/subst_list.h>

#include <ert/enkf/ecl_config.h>
#include <ert/enkf/member_config.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/ensemble_config.h>


/**
   This struct contains information which is private to this
   member. It is initialized at object boot time, and (typically) not
   changed during the simulation. [In principle it could change during
   the simulation, but the current API does not support that.]
*/ 


struct member_config_struct {
  int                     iens;                /* The ensemble member number of this member. */
  char                  * casename;            /* The name of this case - will mosttly be NULL. */
  keep_runpath_type       keep_runpath;        /* Should the run-path directory be left around (for this member)*/
  bool                    pre_clear_runpath;   /* Should the runpath directory be cleared before starting? */ 
  char                  * jobname;             /* The jobname used for this job when submitting to the queue system. */
  char                  * eclbase;             /* The ECLBASE string used for simulations of this member. */
  enkf_fs_type          * fs_cache;            /* UGly */ 
};


/*****************************************************************/
/*
  Observe that there is a potential for conflict between the fields
  pre_clear_runpath and keep_runpath when running normal EnKF. If both
  are set to true the former will win.
*/
  


/******************************************************************/
/** Implementation of the member_config struct. All of this implementation
    is private - however some of it is exported through the enkf_state object,
    and it should be perfectly safe to export more of it.
*/


const char * member_config_update_jobname(member_config_type * member_config , const char * jobname_fmt , const subst_list_type * subst_list) {
  if (jobname_fmt != NULL) {
    util_safe_free( member_config->jobname );
    {
      char * tmp = util_alloc_sprintf( jobname_fmt , member_config->iens);
      member_config->jobname = subst_list_alloc_filtered_string( subst_list , tmp );
      free( tmp );
    }
  }
  return member_config->jobname;
}



const char * member_config_update_eclbase(member_config_type * member_config , const ecl_config_type * ecl_config , const subst_list_type * subst_list) {
  util_safe_free( member_config->eclbase );
  {
    const path_fmt_type * eclbase_fmt = ecl_config_get_eclbase_fmt(ecl_config);
    if (eclbase_fmt != NULL) {
      {
        char * tmp = path_fmt_alloc_path(eclbase_fmt , false , member_config->iens);
        member_config->eclbase = subst_list_alloc_filtered_string( subst_list , tmp );
        free( tmp );
      }

      if (!ecl_util_valid_basename( member_config->eclbase )) 
        util_exit("Sorry - the basename:%s is invalid. ECLIPSE does not handle mIxeD cAsE :-( \n" , member_config->eclbase);
    }
  }
  
  return member_config->eclbase;
}


int member_config_get_iens( const member_config_type * member_config ) {
  return member_config->iens;
}

                                   

void member_config_free(member_config_type * member_config) {
  util_safe_free(member_config->eclbase);
  util_safe_free(member_config->casename );
  free(member_config);
}



void member_config_set_keep_runpath(member_config_type * member_config , keep_runpath_type keep_runpath) {
  member_config->keep_runpath   = keep_runpath;
}


keep_runpath_type member_config_get_keep_runpath(const member_config_type * member_config) {
  return member_config->keep_runpath;
}

bool member_config_pre_clear_runpath(const member_config_type * member_config) {
  return member_config->pre_clear_runpath;
}


void member_config_set_pre_clear_runpath(member_config_type * member_config , bool pre_clear_runpath) {
  member_config->pre_clear_runpath = pre_clear_runpath;
}





const char * member_config_get_eclbase( const member_config_type * member_config ) {
  return member_config->eclbase;
}


const char * member_config_get_jobname( const member_config_type * member_config ) {
  if (member_config->jobname != NULL)
    return member_config->jobname;
  else {
    if (member_config->eclbase != NULL)
      return member_config->eclbase;
    else {
      util_abort("%s: sorry can not submit JOB - must specify name with JOBNAME or ECLBASE config keys\n",__func__);
      return NULL;
    }
  }
}


const char * member_config_get_casename( const member_config_type * member_config ) {
  return member_config->casename;
}


member_config_type * member_config_alloc(int iens , 
                                         const char                 * casename , 
                                         bool                         pre_clear_runpath , 
                                         keep_runpath_type            keep_runpath , 
                                         const ecl_config_type      * ecl_config , 
                                         const ensemble_config_type * ensemble_config,
                                         enkf_fs_type * fs) {

                                                
  member_config_type * member_config = util_malloc( sizeof * member_config );
  member_config->casename            = util_alloc_string_copy( casename );
  member_config->iens                = iens; /* Can only be changed in the allocater. */
  member_config->eclbase             = NULL;
  member_config->jobname             = NULL;
  member_config->pre_clear_runpath   = pre_clear_runpath;
  member_config_set_keep_runpath(member_config , keep_runpath);
  return member_config;
}
