/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_history.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/size_t_vector.h>
#include <ert/util/double_vector.h>

#include <ert/sched/well_history.h>
#include <ert/sched/group_history.h>
#include <ert/sched/sched_kw.h>
#include <ert/sched/sched_kw_wconhist.h>
#include <ert/sched/sched_kw_welspecs.h>
#include <ert/sched/sched_kw_wconinje.h>
#include <ert/sched/sched_kw_wconprod.h>
#include <ert/sched/sched_kw_wconinjh.h>
#include <ert/sched/sched_kw_gruptree.h>
#include <ert/sched/sched_file.h>
#include <ert/sched/sched_types.h>
#include <ert/sched/well_index.h>
#include <ert/sched/group_index.h>
#include <ert/sched/sched_history.h>


struct sched_history_struct {
  hash_type           * well_history;   /* Hash table of well_history_type instances. */
  hash_type           * group_history;   
  time_t_vector_type  * time;            
  hash_type           * index;
  char                * sep_string;
  bool_vector_type    * historical;    /* This is meant to flag whether a certain report_step is "historical", or
                                          if it is in prediction_mode; changing back and forth between the two
                                          modes seem quite broken behaviour, but I guess it is perfectly
                                          legitimate. */
  int                   last_history_step; /* The last historical report_step (i.e. the total length is 1+ this value). */
};


#define FIELD_GROUP     "FIELD"





/*****************************************************************/

well_history_type * sched_history_get_well( const sched_history_type * sched_history , const char * well_name );



static void sched_history_install_well_index( sched_history_type * sched_history , well_index_type * well_index , const char ** var_list , const char * well_name) {
  int          index   = 0;
  char       * gen_key = NULL;
  const char * var     = var_list[ index ];
  bool  first          = true;

  while ( var != NULL ) {
    gen_key = util_realloc_sprintf( gen_key , "%s%s%s" , var , sched_history->sep_string , well_name );
    
    if (first) {
      first = false;
      hash_insert_hash_owned_ref( sched_history->index , gen_key , well_index , well_index_free__);
    } else
      hash_insert_ref( sched_history->index , gen_key , well_index );
    
    index++;
    var  = var_list[ index ];
  }
  
  if (first)
    util_abort("%s: internal error - empty var_list \n",__func__);
  free( gen_key );
}



static void sched_history_install_group_index( sched_history_type * sched_history , group_index_type * group_index , const char ** var_list , const char * group_name) {
  int          index   = 0;
  char       * gen_key = NULL;
  const char * var     = var_list[ index ];
  bool  first          = true;

  while ( var != NULL ) {
    gen_key = util_realloc_sprintf( gen_key , "%s%s%s" , var , sched_history->sep_string , group_name );
    
    if (first) {
      first = false;
      hash_insert_hash_owned_ref( sched_history->index , gen_key , group_index , group_index_free__);
    } else
      hash_insert_ref( sched_history->index , gen_key , group_index );
    
    index++;
    var  = var_list[ index ];
  }
  
  if (first)
    util_abort("%s: internal error - empty var_list \n",__func__);
  free( gen_key );
}





#define VAR_LIST(...) (const char *[]) { __VA_ARGS__ , NULL  }

