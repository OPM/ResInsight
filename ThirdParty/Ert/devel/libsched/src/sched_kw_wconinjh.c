/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_wconinjh.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/int_vector.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/double_vector.h>

#include <ert/sched/sched_kw_wconinjh.h>
#include <ert/sched/sched_util.h>
#include <ert/sched/sched_types.h>

#define WCONINJH_TYPE_ID 88163977
#define WCONINJH_NUM_KW  8


struct sched_kw_wconhist_struct{
  vector_type * wells;
};


typedef struct wconinjh_well_struct   wconinjh_well_type;



struct wconinjh_well_struct{
  /*
    def: Read as defaulted, not defined!
  */
  bool               def[WCONINJH_NUM_KW];

  char             * name;
  sched_phase_enum   inj_phase;
  well_status_enum   status;
  double             inj_rate;
  double             bhp;
  double             thp;
  int                vfptable;
  double             vapdiscon;
};


struct wconinjh_state_struct {
  UTIL_TYPE_ID_DECLARATION;
  const time_t_vector_type * time;
  int_vector_type          * phase;                  /* Contains values from sched_phase_enum */
  int_vector_type          * state;                  /* Contains values from the well_status_enum. */ 
  double_vector_type       * injection_rate;
  double_vector_type       * bhp;
  double_vector_type       * thp;
  int_vector_type          * vfp_table_nr;
  double_vector_type       * vapoil;
};



static wconinjh_well_type * wconinjh_well_alloc_empty()
{
  wconinjh_well_type * well = util_malloc(sizeof * well);
  well->name = NULL;
  return well;
}



static void wconinjh_well_free(wconinjh_well_type * well)
{
  free(well->name);
  free(well);
}



static void wconinjh_well_free__(void * well)
{
  wconinjh_well_free( (wconinjh_well_type *) well);
}



/** Will return NULL if the well is not present. */
static wconinjh_well_type * sched_kw_wconinjh_get_well( const sched_kw_wconinjh_type * kw , const char * well_name) {
  int size = vector_get_size(kw->wells);
  wconinjh_well_type * well = NULL;
  int index = 0;
  do {
    wconinjh_well_type * iwell = vector_iget( kw->wells , index);
    if (strcmp( well_name , iwell->name ) == 0) 
      well = iwell;
    
    index++;
  } while ((well == NULL) && (index < size));
  return well;
}


static void wconinjh_well_fprintf(const wconinjh_well_type * well, FILE * stream)
{
  fprintf(stream, "  ");
  sched_util_fprintf_qst(well->def[0], well->name                                   , 8      , stream);
  sched_util_fprintf_qst(well->def[1], sched_phase_type_string(well->inj_phase)     , 5      , stream);
  sched_util_fprintf_qst(well->def[2], sched_types_get_status_string(well->status)  , 4      , stream);
  sched_util_fprintf_dbl(well->def[3], well->inj_rate                               , 9 , 3  , stream);
  sched_util_fprintf_dbl(well->def[4], well->bhp                                    , 9 , 3  , stream);
  sched_util_fprintf_dbl(well->def[5], well->thp                                    , 9 , 3  , stream);
  sched_util_fprintf_int(well->def[6], well->vfptable                               , 4      , stream);
  sched_util_fprintf_dbl(well->def[7], well->vapdiscon                              , 9 , 3  , stream);
  fprintf(stream, "/\n");
}









static wconinjh_well_type * wconinjh_well_alloc_from_tokens(const stringlist_type * line_tokens ) {

  wconinjh_well_type * well = wconinjh_well_alloc_empty();
  sched_util_init_default( line_tokens , well->def );
  
  well->name      = util_alloc_string_copy(stringlist_iget(line_tokens , 0));
  well->inj_phase = sched_phase_type_from_string(stringlist_iget(line_tokens , 1));
  well->status    = sched_types_get_status_from_string(stringlist_iget(line_tokens , 2));
  well->inj_rate  = sched_util_atof(stringlist_iget(line_tokens , 3));
  well->bhp       = sched_util_atof(stringlist_iget(line_tokens , 4));
  well->thp       = sched_util_atof(stringlist_iget(line_tokens , 5));
  well->vfptable  = sched_util_atoi(stringlist_iget(line_tokens , 6));
  well->vapdiscon = sched_util_atof(stringlist_iget(line_tokens , 7));
  
  return well;
}



