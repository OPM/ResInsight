/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_welspecs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/sched/sched_util.h>
#include <ert/sched/sched_types.h>
#include <ert/sched/sched_macros.h>
#include <ert/sched/sched_kw_welspecs.h>



/*
  TODO

  Create lookup STRING -> ENUM for all types and fix all strcmp!

*/

/*
  Define the maximum number of keywords in a WELSPEC record.
  Note that this includes FrontSim and ECLIPSE 300 KWs.
*/
#define WELSPECS_NUM_KW 16
#define ECL_DEFAULT_KW  "*"


#define DEFAULT_INFLOW_EQUATION    IE_STD
#define DEFAULT_AUTO_SHUT_TYPE     AS_SHUT
#define DEFAULT_CROSSFLOW_ABILITY  CF_YES
#define DEFAULT_HDSTAT_TYPE        HD_SEG


struct sched_kw_welspecs_struct
{
  vector_type * welspec_list;
};


/*
  See ECLIPSE Reference Manual, section WELSPECS for an explantion of
  the members in the welspec_type struct.
*/

typedef enum {PH_OIL , PH_WAT , PH_GAS , PH_LIQ} phase_type;
#define PH_OIL_STRING "OIL"
#define PH_WAT_STRING "WATER"
#define PH_GAS_STRING "GAS"
#define PH_LIQ_STRING "LIQ"



typedef enum {IE_STD, IE_NO, IE_RG, IE_YES, IE_PP, IE_GPP} inflow_eq_type;
#define IE_STD_STRING "STD"
#define IE_NO_STRING  "NO"
#define IE_RG_STRING  "R-G"
#define IE_YES_STRING "YES"
#define IE_PP_STRING  "P-P"
#define IE_GPP_STRING "GPP"



typedef enum {AS_STOP, AS_SHUT} auto_shut_type;
#define AS_STOP_STRING "STOP"
#define AS_SHUT_STRING "SHUT"



typedef enum {CF_YES, CF_NO} crossflow_type; 
#define CF_YES_STRING "YES"
#define CF_NO_STRING  "NO"

typedef enum {HD_SEG,  HD_AVG} hdstat_head_type;
#define HD_SEG_STRING "SEG"
#define HD_AVG_STRING "AVG"

typedef struct
{
  /*
    def : Read as defaulted, not as defined.
  */
  bool             def[WELSPECS_NUM_KW];

  char           * name;
  char           * group;
  int              hh_i;
  int              hh_j;
  double           md;
  phase_type       phase;
  double           drain_rad;
  inflow_eq_type   inflow_eq;
  auto_shut_type   auto_shut;
  crossflow_type   crossflow;
  int              pvt_region;
  hdstat_head_type hdstat_head;
  int              fip_region;
  char           * fs_kw1;
  char           * fs_kw2;
  char           * ecl300_kw;
} welspec_type;



static char * get_phase_string(phase_type phase)
{
  switch(phase)
  {
    case(PH_OIL):
      return PH_OIL_STRING;
    case(PH_WAT):
      return PH_WAT_STRING;
    case(PH_GAS):
      return PH_GAS_STRING;
    case(PH_LIQ):
      return PH_LIQ_STRING;
    default:
      return ECL_DEFAULT_KW;
  }
};



static char * get_inflow_eq_string(inflow_eq_type eq)
{
  switch(eq)
  {
    case(IE_STD):
      return IE_STD_STRING;
    case(IE_NO):
      return IE_NO_STRING;
    case(IE_RG):
      return IE_NO_STRING;
    case(IE_YES):
      return IE_YES_STRING;
    case(IE_PP):
      return IE_PP_STRING;
    case(IE_GPP):
      return IE_GPP_STRING;
    default:
      return ECL_DEFAULT_KW;
  }
};