void sched_history_install_index( sched_history_type * sched_history ) {
  /*1: Installing well based keys like WOPRH. */
  {
    hash_iter_type * well_iter = hash_iter_alloc( sched_history->well_history );
    while (!hash_iter_is_complete( well_iter )) {
      const char * well_name         = hash_iter_get_next_key( well_iter );
      const well_history_type * well = hash_get( sched_history->well_history , well_name );
      
      /* WOPR */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WOPRH" , well , WCONHIST , wconhist_state_iget_WOPRH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WOPR" , "WOPRH") , well_name);
      }
      
      
      /* WGPR */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WGPRH" , well , WCONHIST , wconhist_state_iget_WGPRH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WGPR" , "WGPRH") , well_name);
      }
      
      
      /* WWPR */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WWPRH" , well , WCONHIST , wconhist_state_iget_WWPRH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WWPR" , "WWPRH") , well_name);
      }
      
      
      /* WWCT */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WWCTH" , well , WCONHIST , wconhist_state_iget_WWCTH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WWCT" , "WWCTH") , well_name);
      }

      /* WGOR */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WGORH" , well , WCONHIST , wconhist_state_iget_WGORH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WGOR" , "WGORH") , well_name);
      }

      /* WGPT */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WGPTH" , well , WCONHIST , wconhist_state_iget_WGPTH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WGPT" , "WGPTH") , well_name);
      }

      /* WOPT */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WOPTH" , well , WCONHIST , wconhist_state_iget_WOPTH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WOPT" , "WOPTH") , well_name);
      }
      
      /* WWPT */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WWPTH" , well , WCONHIST , wconhist_state_iget_WWPTH );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WWPT" , "WWPTH") , well_name);
      }

      /* STAT */
      {
        well_index_type * well_index = well_index_alloc( well_name , "STAT" , well , WCONHIST , wconhist_state_iget_STAT );
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("STAT" ) , well_name);
      }

      
      /* WWIRH - this can be got from _either_ the WCONINJH keyowrord
         or the WCONINJE keyword (provided the latter is in rate
         controlled mode. ) */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WWIRH" , well , WCONINJH , wconinjh_state_iget_WWIRH );   /* The first type */
        well_index_add_type( well_index , WCONINJE , wconinje_state_iget_WWIRH );                         /* The second type */
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WWIRH" , "WWIR") , well_name);
      }
      
      /* WGIRH - this can be got from _either_ the WCONINJH keyowrord
         or the WCONINJE keyword (provided the latter is in rate
         controlled mode. ) */
      {
        well_index_type * well_index = well_index_alloc( well_name , "WGIRH" , well , WCONINJH , wconinjh_state_iget_WGIRH );   /* The first type */
        well_index_add_type( well_index , WCONINJE , wconinje_state_iget_WGIRH );                         /* The second type */
        sched_history_install_well_index( sched_history , well_index , VAR_LIST("WGIRH" , "WGIR") , well_name);
      }
    }
    hash_iter_free( well_iter );
  }



  /*2: Installing group based indices */
  {
    hash_iter_type * group_iter = hash_iter_alloc( sched_history->group_history );
    while (!hash_iter_is_complete( group_iter )) {
      const char * group_name          = hash_iter_get_next_key( group_iter );
      const group_history_type * group = hash_get( sched_history->group_history , group_name );
      
      /* GOPR */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GOPRH" , group , group_history_iget_GOPRH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GOPR" , "GOPRH") , group_name);
      }

      /* GGPR */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GGPRH" , group , group_history_iget_GGPRH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GGPR" , "GGPRH") , group_name);
      }

      /* GWPR */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GWPRH" , group , group_history_iget_GWPRH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GWPR" , "GWPRH") , group_name);
      }

      /* GWCT */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GWCTH" , group , group_history_iget_GWCTH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GWCT" , "GWCTH") , group_name);
      }

      /* GGOR */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GGORH" , group , group_history_iget_GGORH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GGOR" , "GGORH") , group_name);
      }

      /* GOPT */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GOPTH" , group , group_history_iget_GOPTH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GOPT" , "GOPTH") , group_name);
      }

      /* GGPT */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GGPTH" , group , group_history_iget_GGPTH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GGPT" , "GGPTH") , group_name);
      }
      
      /* GWPT */
      {
        group_index_type * group_index = group_index_alloc( group_name , "GWPTH" , group , group_history_iget_GWPTH );
        sched_history_install_group_index( sched_history , group_index , VAR_LIST("GWPT" , "GWPTH") , group_name);
      }
    }
    hash_iter_free( group_iter );
  }


  /*3: Installing field based indices (which is just an alias to the FIELD group); */
  {
    const group_history_type * group = hash_get( sched_history->group_history , FIELD_GROUP );
    const char * group_name          = FIELD_GROUP;
    
    /* FWPRH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GWPRH" , group , group_history_iget_GWPRH );
      hash_insert_hash_owned_ref( sched_history->index , "FWPRH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FWPR" , group_index);
    }

    /* FOPRH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GOPRH" , group , group_history_iget_GOPRH );
      hash_insert_hash_owned_ref( sched_history->index , "FOPRH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FOPR" , group_index);
    }

    /* FGPRH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GGPRH" , group , group_history_iget_GGPRH );
      hash_insert_hash_owned_ref( sched_history->index , "FGPRH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FGPR" , group_index);
    }

    /* FWPTH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GWPTH" , group , group_history_iget_GWPTH );
      hash_insert_hash_owned_ref( sched_history->index , "FWPTH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FWPT" , group_index);
    }

    /* FOPTH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GOPTH" , group , group_history_iget_GOPTH );
      hash_insert_hash_owned_ref( sched_history->index , "FOPTH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FOPT" , group_index);
    }

    /* FGPTH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GGPTH" , group , group_history_iget_GGPTH );
      hash_insert_hash_owned_ref( sched_history->index , "FGPTH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FGPT" , group_index);
    }

    /* FGORH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GGORH" , group , group_history_iget_GGORH );
      hash_insert_hash_owned_ref( sched_history->index , "FGORH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FGOR" , group_index);
    }

    /* FWCTH */
    {
      group_index_type * group_index   = group_index_alloc( group_name , "GWCTH" , group , group_history_iget_GWCTH );
      hash_insert_hash_owned_ref( sched_history->index , "FWCTH" , group_index , group_index_free__ );
      hash_insert_ref( sched_history->index , "FWCT" , group_index);
    }
  }
}
#undef VAR_LIST