static hash_type * wconinjh_well_export_obs_hash(const wconinjh_well_type * well) {
  hash_type * obs_hash = hash_alloc();

  if(!well->def[3])
  {
    switch(well->inj_phase)
    {
      case(WATER):
        hash_insert_double(obs_hash, "WWIR", well->inj_rate);
        break;
      case(GAS):
        hash_insert_double(obs_hash, "WGIR", well->inj_rate);
        break;
      case(OIL):
        hash_insert_double(obs_hash, "WOIR", well->inj_rate);
        break;
      default:
        break;
    }
  }
  if(!well->def[4])
    hash_insert_double(obs_hash, "WBHP", well->bhp);
  if(!well->def[5])
    hash_insert_double(obs_hash, "WTHP", well->thp);

  return obs_hash;
}


static void sched_kw_wconinjh_add_well( sched_kw_wconinjh_type * kw , wconinjh_well_type * well) {
  vector_append_owned_ref(kw->wells , well , wconinjh_well_free__);
}



static sched_kw_wconinjh_type * sched_kw_wconinjh_alloc_empty()
{
  sched_kw_wconinjh_type * kw = util_malloc(sizeof * kw);
  kw->wells = vector_alloc_new();
  return kw;
}



/***********************************************************************/


sched_kw_wconinjh_type * sched_kw_wconinjh_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_wconinjh_type * kw = sched_kw_wconinjh_alloc_empty();
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false, WCONINJH_NUM_KW , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      wconinjh_well_type * well = wconinjh_well_alloc_from_tokens( line_tokens );
      sched_kw_wconinjh_add_well( kw , well );
      stringlist_free( line_tokens );
    } 
  } while (!eokw);
  return kw;  
}


void sched_kw_wconinjh_free(sched_kw_wconinjh_type * kw)
{
  vector_free(kw->wells);
  free(kw);
}



void sched_kw_wconinjh_fprintf(const sched_kw_wconinjh_type * kw, FILE * stream)
{
  int size = vector_get_size(kw->wells);
    
  fprintf(stream, "WCONINJH\n");
  for(int i=0; i<size; i++)
  {
    const wconinjh_well_type * well = vector_iget_const( kw->wells, i );
    wconinjh_well_fprintf(well, stream);
  }
  fprintf(stream,"/\n\n");
}




/***********************************************************************/



hash_type * sched_kw_wconinjh_alloc_well_obs_hash(const sched_kw_wconinjh_type * kw)
{
  hash_type * well_hash = hash_alloc();

  int num_wells = vector_get_size(kw->wells);
  
  for(int well_nr=0; well_nr<num_wells; well_nr++)
  {
    const wconinjh_well_type * well = vector_iget_const(kw->wells, well_nr);
    hash_type * obs_hash = wconinjh_well_export_obs_hash(well);
    hash_insert_hash_owned_ref(well_hash, well->name, obs_hash, hash_free__);
  }

  return well_hash;
}

sched_kw_wconinjh_type * sched_kw_wconinjh_copyc(const sched_kw_wconinjh_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}


/*****************************************************************/

void sched_kw_wconinjh_init_well_list( const sched_kw_wconinjh_type * kw , stringlist_type * well_list) {
  stringlist_clear( well_list );
  {
    int iw;
    for (iw = 0; iw < stringlist_get_size( well_list ); iw++) {
      const wconinjh_well_type * well = vector_iget_const( kw->wells , iw );
      stringlist_append_ref( well_list , well->name );
    }
  }
}




/*****************************************************************/

wconinjh_state_type * wconinjh_state_alloc( const time_t_vector_type* time) {
  wconinjh_state_type * wconinjh = util_malloc( sizeof * wconinjh);
  UTIL_TYPE_ID_INIT( wconinjh , WCONINJH_TYPE_ID );

  wconinjh->time           = time;
  wconinjh->phase          = int_vector_alloc( 0 , 0 );
  wconinjh->state          = int_vector_alloc( 0 , 0 );  /* Default wconinjh state ? */
  wconinjh->injection_rate = double_vector_alloc( 0 , 0 );
  wconinjh->bhp            = double_vector_alloc( 0 , 0 );
  wconinjh->thp            = double_vector_alloc( 0 , 0 );
  wconinjh->vfp_table_nr   = int_vector_alloc( 0 , 0 );
  wconinjh->vapoil         = double_vector_alloc( 0 ,0 );

  return wconinjh;
}

