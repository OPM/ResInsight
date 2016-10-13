/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_compdat.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>

#include <ert/sched/sched_kw_compdat.h>
#include <ert/sched/sched_util.h>


#define COMPDAT_NUM_KW        14   
#define SCHED_KW_COMPDAT_ID   771882

typedef enum {X, Y , Z , FX , FY}   well_dir_type; 
#define WELL_DIR_DEFAULT     Z
#define WELL_DIR_X_STRING   "X"
#define WELL_DIR_Y_STRING   "Y"
#define WELL_DIR_Z_STRING   "Z"
#define WELL_DIR_FX_STRING  "FX"
#define WELL_DIR_FY_STRING  "FZ"



typedef enum {OPEN , AUTO , SHUT}   comp_state_type;
#define COMP_DEFAULT_STATE  OPEN
#define COMP_OPEN_STRING   "OPEN"
#define COMP_AUTO_STRING   "AUTO"
#define COMP_SHUT_STRING   "SHUT"



/*
  Structure to hold one line (typically one completed cell) in a compdat section.
*/

typedef struct  {
  char             *well;            /* Name of well                                           */
  int               i,j,k1,k2;       /* The i,j,k coordinated of the perforated cell.          */
  well_dir_type     well_dir;        /* Which direction does the well penetrate the grid block */
  comp_state_type   state;           /* What state is this completion in: AUTO|SHUT|OPEN       */
  int               sat_table;    
  double            conn_factor;
  double            well_diameter;     
  double            eff_perm;          
  double            skin_factor;       
  double            D_factor;          
  double            r0;                   
  
  /*
    def : Read as defaulted, not as defined.
  */
  bool              def[COMPDAT_NUM_KW];
} comp_type;



struct sched_kw_compdat_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type  * completions;
};



/**
   make_lookup comp_get_state_string comp_get_state_from_string comp_state_type AUTO COMP_AUTO_STRING OPEN COMP_OPEN_STRING SHUT COMP_SHUT_STRING
*/



static char * comp_get_state_string(comp_state_type state) {
  switch(state) {
  case(AUTO):
    return COMP_AUTO_STRING;
  case(OPEN):
    return COMP_OPEN_STRING;
  case(SHUT):
    return COMP_SHUT_STRING;
  default:
    util_abort("%s: internal error \n",__func__);
    return NULL;
  }
}



static char * comp_get_dir_string(well_dir_type dir) {
  switch(dir) {
  case(X):
    return WELL_DIR_X_STRING;
  case(Y):
    return WELL_DIR_Y_STRING;
  case(Z):
    return WELL_DIR_Z_STRING;
  case(FX):
    return WELL_DIR_FX_STRING;
  case(FY):
    return WELL_DIR_FY_STRING;
  default:
    util_abort("%s: internal fuckup \n",__func__);
    return NULL;
  }
}


static well_dir_type comp_get_well_dir_from_string(const char * well_dir) {
  if (strcmp(well_dir , WELL_DIR_X_STRING) == 0)
    return X;
  else if (strcmp(well_dir , WELL_DIR_Y_STRING) == 0)
    return Y;
  else if (strcmp(well_dir , WELL_DIR_Z_STRING) == 0)
    return Z;
  else if (strcmp(well_dir , WELL_DIR_FX_STRING) == 0)
    return FX;
  else if (strcmp(well_dir , WELL_DIR_FY_STRING) == 0)
    return FY;
  else {
    util_abort("%s: internal fuckup \n",__func__);
    return -1;
  }
}



static comp_state_type comp_get_state_from_string(const char * state) {
  if (strcmp(state , COMP_AUTO_STRING) == 0)
    return AUTO;
  else if (strcmp(state , COMP_OPEN_STRING) == 0)
    return OPEN;
  else if (strcmp(state , COMP_SHUT_STRING) == 0)
    return SHUT;
  else {
    util_abort("%s: did not recognize:%s as valid completion state: (%s|%s|%s) \n",__func__ , state , COMP_AUTO_STRING , COMP_OPEN_STRING , COMP_SHUT_STRING);
    return -1;
  }
}