double sched_history_iget( const sched_history_type * sched_history , const char * key , int report_step) {
  void * index = hash_get( sched_history->index , key );
  if (!bool_vector_safe_iget( sched_history->historical , report_step ))
    fprintf(stderr,"** Warning - report step:%d is in the prediction phase - can NOT ask for historical data! \n",report_step);
  
  if (well_index_is_instance( index ))
    return well_history_iget( index , report_step );
  else if (group_index_is_instance( index ))
    return group_history_iget( index , report_step );
  else {
    util_abort("%s: can not determine internal type of:%s - fatal internal error\n", __func__ , key);
    return 0;
  }
}






void sched_history_init_vector( const sched_history_type * sched_history , const char * key , double_vector_type * value) {
  const bool * historical = bool_vector_get_ptr( sched_history->historical );
  double_vector_reset( value );
  for (int i=0; i < time_t_vector_size( sched_history->time ); i++) {
    if (historical[i])
      double_vector_iset( value , i , sched_history_iget( sched_history , key , i));
    else
      break;
  }
}



static void sched_history_realloc( sched_history_type * sched_history ) {
  if (sched_history->well_history != NULL)
    hash_free( sched_history->well_history );
  sched_history->well_history = hash_alloc();
  
  if (sched_history->group_history != NULL)
    hash_free( sched_history->group_history );
  sched_history->group_history = hash_alloc();
  
  if (sched_history->historical != NULL)
    bool_vector_free( sched_history->historical );
  sched_history->historical = bool_vector_alloc( 0 , true );

  if (sched_history->time != NULL)
    time_t_vector_free(sched_history->time);
  sched_history->time         = time_t_vector_alloc( 0 , 0 ); 
  sched_history->last_history_step = 0;
}



time_t sched_history_iget_time_t( const sched_history_type * sched_history , int restart_nr ) {
  return time_t_vector_iget( sched_history->time , restart_nr );
}


sched_history_type * sched_history_alloc( const char * sep_string ) {
  sched_history_type * sched_history = util_malloc( sizeof * sched_history );

  sched_history->well_history  = NULL;
  sched_history->group_history = NULL;
  sched_history->time          = NULL;
  sched_history->historical    = NULL;
  sched_history->index         = hash_alloc();
  sched_history->sep_string    = util_alloc_string_copy( sep_string );
  sched_history_realloc( sched_history );
  
  return sched_history;
}



void sched_history_free( sched_history_type * sched_history ) {
  time_t_vector_free( sched_history->time );
  bool_vector_free( sched_history->historical );
  hash_free( sched_history->well_history );
  hash_free( sched_history->group_history );
  hash_free( sched_history->index );
  free( sched_history->sep_string );
  free( sched_history );
}



well_history_type * sched_history_get_well( const sched_history_type * sched_history , const char * well_name ) {
  return hash_get( sched_history->well_history , well_name );
}


