/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ext_joblist.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/subst_list.h>

#include <ert/job_queue/ext_job.h>
#include <ert/job_queue/ext_joblist.h>


//#define MODULE_NAME    "jobs.py"
//#define JOBLIST_NAME   "jobList"


/**
   About the 'license' system:
   ---------------------------

   There is a simple possibility to limit the number of jobs which are
   running in parallell. It works like this:

    1. For the joblist as a whole a license_path is created. This
       license path should contain both a uid and pid of the current
       process. This ensures that:

       a. The license count is per user and per ert instance.
       b. Each ert instance starts with a fresh license count. A
          license path, and license files left dangling after unclean
          shutdown can just be removed.

    2. For each job in the joblist a subdirectory is created under the
       license_path.

    3. For each job a license_file is created, and for each time a new
       instance is checked out a hard_link to this license_file is
       created - i.e. the number of checked out licenses is a
       hard_link count (-1).

       Step three here is implemented by the job_dispatch script
       actually running the jobs.

   It is essential that the license_root_path is on a volume which is
   accessible from all the nodes which will run jobs. Using e.g. /tmp
   as license_root_path will fail HARD.

*/



/*****************************************************************/

struct ext_joblist_struct {
  hash_type * jobs;
};




ext_joblist_type * ext_joblist_alloc( ) {
  ext_joblist_type * joblist = util_malloc( sizeof * joblist );
  joblist->jobs = hash_alloc();
  return joblist;
}


void ext_joblist_free(ext_joblist_type * joblist) {
  hash_free(joblist->jobs);
  free(joblist);
}


void ext_joblist_add_job(ext_joblist_type * joblist , const char * name , ext_job_type * new_job) {
  hash_insert_hash_owned_ref(joblist->jobs , name , new_job , ext_job_free__);
}


ext_job_type * ext_joblist_get_job(const ext_joblist_type * joblist , const char * job_name) {
  if (hash_has_key(joblist->jobs , job_name))
    return hash_get(joblist->jobs , job_name);
  else {
    util_abort("%s: asked for job:%s which does not exist\n",__func__ , job_name);
    return NULL;
  }
}


ext_job_type * ext_joblist_get_job_copy(const ext_joblist_type * joblist , const char * job_name) {
  if (hash_has_key(joblist->jobs , job_name))
    return ext_job_alloc_copy(hash_get(joblist->jobs , job_name));
  else {
    util_abort("%s: asked for job:%s which does not exist\n",__func__ , job_name);
    return NULL;
  }
}


bool ext_joblist_has_job(const ext_joblist_type * joblist , const char * job_name) {
  return hash_has_key(joblist->jobs , job_name);
}


stringlist_type * ext_joblist_alloc_list( const ext_joblist_type * joblist) {
  return hash_alloc_stringlist( joblist->jobs );
}


/**
   Will attempt to remove the job @job_name from the joblist; if the
   job is marked as a shared_job (i.e. installed centrally) the user
   is not allowed to delete it. In this case the function will fail
   silently.

   Returns true if the job is actually removed, and false otherwise.
*/

bool ext_joblist_del_job( ext_joblist_type * joblist , const char * job_name ) {
  ext_job_type * job = ext_joblist_get_job( joblist , job_name );
  if (!ext_job_is_shared( job )) {
    hash_del( joblist->jobs , job_name );
    return true;
  } else
    return false;
}


hash_type * ext_joblist_get_jobs( const ext_joblist_type * joblist ) {
  return joblist->jobs;
}

void ext_joblist_add_jobs_in_directory(ext_joblist_type * joblist  , const char * path, const char * license_root_path, bool user_mode, bool search_path ) {
  DIR * dirH = opendir( path );
  if (dirH) {
    while (true) {
      struct dirent * entry = readdir( dirH );
      if (entry != NULL) {
        if ((strcmp(entry->d_name , ".") != 0) && (strcmp(entry->d_name , "..") != 0)) {
          char * full_path = util_alloc_filename( path , entry->d_name , NULL );
          if (util_is_file( full_path )) {
              ext_job_type * new_job = ext_job_fscanf_alloc(entry->d_name, license_root_path, user_mode, full_path, search_path);
              if (new_job != NULL) {
                ext_joblist_add_job(joblist, entry->d_name, new_job);
              }
              else{
                  fprintf(stderr," Failed to add forward model job: %s \n",full_path);
              }
          }
          free( full_path );
        }
      } else
        break;
    }
    closedir( dirH );
  } else
    fprintf(stderr, "** Warning: failed to open jobs directory: %s\n", path);
}


int ext_joblist_get_size( const ext_joblist_type * joblist ) {
  return hash_get_size( joblist->jobs );
}

