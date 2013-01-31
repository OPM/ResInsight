/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_wconinje.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>

#include <ert/util/stringlist.h>
#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/buffer.h>
#include <ert/util/int_vector.h>
#include <ert/util/double_vector.h>

#include <ert/sched/sched_kw_wconinje.h>
#include <ert/sched/sched_util.h>
#include <ert/sched/sched_types.h>


#define DEFAULT_INJECTOR_STATE OPEN

#define SCHED_KW_WCONINJE_ID  99165
#define WCONINJE_TYPE_ID      5705235
#define WCONINJE_NUM_KW 10
#define ECL_DEFAULT_KW "*"


struct sched_kw_wconinje_struct {
  int           __type_id;
  vector_type * wells;  /* A vector of wconinje_well_type instances. */
};



typedef struct {
  bool                       def[WCONINJE_NUM_KW];            /* Has the item been defaulted? */

  char                      * name;               /* This does NOT support well_name_root or well list notation. */
  sched_phase_enum            injector_type;      /* Injecting GAS/WATER/OIL */
  well_status_enum            status;             /* Well is open/shut/??? */
  well_cm_enum                cmode;              /* How is the well controlled? */
  double                      surface_flow;       
  double                      reservoir_flow;
  double                      BHP_target;
  double                      THP_target;
  int                         vfp_table_nr;
  double                      vapoil_conc;
} wconinje_well_type;
  




struct wconinje_state_struct {
  UTIL_TYPE_ID_DECLARATION;
  char *               well_name;
  const time_t_vector_type * time; 
  int_vector_type    * phase;                  /* Contains values from sched_phase_enum */
  int_vector_type    * state;                  /* Contains values from the well_status_enum. */ 
  int_vector_type    * cmode;                  /* Contains values from the well_cm_enum. */
  double_vector_type * surface_flow;
  double_vector_type * reservoir_flow;
  double_vector_type * bhp_limit;
  double_vector_type * thp_limit;
  int_vector_type    * vfp_table_nr;
  double_vector_type * vapoil;
};




/*****************************************************************/
/* Implemeentation of the internal wconinje_well_type data type. */










static wconinje_well_type * wconinje_well_alloc_empty()
{
  wconinje_well_type * well = util_malloc(sizeof * well);
  well->name = NULL;
  return well;
}



static void wconinje_well_free(wconinje_well_type * well)
{
  free(well->name);
  free(well);
}



static void wconinje_well_free__(void * well)
{
  wconinje_well_free( (wconinje_well_type *) well);
}





static wconinje_well_type * wconinje_well_alloc_from_tokens(const stringlist_type * line_tokens ) {
  wconinje_well_type * well = wconinje_well_alloc_empty();
  sched_util_init_default( line_tokens , well->def );

  well->name           = util_alloc_string_copy( stringlist_iget( line_tokens , 0 ));
  well->injector_type  = sched_phase_type_from_string(stringlist_iget(line_tokens , 1));
  well->cmode          = sched_types_get_cm_from_string( stringlist_iget( line_tokens , 3 ) , false);
  well->surface_flow   = sched_util_atof( stringlist_iget( line_tokens , 4 ));
  well->reservoir_flow = sched_util_atof(stringlist_iget(line_tokens , 5 ));
  well->BHP_target     = sched_util_atof(stringlist_iget(line_tokens , 6 ));
  well->THP_target     = sched_util_atof( stringlist_iget( line_tokens , 7 ));
  well->vfp_table_nr   = sched_util_atoi( stringlist_iget( line_tokens , 8));
  well->vapoil_conc    = sched_util_atof( stringlist_iget( line_tokens , 9 ));

  well->status         = sched_types_get_status_from_string( stringlist_iget( line_tokens , 2 ));
  if (well->status == DEFAULT)
    well->status = DEFAULT_INJECTOR_STATE;
  return well;
}



