/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_nnc_export.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more detals. 
*/ 
#include <stdlib.h>

#include <ert/util/int_vector.h>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_nnc_export.h>
#include <ert/ecl/nnc_info.h>
#include <ert/ecl/ecl_kw_magic.h>


int ecl_nnc_export_get_size( const ecl_grid_type * grid ) {
  return ecl_grid_get_num_nnc( grid );
}




static void  ecl_nnc_export__( const ecl_grid_type * grid , int lgr_index1 , const ecl_file_type * init_file , ecl_nnc_type * nnc_data, int * nnc_offset) {
  int nnc_index = *nnc_offset;
  int lgr_nr1 = ecl_grid_get_lgr_nr( grid );
  int global_index1;
  const ecl_grid_type * global_grid = ecl_grid_get_global_grid( grid );

  if (!global_grid)
    global_grid = grid;

  
  for (global_index1 = 0; global_index1 < ecl_grid_get_global_size( grid ); global_index1++) {
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1( grid , global_index1 );
    if (nnc_info) {
      int lgr_index2;
      for (lgr_index2=0; lgr_index2 < nnc_info_get_size( nnc_info ); lgr_index2++) {
        const nnc_vector_type * nnc_vector = nnc_info_iget_vector( nnc_info , lgr_index2 );
        const int_vector_type * grid2_index_list = nnc_vector_get_grid_index_list( nnc_vector );
        const int_vector_type * nnc_index_list = nnc_vector_get_nnc_index_list( nnc_vector );
        int lgr_nr2 = nnc_vector_get_lgr_nr( nnc_vector );
        const ecl_kw_type * tran_kw = ecl_nnc_export_get_tranx_kw(global_grid  , init_file , lgr_nr1 , lgr_nr2 );
        int index2;
        ecl_nnc_type nnc;

        nnc.grid_nr1 = lgr_nr1;
        nnc.grid_nr2 = lgr_nr2;
        nnc.global_index1 = global_index1;

        for (index2 = 0; index2 < nnc_vector_get_size( nnc_vector ); index2++) {
          nnc.global_index2 = int_vector_iget( grid2_index_list , index2 );
          nnc.trans = ecl_kw_iget_as_double( tran_kw , int_vector_iget( nnc_index_list , index2));

          nnc_data[nnc_index] = nnc;
          nnc_index++;
        }
      }
    }
  }
  *nnc_offset = nnc_index;
}


void  ecl_nnc_export( const ecl_grid_type * grid , const ecl_file_type * init_file , ecl_nnc_type * nnc_data) {
  int nnc_index = 0;
  ecl_nnc_export__( grid , 0 , init_file , nnc_data , &nnc_index );
  {
    int lgr_index; 
    for (lgr_index = 0; lgr_index < ecl_grid_get_num_lgr(grid); lgr_index++) {
      ecl_grid_type * igrid = ecl_grid_iget_lgr( grid , lgr_index );
      ecl_nnc_export__( igrid , lgr_index , init_file , nnc_data , &nnc_index );
    }
  }
  nnc_index = ecl_nnc_export_get_size( grid );
  ecl_nnc_sort( nnc_data , nnc_index );
}



int ecl_nnc_cmp( const ecl_nnc_type * nnc1 , const ecl_nnc_type * nnc2) {

  if (nnc1->grid_nr1 != nnc2->grid_nr1) {
    if (nnc1->grid_nr1 < nnc2->grid_nr1) 
      return -1;
    else
      return 1;
  }

  if (nnc1->grid_nr2 != nnc2->grid_nr2) {
    if (nnc1->grid_nr2 < nnc2->grid_nr2) 
      return -1;
    else
      return 1;
  }

  if (nnc1->global_index1 != nnc2->global_index1) {
    if (nnc1->global_index1 < nnc2->global_index1) 
      return -1;
    else
      return 1;
  }


  if (nnc1->global_index2 != nnc2->global_index2) {
    if (nnc1->global_index2 < nnc2->global_index2) 
      return -1;
    else
      return 1;
  }
  
  return 0;
}

