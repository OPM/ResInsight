/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_wconhist.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/buffer.h>
#include <ert/util/int_vector.h>
#include <ert/util/double_vector.h>

#include <ert/sched/sched_types.h>
#include <ert/sched/sched_kw_wconhist.h>
#include <ert/sched/sched_util.h>


/*
  Define the maximum number of keywords in a WCONHIST record.
  Note that this includes wet gas rate, which is only supported
  by ECL 300.
*/

#define WCONHIST_NUM_KW       11
#define SCHED_KW_WCONHIST_ID  771054    /* Very random intgere for checking type-cast. */
#define WCONHIST_TYPE_ID      7752053


typedef struct wconhist_well_struct wconhist_well_type;



struct wconhist_well_struct{
  /* 
    def: Read as defaulted, not defined!
  */
  bool          def[WCONHIST_NUM_KW];

  char            * name;
  well_status_enum  status;
  well_cm_enum  cmode;
  double        orat;
  double        wrat;
  double        grat;
  int           vfptable;
  double        alift;
  double        thp;
  double        bhp;
  double        wgrat;
};



struct sched_kw_wconhist_struct{
  UTIL_TYPE_ID_DECLARATION;
  vector_type * wells;
};


/*****************************************************************/

/**
   Contains values for ORAT, GRAT, .... for one well for the complete
   history; this is orthogonal to the regualar sched_kw_wconhist
   keyowrd which contains the same data for all (or at least many ...)
   wells at one time instant. 
*/

struct wconhist_state_struct {
  UTIL_TYPE_ID_DECLARATION;
  const time_t_vector_type  * time;                    /* Shared vector with the report_step -> time_t mapping .*/
  int_vector_type           * state;                   /* Contains values from the well_status_enum. */ 
  int_vector_type           * cmode;                   /* Contains values from the well_cm_enum. */
  double_vector_type        * oil_rate;                
  double_vector_type        * water_rate;              
  double_vector_type        * gas_rate;                
  int_vector_type           * vfp_table; 
  double_vector_type        * art_lift;
  double_vector_type        * thp;
  double_vector_type        * bhp;
  double_vector_type        * wgas_rate;
};



/*****************************************************************/



static wconhist_well_type * wconhist_well_alloc_empty( ) 
{
  wconhist_well_type * well = util_malloc(sizeof * well);
  well->name   = NULL;
  well->status = WCONHIST_DEFAULT_STATUS;
  return well;
}



static void wconhist_well_free(wconhist_well_type * well)
{
  free(well->name);
  free(well);
}



static void wconhist_well_free__(void * well)
{
  wconhist_well_free( (wconhist_well_type *) well);
}



static void wconhist_well_fprintf(const wconhist_well_type * well, FILE * stream)
{
  fprintf(stream, "  ");
  sched_util_fprintf_qst(well->def[0],  well->name                                  , 8,     stream);
  sched_util_fprintf_qst(well->def[1],  sched_types_get_status_string(well->status) , 4,     stream);
  sched_util_fprintf_qst(well->def[2],  sched_types_get_cm_string(well->cmode)      , 4,     stream);
  sched_util_fprintf_dbl(well->def[3],  well->orat                                  , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[4],  well->wrat                                  , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[5],  well->grat                                  , 11, 3, stream);
  sched_util_fprintf_int(well->def[6],  well->vfptable                              , 4 ,    stream);
  sched_util_fprintf_dbl(well->def[7],  well->alift                                 , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[8],  well->thp                                   , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[9] , well->bhp                                   , 11, 3, stream);
  sched_util_fprintf_dbl(well->def[10], well->wgrat                                 , 11, 3, stream);
  fprintf(stream, "/\n");
}






