 /*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ecl_sum_tstep.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <time.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/ecl/ecl_sum_tstep.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_smspec.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_type.h>

#define ECL_SUM_TSTEP_ID 88631


/*
  This file implements the ecl_sum_tstep datatype which contains the
  summary information for all summary vectors at one instant in
  time. If we view the summary data as this:


Header direction: ecl_smspec   DAYS     WWCT:OP_3     FOPT     BPR:15,10,25
                               --------------------------------------------
  /|\                           0.00    0.00          0.00           256.00   <-- One timestep ecl_sum_tstep
   |                           10.00    0.56         10.00           255.00
 Time direction: ecl_sum_data  20.00    0.61         18.70           253.00
   |                           30.00    0.63         21.20           251.00
   |                           ...
  \|/                          90.00    0.80         39.70           244.00
                               --------------------------------------------

  The ecl_sum_tstep structure corresponds to one 'horizontal line' in
  the summary data.

  These timesteps correspond exactly to the simulators timesteps,
  i.e. when convergence is poor they are closely spaced. In the
  ECLIPSE summary files these time steps are called "MINISTEPS" - and
  that term is also used some places in the ecl_sum_xxx codebase.
 */


struct ecl_sum_tstep_struct {
  UTIL_TYPE_ID_DECLARATION;
  float                  * data;            /* A memcpy copy of the PARAMS vector in ecl_kw instance - the raw data. */
  time_t                   sim_time;        /* The true time (i.e. 20.th of october 2010) of corresponding to this timestep. */
  int                      ministep;        /* The ECLIPSE internal time-step number; one ministep per numerical timestep. */
  int                      report_step;     /* The report step this time-step is part of - in general there can be many timestep for each report step. */
  double                   sim_seconds;     /* Accumulated simulation time up to this ministep. */
  int                      data_size;       /* Number of elements in data - only used for checking indices. */
  int                      internal_index;  /* Used for lookups of the next / previous ministep based on an existing ministep. */
  const ecl_smspec_type  * smspec;          /* The smespec header information for this tstep - must be compatible. */
};


ecl_sum_tstep_type * ecl_sum_tstep_alloc_remap_copy( const ecl_sum_tstep_type * src , const ecl_smspec_type * new_smspec, float default_value , const int * params_map) {
  int params_size = ecl_smspec_get_params_size( new_smspec );
  ecl_sum_tstep_type * target = util_alloc_copy(src , sizeof * src );

  target->smspec = new_smspec;
  target->data = util_malloc( params_size * sizeof * target->data );
  target->data_size = params_size;
  for (int i=0; i < params_size; i++) {

    if (params_map[i] >= 0)
      target->data[i] = src->data[ params_map[i] ];
    else
      target->data[i] = default_value;

  }
  return target;
}

ecl_sum_tstep_type * ecl_sum_tstep_alloc_copy( const ecl_sum_tstep_type * src ) {
  ecl_sum_tstep_type * target = util_alloc_copy(src , sizeof * src );
  target->data = util_alloc_copy( src->data , src->data_size * sizeof * src->data );
  return target;
}


static ecl_sum_tstep_type * ecl_sum_tstep_alloc( int report_step , int ministep_nr , const ecl_smspec_type * smspec) {
  ecl_sum_tstep_type * tstep = util_malloc( sizeof * tstep );
  UTIL_TYPE_ID_INIT( tstep , ECL_SUM_TSTEP_ID);
  tstep->smspec      = smspec;
  tstep->report_step = report_step;
  tstep->ministep    = ministep_nr;
  tstep->data_size   = ecl_smspec_get_params_size( smspec );
  tstep->data        = util_calloc( tstep->data_size , sizeof * tstep->data );
  return tstep;
}


UTIL_SAFE_CAST_FUNCTION( ecl_sum_tstep , ECL_SUM_TSTEP_ID)
UTIL_SAFE_CAST_FUNCTION_CONST( ecl_sum_tstep , ECL_SUM_TSTEP_ID)


void ecl_sum_tstep_free( ecl_sum_tstep_type * ministep ) {
  free( ministep->data );
  free( ministep );
}



void ecl_sum_tstep_free__( void * __ministep) {
  ecl_sum_tstep_type * ministep = ecl_sum_tstep_safe_cast( __ministep );
  ecl_sum_tstep_free( ministep );
}