static void wconinje_well_fprintf(const wconinje_well_type * well, FILE * stream)
{
  fprintf(stream, "  ");
  sched_util_fprintf_qst(well->def[0],  well->name                                   , 8,     stream);
  sched_util_fprintf_qst(well->def[1],  sched_phase_type_string(well->injector_type) , 5,     stream); /* 5 ?? */
  sched_util_fprintf_qst(well->def[2],  sched_types_get_status_string(well->status)  , 4,     stream);
  sched_util_fprintf_qst(well->def[3],  sched_types_get_cm_string(well->cmode)       , 4,     stream);
  sched_util_fprintf_dbl(well->def[4],  well->surface_flow                           , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[5],  well->reservoir_flow                         , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[6],  well->BHP_target                             , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[7],  well->THP_target                             , 11, 3, stream);
  sched_util_fprintf_int(well->def[8],  well->vfp_table_nr                           , 4,     stream);
  sched_util_fprintf_dbl(well->def[9],  well->vapoil_conc                            , 11, 3, stream);
  fprintf(stream, "/ \n");
}


/*****************************************************************/




static sched_kw_wconinje_type * sched_kw_wconinje_alloc_empty() {
  sched_kw_wconinje_type * kw = util_malloc(sizeof * kw);
  kw->wells     = vector_alloc_new();
  kw->__type_id = SCHED_KW_WCONINJE_ID;
  return kw;
}

sched_kw_wconinje_type * sched_kw_wconinje_safe_cast( void * arg ) {
  sched_kw_wconinje_type * kw = (sched_kw_wconinje_type * ) arg;
  if (kw->__type_id == SCHED_KW_WCONINJE_ID)
    return kw;
  else {
    util_abort("%s: runtime cast failed \n",__func__);
    return NULL;
  }
}




void sched_kw_wconinje_free(sched_kw_wconinje_type * kw)
{
  vector_free( kw->wells );
  free(kw);
}


static void sched_kw_wconinje_add_well( sched_kw_wconinje_type * kw , const wconinje_well_type * well) {
  vector_append_owned_ref(kw->wells , well , wconinje_well_free__);  
}





sched_kw_wconinje_type * sched_kw_wconinje_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_wconinje_type * kw = sched_kw_wconinje_alloc_empty();
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , WCONINJE_NUM_KW , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      wconinje_well_type * well = wconinje_well_alloc_from_tokens( line_tokens );
      sched_kw_wconinje_add_well( kw , well );
      stringlist_free( line_tokens );
    } 
  } while (!eokw);
  return kw;  
}



void sched_kw_wconinje_fprintf(const sched_kw_wconinje_type * kw , FILE * stream) {
  int size = vector_get_size(kw->wells);

  fprintf(stream, "WCONINJE\n");
  for(int i=0; i<size; i++)
  {
    const wconinje_well_type * well = vector_iget_const(kw->wells, i);
    wconinje_well_fprintf(well, stream);
  }
  fprintf(stream,"/\n\n");
}



char ** sched_kw_wconinje_alloc_wells_copy( const sched_kw_wconinje_type * kw , int * num_wells) {
  int size = vector_get_size(kw->wells);
  
  char ** well_names = util_malloc( size * sizeof * well_names );
  for(int i=0; i<size; i++)
  {
    const wconinje_well_type * well = vector_iget_const(kw->wells, i);
    well_names[i] = util_alloc_string_copy(well->name);
  }
  *num_wells = size;
  return well_names;
}




/*****************************************************************/
/* Functions exporting content to be used with the sched_file_update
   api.  */

/** Will return NULL if the well is not present. */
static wconinje_well_type * sched_kw_wconinje_get_well( const sched_kw_wconinje_type * kw , const char * well_name) {
  int size = vector_get_size(kw->wells);
  wconinje_well_type * well = NULL;
  int index = 0;
  do {
    wconinje_well_type * iwell = vector_iget( kw->wells , index);
    if (strcmp( well_name , iwell->name ) == 0) 
      well = iwell;
    
    index++;
  } while ((well == NULL) && (index < size));
  return well;
}



double sched_kw_wconinje_get_surface_flow( const sched_kw_wconinje_type * kw , const char * well_name) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well != NULL)
    return well->surface_flow;
  else
    return -1;
}

void sched_kw_wconinje_scale_surface_flow( const sched_kw_wconinje_type * kw , const char * well_name, double factor) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well != NULL)
    well->surface_flow *= factor;
}

void sched_kw_wconinje_set_surface_flow( const sched_kw_wconinje_type * kw , const char * well_name , double surface_flow) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well != NULL)
    well->surface_flow = surface_flow;
}