static wconhist_well_type * wconhist_well_alloc_from_tokens(const stringlist_type * line_tokens ) {
  wconhist_well_type * well = wconhist_well_alloc_empty( );
  sched_util_init_default( line_tokens , well->def );
  
  well->name  = util_alloc_string_copy(stringlist_iget(line_tokens, 0));
  
  if(!well->def[1])
    well->status = sched_types_get_status_from_string(stringlist_iget(line_tokens , 1));
  if (well->status == DEFAULT)
    well->status = WCONHIST_DEFAULT_STATUS;
  
  
  if(!well->def[2])
    well->cmode = sched_types_get_cm_from_string(stringlist_iget(line_tokens , 2) , true);

  well->orat      = sched_util_atof(stringlist_iget(line_tokens , 3)); 
  well->wrat      = sched_util_atof(stringlist_iget(line_tokens , 4)); 
  well->grat      = sched_util_atof(stringlist_iget(line_tokens , 5)); 
  well->vfptable  = sched_util_atoi(stringlist_iget(line_tokens , 6));
  well->alift     = sched_util_atof(stringlist_iget(line_tokens , 7));
  well->thp       = sched_util_atof(stringlist_iget(line_tokens , 8));
  well->bhp       = sched_util_atof(stringlist_iget(line_tokens , 9));
  well->wgrat     = sched_util_atof(stringlist_iget(line_tokens , 10));

  return well;
}






static hash_type * wconhist_well_export_obs_hash(const wconhist_well_type * well)
{
  hash_type * obs_hash = hash_alloc();

  if(!well->def[3])
    hash_insert_double(obs_hash, "WOPR", well->orat);

  if(!well->def[4])
    hash_insert_double(obs_hash, "WWPR", well->wrat);

  if(!well->def[5])
    hash_insert_double(obs_hash, "WGPR", well->grat);
  
  if(!well->def[8])
    hash_insert_double(obs_hash, "WTHP", well->thp);
  
  if(!well->def[9])
    hash_insert_double(obs_hash, "WBHP", well->bhp);
  
  if(!well->def[10])
    hash_insert_double(obs_hash, "WWGPR", well->wgrat);

  // Water cut.
  if(!well->def[3] && !well->def[4])
  {
    double wct;
    if(well->orat + well->wrat > 0.0)
      wct = well->wrat / (well->orat + well->wrat);
    else
      wct = 0.0;

    hash_insert_double(obs_hash, "WWCT", wct);
  }
  
  // Gas oil ratio.
  if(!well->def[3] && !well->def[5])
  {
    double gor;
    if(well->orat > 0.0)
    {
      gor = well->grat / well->orat;
      hash_insert_double(obs_hash, "WGOR", gor);
    }
  }

  return obs_hash;
}


static void sched_kw_wconhist_add_well( sched_kw_wconhist_type * kw , wconhist_well_type * well) {
  vector_append_owned_ref(kw->wells, well, wconhist_well_free__);
}




static sched_kw_wconhist_type * sched_kw_wconhist_alloc_empty()
{
  sched_kw_wconhist_type * kw = util_malloc(sizeof * kw);
  UTIL_TYPE_ID_INIT( kw , SCHED_KW_WCONHIST_ID );
  kw->wells     = vector_alloc_new();
  return kw;
}



sched_kw_wconhist_type * sched_kw_wconhist_safe_cast( void * arg ) {
  sched_kw_wconhist_type * kw = (sched_kw_wconhist_type * ) arg;
  if (kw->__type_id == SCHED_KW_WCONHIST_ID)
    return kw;
  else {
    util_abort("%s: runtime cast failed \n",__func__);
    return NULL;
  }
}



/***********************************************************************/



sched_kw_wconhist_type * sched_kw_wconhist_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_wconhist_type * kw = sched_kw_wconhist_alloc_empty();
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , WCONHIST_NUM_KW , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      wconhist_well_type * well = wconhist_well_alloc_from_tokens( line_tokens );
      sched_kw_wconhist_add_well( kw , well );
      stringlist_free( line_tokens );
    } 
    
  } while (!eokw);
  return kw;
}


void sched_kw_wconhist_free(sched_kw_wconhist_type * kw)
{
  vector_free(kw->wells);
  free(kw);
}



void sched_kw_wconhist_fprintf(const sched_kw_wconhist_type * kw, FILE * stream)
{
  int size = vector_get_size(kw->wells);

  fprintf(stream, "WCONHIST\n");
  for(int i=0; i<size; i++)
  {
    const wconhist_well_type * well = vector_iget_const(kw->wells, i);
    wconhist_well_fprintf(well, stream);
  }
  fprintf(stream,"/\n\n");
}


