/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'layer.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/type_macros.h>
#include <ert/util/int_vector.h>

#include <ert/ecl/layer.h>

#define LAYER_TYPE_ID  55185409



typedef struct {
  int cell_value;
  int edges[4];
} cell_type; 


struct layer_struct {
  UTIL_TYPE_ID_DECLARATION;
  int nx, ny;
  cell_type * data;
  int cell_sum;
};


UTIL_IS_INSTANCE_FUNCTION( layer , LAYER_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION( layer , LAYER_TYPE_ID )


layer_type * layer_alloc(int nx , int ny) {
  layer_type * layer = util_malloc( sizeof * layer );
  UTIL_TYPE_ID_INIT( layer , LAYER_TYPE_ID );
  layer->nx = nx;
  layer->ny = ny;
  layer->cell_sum = 0;
  layer->data = util_malloc( (layer->nx + 1)* (layer->ny + 1) * sizeof * layer->data );
  {
    int g;
    for (g=0; g < (layer->nx + 1)*(layer->ny + 1); g++) {
      cell_type * cell = &layer->data[g];
      cell->cell_value = 0;
      cell->edges[RIGHT_EDGE] = 0;
      cell->edges[LEFT_EDGE] = 0;
      cell->edges[TOP_EDGE] = 0;
      cell->edges[BOTTOM_EDGE] = 0;
    }
  }
  
  return layer;
}




void  layer_free( layer_type * layer ) {
  free( layer->data );
  free(layer);
}
  
static int layer_get_global_cell_index( const layer_type * layer , int i , int j) {
  if ((i < 0) || (i >= layer->nx))
    util_abort("%s: invalid i value:%d Valid range: [0,%d) \n",__func__ , i , layer->nx);

  if ((j < 0) || (j >= layer->ny))
    util_abort("%s: invalid j value:%d Valid range: [0,%d) \n",__func__ , j , layer->ny);
  
  return i + j*(layer->nx + 1);
}



int layer_iget_cell_value( const layer_type * layer, int i , int j) {
  int g = layer_get_global_cell_index( layer , i , j );
  return layer->data[g].cell_value;
}


int layer_get_cell_sum( const layer_type * layer ) {
  return layer->cell_sum;
}


static void layer_cancel_edge( layer_type * layer , int i , int j , edge_dir_enum dir) {
  int g = layer_get_global_cell_index( layer , i , j );
  cell_type * cell = &layer->data[g];
  cell->edges[dir] = 0;
}


int layer_get_nx( const layer_type * layer ) {
  return layer->nx;
}

int layer_get_ny( const layer_type * layer ) {
  return layer->ny;
}

void layer_iset_cell_value( layer_type * layer , int i , int j , int value) {
  int g = layer_get_global_cell_index( layer , i , j );
  cell_type * cell = &layer->data[g];
  
  layer->cell_sum += (value - cell->cell_value);
  cell->cell_value = value;
  

  if (i > 0) {
    int neighbour_value = layer_iget_cell_value( layer , i - 1 , j);
    if (value == neighbour_value) {
      cell->edges[LEFT_EDGE]   = 0;
      layer_cancel_edge( layer , i - 1, j , RIGHT_EDGE);
    } else
      cell->edges[LEFT_EDGE]   = -value;
  } else
    cell->edges[LEFT_EDGE]   = -value;


  if (i < (layer->nx - 1)) {
    int neighbour_value = layer_iget_cell_value( layer , i + 1 , j);
    if (value == neighbour_value) {
      cell->edges[RIGHT_EDGE]   = 0;
      layer_cancel_edge( layer , i + 1, j , LEFT_EDGE);
    } else
      cell->edges[RIGHT_EDGE]   = value;
  } else
    cell->edges[RIGHT_EDGE]   = value;
  

  if (j < (layer->ny - 1)) {
    int neighbour_value = layer_iget_cell_value( layer , i , j + 1);
    if (value == neighbour_value) {
      cell->edges[TOP_EDGE]   = 0;
      layer_cancel_edge( layer , i , j + 1, BOTTOM_EDGE);
    } else
      cell->edges[TOP_EDGE]   = -value;
  } else
    cell->edges[TOP_EDGE]   = -value;
  
  
  if (j > 0) {
    int neighbour_value = layer_iget_cell_value( layer , i , j - 1);
    if (value == neighbour_value) {
      cell->edges[BOTTOM_EDGE]   = 0;
      layer_cancel_edge( layer , i , j - 1, TOP_EDGE);
    } else
      cell->edges[BOTTOM_EDGE]   = value;
  } else
    cell->edges[BOTTOM_EDGE]   = value;

}


static int layer_get_global_edge_index( const layer_type * layer , int i , int j , edge_dir_enum dir) {
  if ((i < 0) || (j < 0))
    util_abort("%s: invalid value for i,j \n",__func__);
  
  if ((i > layer->nx) || (j > layer->ny))
    util_abort("%s: invalid value for i,j \n",__func__);

  if (i == layer->nx) {
    if (j == layer->ny)
      util_abort("%s: invalid value for i,j \n",__func__);

    if (dir != LEFT_EDGE)
      util_abort("%s: invalid value for i,j \n",__func__);
  }


  if (j == layer->ny) {
    if (i == layer->nx)
      util_abort("%s: invalid value for i,j \n",__func__);

    if (dir != BOTTOM_EDGE)
      util_abort("%s: invalid value for i,j \n",__func__);
  }

  return i + j*(layer->nx + 1);
}



int layer_iget_edge_value( const layer_type * layer , int i , int j , edge_dir_enum dir) {
  int g = layer_get_global_edge_index( layer , i , j , dir); 
  cell_type * cell = &layer->data[g];
  return cell->edges[dir];
}


bool layer_cell_on_edge( const layer_type * layer , int i , int j) {
  int g = layer_get_global_cell_index( layer , i , j); 
  cell_type * cell = &layer->data[g];

  if (cell->cell_value == cell->edges[LEFT_EDGE])
    return true;
  if (cell->cell_value == cell->edges[RIGHT_EDGE])
    return true;
  if (cell->cell_value == cell->edges[BOTTOM_EDGE])
    return true;
  if (cell->cell_value == cell->edges[TOP_EDGE])
    return true;
  
  return false;
}


static void point_shift(int_point2d_type * point , int di , int dj) {
  point->i += di;
  point->j += dj;
}

static bool point_equal(int_point2d_type * p1 , int_point2d_type *p2) {
  if ((p1->i == p2->i) && (p1->j == p2->j))
    return true;
  else
    return false;
}


/*
  Possible edge transitions:

  BOTTOM_EDGE -> BOTTOM_EDGE{ i + 1, j } , RIGHT_EDGE{i,j}     ,  LEFT_EDGE{i +1,j -1}
  RIGHT_EDGE  -> TOP_EDGE{i,j}           , RIGHT_EDGE{i,j+1}   ,  BOTTOM_EDGE{i+1 , j+1}
  TOP_EDGE    -> LEFT_EDGE{i,j}          , TOP_EDGE{i-1,j}     ,  RIGHT_EDGE{i-1,j+1}
  LEFT_EDGE   -> BOTTOM_EDGE{i,j}        , LEFT_EDGE{i,j-1}    ,  TOP_EDGE{i-1 , j-1}
  


*/

static void layer_trace_block_edge__( const layer_type * layer , int_point2d_type start_point , int i , int j , int value , edge_dir_enum dir , struct_vector_type * corner_list, int_vector_type * cell_list) {
  int_point2d_type current_point;
  int_point2d_type next_point;
  current_point.i = i;
  current_point.j = j;
  next_point = current_point;
  
  if (dir == BOTTOM_EDGE) 
    point_shift( &next_point , 1 , 0 );
  else if (dir == RIGHT_EDGE) {
    point_shift( &current_point , 1 , 0 );
    point_shift( &next_point , 1 , 1 );
  } else if (dir == TOP_EDGE) {
    point_shift( &current_point , 1 , 1 );
    point_shift( &next_point , 0 , 1 );
  } else if (dir == LEFT_EDGE)
    point_shift( &current_point , 0 , 1 );

  struct_vector_append( corner_list , &current_point );
  {
    int cell_index = i + j*layer->nx;
    int_vector_append( cell_list , cell_index );
  }
  
  if ( !point_equal(&start_point , &next_point) ) {
    
    if (dir == BOTTOM_EDGE) {
      if (layer_iget_edge_value( layer , i,j,RIGHT_EDGE) == value)
        layer_trace_block_edge__( layer , start_point , i , j , value , RIGHT_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i + 1 , j ,BOTTOM_EDGE) == value)
        layer_trace_block_edge__( layer , start_point , i + 1 , j , value , BOTTOM_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i + 1 , j - 1 , LEFT_EDGE) == -value)
        layer_trace_block_edge__( layer , start_point , i + 1 , j -1 , value , LEFT_EDGE , corner_list , cell_list);
      else 
        util_abort("%s: dir == BOTTOM_EDGE \n",__func__);
    } 


    if (dir == RIGHT_EDGE) {
      if (layer_iget_edge_value( layer , i,j,TOP_EDGE) == -value)
        layer_trace_block_edge__( layer , start_point , i , j , value , TOP_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i  , j + 1 ,RIGHT_EDGE) == value)
        layer_trace_block_edge__( layer , start_point , i  , j + 1, value , RIGHT_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i + 1 , j + 1 ,BOTTOM_EDGE) == value)
        layer_trace_block_edge__( layer , start_point , i + 1  , j + 1, value , BOTTOM_EDGE , corner_list , cell_list);
      else 
        util_abort("%s: dir == RIGHT_EDGE \n",__func__);
    }


    if (dir == TOP_EDGE) {
      if (layer_iget_edge_value( layer , i , j , LEFT_EDGE) == -value)
        layer_trace_block_edge__( layer , start_point , i , j , value , LEFT_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i - 1  , j  ,TOP_EDGE) == -value)
        layer_trace_block_edge__( layer , start_point , i - 1 , j , value , TOP_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i - 1  , j + 1  ,RIGHT_EDGE) == value)
        layer_trace_block_edge__( layer , start_point , i - 1 , j + 1, value , RIGHT_EDGE , corner_list , cell_list);
      else 
        util_abort("%s: dir == TOP_EDGE \n",__func__);
    }

    
    if (dir == LEFT_EDGE) {
      if (layer_iget_edge_value( layer , i , j , BOTTOM_EDGE) == value)
        layer_trace_block_edge__( layer , start_point , i , j , value , BOTTOM_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i   , j - 1 , LEFT_EDGE) == -value)
        layer_trace_block_edge__( layer , start_point , i , j - 1, value , LEFT_EDGE , corner_list , cell_list);
      else if (layer_iget_edge_value( layer , i -1  , j - 1 , TOP_EDGE) == -value)
        layer_trace_block_edge__( layer , start_point , i-1 , j - 1, value , TOP_EDGE , corner_list , cell_list);
      else
        util_abort("%s: dir == LEFT_EDGE \n",__func__);
    }

  }
}



