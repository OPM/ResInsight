/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'forward_load_context.c.h' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/type_macros.h>
#include <ert/util/stringlist.h>

#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/forward_load_context.h>
#include <ert/enkf/run_arg.h>
#include <ert/enkf/ecl_config.h>


#define FORWARD_LOAD_CONTEXT_TYPE_ID 644239127

struct forward_load_context_struct {
  UTIL_TYPE_ID_DECLARATION;
  // Everyuthing can be NULL here ... - when created from gen_data.

  ecl_sum_type        * ecl_sum;
  ecl_file_type       * restart_file;
  const run_arg_type  * run_arg;
  char                * eclbase;
  const ecl_config_type * ecl_config;   // Can be NULL

  int step1;
  int step2;
  stringlist_type * messages;          // This is managed by external scope - can be NULL


  /* The variables below are updated during the load process. */
  int load_step;
  int load_result;
};

UTIL_IS_INSTANCE_FUNCTION( forward_load_context , FORWARD_LOAD_CONTEXT_TYPE_ID)



static void forward_load_context_load_ecl_sum(forward_load_context_type * load_context) {
  ecl_sum_type * summary                 = NULL;

  if (ecl_config_active( load_context->ecl_config )) {
    const run_arg_type * run_arg           = forward_load_context_get_run_arg(load_context);
    const char * run_path                  = run_arg_get_runpath( run_arg );
    const char * eclbase                   = load_context->eclbase;

    const bool fmt_file                    = ecl_config_get_formatted(load_context->ecl_config);
    char * header_file                     = ecl_util_alloc_exfilename(run_path , eclbase , ECL_SUMMARY_HEADER_FILE , fmt_file , -1);
    char * unified_file                    = ecl_util_alloc_exfilename(run_path , eclbase , ECL_UNIFIED_SUMMARY_FILE , fmt_file ,  -1);
    stringlist_type * data_files           = stringlist_alloc_new();

    /* Should we load from a unified summary file, or from several non-unified files? */
    if (unified_file != NULL)
      /* Use unified file: */
      stringlist_append_ref( data_files , unified_file);
    else {
      /* Use several non unified files. */
      /* Bypassing the query to model_config_load_results() */
      int report_step = run_arg_get_load_start( run_arg );
      if (report_step == 0)
        report_step++;     // Ignore looking for the .S0000 summary file (it does not exist).
      while (true) {
        char * summary_file = ecl_util_alloc_exfilename(run_arg_get_runpath( run_arg ) , eclbase , ECL_SUMMARY_FILE , fmt_file ,  report_step);

        if (summary_file != NULL)
          stringlist_append_owned_ref( data_files , summary_file);
        else
          /*
             We stop the loading at first 'hole' in the series of summary files;
             the internalize layer must report failure if we are missing data.
          */
          break;

        report_step++;
      }
    }

    if ((header_file != NULL) && (stringlist_get_size(data_files) > 0)) {
      summary = ecl_sum_fread_alloc(header_file , data_files , SUMMARY_KEY_JOIN_STRING );
      {
        time_t end_time = ecl_config_get_end_date( load_context->ecl_config );
        if (end_time > 0) {
          if (ecl_sum_get_end_time( summary ) < end_time) {
            /* The summary vector was shorter than expected; we interpret this as
               a simulation failure and discard the current summary instance. */

            if (forward_load_context_accept_messages(load_context)) {
              int end_day,end_month,end_year;
              int sum_day,sum_month,sum_year;

              util_set_date_values_utc( end_time , &end_day , &end_month , &end_year );
              util_set_date_values_utc( ecl_sum_get_end_time( summary ) , &sum_day , &sum_month , &sum_year );
              {
                char * msg = util_alloc_sprintf("Summary ended at %02d/%02d/%4d - expected at least END_DATE: %02d/%02d/%4d" ,
                                                sum_day , sum_month , sum_year ,
                                                end_day , end_month , end_year );
                forward_load_context_add_message( load_context , msg );
                free( msg );
              }
            }

          }
          ecl_sum_free( summary );
          summary = NULL;
        }
      }
    }
    stringlist_free( data_files );
    util_safe_free( header_file );
    util_safe_free( unified_file );
  }

  if (summary)
    load_context->ecl_sum = summary;
  else
    forward_load_context_update_result(load_context, LOAD_FAILURE);
}