static char * get_auto_shut_string(auto_shut_type as)
{
  switch(as)
  {
    case(AS_STOP):
      return AS_STOP_STRING;
    case(AS_SHUT):
      return AS_SHUT_STRING;
    default:
      return ECL_DEFAULT_KW;
  }
};



static char * get_crossflow_string(crossflow_type cf)
{
  switch(cf)
  {
    case(CF_YES):
      return CF_YES_STRING;
    case(CF_NO):
      return CF_NO_STRING;
    default:
      return ECL_DEFAULT_KW;
  }
}



static char * get_hdstat_head_string(hdstat_head_type hd)
{
  switch(hd)
  {
    case(HD_SEG):
      return HD_SEG_STRING;
    case(HD_AVG):
      return HD_AVG_STRING;
    default:
      return ECL_DEFAULT_KW;
  }
};



static inflow_eq_type get_inflow_eq_from_string(const char * string)
{
  if(strcmp(string     , IE_STD_STRING) == 0)
    return IE_STD;
  else if(strcmp(string, IE_NO_STRING)  == 0)
    return IE_NO;
  else if(strcmp(string, IE_RG_STRING)  == 0)
    return IE_RG;
  else if(strcmp(string, IE_YES_STRING) == 0)
    return IE_YES;
  else if(strcmp(string, IE_PP_STRING)  == 0)
    return IE_PP;
  else if(strcmp(string, IE_GPP_STRING) == 0)
    return IE_GPP;
  else if (strcmp(string , SCHED_KW_DEFAULT_ITEM) == 0)
    return DEFAULT_INFLOW_EQUATION;
  else
    util_abort("%s: Inflow equation %s not recognized - aborting.\n",__func__, string);
  return IE_STD;
}



/* No default is defined according to the documentation. */
static phase_type get_phase_from_string(const char * string)
{
    if(strcmp(string     , PH_OIL_STRING) == 0)
      return PH_OIL;
    else if(strcmp(string, PH_WAT_STRING) == 0)
      return PH_WAT;
    else if(strcmp(string, PH_GAS_STRING) == 0)
      return PH_GAS;
    else if(strcmp(string, PH_LIQ_STRING) == 0)
      return PH_LIQ;
    else
      util_abort("%s: Phase %s not recognized - aborting.\n",__func__,string);
      return 0;
}



static auto_shut_type get_auto_shut_from_string(const char * string)
{
  if(strcmp(string,     AS_STOP_STRING) == 0)
    return AS_STOP;
  else if (strcmp(string,AS_SHUT_STRING) == 0)
    return AS_SHUT;
  else if (strcmp(string , SCHED_KW_DEFAULT_ITEM) == 0)
    return DEFAULT_AUTO_SHUT_TYPE;
  else
    util_abort("%s: Automatic shut-in mode %s not recognized - aborting.\n",__func__,string);
  return 0;
}



static crossflow_type get_crossflow_from_string(const char * string)
{
  if(strcmp(string     ,CF_YES_STRING) == 0)
    return CF_YES;
  else if(strcmp(string,CF_NO_STRING) == 0)
    return CF_NO;
  else if(strcmp(string,SCHED_KW_DEFAULT_ITEM) == 0)
    return DEFAULT_CROSSFLOW_ABILITY;
  else
    util_abort("%s: Crossflow ability mode %s not recognized - aborting.\n",__func__,string);
  return 0;
}



static hdstat_head_type get_hdstat_head_from_string(const char * string)
{
  if(strcmp(string     ,HD_SEG_STRING) == 0)
    return HD_SEG;
  else if(strcmp(string,HD_AVG_STRING) == 0)
    return HD_AVG;
  else if(strcmp(string,SCHED_KW_DEFAULT_ITEM) == 0)
    return DEFAULT_HDSTAT_TYPE;
  else
    util_abort("%s: Hydrostatic head model %s not recognized - aborting.\n",__func__,string);
  return 0;
}