void sched_kw_wconinje_shift_surface_flow( const sched_kw_wconinje_type * kw , const char * well_name , double delta_surface_flow) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well != NULL)
    well->surface_flow += delta_surface_flow;
}


sched_phase_enum sched_kw_wconinje_get_phase( const sched_kw_wconinje_type * kw , const char * well_name) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well != NULL)
    return well->injector_type;
  else
    return -1;
}



bool sched_kw_wconinje_has_well( const sched_kw_wconinje_type * kw , const char * well_name) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well == NULL)
    return false;
  else
    return true;
}


sched_kw_wconinje_type * sched_kw_wconinje_copyc(const sched_kw_wconinje_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}


bool sched_kw_wconinje_well_open( const sched_kw_wconinje_type * kw, const char * well_name) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well == NULL)
    return false;
  else {
    /* OK - we have the well. */

    if (well->status == OPEN) {
      /* The well seems to be open - any rates around? */
      if (well->surface_flow > 0)
        return true;
      else
        return false;
    } else
      return false;  }
}

/*****************************************************************/


/*****************************************************************/

wconinje_state_type * wconinje_state_alloc( const char * well_name , const time_t_vector_type * time) {
  wconinje_state_type * wconinje = util_malloc( sizeof * wconinje);
  UTIL_TYPE_ID_INIT( wconinje , WCONINJE_TYPE_ID );

  wconinje->phase          = int_vector_alloc( 0 , 0 );
  wconinje->state          = int_vector_alloc( 0 , 0 );  /* Default wconinje state ? */
  wconinje->cmode          = int_vector_alloc( 0 , 0 );  /* Default control mode ?? */
  wconinje->surface_flow   = double_vector_alloc( 0 , 0 );
  wconinje->reservoir_flow = double_vector_alloc( 0 , 0 );
  wconinje->bhp_limit      = double_vector_alloc( 0 , 0 );
  wconinje->thp_limit      = double_vector_alloc( 0 , 0 );
  wconinje->vfp_table_nr   = int_vector_alloc( 0 , 0 );
  wconinje->vapoil         = double_vector_alloc( 0 ,0 );
  wconinje->time           = time;
  wconinje->well_name      = util_alloc_string_copy( well_name );

  return wconinje;
}


