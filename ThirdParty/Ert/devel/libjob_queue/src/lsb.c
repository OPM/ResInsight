/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lsb.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
  This file implements a very small wrapper structure around the
  lsb_xxxx() functions from the libbat.so shared library which are
  used to submit, monitor and control simulations with LSF. 

  Loading and initializing the lsf libraries is quite painful, in an
  attempt to reduce unecessary dependencies the lsf libraries are
  loaded with dlopen() in the lsb_alloc() function below. This means
  that the libjob_queue.so shared library can be loaded without access
  to the lsf libraries.
*/

#include <stdlib.h>
#include <dlfcn.h>

#include <lsf/lsbatch.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/job_queue/lsb.h>



typedef int                 (lsb_submit_ftype)       ( struct submit * , struct submitReply *);
typedef int                 (lsb_openjobinfo_ftype)  (int , char * , char * , char * , char * , int);
typedef struct jobInfoEnt * (lsb_readjobinfo_ftype)  (int * );
typedef int                 (lsb_closejobinfo_ftype) ( );
typedef int                 (lsb_forcekilljob_ftype) ( int );
typedef int                 (lsb_init_ftype)         ( char * );
typedef char *              (lsb_sysmsg_ftype)       ( );



struct lsb_struct {
  lsb_submit_ftype       * submit;
  lsb_openjobinfo_ftype  * open_job;
  lsb_readjobinfo_ftype  * read_job;
  lsb_closejobinfo_ftype * close_job;
  lsb_forcekilljob_ftype * kill_job;
  lsb_init_ftype         * lsb_init;
  lsb_sysmsg_ftype       * sys_msg;

  stringlist_type        * error_list;
  void                   * lib_handle;
  bool                     ready;
};



static void * lsb_dlsym( lsb_type * lsb , const char * function_name ) {
  void * function = dlsym( lsb->lib_handle , function_name );
  if (!function) {
    lsb->ready = false;
    stringlist_append_owned_ref( lsb->error_list , util_alloc_sprintf( "Failed to locate symbol:%s  dlerror:%s" , function_name , dlerror()));
  }
  
  return function;
}


void * lsb_dlopen( lsb_type * lsb , const char * lib_name) {
  void * lib_handle = dlopen( lib_name , RTLD_NOW | RTLD_GLOBAL);
  if (!lib_handle) {
    lsb->ready = false;
    stringlist_append_owned_ref( lsb->error_list , util_alloc_sprintf("dlopen(%s) - failed:%s \n" , lib_name , dlerror()));
  }
  return lib_handle;
}

/*
  The following environment variables must be set (at some stage) before
  LSF will work properly:

    LSF_BINDIR      $LSF_HOME/bin
    LSF_LIBDIR      $LSF_HOME/lib
    XLSF_UIDDIR     $LSF_HOME/lib/uid
    LSF_SERVERDIR   $LSF_HOME/etc
    LSF_ENVDIR      /prog/LSF/conf

  The runtime linker must locate the libnsl, libbat and liblsf
  libraries using whatever method it usually does. If the loading
  fails the lsb object will get the ->ready flag set to false, and the
  lsf_driver will discard the lsb instance (and hopefully use shell
  commands to perform job management).
*/

lsb_type * lsb_alloc() {
  lsb_type * lsb = util_malloc( sizeof * lsb );
  lsb->ready = true;
  lsb->error_list = stringlist_alloc_new();

  lsb_dlopen(lsb , "libnsl.so" );
  lsb_dlopen(lsb , "liblsf.so" );
  lsb->lib_handle = lsb_dlopen(lsb , "libbat.so");
  
  if (lsb->lib_handle) {
    lsb->submit    = (lsb_submit_ftype *) lsb_dlsym( lsb , "lsb_submit");
    lsb->open_job  = (lsb_openjobinfo_ftype *) lsb_dlsym( lsb , "lsb_openjobinfo");
    lsb->read_job  = (lsb_readjobinfo_ftype *) lsb_dlsym( lsb , "lsb_readjobinfo");
    lsb->close_job = (lsb_closejobinfo_ftype *) lsb_dlsym( lsb , "lsb_closejobinfo");
    lsb->kill_job  = (lsb_forcekilljob_ftype *) lsb_dlsym( lsb , "lsb_forcekilljob");
    lsb->lsb_init  = (lsb_init_ftype *) lsb_dlsym( lsb , "lsb_init");
    lsb->sys_msg   = (lsb_sysmsg_ftype *) lsb_dlsym( lsb , "lsb_sysmsg");
  } 

  return lsb;
}



void lsb_free( lsb_type * lsb) {
  stringlist_free( lsb->error_list );
  if (lsb->lib_handle)
    dlclose( lsb->lib_handle );
  free( lsb );
}



bool lsb_ready( const lsb_type * lsb) {
  return lsb->ready;
}

stringlist_type * lsb_get_error_list( const lsb_type * lsb ) {
  return lsb->error_list;
}


/*****************************************************************/

int lsb_initialize( const lsb_type * lsb) {
  /*
    The environment variable LSF_ENVDIR must be set to point the
    directory containing LSF configuration information, the whole
    thing will crash and burn if this is not properly set.
  */
  if ( lsb->lsb_init(NULL) != 0 ) {
    fprintf(stderr,"LSF_ENVDIR: ");
    if (getenv("LSF_ENVDIR") != NULL)
      fprintf(stderr,"%s\n", getenv("LSF_ENVDIR"));
    else
      fprintf(stderr, "not set\n");
    
    util_abort("%s failed to initialize LSF environment : %s  \n",__func__ , lsb->sys_msg() );
  }
  return 0;
}


int lsb_submitjob( const lsb_type * lsb ,  struct submit * submit_data, struct submitReply * reply_data) {
  return lsb->submit( submit_data , reply_data );
}


int lsb_killjob( const lsb_type * lsb , int lsf_jobnr) {
  return lsb->kill_job(lsf_jobnr);
}


int lsb_openjob( const lsb_type * lsb , int lsf_jobnr) {
  return lsb->open_job(lsf_jobnr , NULL , NULL , NULL , NULL , ALL_JOB);
}


int lsb_closejob( const lsb_type * lsb) {
  return lsb->close_job();
}

char * lsb_sys_msg( const lsb_type * lsb) {
  return lsb->sys_msg();
}


struct jobInfoEnt * lsb_readjob( const lsb_type * lsb ) {
  struct jobInfoEnt * job_info = lsb->read_job( NULL );
  return job_info;
}