static void layer_fprintf_dash( const layer_type * layer , FILE * stream, int i1 , int i2) {
  int i;
  fprintf(stream,"      --");
  for (i=i1; i <= i2; i++) 
    fprintf(stream , "----");
  fprintf(stream , "----\n");
}


static void layer_fprintf_header( const layer_type * layer , FILE * stream, int i1 , int i2) {
  int i;
  fprintf(stream,"        ");
  for (i=i1; i <= i2; i++) 
    fprintf(stream , " %3d" , i);
  fprintf(stream , "\n");
}


void layer_fprintf_box( const layer_type * layer , FILE * stream , int i1 , int i2 , int j1 , int j2) {
  int i,j;
  layer_fprintf_header( layer , stream , i1 , i2);
  layer_fprintf_dash( layer , stream , i1 , i2);
  

  for (j=j2; j >= j1; j--) {
    fprintf(stream , " %3d  | " , j);
    for (i=i1; i <= i2; i++) {
      int g = layer_get_global_cell_index( layer , i , j); 
      cell_type * cell = &layer->data[g];
      fprintf(stream , " %3d" , cell->cell_value);
    }
    fprintf(stream , "   | %3d  \n" , j);
  }

  layer_fprintf_dash( layer , stream , i1 , i2);
  layer_fprintf_header( layer , stream , i1 , i2);
}