group_history_type * sched_history_get_group( const sched_history_type * sched_history , const char * group_name ) {
  return hash_get( sched_history->group_history , group_name );
}



static void sched_history_add_wells( sched_history_type * sched_history , const sched_kw_welspecs_type * welspecs , const stringlist_type * wells) {
  for (int iw = 0; iw < stringlist_get_size( wells ); iw++) {
    const char * well = stringlist_iget( wells , iw );
    if (!hash_has_key( sched_history->well_history , well)) 
      hash_insert_hash_owned_ref( sched_history->well_history , well , well_history_alloc( well , sched_history->time ), well_history_free__ );
    
    /* Could possibly extract more information from the welspecs
       keyword and update well_history object here, but it does not
       seem to contain any more interesting info???
    */
    
  }
}


static void sched_history_add_group( sched_history_type * sched_history , group_history_type * new_group, group_history_type * parent_group , int report_step ) {
  hash_insert_hash_owned_ref( sched_history->group_history , group_history_get_name( new_group ) , new_group , group_history_free__ );
  if (parent_group == NULL)
    parent_group = sched_history_get_group( sched_history , FIELD_GROUP );
  
  group_history_add_child( parent_group , new_group , group_history_get_name( new_group ) , report_step );
}


/**
   Because the FIELD group is added without any parent; it does not
   use the standard sched_history_group_add() function. */

static void sched_history_add_FIELD_group( sched_history_type * sched_history ) {
  group_history_type * field_group = group_history_alloc( FIELD_GROUP , sched_history->time , 0 );
  hash_insert_hash_owned_ref( sched_history->group_history , FIELD_GROUP , field_group , group_history_free__ );
}


void sched_history_fprintf_group_structure( sched_history_type * sched_history , int report_step ) {
  group_history_type * field_group = sched_history_get_group( sched_history , FIELD_GROUP );
  group_history_fprintf( field_group , report_step , true ,stdout );
}


static void sched_history_add_groups_gruptree( sched_history_type * sched_history , const sched_kw_gruptree_type * gruptree , int report_step , const stringlist_type * child_groups , const stringlist_type * parent_groups) {
  for (int i = 0; i < stringlist_get_size( child_groups ); i++) {
    const char * parent_group_name  = stringlist_iget( parent_groups , i );
    const char * child_group_name   = stringlist_iget( child_groups , i );
    group_history_type * parent_group;
    group_history_type * child_group;


    if (!hash_has_key( sched_history->group_history , parent_group_name )) 
      sched_history_add_group( sched_history , group_history_alloc( parent_group_name , sched_history->time , report_step) , NULL , report_step );
    parent_group = sched_history_get_group( sched_history , parent_group_name );

    if (!hash_has_key( sched_history->group_history , child_group_name )) 
      sched_history_add_group( sched_history , group_history_alloc( child_group_name , sched_history->time , report_step ) , parent_group , report_step );
    child_group = sched_history_get_group( sched_history , child_group_name );

    group_history_add_child( parent_group , child_group , child_group_name , report_step);
  }
}


int sched_history_get_last_history( const sched_history_type * sched_history ) {
  return sched_history->last_history_step;
}


static void sched_history_set_historical( sched_history_type * sched_history , int report_step ) {
  bool_vector_iset_default( sched_history->historical , report_step , true );
  sched_history->last_history_step = report_step;
}


/**
   When new wells are added with the WELSPECS keyword their parent
   group is implicitly introduced as the second argument of the
   WELSPEC keyword, in addition the GRUPTREE keyword will also
   implicitly introduce groups. 

   This functions creates group_history objects for all the groups
   introduced by the WELSPECS keyword, and attach wells to them (the
   input parameters @wells and @groups come driectly from the welspecs
   keyword, via the sched_kw_welspecs_init_child_parent_list()
   function.
*/


static void sched_history_add_groups_welspecs( sched_history_type * sched_history , const sched_kw_welspecs_type * welspecs , int report_step , const stringlist_type * wells, const stringlist_type * groups) {
  for (int i = 0; i < stringlist_get_size( groups ); i++) {
    const char * group_name  = stringlist_iget( groups , i );
    const char * well_name   = stringlist_iget( wells , i );
    well_history_type * well = sched_history_get_well( sched_history , well_name );
    group_history_type * group;
    if (!hash_has_key( sched_history->group_history , group_name )) 
      sched_history_add_group( sched_history , group_history_alloc( group_name , sched_history->time , report_step ) , NULL , report_step );
    
    group = sched_history_get_group( sched_history , group_name );
    group_history_add_child( group , well , well_name , report_step);
  }
}