static void comp_sched_fprintf(const comp_type * comp , FILE *stream) {
  fprintf(stream , " ");
  sched_util_fprintf_qst(comp->def[0] , comp->well                           , 8  , stream);
  sched_util_fprintf_int(comp->def[1] , comp->i                              , 4  , stream);
  sched_util_fprintf_int(comp->def[2] , comp->j                              , 4  , stream);
  sched_util_fprintf_int(comp->def[3] , comp->k1                             , 4  , stream);
  sched_util_fprintf_int(comp->def[4] , comp->k2                             , 4  , stream);
  sched_util_fprintf_qst(comp->def[5] , comp_get_state_string( comp->state ) , 4  , stream);
  sched_util_fprintf_int(comp->def[6] , comp->sat_table                      , 6  ,     stream);
  sched_util_fprintf_dbl(comp->def[7] , comp->conn_factor                    , 9  , 3 , stream);
  sched_util_fprintf_dbl(comp->def[8] , comp->well_diameter                  , 9  , 3 , stream);
  sched_util_fprintf_dbl(comp->def[9] , comp->eff_perm                       , 9  , 3 , stream);
  sched_util_fprintf_dbl(comp->def[10], comp->skin_factor                    , 9  , 3 , stream);
  sched_util_fprintf_dbl(comp->def[11], comp->D_factor                       , 9  , 3 , stream);
  sched_util_fprintf_qst(comp->def[12], comp_get_dir_string( comp->well_dir) , 2  , stream);
  sched_util_fprintf_dbl(comp->def[13], comp->r0                             , 9  , 3 , stream);
  fprintf(stream , " /\n");
}




static comp_type * comp_alloc_empty( ) {
  comp_type *node = util_malloc(sizeof * node);
  node->well      = NULL;
  return node;
}


static comp_type * comp_alloc_from_tokens( const stringlist_type * line_tokens ) {
  comp_type * comp = comp_alloc_empty();
  sched_util_init_default( line_tokens , comp->def );
  
  
  comp->well         = util_alloc_string_copy(stringlist_iget( line_tokens , 0)); 
  comp->i            = sched_util_atoi(stringlist_iget( line_tokens , 1));
  comp->j            = sched_util_atoi(stringlist_iget( line_tokens , 2));
  comp->k1           = sched_util_atoi(stringlist_iget( line_tokens , 3));
  comp->k2           = sched_util_atoi(stringlist_iget( line_tokens , 4));

  if (comp->def[5]) 
    comp->state = COMP_DEFAULT_STATE;
  else 
    comp->state = comp_get_state_from_string( stringlist_iget( line_tokens , 5 ));
  
  comp->sat_table       = sched_util_atoi(stringlist_iget( line_tokens , 6));
  comp->conn_factor     = sched_util_atof(stringlist_iget( line_tokens , 7));
  comp->well_diameter   = sched_util_atof(stringlist_iget( line_tokens , 8));     
  comp->eff_perm        = sched_util_atof(stringlist_iget( line_tokens , 9));          
  comp->skin_factor     = sched_util_atof(stringlist_iget( line_tokens , 10));       
  comp->D_factor        = sched_util_atof(stringlist_iget( line_tokens , 11));         

  if (comp->def[12]) 
    comp->well_dir = WELL_DIR_DEFAULT;
  else
    comp->well_dir = comp_get_well_dir_from_string( stringlist_iget( line_tokens , 12 ));
  
  comp->r0 = sched_util_atof(stringlist_iget( line_tokens , 13));                
  return comp;
}



static void comp_free(comp_type *comp) {
  free(comp->well);
  free(comp);
}


static void comp_free__(void *__comp) {
  comp_type *comp = (comp_type *) __comp;
  comp_free(comp);
}





void sched_kw_compdat_fprintf(const sched_kw_compdat_type *kw , FILE *stream) {
  fprintf(stream , "COMPDAT\n");
  {
    int index;
    for (index = 0; index < vector_get_size( kw->completions ); index++) {
      const comp_type * comp = vector_iget_const( kw->completions , index );
      comp_sched_fprintf(comp , stream);
    }
  }
  fprintf(stream , "/\n\n");
}



sched_kw_compdat_type * sched_kw_compdat_alloc_empty( ) {
  sched_kw_compdat_type * kw = util_malloc(sizeof *kw );
  kw->completions = vector_alloc_new();
  UTIL_TYPE_ID_INIT( kw , SCHED_KW_COMPDAT_ID );
  return kw;
}


UTIL_SAFE_CAST_FUNCTION( sched_kw_compdat , SCHED_KW_COMPDAT_ID )

void sched_kw_compdat_add_comp( sched_kw_compdat_type * kw , comp_type * comp) {
  vector_append_owned_ref(kw->completions , comp , comp_free__);
}


sched_kw_compdat_type * sched_kw_compdat_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_compdat_type * kw = sched_kw_compdat_alloc_empty();
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , COMPDAT_NUM_KW , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      comp_type * comp = comp_alloc_from_tokens( line_tokens );
      sched_kw_compdat_add_comp( kw , comp );
      stringlist_free( line_tokens );
    } 
  }
  
  while (!eokw);
  return kw;
}

      



void sched_kw_compdat_free(sched_kw_compdat_type * kw) {
  vector_free(kw->completions);
  free(kw);
}


sched_kw_compdat_type * sched_kw_compdat_copyc(const sched_kw_compdat_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}


/*****************************************************************/
KW_IMPL(compdat)