void layer_fprintf( const layer_type * layer , FILE * stream) {
  layer_fprintf_box( layer , stream , 0 , layer->nx - 1 , 0 , layer->ny - 1);
}


void layer_fprintf_cell( const layer_type * layer , int i , int j , FILE * stream) {
  int g = layer_get_global_cell_index( layer , i , j); 
  cell_type * cell = &layer->data[g];
  
  fprintf(stream , " i:%d   j:%d  \n",i,j);
  fprintf(stream , "       *--- %4d ---* \n",cell->edges[TOP_EDGE]);
  fprintf(stream , "       |            | \n");
  fprintf(stream , "     %4d   %4d %4d\n" , cell->edges[LEFT_EDGE] , cell->cell_value , cell->edges[RIGHT_EDGE]);
  fprintf(stream , "       |            | \n");
  fprintf(stream , "       *--- %4d ---* \n",cell->edges[BOTTOM_EDGE]);
}


static bool layer_find_edge( const layer_type * layer , int *i , int *j , int value) {
  int g = layer_get_global_cell_index( layer , *i , *j); 
  cell_type * cell = &layer->data[g];
  if (cell->cell_value == value) {

    while (!layer_cell_on_edge( layer , *i , *j))
      (*i) += 1;
    
    return true;
  } else
    return false;
}