forward_load_context_type * forward_load_context_alloc( const run_arg_type * run_arg , bool load_summary , const ecl_config_type * ecl_config , const char * eclbase , stringlist_type * messages) {
  forward_load_context_type * load_context = util_malloc( sizeof * load_context );
  UTIL_TYPE_ID_INIT( load_context , FORWARD_LOAD_CONTEXT_TYPE_ID );

  load_context->ecl_sum = NULL;
  load_context->restart_file = NULL;
  load_context->run_arg = run_arg;
  load_context->load_step = -1;  // Invalid - must call forward_load_context_select_step()
  load_context->load_result = 0;
  load_context->messages = messages;
  load_context->ecl_config = ecl_config;
  load_context->eclbase = util_alloc_string_copy( eclbase );

  if (load_summary)
    forward_load_context_load_ecl_sum(load_context);

  return load_context;
}



bool forward_load_context_accept_messages( const forward_load_context_type * load_context ) {
  if (load_context->messages)
    return true;
  else
    return false;
}


/*
  The messages can be NULL; in which case the message is completely ignored.
*/

void forward_load_context_add_message( forward_load_context_type * load_context , const char * message ) {
  if (load_context->messages)
    stringlist_append_copy( load_context->messages , message );
}


int forward_load_context_get_result( const forward_load_context_type * load_context ) {
  return load_context->load_result;
}

void forward_load_context_update_result( forward_load_context_type * load_context , int flags) {
  load_context->load_result |= flags;
}


void forward_load_context_free( forward_load_context_type * load_context ) {
  if (load_context->restart_file)
    ecl_file_close( load_context->restart_file );

  if (load_context->ecl_sum)
    ecl_sum_free( load_context->ecl_sum );

  util_safe_free( load_context->eclbase );
  free( load_context );
}

bool forward_load_context_load_restart_file( forward_load_context_type * load_context, int report_step) {
  if (load_context->ecl_config) {
    const bool unified = ecl_config_get_unified_restart( load_context->ecl_config );
    if (unified)
      util_abort("%s: sorry - unified restart files are not supported \n",__func__);

    forward_load_context_select_step(load_context, report_step);
    {
      const bool fmt_file  = ecl_config_get_formatted( load_context->ecl_config );
      char * filename      = ecl_util_alloc_exfilename( run_arg_get_runpath(load_context->run_arg) ,
                                                        load_context->eclbase,
                                                        ECL_RESTART_FILE ,
                                                        fmt_file ,
                                                        load_context->load_step );

      if (load_context->restart_file)
        ecl_file_close( load_context->restart_file );
      load_context->restart_file = NULL;

      if (filename) {
        load_context->restart_file = ecl_file_open( filename , 0 );
        free(filename);
      }

      if (load_context->restart_file)
        return true;
      else
        return false;
    }
  } else {
    util_abort("%s: internal error - tried to load restart with load_context with ecl_config==NULL \n",__func__);
    return false;
  }
}




const ecl_sum_type * forward_load_context_get_ecl_sum( const forward_load_context_type * load_context) {
  return load_context->ecl_sum;
}

const ecl_file_type * forward_load_context_get_restart_file( const forward_load_context_type * load_context) {
  return load_context->restart_file;
}

const run_arg_type * forward_load_context_get_run_arg( const forward_load_context_type * load_context ) {
  return load_context->run_arg;
}

const char * forward_load_context_get_run_path( const forward_load_context_type * load_context ) {
  return run_arg_get_runpath( load_context->run_arg );
}


enkf_fs_type * forward_load_context_get_result_fs( const forward_load_context_type * load_context ) {
  return run_arg_get_result_fs( load_context->run_arg );
}


void forward_load_context_select_step( forward_load_context_type * load_context , int report_step) {
  load_context->load_step = report_step;
}

int forward_load_context_get_load_step(const forward_load_context_type * load_context) {
  if (load_context->load_step < 0)
    util_abort("%s: this looks like an internal error - missing call to forward_load_context_select_step() \n",__func__);

  return load_context->load_step;
}