/**
   This function sets the internal time representation in the
   ecl_sum_tstep. The treatment of time is a bit weird; on the one
   hand the time elements in the summary data are just like any other
   element like e.g. the FOPT or GGPR:NAME - on the other hand the
   time information is strictly required and the summary file will
   fall to pieces if it is missing.

   The time can be provided in using (at least) two different
   keywords:

      DAYS/HOURS: The data vector will contain the number of
            days/hours since the simulation start (hours in the case
            of lab units).

      DAY,MONTH,YEAR: The data vector will contain the true date of
           the tstep.

   The ecl_sum_tstep class can utilize both types of information, but
   will select the DAYS variety if both are present.
*/

static void ecl_sum_tstep_set_time_info_from_seconds( ecl_sum_tstep_type * tstep , time_t sim_start , float sim_seconds) {
  tstep->sim_seconds = sim_seconds;
  tstep->sim_time = sim_start;
  util_inplace_forward_seconds_utc( &tstep->sim_time , tstep->sim_seconds );
}


static void ecl_sum_tstep_set_time_info_from_date( ecl_sum_tstep_type * tstep , time_t sim_start , time_t sim_time) {
  tstep->sim_time = sim_time;
  tstep->sim_seconds = util_difftime_seconds( sim_start , tstep->sim_time);
}


static void ecl_sum_tstep_set_time_info( ecl_sum_tstep_type * tstep , const ecl_smspec_type * smspec ) {
  int date_day_index   = ecl_smspec_get_date_day_index( smspec );
  int date_month_index = ecl_smspec_get_date_month_index( smspec );
  int date_year_index  = ecl_smspec_get_date_year_index( smspec );
  int sim_time_index   = ecl_smspec_get_time_index( smspec );
  time_t sim_start     = ecl_smspec_get_start_time( smspec );

  if (sim_time_index >= 0) {
    float sim_time = tstep->data[ sim_time_index ];
    double sim_seconds = sim_time * ecl_smspec_get_time_seconds( smspec );
    ecl_sum_tstep_set_time_info_from_seconds( tstep , sim_start , sim_seconds );
  } else if ( date_day_index >= 0) {
    int day   = util_roundf(tstep->data[date_day_index]);
    int month = util_roundf(tstep->data[date_month_index]);
    int year  = util_roundf(tstep->data[date_year_index]);

    time_t sim_time = ecl_util_make_date(day , month , year);
    ecl_sum_tstep_set_time_info_from_date( tstep , sim_start , sim_time );
  } else
    util_abort("%s: Hmmm - could not extract date/time information from SMSPEC header file? \n",__func__);

}



/**
   If the ecl_kw instance is in some way invalid (i.e. wrong size);
   the function will return NULL:
*/


ecl_sum_tstep_type * ecl_sum_tstep_alloc_from_file( int report_step    ,
                                                    int ministep_nr    ,
                                                    const ecl_kw_type * params_kw ,
                                                    const char * src_file ,
                                                    const ecl_smspec_type * smspec) {

  int data_size = ecl_kw_get_size( params_kw );

  if (data_size == ecl_smspec_get_params_size( smspec )) {
    ecl_sum_tstep_type * ministep = ecl_sum_tstep_alloc( report_step , ministep_nr , smspec);
    ecl_kw_get_memcpy_data( params_kw , ministep->data );
    ecl_sum_tstep_set_time_info( ministep , smspec );
    return ministep;
  } else {
    /*
       This is actually a fatal error / bug; the difference in smspec
       header structure should have been detected already in the
       ecl_smspec_load_restart() function and the restart case
       discarded.
    */
    fprintf(stderr , "** Warning size mismatch between timestep loaded from:%s and header:%s - timestep discarded.\n" , src_file , ecl_smspec_get_header_file( smspec ));
    return NULL;
  }
}


/*
  Should be called in write mode.
*/

ecl_sum_tstep_type * ecl_sum_tstep_alloc_new( int report_step , int ministep , float sim_seconds , const ecl_smspec_type * smspec ) {
  ecl_sum_tstep_type * tstep = ecl_sum_tstep_alloc( report_step , ministep , smspec );
  const float_vector_type * default_data = ecl_smspec_get_params_default( smspec );
  float_vector_memcpy_data( tstep->data , default_data );

  ecl_sum_tstep_set_time_info_from_seconds( tstep , ecl_smspec_get_start_time( smspec ) , sim_seconds );
  ecl_sum_tstep_iset( tstep , ecl_smspec_get_time_index( smspec ) , sim_seconds / ecl_smspec_get_time_seconds( smspec ) );
  return tstep;
}