static void welspec_sched_fprintf(const welspec_type * ws, FILE * stream)
{
  fprintf(stream, " ");
  sched_util_fprintf_qst(ws->def[0]  , ws->name                                , 8,     stream);
  sched_util_fprintf_qst(ws->def[1]  , ws->group                               , 8,     stream);
  sched_util_fprintf_int(ws->def[2]  , ws->hh_i                                , 4,     stream);
  sched_util_fprintf_int(ws->def[3]  , ws->hh_j                                , 4,     stream);
  sched_util_fprintf_dbl(ws->def[4]  , ws->md                                  , 8, 3,  stream);
  sched_util_fprintf_qst(ws->def[5]  , get_phase_string(ws->phase)             , 5,     stream);
  sched_util_fprintf_dbl(ws->def[6]  , ws->drain_rad                           , 8, 3,  stream);
  sched_util_fprintf_qst(ws->def[7]  , get_inflow_eq_string(ws->inflow_eq)     , 3,     stream);
  sched_util_fprintf_qst(ws->def[8]  , get_auto_shut_string(ws->auto_shut)     , 4,     stream);
  sched_util_fprintf_qst(ws->def[9]  , get_crossflow_string(ws->crossflow)     , 3,     stream);
  sched_util_fprintf_int(ws->def[10] , ws->pvt_region                          , 4,     stream);
  sched_util_fprintf_qst(ws->def[11] , get_hdstat_head_string(ws->hdstat_head) , 3,     stream);
  sched_util_fprintf_int(ws->def[12] , ws->fip_region                          , 4,     stream);
  /*
  sched_util_fprintf_qst(ws->def[13] , ws->fs_kw1                              , 8,     stream);
  sched_util_fprintf_qst(ws->def[14] , ws->fs_kw2                              , 8,     stream);
  sched_util_fprintf_qst(ws->def[15] , ws->ecl300_kw                           , 8,     stream);
  */
  fprintf(stream,"/\n");
};



static welspec_type * welspec_alloc_empty()
{
  welspec_type *ws = util_malloc(sizeof *ws);
  
  ws->name      = NULL;
  ws->group     = NULL;
  ws->fs_kw1    = NULL;
  ws->fs_kw2    = NULL;
  ws->ecl300_kw = NULL;

  return ws;
}






static void welspec_free(welspec_type * ws)
{
  free(ws->group    );
  util_safe_free(ws->fs_kw1   );
  util_safe_free(ws->fs_kw2   );
  util_safe_free(ws->ecl300_kw);
  free(ws->name);
  free(ws);
};



static void welspec_free__(void * __ws)
{
  welspec_type * ws = (welspec_type *) __ws;
  welspec_free(ws);
};




static welspec_type * welspec_alloc_from_tokens(const stringlist_type * line_tokens )
{
  welspec_type * ws = welspec_alloc_empty();
  sched_util_init_default( line_tokens , ws->def );
  ws->name = util_alloc_string_copy(stringlist_iget( line_tokens , 0));
  
  if(ws->def[1])
    ws->group = util_alloc_string_copy("FIELD");
  else
    ws->group = util_alloc_string_copy(stringlist_iget( line_tokens , 1 ));

  ws->hh_i        = sched_util_atoi(stringlist_iget( line_tokens , 2));
  ws->hh_j        = sched_util_atoi(stringlist_iget( line_tokens , 3));
  ws->md          = sched_util_atof(stringlist_iget( line_tokens , 4));
  ws->phase       = get_phase_from_string(stringlist_iget( line_tokens , 5));        
  ws->drain_rad   = sched_util_atof(stringlist_iget( line_tokens , 6));
  ws->inflow_eq   = get_inflow_eq_from_string(stringlist_iget( line_tokens , 7));
  ws->auto_shut   = get_auto_shut_from_string(stringlist_iget( line_tokens , 8));
  ws->crossflow   = get_crossflow_from_string(stringlist_iget( line_tokens , 9));
  ws->pvt_region  = sched_util_atoi(stringlist_iget( line_tokens , 10));
  ws->hdstat_head = get_hdstat_head_from_string(stringlist_iget( line_tokens , 11));
  ws->fip_region  = sched_util_atoi(stringlist_iget( line_tokens , 12));

  ws->fs_kw1      = util_alloc_string_copy(stringlist_iget( line_tokens , 13));   /* Reserved for use with FRONTSIM */
  ws->fs_kw2      = util_alloc_string_copy(stringlist_iget( line_tokens , 14));   /* Reserved for use with FRONTSIM */ 
  ws->ecl300_kw   = util_alloc_string_copy(stringlist_iget( line_tokens , 15));   /* Could not find this in the dcoumentation ...??? */

  return ws;
};