void sched_history_update( sched_history_type * sched_history, const sched_file_type * sched_file ) {
  
  sched_history_realloc( sched_history );
  sched_history_add_FIELD_group( sched_history );
  {
    int block_nr;
    stringlist_type * well_list  = stringlist_alloc_new();    
    stringlist_type * group_list = stringlist_alloc_new();

    for (block_nr = 0; block_nr < sched_file_get_num_restart_files( sched_file ); block_nr++) {
      sched_block_type * block = sched_file_iget_block( sched_file , block_nr );
      int kw_nr;
      int report_step = block_nr;

      time_t_vector_iset( sched_history->time , block_nr , sched_file_iget_block_end_time( sched_file , block_nr));
      for (kw_nr = 0; kw_nr < sched_block_get_size( block ); kw_nr++) {
        sched_kw_type * kw         = sched_block_iget_kw( block , kw_nr );
        sched_kw_type_enum kw_type = sched_kw_get_type( kw );

        switch( kw_type ) {
        case(WCONHIST):
          {
            const sched_kw_wconhist_type * wconhist = sched_kw_get_data( kw );
            sched_kw_wconhist_init_well_list( wconhist , well_list );
            int iw;
            for (iw = 0; iw < stringlist_get_size( well_list ); iw++) {
              const char * well_name           = stringlist_iget( well_list , iw );
              well_history_type * well_history = sched_history_get_well( sched_history , well_name );
              well_history_add_keyword( well_history , kw , report_step );
            }
          }
          sched_history_set_historical( sched_history , block_nr );
          break; 
        case(WCONPROD):    /* This is only added to turn the well OFF from WCONHIST behaviour.  It is currently not
                              possible to query the well for anything when it is in WCONPROD state. */

          { 
            const sched_kw_wconprod_type * wconprod = sched_kw_get_data( kw ); 
            sched_kw_wconprod_init_well_list( wconprod , well_list );
            int iw;
            for (iw = 0; iw < stringlist_get_size( well_list ); iw++) {
              const char * well_name           = stringlist_iget( well_list , iw );
              well_history_type * well_history = sched_history_get_well( sched_history , well_name );
              well_history_add_keyword( well_history , kw , report_step);
            }
          }
          bool_vector_iset_default( sched_history->historical , block_nr , false );
          break;
        case(WCONINJE):
          {
            const sched_kw_wconinje_type * wconinje = sched_kw_get_data( kw );
            sched_kw_wconinje_init_well_list( wconinje , well_list );
            int iw;
            for (iw = 0; iw < stringlist_get_size( well_list ); iw++) {
              const char * well_name           = stringlist_iget( well_list , iw );
              well_history_type * well_history = sched_history_get_well( sched_history , well_name );
              well_history_add_keyword( well_history , kw , report_step);
            }
            if (sched_kw_wconinje_historical( wconinje ))
              sched_history_set_historical( sched_history , block_nr );
          }
          break;
        case(WCONINJH):
          /* ... */
          sched_history_set_historical( sched_history , block_nr );
          break;
        case(WELSPECS):
          {
            const sched_kw_welspecs_type * welspecs = sched_kw_get_data( kw );
            sched_kw_welspecs_init_child_parent_list( welspecs , well_list , group_list );
            sched_history_add_wells( sched_history , welspecs , well_list );
            sched_history_add_groups_welspecs( sched_history , welspecs , report_step , well_list , group_list );
            for (int iw = 0; iw < stringlist_get_size( well_list ); iw++) {
              const char * well_name           = stringlist_iget( well_list , iw );
              well_history_type * well_history = sched_history_get_well( sched_history , well_name );
              well_history_add_keyword( well_history , kw , report_step);
            }
          }
          break;
        case(GRUPTREE):
          {
            const sched_kw_gruptree_type * gruptree = sched_kw_get_data( kw );
            stringlist_type * parent_group_list = group_list;
            stringlist_type * child_group_list  = well_list;

            sched_kw_gruptree_init_child_parent_list( gruptree , child_group_list , parent_group_list );
            sched_history_add_groups_gruptree( sched_history , gruptree , report_step , well_list , group_list );
          }
          break;
        default:
          /*   */
          break;
        }
      }
    }
    stringlist_free( well_list );
    stringlist_free( group_list );
  }
  sched_history_install_index( sched_history );
}