/***********************************************************************/



hash_type * sched_kw_wconhist_alloc_well_obs_hash(const sched_kw_wconhist_type * kw)
{
  hash_type * well_hash = hash_alloc();

  int num_wells = vector_get_size(kw->wells);
  
  for(int well_nr=0; well_nr<num_wells; well_nr++)
  {
    wconhist_well_type * well = vector_iget(kw->wells, well_nr);
    hash_type * obs_hash = wconhist_well_export_obs_hash(well);
    hash_insert_hash_owned_ref(well_hash, well->name, obs_hash, hash_free__);
  }

  return well_hash;
}

sched_kw_wconhist_type * sched_kw_wconhist_copyc(const sched_kw_wconhist_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}



/***********************************************************************/
/* Functions exported for the sched_file_update api.                   */



/** Will return NULL if the well is not present. */
static wconhist_well_type * sched_kw_wconhist_get_well( const sched_kw_wconhist_type * kw , const char * well_name) {
  int size = vector_get_size(kw->wells);
  wconhist_well_type * well = NULL;
  int index = 0;
  do {
    wconhist_well_type * iwell = vector_iget( kw->wells , index);
    if (strcmp( well_name , iwell->name ) == 0) 
      well = iwell;
    
    index++;
  } while ((well == NULL) && (index < size));
  return well;
}


/*****************************************************************/


double sched_kw_wconhist_get_orat( sched_kw_wconhist_type * kw , const char * well_name) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    return well->orat;
  else
    return -1;
}

void sched_kw_wconhist_scale_orat( sched_kw_wconhist_type * kw , const char * well_name, double factor) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    well->orat *= factor;
}

void sched_kw_wconhist_shift_orat( sched_kw_wconhist_type * kw , const char * well_name, double shift_value) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL) {
    well->orat += shift_value;
    if (well->orat < 0)
      well->orat = 0;
  }
}

void sched_kw_wconhist_set_orat( sched_kw_wconhist_type * kw , const char * well_name , double orat) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    well->orat = orat;
}

/*****************************************************************/
/* WRAT functions                                                */

double sched_kw_wconhist_get_wrat( sched_kw_wconhist_type * kw , const char * well_name) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    return well->wrat;
  else
    return -1;
}

void sched_kw_wconhist_scale_wrat( sched_kw_wconhist_type * kw , const char * well_name, double factor) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    well->wrat *= factor;
}

void sched_kw_wconhist_shift_wrat( sched_kw_wconhist_type * kw , const char * well_name, double shift_value) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL) {
    well->wrat += shift_value;
    if (well->wrat < 0)
      well->wrat = 0;
  }
}

void sched_kw_wconhist_set_wrat( sched_kw_wconhist_type * kw , const char * well_name , double wrat) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    well->wrat = wrat;
}

/*****************************************************************/
/* GRAT functions                                                */

double sched_kw_wconhist_get_grat( sched_kw_wconhist_type * kw , const char * well_name) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    return well->grat;
  else
    return -1;
}

void sched_kw_wconhist_scale_grat( sched_kw_wconhist_type * kw , const char * well_name, double factor) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    well->grat *= factor;
}

void sched_kw_wconhist_shift_grat( sched_kw_wconhist_type * kw , const char * well_name, double shift_value) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL) {
    well->grat += shift_value;
    if (well->grat < 0)
      well->grat = 0;
  }
}

void sched_kw_wconhist_set_grat( sched_kw_wconhist_type * kw , const char * well_name , double grat) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL)
    well->grat = grat;
}


/*****************************************************************/


bool sched_kw_wconhist_has_well( const sched_kw_wconhist_type * kw , const char * well_name) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well == NULL)
    return false;
  else
    return true;
}

/**
   This keyword checks if the well @well_name is open in this wconhist
   instance. The check is quite simple: if the wconhist instance has
   this well, and that well has status == OPEN AND a finite rate of at
   least one phase - we return true, in ALL other cases we return
   false. 
   
   Observe that this function has no possibility to check well-names
   +++ - if you ask for a non-existing well you will just get false.
*/
   