static sched_kw_welspecs_type * sched_kw_welspecs_alloc_empty()
{
  sched_kw_welspecs_type * kw = util_malloc(sizeof * kw );
  kw->welspec_list = vector_alloc_new();
  return kw;
};


static void sched_kw_welspecs_add_well( sched_kw_welspecs_type * kw , const welspec_type * ws) {
  vector_append_owned_ref( kw->welspec_list , ws , welspec_free__ );
}


/*****************************************************************************/


sched_kw_welspecs_type * sched_kw_welspecs_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_welspecs_type * kw = sched_kw_welspecs_alloc_empty();
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , WELSPECS_NUM_KW , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      welspec_type   * well         = welspec_alloc_from_tokens( line_tokens );
      sched_kw_welspecs_add_well( kw , well );
      stringlist_free( line_tokens );
    } 
    
  } while (!eokw);
  return kw;
}




void sched_kw_welspecs_free(sched_kw_welspecs_type * kw)
{
  vector_free(kw->welspec_list);
  free(kw);
};



void sched_kw_welspecs_fprintf(const sched_kw_welspecs_type * kw, FILE * stream)
{
  fprintf(stream, "WELSPECS\n");
  int i;
  for (i=0; i < vector_get_size( kw->welspec_list ); i++) {
    const welspec_type * ws = vector_iget_const( kw->welspec_list , i );
    welspec_sched_fprintf(ws, stream);
  }
  fprintf(stream,"/\n\n");
};
  



void sched_kw_welspecs_init_child_parent_list( const sched_kw_welspecs_type * kw , stringlist_type * child , stringlist_type * parent) {
  stringlist_clear( child );
  stringlist_clear( parent );
  {
    for (int i=0; i < vector_get_size( kw->welspec_list ); i++) {
      const welspec_type * well = vector_iget_const(kw->welspec_list , i);
      stringlist_append_ref( child , well->name );

      if (!well->def[1])
        stringlist_append_ref( parent , well->group );
      else
        stringlist_append_ref( parent , FIELD_GROUP );

    }
  }
}




void sched_kw_welspecs_alloc_child_parent_list(const sched_kw_welspecs_type * kw, char *** __children, char *** __parents, int * num_pairs)
{
  int num_wells    = vector_get_size(kw->welspec_list);
  char ** children = util_malloc(num_wells * sizeof * children);
  char ** parents  = util_malloc(num_wells * sizeof * parents);

  for(int well_nr = 0; well_nr < num_wells; well_nr++)
  {
    const welspec_type * well = vector_iget_const(kw->welspec_list , well_nr);
    children[well_nr] = util_alloc_string_copy(well->name);
    if(!well->def[1])
      parents[well_nr] = util_alloc_string_copy(well->group);
    else
      parents[well_nr] = util_alloc_string_copy("FIELD");
  }

  *num_pairs = num_wells;
  *__children = children;
  *__parents  = parents;
}


sched_kw_welspecs_type * sched_kw_welspecs_copyc(const sched_kw_welspecs_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}



/***********************************************************************/

KW_IMPL(welspecs)