static int ecl_nnc_cmp__( const void * nnc1 , const void * nnc2) {
  return ecl_nnc_cmp( nnc1 , nnc2 );
}


void ecl_nnc_sort( ecl_nnc_type * nnc_list , int size) {
  qsort( nnc_list , size , sizeof * nnc_list , ecl_nnc_cmp__ );
}



ecl_kw_type * ecl_nnc_export_get_tranll_kw( const ecl_grid_type * grid , const ecl_file_type * init_file ,  int lgr_nr1, int lgr_nr2 ) {
  const char * lgr_name1 = ecl_grid_get_lgr_name( grid , lgr_nr1 );
  const char * lgr_name2 = ecl_grid_get_lgr_name( grid , lgr_nr2 );

  ecl_kw_type * tran_kw = NULL;
  const int file_num_kw = ecl_file_get_size( init_file );
  int global_kw_index = 0;
  
  while (true) {
    if (global_kw_index >= file_num_kw) 
      break;
    {
      ecl_kw_type * ecl_kw = ecl_file_iget_kw( init_file , global_kw_index );
      if (strcmp( LGRJOIN_KW , ecl_kw_get_header( ecl_kw)) == 0) {
        
        if (ecl_kw_icmp_string( ecl_kw , 0 , lgr_name1) && ecl_kw_icmp_string( ecl_kw , 1 , lgr_name2)) {
          tran_kw = ecl_file_iget_kw( init_file , global_kw_index + 1);
          break;
        }
      } 
      global_kw_index++;
    }
  }

  return tran_kw;
}



ecl_kw_type * ecl_nnc_export_get_tran_kw( const ecl_file_type * init_file , const char * kw , int lgr_nr ) {
  ecl_kw_type * tran_kw = NULL;

  if (lgr_nr == 0) {
    if (strcmp(kw , TRANNNC_KW) == 0)
      tran_kw = ecl_file_iget_named_kw( init_file , TRANNNC_KW , 0 );
  } else {
    if ((strcmp(kw , TRANNNC_KW) == 0) ||
        (strcmp(kw , TRANGL_KW) == 0)) {
      int tran_kw_forward_skip;
      const int file_num_kw = ecl_file_get_size( init_file );
      int global_kw_index = 0;
      bool finished = false;
      
      if (strcmp(kw , TRANNNC_KW) == 0)
        tran_kw_forward_skip = 3;
      else
        tran_kw_forward_skip = 4;
      
      while (!finished) {
        ecl_kw_type * ecl_kw = ecl_file_iget_kw( init_file , global_kw_index );
        
        if (strcmp( LGRHEADI_KW , ecl_kw_get_header( ecl_kw )) == 0) {
          if (ecl_kw_iget_int( ecl_kw , LGRHEADI_LGR_NR_INDEX) == lgr_nr) {
            
            if ((global_kw_index + tran_kw_forward_skip) < file_num_kw) {
              ecl_kw_type * ecl_kw = ecl_file_iget_kw( init_file , global_kw_index + tran_kw_forward_skip);

              /* We found the TRANGL / TRANNC keyword we are after. */
              if (strcmp( kw , ecl_kw_get_header( ecl_kw )) == 0) {
                tran_kw = ecl_kw;
                finished = true;
                break;
              } 
            }

          } 
        } 
        
        global_kw_index++;
        if (global_kw_index == file_num_kw)
          finished = true;
      } 
    }
  }
  return tran_kw;
}


ecl_kw_type * ecl_nnc_export_get_tranx_kw( const ecl_grid_type * grid , const ecl_file_type * init_file ,  int lgr_nr1, int lgr_nr2 ) {
  if (lgr_nr1 == lgr_nr2)
    return ecl_nnc_export_get_tran_kw( init_file , TRANNNC_KW , lgr_nr2 );
  else {
    if (lgr_nr1 == 0)
      return ecl_nnc_export_get_tran_kw( init_file , TRANGL_KW , lgr_nr2 );
    else
      return ecl_nnc_export_get_tranll_kw( grid , init_file , lgr_nr1 , lgr_nr2 );
  }
}