bool layer_trace_block_edge( const layer_type * layer , int start_i , int start_j , int value , struct_vector_type * corner_list , int_vector_type * cell_list) {
  int g = layer_get_global_cell_index( layer , start_i , start_j); 
  cell_type * cell = &layer->data[g];
  if ((cell->cell_value == value)) {
    int i = start_i;
    int j = start_j;
    
    if (layer_find_edge( layer , &i , &j , value)) {
      int_point2d_type start_corner;

      g = layer_get_global_cell_index( layer , i , j); 
      cell = &layer->data[g];

      start_corner.i = i;
      start_corner.j = j;
      struct_vector_reset( corner_list );
      int_vector_reset( cell_list );

      
      if (cell->edges[BOTTOM_EDGE] == value) {
        point_shift( &start_corner , 0 , 0 );
        layer_trace_block_edge__(layer , start_corner , i , j , value , BOTTOM_EDGE , corner_list , cell_list);
      } else if (cell->edges[RIGHT_EDGE] == value) {
        point_shift( &start_corner , 1 , 0 );
        layer_trace_block_edge__(layer , start_corner , i , j , value , RIGHT_EDGE , corner_list , cell_list);
      } else if (cell->edges[TOP_EDGE] == -value) {
        point_shift( &start_corner , 1 , 1 );
        layer_trace_block_edge__(layer , start_corner , i , j , value , TOP_EDGE , corner_list , cell_list);
      } else if (cell->edges[LEFT_EDGE] == -value) {
        point_shift( &start_corner , 0 , 1 );
        layer_trace_block_edge__(layer , start_corner , i  , j , value , LEFT_EDGE , corner_list , cell_list);
      } else
        util_abort("%s: what the fuck - internal error \n",__func__);
  
      int_vector_select_unique( cell_list );
      return true;
    }
  } 
  
  return false;
}
                                                                              


static void layer_trace_block_content__( layer_type * layer , bool erase , int i , int j , int value , bool * visited , int_vector_type * i_list , int_vector_type * j_list) {
  int g = layer_get_global_cell_index( layer , i , j); 
  cell_type * cell = &layer->data[g];
  if (cell->cell_value != value || visited[g])
    return;
  {
    visited[g] = true;
    if (erase)
      layer_iset_cell_value( layer , i , j , 0);
    
    int_vector_append( i_list , i );
    int_vector_append( j_list , j );

    if (i > 0)
      layer_trace_block_content__( layer , erase , i - 1 , j , value , visited , i_list , j_list);

    if (i < (layer->nx - 1))
      layer_trace_block_content__( layer , erase , i + 1 , j , value , visited , i_list , j_list);

    if (j > 0)
      layer_trace_block_content__( layer , erase , i , j - 1, value , visited , i_list , j_list);

    if (j < (layer->ny - 1))
      layer_trace_block_content__( layer , erase , i , j + 1, value , visited , i_list , j_list);
    
  }
}


static bool * layer_alloc_visited_mask( const layer_type * layer ) {
  int total_size = (layer->nx + 1)* (layer->ny + 1);
  bool * visited = util_malloc( total_size * sizeof * visited );
  int g;
  for (g = 0; g < total_size; g++)
    visited[g] = false;
  
  return visited;
}




bool layer_trace_block_content( layer_type * layer , bool erase , int start_i , int start_j , int value , int_vector_type * i_list, int_vector_type * j_list) {
  bool start_tracing = false;
  int g = layer_get_global_cell_index( layer , start_i , start_j); 
  cell_type * cell = &layer->data[g];

  if ((value == 0) && (cell->cell_value != 0))
    start_tracing = true;
  else if ((cell->cell_value == value) && (cell->cell_value != 0))
    start_tracing = true;

  if (start_tracing) {
    bool * visited = layer_alloc_visited_mask( layer );

    value = cell->cell_value;
    int_vector_reset( i_list );
    int_vector_reset( j_list );
    layer_trace_block_content__(layer , erase , start_i , start_j , value , visited , i_list , j_list );
    
    free( visited );
    return true;
  } else
    return false;
}


int layer_replace_cell_values( layer_type * layer , int old_value , int new_value) {
  int i,j;
  int replace_count = 0;

  for (j=0; j < layer->ny; j++) {
    for (i=0; i < layer->nx; i++) {
      if (layer_iget_cell_value( layer , i , j ) == old_value) {
        layer_iset_cell_value( layer , i , j , new_value);
        replace_count++;
      }
    }
  }

  return replace_count;
}