double ecl_sum_tstep_iget(const ecl_sum_tstep_type * ministep , int index) {
  if ((index >= 0) && (index < ministep->data_size))
    return ministep->data[index];
  else {
    util_abort("%s: param index:%d invalid: Valid range: [0,%d) \n",__func__ , index , ministep->data_size);
    return -1;
  }
}


time_t ecl_sum_tstep_get_sim_time(const ecl_sum_tstep_type * ministep) {
  return ministep->sim_time;
}


double ecl_sum_tstep_get_sim_days(const ecl_sum_tstep_type * ministep) {
  return ministep->sim_seconds / (24 * 3600);
}

double ecl_sum_tstep_get_sim_seconds(const ecl_sum_tstep_type * ministep) {
  return ministep->sim_seconds;
}

int ecl_sum_tstep_get_report(const ecl_sum_tstep_type * ministep) {
  return ministep->report_step;
}


int ecl_sum_tstep_get_ministep(const ecl_sum_tstep_type * ministep) {
  return ministep->ministep;
}


/*****************************************************************/

void ecl_sum_tstep_fwrite( const ecl_sum_tstep_type * ministep , const int_vector_type * index_map , fortio_type * fortio) {
  {
    ecl_kw_type * ministep_kw = ecl_kw_alloc( MINISTEP_KW , 1 , ECL_INT );
    ecl_kw_iset_int( ministep_kw , 0 , ministep->ministep );
    ecl_kw_fwrite( ministep_kw , fortio );
    ecl_kw_free( ministep_kw );
  }

  {
    int compact_size = int_vector_size( index_map );
    ecl_kw_type * params_kw = ecl_kw_alloc( PARAMS_KW , compact_size , ECL_FLOAT );

    const int * index = int_vector_get_ptr( index_map );
    float * data      = ecl_kw_get_ptr( params_kw );

    {
      int i;
      for (i=0; i < compact_size; i++)
        data[i] = ministep->data[ index[i] ];
    }
    ecl_kw_fwrite( params_kw , fortio );
    ecl_kw_free( params_kw );
  }
}


/*****************************************************************/

void ecl_sum_tstep_iset( ecl_sum_tstep_type * tstep , int index , float value) {
  if ((index < tstep->data_size) && (index >= 0))
    tstep->data[index] = value;
  else
    util_abort("%s: index:%d invalid. Valid range: [0,%d) \n",__func__  ,index , tstep->data_size);
}

void ecl_sum_tstep_iscale(ecl_sum_tstep_type * tstep, int index, float scalar) {
  ecl_sum_tstep_iset(tstep, index, ecl_sum_tstep_iget(tstep, index) * scalar);
}

void ecl_sum_tstep_ishift(ecl_sum_tstep_type * tstep, int index, float addend) {
  ecl_sum_tstep_iset(tstep, index, ecl_sum_tstep_iget(tstep, index) + addend);
}

void ecl_sum_tstep_set_from_node( ecl_sum_tstep_type * tstep , const smspec_node_type * smspec_node , float value) {
  int data_index = smspec_node_get_params_index( smspec_node );
  ecl_sum_tstep_iset( tstep , data_index , value);
}

double ecl_sum_tstep_get_from_node( const ecl_sum_tstep_type * tstep , const smspec_node_type * smspec_node) {
  int data_index = smspec_node_get_params_index( smspec_node );
  return ecl_sum_tstep_iget( tstep , data_index);
}


void ecl_sum_tstep_set_from_key( ecl_sum_tstep_type * tstep , const char * gen_key , float value) {
  const smspec_node_type * smspec_node = ecl_smspec_get_general_var_node( tstep->smspec , gen_key );
  ecl_sum_tstep_set_from_node( tstep , smspec_node , value);
}

double ecl_sum_tstep_get_from_key(const ecl_sum_tstep_type * tstep , const char * gen_key) {
  const smspec_node_type * smspec_node = ecl_smspec_get_general_var_node( tstep->smspec , gen_key );
  return ecl_sum_tstep_get_from_node(tstep , smspec_node );
}

bool ecl_sum_tstep_has_key(const ecl_sum_tstep_type * tstep , const char * gen_key) {
    return ecl_smspec_has_general_var(tstep->smspec, gen_key);
}


bool ecl_sum_tstep_sim_time_equal( const ecl_sum_tstep_type * tstep1 , const ecl_sum_tstep_type * tstep2 ) {
  if (tstep1->sim_time == tstep2->sim_time)
    return true;
  else
    return false;
}