static UTIL_SAFE_CAST_FUNCTION( wconinje_state , WCONINJE_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION_CONST( wconinje_state , WCONINJE_TYPE_ID )

void wconinje_state_free( wconinje_state_type * wconinje ) {

  int_vector_free(wconinje->phase);
  int_vector_free(wconinje->state);
  int_vector_free(wconinje->cmode);
  double_vector_free(wconinje->surface_flow);
  double_vector_free(wconinje->reservoir_flow);
  double_vector_free(wconinje->bhp_limit);
  double_vector_free(wconinje->thp_limit);
  int_vector_free(wconinje->vfp_table_nr);
  double_vector_free(wconinje->vapoil);
  free( wconinje->well_name );
  free( wconinje );

}

void wconinje_state_free__( void * arg ) {
  wconinje_state_free( wconinje_state_safe_cast( arg ));
}



/**
   This function asks for the historical water injection rate; however
   this is the WCONINJE keyword, and it is NOT necessarily meaningful
   to query this keyword for that rate. To be meaningfull we check the
   following conditions:

     1. We verify that the well is rate-controlled; then the behaviour
        of the well should(??) coincide with that of wells specifed by
        the WCONINJH keyword.

     2. We verify that the injected phase is indeed water.

   If these conditions are not met 0 is returned, AND a warning is
   written to stderr.
*/

double wconinje_state_iget_WWIRH( const void * __state , int report_step ) {
  const wconinje_state_type * state = wconinje_state_safe_cast_const( __state );
  sched_phase_enum phase = int_vector_safe_iget( state->phase , report_step );
  well_cm_enum cmode     = int_vector_safe_iget( state->cmode , report_step);

  if (( phase == WATER) && (cmode == RATE))
    return double_vector_safe_iget( state->surface_flow , report_step);
  else {
    if ( phase != WATER ) 
      fprintf(stderr,"** Warning you have asked for historical water injection rate in well:%s which is not a water injector.\n", state->well_name);
    
    if ( cmode != RATE ) 
      fprintf(stderr,"** Warning you have asked for historical water injection rate in well:%s which is not rate controlled - I have no clue?! \n" , state->well_name);
    
    return 0;
  }
}

/**
   See comment above wconinje_state_get_WWIRH();
*/
double wconinje_state_iget_WGIRH( const void * __state , int report_step ) {
  const wconinje_state_type * state = wconinje_state_safe_cast_const( __state );
  sched_phase_enum phase = int_vector_safe_iget( state->phase , report_step );
  well_cm_enum cmode     = int_vector_safe_iget( state->cmode , report_step);

  if (( phase == GAS) && (cmode == RATE))
    return double_vector_safe_iget( state->surface_flow , report_step);
  else {
    if ( phase != GAS ) 
      fprintf(stderr,"** Warning you have asked for historical gas injection rate in well:%s(%d) which is not a gas injector.\n", state->well_name, report_step);
    
    if ( cmode != RATE ) 
      fprintf(stderr,"** Warning you have asked for historical gas injection rate in well:%s(%d) which is not rate controlled - I have no clue?! \n" , state->well_name, report_step);
    
    return 0;
  }
}

/**
   Will update the input parameter @well_list to contain all the
   well_names present in the current sced_kw_wconhist keyword.
*/
   
void sched_kw_wconinje_init_well_list( const sched_kw_wconinje_type * kw , stringlist_type * well_list) {
  stringlist_clear( well_list );
  {
    int iw;
    for (iw = 0; iw < vector_get_size( kw->wells ); iw++) {
      const wconinje_well_type * well = vector_iget_const( kw->wells , iw );
      stringlist_append_ref( well_list , well->name );
    }
  }
}


/**
   For production the WCONHIST keyword will (typically) be used for
   the historical period, and WCONPROD for the predicton. This can be
   used to differentiate between hisorical period and prediction
   period. When it comes to injection things are not so clear;
   typically the WCONINJE keyword is used both for prediction and
   historical period.

   This function will check if all the wells (at least one) in the
   WCONINJE keyword are rate-controlled, if so it is interpreted as
   beeing in the historical period.
*/

bool sched_kw_wconinje_historical( const sched_kw_wconinje_type * kw ) {
  bool historical = false;
  int iw;
  for (iw = 0; iw < vector_get_size( kw->wells ); iw++) {
    const wconinje_well_type * well = vector_iget_const( kw->wells , iw );
    if (well->cmode == RATE) {
      historical = true;
      break;
    }
  }
  return historical;
}





void sched_kw_wconinje_update_state( const sched_kw_wconinje_type * kw , wconinje_state_type * state , const char * well_name , int report_step ) {
  wconinje_well_type * well = sched_kw_wconinje_get_well( kw , well_name );
  if (well != NULL) {
    int_vector_iset_default(state->phase             , report_step , well->injector_type );  
    int_vector_iset_default(state->state             , report_step , well->status);          
    int_vector_iset_default(state->cmode             , report_step , well->cmode);           
    double_vector_iset_default(state->surface_flow   , report_step , well->surface_flow);    
    double_vector_iset_default(state->reservoir_flow , report_step , well->reservoir_flow);  
    double_vector_iset_default(state->bhp_limit      , report_step , well->BHP_target);      
    double_vector_iset_default(state->thp_limit      , report_step , well->THP_target);      
    int_vector_iset_default(state->vfp_table_nr      , report_step , well->vfp_table_nr);    
    double_vector_iset_default(state->vapoil         , report_step , well->vapoil_conc);     
  }
}


void sched_kw_wconinje_close_state(wconinje_state_type * state , int report_step ) {
  fprintf(stderr,"** Warning: %s not implemented \n",__func__);
  //int_vector_iset_default( state->state            , report_step ,  SHUT    );  /* SHUT or STOP ?? */
  //double_vector_iset_default(state->injection_rate , report_step , -1 );
  //double_vector_iset_default(state->bhp            , report_step , -1 );
  //double_vector_iset_default(state->thp            , report_step , -1 );
  //int_vector_iset_default(state->vfp_table_nr      , report_step , -1 );
  //double_vector_iset_default(state->vapoil         , report_step , -1 );
}





KW_IMPL(wconinje)