UTIL_SAFE_CAST_FUNCTION( wconinjh_state , WCONINJH_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION_CONST( wconinjh_state , WCONINJH_TYPE_ID )

void wconinjh_state_free( wconinjh_state_type * wconinjh ) {
  
  int_vector_free( wconinjh->phase );
  int_vector_free( wconinjh->state );
  double_vector_free( wconinjh->injection_rate );
  double_vector_free( wconinjh->bhp );
  double_vector_free( wconinjh->thp );
  int_vector_free( wconinjh->vfp_table_nr );
  double_vector_free( wconinjh->vapoil );
  
  free( wconinjh );
}



void wconinjh_state_free__( void * arg ) {
  wconinjh_state_free( wconinjh_state_safe_cast( arg ));
}


/**
   memnonic ??
*/


sched_phase_enum wconinjh_state_iget_phase( const wconinjh_state_type * state , int report_step) {
  return int_vector_safe_iget( state->phase , report_step );
}


well_status_enum wconinjh_state_iget_status( const wconinjh_state_type * state , int report_step ) {
  return int_vector_safe_iget( state->state , report_step );
}

/**
   Water injection rate - will return 0.0 if inj_phase == GAS.
*/

double wconinjh_state_iget_WWIRH( const void * __state , int report_step) {
  const wconinjh_state_type * state = wconinjh_state_safe_cast_const( __state );
  if (wconinjh_state_iget_phase( state , report_step ) == WATER)
    return double_vector_safe_iget( state->injection_rate , report_step);
  else
    return 0.0;
}


/**
   Gas injection rate - will return 0.0 if inj_phase == WATER.
*/

double wconinjh_state_iget_WGIRH( const void * __state , int report_step) {
  const wconinjh_state_type * state = wconinjh_state_safe_cast_const( __state );
  if (wconinjh_state_iget_phase( state , report_step ) == GAS)
    return double_vector_safe_iget( state->injection_rate , report_step);
  else
    return 0.0;
}


/**
   OIL injection rate - will return 0.0 if inj_phase == WATER|GAS
*/

double wconinjh_state_iget_WOIRH( const void * __state , int report_step) {
  const wconinjh_state_type * state = wconinjh_state_safe_cast_const( __state );
  if (wconinjh_state_iget_phase( state , report_step ) == OIL)
    return double_vector_safe_iget( state->injection_rate , report_step);
  else
    return 0.0;
}



double wconinjh_state_iget_WBHPH( const void * __state , int report_step) {
  const wconinjh_state_type * state = wconinjh_state_safe_cast_const( __state );
  return double_vector_safe_iget( state->bhp , report_step );
}


double wconinjh_state_iget_WTHPH( const void * __state , int report_step) {
  const wconinjh_state_type * state = wconinjh_state_safe_cast_const( __state );
  return double_vector_safe_iget( state->thp , report_step );
}

/** Memnonic ??*/
double wconinjh_state_iget_WVPRH( const void * __state , int report_step) {
  const wconinjh_state_type * state = wconinjh_state_safe_cast_const( __state );
  return double_vector_safe_iget( state->vapoil , report_step );
}


int wconinjh_state_iget_vfp_table_nr( const wconinjh_state_type * state , int report_step) {
  return int_vector_safe_iget( state->vfp_table_nr , report_step );
}




void sched_kw_wconinjh_close_state(wconinjh_state_type * state , int report_step ) {
  int_vector_iset_default( state->state            , report_step ,  SHUT    );  /* SHUT or STOP ?? */
  double_vector_iset_default(state->injection_rate , report_step , -1 );
  double_vector_iset_default(state->bhp            , report_step , -1 );
  double_vector_iset_default(state->thp            , report_step , -1 );
  int_vector_iset_default(state->vfp_table_nr      , report_step , -1 );
  double_vector_iset_default(state->vapoil         , report_step , -1 );
}


void sched_kw_wconinjh_update_state( const sched_kw_wconinjh_type * kw , wconinjh_state_type * state , const char * well_name , int report_step ) {
  wconinjh_well_type * well = sched_kw_wconinjh_get_well( kw , well_name );
  if (well != NULL) {
    int_vector_iset_default(state->phase             , report_step , well->inj_phase );  
    int_vector_iset_default(state->state             , report_step , well->status );          
    double_vector_iset_default(state->injection_rate , report_step , well->inj_rate);    
    double_vector_iset_default(state->bhp            , report_step , well->bhp);      
    double_vector_iset_default(state->thp            , report_step , well->thp);      
    int_vector_iset_default(state->vfp_table_nr      , report_step , well->vfptable);    
    double_vector_iset_default(state->vapoil         , report_step , well->vapdiscon);     
  }
}




/***********************************************************************/
KW_IMPL(wconinjh)
