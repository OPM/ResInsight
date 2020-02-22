/*
   Copyright (c) 2017  equinor asa, norway.

   The file 'ecl_nnc_geometry.c' is part of ert - ensemble based reservoir tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the gnu general public license as published by
   the free software foundation, either version 3 of the license, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but without any
   warranty; without even the implied warranty of merchantability or
   fitness for a particular purpose.

   See the gnu general public license at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <vector>
#include <algorithm>

#include <ert/ecl/ecl_nnc_geometry.hpp>

#define ECL_NNC_GEOMETRY_TYPE_ID 6124343

struct ecl_nnc_geometry_struct {
  UTIL_TYPE_ID_DECLARATION;
  std::vector<ecl_nnc_pair_struct> * data;
};


UTIL_IS_INSTANCE_FUNCTION( ecl_nnc_geometry, ECL_NNC_GEOMETRY_TYPE_ID )


int ecl_nnc_geometry_size( const ecl_nnc_geometry_type * nnc_geo ) {
  return nnc_geo->data->size();
}

/*
  Will go through the grid and add links for all NNC connections in
  the grid. The endpoints of an NNC are defined by the tuple:

     (grid_nr, global_index),

  and a NNC link is defined by a pair of such connections, linking
  cells (grid_nr1, global_index1) and (grid_nr2, global_index2).
*/

static void ecl_nnc_geometry_add_pairs( const ecl_nnc_geometry_type * nnc_geo , const ecl_grid_type * grid ) {
  int lgr_nr1 = ecl_grid_get_lgr_nr( grid );
  const ecl_grid_type * global_grid = ecl_grid_get_global_grid( grid );

  if (!global_grid)
    global_grid = grid;


  for (int global_index1 = 0; global_index1 < ecl_grid_get_global_size( grid ); global_index1++) {
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1( grid , global_index1 );
    if (!nnc_info)
      continue;

    for (int lgr_index2 = 0; lgr_index2 < nnc_info_get_size( nnc_info ); lgr_index2++) {
      const nnc_vector_type * nnc_vector = nnc_info_iget_vector( nnc_info , lgr_index2 );
      const std::vector<int>& grid2_index_list = nnc_vector_get_grid_index_list( nnc_vector );
      const std::vector<int>& nnc_index_list = nnc_vector_get_nnc_index_list( nnc_vector );
      int lgr_nr2 = nnc_vector_get_lgr_nr( nnc_vector );

      for (int index2 = 0; index2 < nnc_vector_get_size( nnc_vector ); index2++) {
        ecl_nnc_pair_type pair;
        pair.grid_nr1 = lgr_nr1;
        pair.global_index1 = global_index1;
        pair.grid_nr2 = lgr_nr2;
        pair.global_index2 = grid2_index_list[index2];
        pair.input_index = nnc_index_list[index2];
        nnc_geo->data->push_back(pair);
      }
    }
  }
}


static bool ecl_nnc_cmp(const ecl_nnc_pair_type& nnc1, const ecl_nnc_pair_type& nnc2) {
  if (nnc1.grid_nr1 != nnc2.grid_nr1)
    return nnc1.grid_nr1 < nnc2.grid_nr1;

  if (nnc1.grid_nr2 != nnc2.grid_nr2)
    return nnc1.grid_nr2 < nnc2.grid_nr2;

  if (nnc1.global_index1 != nnc2.global_index1)
    return nnc1.global_index1 < nnc2.global_index1;

  if (nnc1.global_index2 != nnc2.global_index2)
    return nnc1.global_index2 < nnc2.global_index2;

  return false;
}



ecl_nnc_geometry_type * ecl_nnc_geometry_alloc( const ecl_grid_type * grid ) {
  ecl_nnc_geometry_type * nnc_geo = (ecl_nnc_geometry_type*)util_malloc( sizeof * nnc_geo );
  UTIL_TYPE_ID_INIT( nnc_geo , ECL_NNC_GEOMETRY_TYPE_ID );
  nnc_geo->data = new std::vector<ecl_nnc_pair_type>();

  ecl_nnc_geometry_add_pairs( nnc_geo , grid );
  for (int lgr_index = 0; lgr_index < ecl_grid_get_num_lgr(grid); lgr_index++) {
    ecl_grid_type * igrid = ecl_grid_iget_lgr( grid , lgr_index );
    ecl_nnc_geometry_add_pairs( nnc_geo, igrid );
  }
  std::sort(nnc_geo->data->begin(), nnc_geo->data->end(), ecl_nnc_cmp);
  return nnc_geo;
}


void ecl_nnc_geometry_free( ecl_nnc_geometry_type * nnc_geo) {
  delete nnc_geo->data;
  free( nnc_geo );
}


const ecl_nnc_pair_type * ecl_nnc_geometry_iget( const ecl_nnc_geometry_type * nnc_geo , int index) {
  const std::vector<ecl_nnc_pair_type>& nnc_data = *nnc_geo->data;
  return &nnc_data[index];
}


bool ecl_nnc_geometry_same_kw( const ecl_nnc_pair_type * nnc1 , const ecl_nnc_pair_type * nnc2) {
  if ((nnc1->grid_nr1 == nnc2->grid_nr1) && (nnc1->grid_nr2 == nnc2->grid_nr2))
    return true;
  else
    return false;
}