bool sched_kw_wconhist_well_open( const sched_kw_wconhist_type * kw, const char * well_name) {
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well == NULL)
    return false;
  else {
    /* OK - we have the well. */
    if (well->status == OPEN) {
      /* The well seems to be open - any rates around? */
      if ((well->orat + well->grat + well->wrat) > 0.0)
        return true;
      else
        return false;
    } else
      return false;
  }
}

/*****************************************************************/


/**
   Will update the input parameter @well_list to contain all the
   well_names present in the current sced_kw_wconhist keyword.
*/
   
void sched_kw_wconhist_init_well_list( const sched_kw_wconhist_type * kw , stringlist_type * well_list) {
  stringlist_clear( well_list );
  {
    int iw;
    for (iw = 0; iw < vector_get_size( kw->wells ); iw++) {
      const wconhist_well_type * well = vector_iget_const( kw->wells , iw );
      stringlist_append_ref( well_list , well->name );
    }
  }
}


/*****************************************************************/

static UTIL_SAFE_CAST_FUNCTION_CONST( wconhist_state , WCONHIST_TYPE_ID)
static UTIL_SAFE_CAST_FUNCTION( wconhist_state , WCONHIST_TYPE_ID)




static double well_util_total( const time_t_vector_type * time, const double_vector_type * rate , int report_step ) {
  double total = 0;
  for (int index = 1; index <= report_step; index++) {
    /* It is a HARD assumption that the rate values in @rate are given as volume / day. */
    double days = (time_t_vector_iget( time , index ) - time_t_vector_iget( time , index - 1)) / 86400;
    total +=      days * double_vector_iget( rate , index );
  }
  
  return total; 
}


double wconhist_state_iget_WOPTH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return well_util_total( wconhist_state->time , wconhist_state->oil_rate , report_step );
}


double wconhist_state_iget_WGPTH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return well_util_total( wconhist_state->time , wconhist_state->gas_rate , report_step );
}


double wconhist_state_iget_WWPTH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return well_util_total( wconhist_state->time , wconhist_state->water_rate , report_step );
}


/*
  Functions implementing the wconhist state; the naming convention
  here should follow the one used in summary files, i.e. WOPR to get Oil Production Rate.
*/


double wconhist_state_iget_WBHPH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return double_vector_safe_iget( wconhist_state->bhp , report_step );
}


double wconhist_state_iget_WOPRH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return double_vector_safe_iget( wconhist_state->oil_rate , report_step );
}


double wconhist_state_iget_WGPRH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return double_vector_safe_iget( wconhist_state->gas_rate , report_step );
}


double wconhist_state_iget_WWPRH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return double_vector_safe_iget( wconhist_state->water_rate , report_step );
}


double wconhist_state_iget_WWCTH( const void * state , int report_step ) {
  double WWPR = wconhist_state_iget_WWPRH( state , report_step );
  double WOPR = wconhist_state_iget_WOPRH( state , report_step );
  
  return WWPR / ( WWPR + WOPR );
}


double wconhist_state_iget_WGORH(const void * state , int report_step ) {
  double WGPR = wconhist_state_iget_WGPRH( state , report_step );
  double WOPR = wconhist_state_iget_WOPRH( state , report_step );
  return WGPR / WOPR;
}


/*
  Uncertain about this memnonic?? 
*/

well_cm_enum wconhist_state_iget_WMCTLH( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return int_vector_iget( wconhist_state->cmode , report_step );
}


//well_status_enum wconhist_state_iget_status( const void * state , int report_step ) {

// All callbacks return double ... should really be an enum value
double wconhist_state_iget_STAT( const void * state , int report_step ) {
  const wconhist_state_type * wconhist_state = wconhist_state_safe_cast_const( state );
  return int_vector_iget( wconhist_state->state , report_step );
}