const char * sched_history_get_join_string( const sched_history_type * sched_history ) {
  return sched_history->sep_string; 
}


bool sched_history_has_well( const sched_history_type * sched_history , const char * well_name) {
  return hash_has_key( sched_history->well_history , well_name );
}

bool sched_history_has_group( const sched_history_type * sched_history , const char * group_name) {
  return hash_has_key( sched_history->group_history , group_name );
}

bool sched_history_has_key( const sched_history_type * sched_history , const char * key) {
  return hash_has_key( sched_history->index , key );
}

bool sched_history_well_open( const sched_history_type * sched_history , const char * well_name , int report_step ) {
  const well_history_type * well_history = sched_history_get_well( sched_history , well_name );
  return well_history_well_open( well_history , report_step );
}



/**
   Will take a general key as input, and return the result of
   schd_history_well_open() or sched_history_group_exists()
   respectively - depending on the type key refers to.
*/

bool sched_history_open( const sched_history_type * sched_history , const char * key , int report_step) {
  const void * index = hash_get(sched_history->index , key );
  if (well_index_is_instance( index )) {
    const well_index_type * well_index = well_index_safe_cast_const( index );
    const char * well_name = well_index_get_name( well_index );
    return sched_history_well_open( sched_history , well_name , report_step);
  } else if (group_index_is_instance( index )) {
    const group_index_type * group_index = group_index_safe_cast_const( index );
    const char * group_name = group_index_get_name( group_index );
    return sched_history_group_exists( sched_history , group_name , report_step);
  } else {
    util_abort("%s: - hmm internal fuckup \n",__func__);
    return false;
  }
}





/**
   This function checks if the group @group exists (i.e. has been
   defined with the GRUPTREE or WELSPECS keyword) at the report_step
   @report_step; if the group does not exist in the schedule file
   _AT_ALL_ - the function will fail HARD. Use the function
   sched_history_has_group() to check if a group is in the schedule
   file at all.
*/


bool sched_history_group_exists( const sched_history_type * sched_history , const char * group_name , int report_step ) {
  const group_history_type * group_history = sched_history_get_group( sched_history , group_name );
  return group_history_group_exists( group_history , report_step );
}



void sched_history_fprintf_index_keys( const sched_history_type * sched_history , FILE * stream ) {
  hash_iter_type * iter = hash_iter_alloc( sched_history->index );
  int c = 0;
  while (!hash_iter_is_complete( iter )) {
    fprintf(stream , "%18s" , hash_iter_get_next_key( iter ));
    c += 1;
    if ((c % 6) == 0)
      fprintf(stream , "\n");
  }
  hash_iter_free( iter );
}




void sched_history_fprintf( const sched_history_type * sched_history , const stringlist_type * key_list , FILE * stream) {
  int step = 1;
  time_t start_time = time_t_vector_iget( sched_history->time , 0);
  int total_length = bool_vector_size( sched_history->historical );
  while (true) {
    if (bool_vector_safe_iget( sched_history->historical , step)) {
      {
        int mday,month,year;
        time_t t = time_t_vector_iget( sched_history->time , step );
        double days = (t - start_time) * 1.0 / 86400;
        util_set_date_values( t , &mday , &month , &year);
        //fprintf(stream , "%02d-%02d-%4d  " , mday , month , year );
        fprintf(stream , " %5.0f " , days);
      }
      
      for (int ikey =0; ikey < stringlist_get_size( key_list ); ikey++) 
        fprintf(stream , "%16.3f " , sched_history_iget( sched_history , stringlist_iget( key_list , ikey) , step));
      
      fprintf( stream, "\n");
    } else
      break; // We have completed the historical period - and switched to prediction
    step++;

    if (step == total_length)
      break;
  }
}


