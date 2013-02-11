/*
   Copyright (C) 2011  Statoil ASA, Norway. 
   The file 'history.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdio.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/bool_vector.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_util.h>

#include <ert/sched/gruptree.h>
#include <ert/sched/sched_history.h>
#include <ert/sched/history.h>


struct history_struct{
  const ecl_sum_type    * refcase;        /* ecl_sum instance used when the data are taken from a summary instance. Observe that this is NOT owned by history instance.*/
  const sched_file_type * sched_file;     /* Not owned. */
  sched_history_type    * sched_history;
  history_source_type     source;
};


history_source_type history_get_source_type( const char * string_source ) {
  history_source_type source_type = HISTORY_SOURCE_INVALID;

  if (strcmp( string_source , "REFCASE_SIMULATED") == 0)
    source_type = REFCASE_SIMULATED;
  else if (strcmp( string_source , "REFCASE_HISTORY") == 0)
    source_type = REFCASE_HISTORY;
  else if (strcmp( string_source , "SCHEDULE") == 0)
    source_type = SCHEDULE;
  else
    util_abort("%s: Sorry source:%s not recognized\n",__func__ , string_source);

  return source_type;
}


const char * history_get_source_string( history_source_type history_source ) {
  switch( history_source ) {
  case( REFCASE_SIMULATED ):
    return "REFCASE_SIMULATED";
    break;
  case(REFCASE_HISTORY ):
    return "REFCASE_HISTORY";
    break;
  case(SCHEDULE ):
    return "SCHEDULE";
    break;
  default: 
    util_abort("%s: internal fuck up \n",__func__);
    return NULL;
  }
}






static history_type * history_alloc_empty(  )
{
  history_type * history = util_malloc(sizeof * history);
  history->refcase       = NULL; 
  history->sched_history = NULL;
  history->sched_file    = NULL;
  return history;
}



/******************************************************************/
// Exported functions for manipulating history_type. Acess functions further below.


void history_free(history_type * history)
{
  if (history->sched_history != NULL)
    sched_history_free( history->sched_history );
  
  free(history);
}


history_type * history_alloc_from_sched_file(const char * sep_string , const sched_file_type * sched_file)
{
  history_type * history = history_alloc_empty( );
  history->sched_file    = sched_file;
  history->sched_history = sched_history_alloc( sep_string );
  sched_history_update( history->sched_history , sched_file );
  history->source = SCHEDULE;
  
  return history;
}


history_type * history_alloc_from_refcase(const ecl_sum_type * refcase , bool use_h_keywords) {
  history_type * history = history_alloc_empty( true );

  history->refcase = refcase;     /* This function does not really do anthing - it just sets the ecl_sum field of the history instance. */
  if (use_h_keywords)
    history->source = REFCASE_HISTORY;
  else
    history->source = REFCASE_SIMULATED;
  
  return history;
}





/******************************************************************/
// Exported functions for accessing history_type.




int history_get_last_restart(const history_type * history)
{
  if (history->refcase != NULL)
    return ecl_sum_get_last_report_step( history->refcase);
  else
    return sched_history_get_last_history( history->sched_history );
}








bool history_init_ts( const history_type * history , const char * summary_key , double_vector_type * value, bool_vector_type * valid) {
  bool initOK = false;

  double_vector_reset( value );
  bool_vector_reset( valid );
  bool_vector_set_default( valid , false);

  if (history->source == SCHEDULE) {

    for (int tstep = 0; tstep <= sched_history_get_last_history(history->sched_history); tstep++) {
      if (sched_history_open( history->sched_history , summary_key , tstep)) {
        bool_vector_iset( valid , tstep , true );
        double_vector_iset( value , tstep , sched_history_iget( history->sched_history , summary_key , tstep));
      } else
        bool_vector_iset( valid , tstep , false );
    }
    initOK = true;

  } else {

    char * local_key;
    if (history->source == REFCASE_HISTORY) {
      /* Must create a new key with 'H' for historical values. */
      const ecl_smspec_type * smspec      = ecl_sum_get_smspec( history->refcase );
      const char            * join_string = ecl_smspec_get_join_string( smspec ); 
        
      local_key = util_alloc_sprintf( "%sH%s%s" , ecl_sum_get_keyword( history->refcase , summary_key ) , join_string , ecl_sum_get_wgname( history->refcase , summary_key ));
    } else
      local_key = (char *) summary_key;

    if (ecl_sum_has_general_var( history->refcase , local_key )) {
      for (int tstep = 0; tstep <= history_get_last_restart(history); tstep++) {
        int time_index = ecl_sum_iget_report_end( history->refcase , tstep );
        if (time_index >= 0) {
          double_vector_iset( value , tstep , ecl_sum_get_general_var( history->refcase , time_index , local_key ));
          bool_vector_iset( valid , tstep , true );
        } else
          bool_vector_iset( valid , tstep , false );    /* Did not have this report step */
      }
      initOK = true;
    }
    
    if (history->source == REFCASE_HISTORY) 
      free( local_key );
  }
  return initOK;
}


time_t history_get_start_time( const history_type * history ) {
  if (history->source == SCHEDULE)
    return sched_history_iget_time_t( history->sched_history , 0);
  else
    return ecl_sum_get_start_time( history->refcase );
}



/* Uncertain about the first node - offset problems +++ ?? 
   Changed to use node_end_time() at svn ~ 2850

   Changed to sched_history at svn ~2940
*/
time_t history_get_time_t_from_restart_nr( const history_type * history , int restart_nr) {
  if (history->source == SCHEDULE)
    return sched_history_iget_time_t( history->sched_history , restart_nr);
  else {
    if (restart_nr == 0)
      return ecl_sum_get_start_time( history->refcase );
    else
      return ecl_sum_get_report_time( history->refcase , restart_nr );
  }
}


int history_get_restart_nr_from_time_t( const history_type * history , time_t time) {
  if (time == history_get_start_time( history ))
    return 0;
  else {
    if (history->source == SCHEDULE)
      return sched_file_get_restart_nr_from_time_t( history->sched_file , time );
    else {
      int report_step = ecl_sum_get_report_step_from_time( history->refcase , time );
      if (report_step >= 1)
        return report_step;
      else {
        int mday,year,month;
        util_set_date_values( time , &mday , &month , &year);
        util_abort("%s: Date: %02d/%02d/%04d  does not cooincide with any report time. Aborting.\n", __func__ , mday , month , year);
        return -1;
      }
    }
  }
}


int history_get_restart_nr_from_days( const history_type * history , double sim_days) {
  if (history->source == SCHEDULE)
    return sched_file_get_restart_nr_from_days( history->sched_file , sim_days);
  else {
    int report_step = ecl_sum_get_report_step_from_days( history->refcase , sim_days);
    if (report_step >= 1)
      return report_step;
    else {
      util_abort("%s: Days:%g  does not cooincide with any report time. Aborting.\n", __func__ , sim_days);
      return -1;
    }
  }
}