wconhist_state_type * wconhist_state_alloc( const time_t_vector_type * time) {
  wconhist_state_type * wconhist = util_malloc( sizeof * wconhist );
  UTIL_TYPE_ID_INIT( wconhist , WCONHIST_TYPE_ID );
  
  wconhist->time       = time;
  wconhist->state      = int_vector_alloc( 0 , WCONHIST_DEFAULT_STATUS );
  wconhist->cmode      = int_vector_alloc( 0 , 0 ); 
  
  wconhist->oil_rate   = double_vector_alloc( 0 , 0 );
  wconhist->gas_rate   = double_vector_alloc( 0 , 0 );
  wconhist->water_rate = double_vector_alloc( 0 , 0 );
  
  /*
    The vfp_table and art_list keywords have an EXTREMELY ugly
    DEFAULT behaviour - it changes as function of time:
  
     1. The first occurence of a default value should be interpreted
        as a '0'.

     2. The second occurence of a default value should be interpreted
        as 'No change prom previous value' - whatever that was.
       
    This behaviour is not properly supported in this implementation.    
  */


  wconhist->vfp_table  = int_vector_alloc( 0 , 0 );
  wconhist->art_lift   = double_vector_alloc( 0 , 0);

  wconhist->thp        = double_vector_alloc( 0 , 0 );
  wconhist->bhp        = double_vector_alloc( 0 , 0 );
  wconhist->wgas_rate  = double_vector_alloc( 0 , 0 );
  
  return wconhist;
}


void wconhist_state_free( wconhist_state_type * wconhist ) {
  int_vector_free( wconhist->state );
  int_vector_free( wconhist->cmode );
  int_vector_free( wconhist->vfp_table );
  
  double_vector_free( wconhist->oil_rate );
  double_vector_free( wconhist->gas_rate );
  double_vector_free( wconhist->water_rate );
  double_vector_free( wconhist->art_lift );
  double_vector_free( wconhist->thp );
  double_vector_free( wconhist->bhp );
  double_vector_free( wconhist->wgas_rate );
  
  free( wconhist );
}


void wconhist_state_free__( void * arg ) {
  wconhist_state_free( wconhist_state_safe_cast( arg ));
}



void sched_kw_wconhist_update_state(const sched_kw_wconhist_type * kw , wconhist_state_type * state , const char * well_name , int report_step ) {
  
  wconhist_well_type * well = sched_kw_wconhist_get_well( kw , well_name );
  if (well != NULL) {

    int_vector_iset_default( state->state         , report_step  ,  well->status   );
    int_vector_iset_default( state->cmode         , report_step  ,  well->cmode    );
    double_vector_iset_default( state->oil_rate   , report_step  ,  well->orat     );
    double_vector_iset_default( state->water_rate , report_step  ,  well->wrat     );
    double_vector_iset_default( state->gas_rate   , report_step  ,  well->grat     );
    int_vector_iset_default( state->vfp_table     , report_step  ,  well->vfptable );
    double_vector_iset_default( state->art_lift   , report_step  ,  well->alift    );
    double_vector_iset_default( state->thp        , report_step  ,  well->thp      );
    double_vector_iset_default( state->bhp        , report_step  ,  well->bhp      );
    double_vector_iset_default( state->wgas_rate  , report_step  ,  well->wgrat    );
    
  }
}



void sched_kw_wconhist_close_state( wconhist_state_type * state , int report_step ) {
  int_vector_iset_default( state->state         , report_step  ,  SHUT    );  /* SHUT or STOP?? This will fuck up a bit when what is actually happening is that the well goes over to
                                                                                 WCONPROD control, because the WCONPROD keyword is not internalized at all; so the well be stuck in this
                                                                                 status. */
  int_vector_iset_default( state->cmode         , report_step  ,  CM_SHUT );
  /*
    If code ever ends up by querying one of the states below here
    there is something wrong.
  */
  
  double_vector_iset_default( state->oil_rate   , report_step  ,  -1);
  double_vector_iset_default( state->water_rate , report_step  ,  -1);
  double_vector_iset_default( state->gas_rate   , report_step  ,  -1);
  int_vector_iset_default( state->vfp_table     , report_step  ,  -1);
  double_vector_iset_default( state->art_lift   , report_step  ,  -1);
  double_vector_iset_default( state->thp        , report_step  ,  -1);
  double_vector_iset_default( state->bhp        , report_step  ,  -1);
  double_vector_iset_default( state->wgas_rate  , report_step  ,  -1);

}


KW_IMPL(wconhist)
