/*
   Copyright (c) 2011  statoil asa, norway. 
    
   The file 'ecl_grid.c' is part of ert - ensemble based reservoir tool. 
    
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/double_vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/hash.h>
#include <ert/util/vector.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_coarse_cell.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/point.h>
#include <ert/ecl/tetrahedron.h>
#include <ert/ecl/grid_dims.h>


/*
  If openmp is enabled the main loop in ecl_grid_init_GRDECL_data is
  parallelized with openmp.  
*/
#ifdef HAVE_OPENMP
#include <omp.h>
#endif

/**
  this function implements functionality to load eclispe grid files,
  both .egrid and .grid files - in a transparent fashion.

  observe the following convention:

  global_index:  [0 , nx*ny*nz)
  active_index:  [0 , nactive)

  about indexing
  --------------

  there are three different ways to index/access a cell:

    1. by ijk
    2. by global index, [0 , nx*ny*nz)
    3. by active index, [0 , nactive)

  most of the query functions can take input in several of the
  ways. the expected arguments are indicated as the last part of the
  function name:

    ecl_grid_get_pos3()  - 3:  this function expects i,j,k
    ecl_grid_get_pos1()  - 1:  this function expects a global index
    ecl_grid_get_pos1a() - 1a: this function expects an active index.
    
*/


/**
   note about lgr
   --------------

   the eclipse local grid refinement (lgr) is organised as follows:

     1. you start with a normal grid.
     2. some of the cells can be subdivided into further internal
        grids, this is the lgr.

   this is illustrated below:


    +--------------------------------------+
    |            |            |            |
    |            |            |            |
    |     x      |      x     |     x      |
    |            |            |            |
    |            |            |            |
    |------------|------------|------------|
    |     |      |      |     |            |
    |     |  x   |   x  |     |            |
    |-----x------|------x-----|     x      |
    |  x  |  x   |   x  |     |            |
    |     |      |      |     |            |
    -------------|------------|------------|
    |            |            |            |
    |            |            |            |
    |     x      |            |            |
    |            |            |            |
    |            |            |            |
    +--------------------------------------+


  the grid above shows the following:

    1. the coarse (i.e. normal) grid has 9 cells, of which 7 marked
       with 'x' are active.

    2. two of the cells have been refined into new 2x2 grids. in the
       refined cells only three and two of the refined cells are
       active.

  in a grid file the keywords for this grid will look like like this:


  .....    __
  coords     \
  corners     |
  coords      |
  corners     |
  coords      |
  corners     |      normal coord / corners kewyords
  coords      |      for the nine coarse cells. observe
  corners     |      that when reading in these cells it is
  coords      |      impossible to know that some of the
  corners     |      cells will be subdivieded in a following
  coords      |      lgr definition.
  corners     |
  coords      |
  corners     |
  coords      |    
  corners     |      
  coords      |      
  corners  __/________________________________________________________  
  lgr        \
  lgrilg      |     
  dimens      |       
  coords      |
  corners     |      first lgr, with some header information, 
  coords      |      and then normal coords/corners keywords for
  corners     |      the four refined cells.
  coords      |
  corners     |
  coords      |
  corners  __/
  lgr        \
  lgrilg      |
  dimens      |
  coords      |      second lgr.
  corners     |
  coords      |
  corners     |
  coords      |
  corners     |
  coords      |
  corners  __/

  
  for egrid files it is essentially the same, except for replacing the
  keywords coords/corners with coord/zcorn/actnum. also the lgr
  headers are somewhat different.  

  solution data in restart files comes in a similar way, a restart
  file with lgr can typically look like this:

  .....   __
  .....     \ 
  startsol   |   all restart data for the ordinary
  pressure   |   grid.
  swat       |
  sgas       |
  ....       |
  endsol  __/____________________________ 
  lgr       \
  ....       |
  startsol   |   restart data for 
  pressure   |   the first lgr.
  sgas       |
  swat       |
  ...        |
  endsol     |
  endlgr  __/   
  lgr       \ 
  ....       |
  startsol   |   restart data for 
  pressure   |   the second lgr.
  sgas       |
  swat       |
  ...        |
  endsol     |
  endlgr  __/


  the lgr implementation in is based on the following main principles:

   1. when loading a egrid/grid file one ecl_grid_type instance will
      be allocated; this grid will contain the main grid, and all the
      lgr grids. 

   2. only one datatype (ecl_grid_type) is used both for the main grid
      and the lgr grids.

   3. the main grid will own (memory wise) all the lgr grids, this
      even applies to nested subgrids whose parent is also a lgr.

   4. when it comes to indexing and so on there is no difference
      between lgr grid and the main grid.


      example:
      --------

      {
         ecl_file_type * restart_data = ecl_file_fread_alloc(restart_filename , true);                      // load some restart info to inspect
         ecl_grid_type * grid         = ecl_grid_alloc(grid_filename , true);                               // bootstrap ecl_grid instance
         stringlist_type * lgr_names  = ecl_grid_alloc_name_list( grid );                                   // get a list of all the lgr names.
      
         printf("grid:%s has %d a total of %d lgr's \n", grid_filename , stringlist_get_size( lgr_names ));
         for (int lgr_nr = 0; lgr_nr < stringlist_get_size( lgr_names); lgr_nr++) {
            ecl_grid_type * lgr_grid  = ecl_grid_get_lgr( grid , stringlist_iget( lgr_names , lgr_nr ));    // get the ecl_grid instance of the lgr - by name.
            ecl_kw_type   * pressure_kw;
            int nx,ny,nz,active_size;
            ecl_grid_get_dims( lgr_grid , &nx , &ny , &nz , &active_size);                             // get some size info from this lgr.
            printf("lgr:%s has %d x %d x %d elements \n",stringlist_iget(lgr_names , lgr_nr ) , nx , ny , nz);

            // ok - now we want to extract the solution vector (pressure) corresponding to this lgr:
            pressure_kw = ecl_file_iget_named_kw( ecl_file , "pressure" , ecl_grid_get_grid_nr( lgr_grid ));
                                                                                      /|\
                                                                                       |
                                                                                       |   
                                                                        we query the lgr_grid instance to find which
                                                                        occurence of the solution data we should look
                                                                        up in the ecl_file instance with restart data. puuhh!!

            {
               int center_index = ecl_grid_get_global_index3( lgr_grid , nx/2 , ny/2 , nz/2 );          // ask the lgr_grid to get the index at the center of the lgr grid.
               printf("the pressure in the middle of %s is %g \n", stinglist_iget( lgr_names , lgr_nr ) , ecl_kw_iget_as_double( pressure_kw , center_index ));
            }
         }
         ecl_file_free( restart_data );
         ecl_grid_free( grid );
         stringlist_free( lgr_names );
     }

*/


 /*
  About coarse groups
  -------------------

  It is possible to get ECLIPSE to join several cells together as one
  coarse cell, to reduce the size of the numerical problem. In this
  implementation this is supported as follows:

    1. Each cell has an integer flag - coarse_group, which points to
       the coarse group that the current cell is part of, or the value
       COARSE_NONE for cells which are not part of a coarsening group.

    2. The details of a coarse cell is implemented in ecl_coarse_cell.c.

    3. The ecl_grid structure contains a list of ecl_coarse_cell
       instances in the coarse_cells vector.
       
    4. The introduction of coarse groups makes the concept of active
       cells slightly more complex:

         a) All the cells in the coarse group count as one (or zero)
            active cells when asking the grid how many active cells
            there are.

         b) In the case that several of the un-coarsened cells in the
            coarse group are active the mapping between global index
            and active index will no longer be unique - there will be
            several different global indices mapping to the same
            active index.
            
  The api for coarse related tasks is briefly:

    - int ecl_grid_get_num_coarse_groups( const ecl_grid_type * main_grid )
    - bool ecl_grid_have_coarse_cells( const ecl_grid_type * main_grid )
    - ecl_coarse_cell_type * ecl_grid_iget_coarse_group( const ecl_grid_type * ecl_grid , int coarse_nr );
    - ecl_coarse_cell_type * ecl_grid_get_cell_coarse_group1( const ecl_grid_type * ecl_grid , int global_index);

  In addition to the API presented by the ecl_coarse_cell.c implementation.
*/


/*
  About dual porosity
  -------------------

  Eclipse has support for dual porosity systems where the reservoir is
  made from an interleaved system of matrix blocks and fractures. The
  current implementation has some support for reading such properties
  from the grid files:

    - The active property of the cells is an integer which is a sum of
      the flag values ACTIVE_MATRIX and ACTIVE_FRACTURE. 

    - All functions operating on fracture properties have 'fracture'
      as part of the name. The functions operating on the matrix do
      (typically) not have matrix as part of the name. The matrix
      properties are the default properties which apply in the single
      porosity case.

    - In the EGRID files the dual porosity properties are set in the
      ACTNUM field in the file. The numerical values are [0,1,2,3] for
      inactive cells, matrix active, fracture active and
      matrix+fracture active respectively.

    - For the GRID files there is abolutely no metadata to tell that
      this is a dual porosity run (I think ...) - instead the whole
      grid is repeated one more time with cells for the fractures
      following after the matrix cells. 
      
      Naively the GRID file of a dual porosity run will report that it
      contains 2*NZ layers. In the current implementation heuristics
      is used to detect the situation, and the grid will only be
      loaded as consisting of 'NZ' geometric layers.

    - The documentation seems to indicate that the number of active
      fracture cells can in general be different from the number of
      active matrix cells, and the current implementation takes pains
      to support that possibility - but all examples I have come over
      have nactive_fracture == nactive_matrix?

    - Properties and solution data in restart/init/grdecl files are
      not treated here. These properties will just increase (typically
      double) in size - and how to treat them will be a question of
      convention. The following shows a possible solution:

      {
         char fracture_kw[9];
         char matrix_kw[9];
         int  matrix_size   = ecl_grid_get_nactive( ecl_grid );
         int  fracture_size = ecl_grid_get_nactive_fracture( ecl_grid );

         swat = ecl_file_iget_name_kw( rst_file , "SWAT" , 0);

         snsprintf(fracture_kw , 9 , "F-%6s" , ecl_kw_get_header( swat ));
         snsprintf(matrix_kw   , 9 , "M-%6s" , ecl_kw_get_header( swat ));

         ecl_kw_type * M = ecl_kw_alloc_sub_copy( swat , matrix_kw   , 0  , matrix_size );
         ecl_kw_type * F = ecl_kw_alloc_sub_copy( swat , fracture_kw , matrix_size  , fracture_size );
      }
*/



/*
  about tetraheder decomposition
  ------------------------------

  the table tetraheder_permutations describe how the cells can be
  divided into twelve tetrahedrons. the dimensions in the the table
  are as follows:

   1. the first dimension is the "way" the cell is divided into
      tetrahedrons, there are two different ways. for cells where the
      four point corners on a face are not in the same plane, the two
      methods will not give the same result. which one is "right"??

   2. the second dimension is the tetrahedron number, for each way of
      the two ways there are a total of twelve tetrahedrons.

   3. the third and list dimension is the point number in this
      tetrahedron. when forming a tetrahedron the first input point
      should always be the point corresponding to center of the
      cell. that is not explicit in this table.

   i.e. for instance the third tetrahedron for the first method
   consists of the cells:

        tetraheheder_permutations[0][2] = {0 , 4 , 5}

   in addition to the central point. the value [0..7] correspond the
   the number scheme of the corners in a cell used by eclipse:


       lower layer:   upper layer  (higher value of z - i.e. lower down in resrvoir).
                    
         2---3           6---7
         |   |           |   |
         0---1           4---5


   table entries are ripped from eclpost code - file: kvpvos.f in
   klib/
*/


static const int tetrahedron_permutations[2][12][3] = {{{0 , 2 , 6},
                                                        {0 , 4 , 6},
                                                        {0 , 4 , 5},
                                                        {0 , 1 , 5},
                                                        {1 , 3 , 7},
                                                        {1 , 5 , 7},
                                                        {2 , 3 , 7},
                                                        {2 , 6 , 7},
                                                        {0 , 1 , 2},
                                                        {1 , 2 , 3},
                                                        {4 , 5 , 6},
                                                        {5 , 6 , 7}},
                                                       {{0 , 2 , 4},  
                                                        {2 , 4 , 6},  
                                                        {0 , 4 , 1},  
                                                        {4 , 5 , 1},  
                                                        {1 , 3 , 5},  
                                                        {3 , 5 , 7},  
                                                        {2 , 3 , 6},   
                                                        {3 , 6 , 7},   
                                                        {0 , 1 , 3},  
                                                        {0 , 2 , 3},  
                                                        {4 , 5 , 7},  
                                                        {4 , 6 , 7}}};

/*

  the implementation is based on a hierarchy of three datatypes:

   1. ecl_grid   - this is the only exported datatype
   2. ecl_cell   - internal
   3. point      - implemented in file point.c

*/

/*
  Indices used in the cell->active_index[] array.
*/
#define MATRIX_INDEX   0
#define FRACTURE_INDEX 1

#define COARSE_GROUP_NONE  -1
#define HOST_CELL_NONE     -1

#define CELL_FLAG_VALID    1     /* In the case of GRID files not necessarily all cells geometry values set - in that case this will be left as false. */
#define CELL_FLAG_CENTER   2     /* Has the center value been calculated - this is by default not done to speed up loading a tiny bit. */
#define CELL_FLAG_TAINTED  4     /* lazy fucking stupid reservoir engineers make invalid grid
                                    cells - for kicks??  must try to keep those cells out of
                                    real-world calculations with some hysteric heuristics.*/

typedef struct ecl_cell_struct           ecl_cell_type;

#define GET_CELL_FLAG(cell,flag) (((cell->cell_flags & (flag)) == 0) ? false : true)
#define SET_CELL_FLAG(cell,flag) ((cell->cell_flags |= (flag)))

struct ecl_cell_struct {
  point_type center;
  point_type corner_list[8];

  
  int                    active;
  int                    active_index[2];    /* [0]: The active matrix index; [1]: the active fracture index */
  const ecl_grid_type   *lgr;                /* if this cell is part of an lgr; this will point to a grid instance for that lgr; NULL if not part of lgr. */
  int                    host_cell;          /* the global index of the host cell for an lgr cell, set to -1 for normal cells. */
  int                    coarse_group;       /* The index of the coarse group holding this cell -1 for non-coarsened cells. */
  int                    cell_flags;
};  



#define LARGE_CELL_MALLOC 1
#define ECL_GRID_ID       991010

struct ecl_grid_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                   grid_nr;       /* this corresponds to item 4 in gridhead - 0 for the main grid. */ 
  char                * name;          /* the name of the file for the main grid - name of the lgr for lgrs. */
  int                   ny,nz,nx;
  int                   size;          /* == nx*ny*nz */
  int                   total_active; 
  int                   total_active_fracture;
  bool                * visited;                /* internal helper struct used when searching for index - can be NULL. */
  int                 * index_map;              /* this a list of nx*ny*nz elements, where value -1 means inactive cell .*/
  int                 * inv_index_map;          /* this is list of total_active elements - which point back to the index_map. */

  int                 * fracture_index_map;     /* For fractures: this a list of nx*ny*nz elements, where value -1 means inactive cell .*/
  int                 * inv_fracture_index_map; /* For fractures: this is list of total_active elements - which point back to the index_map. */ 

#ifdef LARGE_CELL_MALLOC
  ecl_cell_type      *  cells;
#else
  ecl_cell_type      ** cells;         
#endif

  char                * parent_name;   /* the name of the parent for a nested lgr - for the main grid, and also a
                                          lgr descending directly from the main grid this will be NULL. */
  hash_type           * children;      /* a table of lgr children for this grid. */
  const ecl_grid_type * parent_grid;   /* the parent grid for this (lgr) - NULL for the main grid. */      
  const ecl_grid_type * global_grid;   /* the global grid - NULL for the main grid. */
  
  bool                  coarsening_active;
  vector_type         * coarse_cells;
  /*
    the two fields below are for *storing* lgr grid instances. observe
    that these fields will be NULL for lgr grids, i.e. grids with
    grid_nr > 0. 
  */
  vector_type         * LGR_list;      /* a vector of ecl_grid instances for lgr's - the index corresponds to the grid_nr. */
  hash_type           * LGR_hash;      /* a hash of pointers to ecl_grid instances - for name based lookup of lgr. */
  int                   parent_box[6]; /* integers i1,i2, j1,j2, k1,k2 of the parent grid region containing this lgr. the indices are inclusive - zero offset */
                                       /* not used yet .. */ 
  
  int                   dualp_flag;    
  bool                  use_mapaxes;
  double                unit_x[2];
  double                unit_y[2];
  double                origo[2];
  float                 mapaxes[6];
  /*------------------------------:       the fields below this line are used for blocking algorithms - and not allocated by default.*/
  int                    block_dim; /* == 2 for maps and 3 for fields. 0 when not in use. */
  int                    block_size;
  int                    last_block_index;
  double_vector_type  ** values;
  ecl_kw_type          * coord_kw;   /* Retained for writing the grid to file.
                                        In principal it should be possible to 
                                        recalculate this from the cell coordinates,
                                        but in cases with skewed cells this has proved
                                        numerically challenging. */
};





static void ecl_cell_compare(const ecl_cell_type * c1 , ecl_cell_type * c2, bool * equal) {
  int i;
  
  if (c1->active != c2->active) 
    *equal = false;


  if (c1->active_index[0] != c2->active_index[0])
    *equal = false;

  if (c1->active_index[1] != c2->active_index[1])
    *equal = false;

  if (c1->coarse_group != c2->coarse_group)
    *equal = false;
  
  if (c1->host_cell != c2->host_cell) 
    *equal = false;

  if (*equal) {
    for (i=0; i < 8; i++) 
      point_compare( &c1->corner_list[i] , &c2->corner_list[i] , equal );
    
  }
}


static void ecl_cell_dump( const ecl_cell_type * cell , FILE * stream) {
  int i;
  for (i=0; i < 8; i++)
    point_dump( &cell->corner_list[i] , stream );
}


static void ecl_cell_dump_ascii( const ecl_cell_type * cell , int i , int j , int k , FILE * stream) {
  fprintf(stream , "Cell: i:%3d  j:%3d    k:%3d   CoarseGroup:%4d active_nr:%6d\nCorners:\n",i,j,k,cell->coarse_group , cell->active_index[MATRIX_INDEX]);
  {
    int l;
    for (l=0; l < 8; l++) {
      fprintf(stream , "Corner %d : ",l);
      point_dump_ascii( &cell->corner_list[l] , stream );
      fprintf(stream , "\n");
    }
  }
  fprintf(stream , "\n");
}


static void ecl_cell_fwrite_GRID( const ecl_grid_type * grid , const ecl_cell_type * cell , bool fracture_cell , int coords_size , int i, int j , int k , int global_index , ecl_kw_type * coords_kw , ecl_kw_type * corners_kw, fortio_type * fortio) {
  ecl_kw_iset_int( coords_kw , 0 , i + 1);
  ecl_kw_iset_int( coords_kw , 1 , j + 1);
  ecl_kw_iset_int( coords_kw , 2 , k + 1);
  ecl_kw_iset_int( coords_kw , 3 , global_index + 1);

  ecl_kw_iset_int( coords_kw , 4 , 0);
  if (fracture_cell) {
    if (cell->active & ACTIVE_FRACTURE)
      ecl_kw_iset_int( coords_kw , 4 , 1);
  } else {
    if (cell->active & ACTIVE_MATRIX)
      ecl_kw_iset_int( coords_kw , 4 , 1);
  }
  
  if (coords_size == 7) {
    ecl_kw_iset_int( coords_kw , 5 , cell->host_cell + 1);
    ecl_kw_iset_int( coords_kw , 6 , cell->coarse_group + 1);
  }

  ecl_kw_fwrite( coords_kw , fortio );
  {
    float * corners = ecl_kw_get_void_ptr( corners_kw );
    point_type point;
    int c;
    
    for (c = 0; c < 8; c++) {
      point_copy_values( &point , &cell->corner_list[c] );
      if (grid->use_mapaxes)
        point_mapaxes_invtransform( &point , grid->origo , grid->unit_x , grid->unit_y );
      
      corners[3*c ]    = point.x;
      corners[3*c + 1] = point.y;
      corners[3*c + 2] = point.z;
    }
  }
  ecl_kw_fwrite( corners_kw , fortio );
}

//static const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };
static void ecl_cell_ri_export( const ecl_cell_type * cell , double * ri_points) {
  int ecl_offset = 4;
  int ri_offset =  ecl_offset * 3;
  {
    int point_nr;
    // Handling the points 0,1 & 4,5 which map directly between ECLIPSE and RI
    for (point_nr =0; point_nr < 2; point_nr++) {
      // Points 0 & 1
      ri_points[ point_nr * 3     ] =  cell->corner_list[point_nr].x;
      ri_points[ point_nr * 3 + 1 ] =  cell->corner_list[point_nr].y;
      ri_points[ point_nr * 3 + 2 ] = -cell->corner_list[point_nr].z;

      // Points 4 & 5
      ri_points[ ri_offset + point_nr * 3     ] =  cell->corner_list[ecl_offset + point_nr].x;
      ri_points[ ri_offset + point_nr * 3 + 1 ] =  cell->corner_list[ecl_offset + point_nr].y;
      ri_points[ ri_offset + point_nr * 3 + 2 ] = -cell->corner_list[ecl_offset + point_nr].z;
    }
  }

  {
    int ecl_point;
    /*
      Handling the points 2,3 & 6,7 which are flipped when (2,3) ->
      (3,2) and (6,7) -> (7,6) when going between ECLIPSE and ResInsight.
    */
    for (ecl_point =2; ecl_point < 4; ecl_point++) {
      int ri_point = 5 - ecl_point;
      // Points 2 & 3
      ri_points[ ri_point * 3     ] =  cell->corner_list[ecl_point].x;
      ri_points[ ri_point * 3 + 1 ] =  cell->corner_list[ecl_point].y;
      ri_points[ ri_point * 3 + 2 ] = -cell->corner_list[ecl_point].z;
      
      
      // Points 6 & 7
      ri_points[ ri_offset + ri_point * 3     ] =  cell->corner_list[ecl_offset + ecl_point].x;
      ri_points[ ri_offset + ri_point * 3 + 1 ] =  cell->corner_list[ecl_offset + ecl_point].y;
      ri_points[ ri_offset + ri_point * 3 + 2 ] = -cell->corner_list[ecl_offset + ecl_point].z;
    }
  }
}

/*****************************************************************/

static double max2( double x1 , double x2) {
  return (x1 > x2) ? x1 : x2;
}  


static double min2( double x1 , double x2) {
  return  (x1 < x2) ? x1 : x2;
}  


static  double min4(double x1 , double x2 , double x3 , double x4) {
  return min2( min2(x1 , x2) , min2(x3 , x4 ));
}

static  double max4(double x1 , double x2 , double x3 , double x4) {
  return max2( max2(x1 , x2) , max2(x3 , x4 ));
}

static  double max8( double x1 , double x2 , double x3, double x4 , double x5 , double x6 , double x7 , double x8) {
  return max2( max4(x1,x2,x3,x4) , max4(x5,x6,x7,x8));
}

static  double min8( double x1 , double x2 , double x3, double x4 , double x5 , double x6 , double x7 , double x8) {
  return min2( min4(x1,x2,x3,x4) , min4(x5,x6,x7,x8));
}

/*****************************************************************/

static double ecl_cell_min_z( const ecl_cell_type * cell) {
  return min4( cell->corner_list[0].z , cell->corner_list[1].z , cell->corner_list[2].z , cell->corner_list[3].z);
}

static double ecl_cell_max_z( const ecl_cell_type * cell ) {
  return max4( cell->corner_list[4].z , cell->corner_list[5].z , cell->corner_list[6].z , cell->corner_list[7].z );
}


/**
   the grid can be rotated so that it is not safe to consider only one
   plane for the x/y min/max.
*/

static double ecl_cell_min_x( const ecl_cell_type * cell) {
  return min8( cell->corner_list[0].x , cell->corner_list[1].x , cell->corner_list[2].x , cell->corner_list[3].x,
               cell->corner_list[4].x , cell->corner_list[5].x , cell->corner_list[6].x , cell->corner_list[7].x );
}


static double ecl_cell_max_x( const ecl_cell_type * cell ) {
  return max8( cell->corner_list[0].x , cell->corner_list[1].x , cell->corner_list[2].x , cell->corner_list[3].x,
               cell->corner_list[4].x , cell->corner_list[5].x , cell->corner_list[6].x , cell->corner_list[7].x );
}

static double ecl_cell_min_y( const ecl_cell_type * cell) {
  return min8( cell->corner_list[0].y , cell->corner_list[1].y , cell->corner_list[2].y , cell->corner_list[3].y,
               cell->corner_list[4].y , cell->corner_list[5].y , cell->corner_list[6].y , cell->corner_list[7].y );
}


static double ecl_cell_max_y( const ecl_cell_type * cell ) {
  return max8( cell->corner_list[0].y , cell->corner_list[1].y , cell->corner_list[2].y , cell->corner_list[3].y,
               cell->corner_list[4].y , cell->corner_list[5].y , cell->corner_list[6].y , cell->corner_list[7].y );
}



/**
   The problem is that some extremely fucking stupid reservoir
   engineers purpousely have made grids with invalid cells. Typically
   the cells accomodating numerical aquifers are located at an utm
   position (0,0).

   Cells which have some pillars located in (0,0) and some cells
   located among the rest of the grid become completely warped - with
   insane volumes, parts of the reservoir volume doubly covered, and
   so on.
   
   To keep these cells out of the real-world (i.e. involving utm
   coordinates) computations they are marked as 'tainted' in this
   function. The tainting procedure is completely heuristic, and
   probably wrong.
   
   --------------

   There is second heuristic which marks cells as invalid. In some
   cases (which ??) cells outside the area of interest are just set to
   have all four corner at the same arbitrary depth; these cells are
   inactive and do not affect flow simulations - however the arbitrary
   location of the cells warps visualisation of normal inactive cells
   completely. We therefor try to invalidate such cells here. The
   algorithm used is the same as used for RMS; however RMS will mark
   the cells as inactive - whereas we mark already inactive cells as
   invalid. 

   It is very important that this mechanism is NOT used to set cells
   to inactive; as that will break completely when it comes to
   e.g. data like PRESSURE and SWAT which are defined as a number of
   active elements.
 */


static void ecl_cell_taint_cell( ecl_cell_type * cell ) {
  int c;
  for (c = 0; c < 8; c++) {
    const point_type p = cell->corner_list[c];
    if ((p.x == 0) && (p.y == 0)) {
      SET_CELL_FLAG(cell , CELL_FLAG_TAINTED);
      break;
    }
  }

  /*
    Second heuristic to invalidate cells.
  */
  if (cell->active == INACTIVE) {
    if (!GET_CELL_FLAG(cell , CELL_FLAG_TAINTED)) {
      const point_type p0 = cell->corner_list[0];
      int cell_index = 1;
      while (true) {
        const point_type pi = cell->corner_list[cell_index];
        if (pi.z != p0.z)
          // There is a difference - the cell is certainly valid.
          break; 
        else {
          cell_index++;
          if (cell_index == 8) {
            // They have all been at the same height up until now;
            // the cell is marked as invalid.
            SET_CELL_FLAG(cell , CELL_FLAG_TAINTED);
            break;
          }
        }
      }
    }
  }
}


/*****************************************************************/




/**
   Observe that when allocating based on a grid file not all cells are
   necessarily accessed beyond this function. In general not all cells
   will have a coords/corners section in the grid file.  
*/

static void ecl_cell_init( ecl_cell_type * cell , bool init_valid) {
  cell->active                = INACTIVE;  
  cell->lgr                   = NULL;
  cell->host_cell             = HOST_CELL_NONE;
  cell->coarse_group          = COARSE_GROUP_NONE;
  cell->cell_flags            = 0;
  cell->active_index[MATRIX_INDEX]   = -1;
  cell->active_index[FRACTURE_INDEX] = -1;
  
  if (init_valid)
    cell->cell_flags = CELL_FLAG_VALID;
}


static void ecl_cell_set_center( ecl_cell_type * cell) {
  point_set(&cell->center , 0 , 0 , 0);
  {
    int c;
    for (c = 0; c < 8; c++)
      point_inplace_add(&cell->center , &cell->corner_list[c]);
  }
  point_inplace_scale(&cell->center , 1.0 / 8.0);
  SET_CELL_FLAG( cell , CELL_FLAG_CENTER );
}


static void ecl_cell_assert_center( ecl_cell_type * cell) {
  if (!GET_CELL_FLAG(cell , CELL_FLAG_CENTER))
    ecl_cell_set_center( cell );
}



static void ecl_cell_memcpy( ecl_cell_type * target_cell , const ecl_cell_type * src_cell ) {
  memcpy( target_cell , src_cell , sizeof * target_cell );
}

static void ecl_cell_install_lgr( ecl_cell_type * cell , const ecl_grid_type * lgr_grid) {
  cell->lgr       = lgr_grid;
}




static void ecl_cell_init_tetrahedron( const ecl_cell_type * cell , tetrahedron_type * tet , int method_nr , int tetrahedron_nr) {
  int point0 = tetrahedron_permutations[ method_nr ][ tetrahedron_nr ][ 0 ];
  int point1 = tetrahedron_permutations[ method_nr ][ tetrahedron_nr ][ 1 ];
  int point2 = tetrahedron_permutations[ method_nr ][ tetrahedron_nr ][ 2 ];

  tetrahedron_init( tet , &cell->center , &cell->corner_list[ point0 ] , &cell->corner_list[point1] , &cell->corner_list[point2]);
}



static double ecl_cell_get_volume( ecl_cell_type * cell ) {
  ecl_cell_assert_center( cell );
  {
    tetrahedron_type tet;
    int              itet;
    double           volume = 0;
    for (itet = 0; itet < 12; itet++) {
      /* 
         using both tetrahedron decompositions - gives good agreement
         with porv from eclipse init files.
      */
      ecl_cell_init_tetrahedron( cell , &tet , 0 , itet );
      volume += tetrahedron_volume( &tet );
      
      ecl_cell_init_tetrahedron( cell , &tet , 1 , itet );
      volume += tetrahedron_volume( &tet );
    }
    
    return volume * 0.5;
  }
}




static double triangle_area(double x1 , double y1 , double x2 , double y2 , double x3 ,double y3) {
  return fabs(x1*y2 + x2*y3 + x3*y1 - x1*y3 - x3*y2 - x2*y1)*0.5;
}


static bool triangle_contains(const point_type *p0 , const point_type * p1 , const point_type *p2 , double x , double y) {
  double epsilon = 1e-10;

  double vt = triangle_area(p0->x , p0->y,
                            p1->x , p1->y,
                            p2->x , p2->y);

  if (vt < epsilon)  /* zero size cells do not contain anything. */
    return false;
  {
    double v1 = triangle_area(p0->x , p0->y,
                              p1->x , p1->y,
                              x     , y); 
    
    double v2 = triangle_area(p0->x , p0->y,
                              x     , y,
                              p2->x , p2->y);
    
    double v3 = triangle_area(x     , y,
                              p1->x , p1->y,
                              p2->x , p2->y);
    
    
    if (fabs( vt - (v1 + v2 + v3 )) < epsilon)
      return true;
    else
      return false;
  }
}




/* 
   if the layer defined by the cell corners 0-1-2-3 (lower == true) or
   4-5-6-7 (lower == false) contain the point (x,y) the function will
   return true - otehrwise false.
   
   the function works by dividing the cell face into two triangles,
   which are checked one at a time with the function
   triangle_contains().
*/


static bool ecl_cell_layer_contains_xy( const ecl_cell_type * cell , bool lower_layer , double x , double y) {
  if (GET_CELL_FLAG(cell,CELL_FLAG_TAINTED))
    return false;
  {
    const point_type *p0,*p1,*p2,*p3;
    {
      int corner_offset;
      if (lower_layer) 
        corner_offset = 0;
      else
        corner_offset = 4;
      
      p0 = &cell->corner_list[corner_offset + 0];
      p1 = &cell->corner_list[corner_offset + 1];
      p2 = &cell->corner_list[corner_offset + 2];
      p3 = &cell->corner_list[corner_offset + 3];
    }
    
    if (triangle_contains(p0,p1,p2,x,y))
      return true;
    else
      return triangle_contains(p1,p2,p3,x,y);
  }
}

/*
deeper layer: (larger (negative) z values).
------------

  6---7
  |   |
  4---5


  2---3
  |   |
  0---1


*/

  


static bool ecl_cell_contains_point( ecl_cell_type * cell , const point_type * p) {
  /*
    1. first check if the point z value is below the deepest point of
       the cell, or above the shallowest => return false.

    2. should do similar fast checks in x/y direction.

    3. full geometric verification.
  */

  if (GET_CELL_FLAG(cell , CELL_FLAG_TAINTED))
    return false;
  
  if (p->z < ecl_cell_min_z( cell ))
    return false;
  
  if (p->z > ecl_cell_max_z( cell ))
    return false;

  if (p->x < ecl_cell_min_x( cell ))
    return false;
  
  if (p->x > ecl_cell_max_x( cell ))
    return false;
  
  if (p->y < ecl_cell_min_y( cell ))
    return false;
  
  if (p->y > ecl_cell_max_y( cell ))
    return false;
  
  

  {
    const int method   = 0;
    int tetrahedron_nr = 0;
    tetrahedron_type tet;
    
    if (ecl_cell_get_volume( cell ) > 0) {
      /* does never exit from this loop - only returns from the whole function. */
      while (true) {   
        ecl_cell_init_tetrahedron( cell , &tet , method , tetrahedron_nr );
        if (tetrahedron_contains( &tet , p )) 
          return true;
        
        tetrahedron_nr++;
        if (tetrahedron_nr == 12)
          return false;  /* ok - cell did not contain point. */
      } 
    } 
  }
  util_abort("%s: internal error - should not be here \n",__func__);
  return false;
}



/**
       lower layer:   upper layer  
                    
         2---3           6---7
         |   |           |   |
         0---1           4---5
*/
static void ecl_cell_init_regular( ecl_cell_type * cell , const double * offset , int i , int j , int k , int global_index , const double * ivec , const double * jvec , const double * kvec , const int * actnum ) {
  {
    double x0 = offset[0] + i*ivec[0] + j*jvec[0] + k*kvec[0];
    double y0 = offset[1] + i*ivec[1] + j*jvec[1] + k*kvec[1];
    double z0 = offset[2] + i*ivec[2] + j*jvec[2] + k*kvec[2];
    
    point_set(&cell->corner_list[0] , x0 , y0 , z0 );                // Point 0
  } 
  cell->corner_list[1] = cell->corner_list[0];                       // Point 1
  point_shift(&cell->corner_list[1] , ivec[0] , ivec[1] , ivec[2]);

  cell->corner_list[2] = cell->corner_list[0];                       // Point 2
  point_shift(&cell->corner_list[2] , jvec[0] , jvec[1] , jvec[2]);
  
  cell->corner_list[3] = cell->corner_list[1];                       // Point 3
  point_shift(&cell->corner_list[3] , jvec[0] , jvec[1] , jvec[2]);


  {
    int i;
    for (i=0; i < 4; i++) {
      cell->corner_list[i+4] = cell->corner_list[i];                      // Point 4-7
      point_shift(&cell->corner_list[i+4] , kvec[0] , kvec[1] , kvec[2]);
    }
  }

  if (actnum != NULL) 
    cell->active = actnum[global_index];
  else
    cell->active = ACTIVE;
}


/* end of cell implementation                                    */
/*****************************************************************/
/* starting on the ecl_grid proper implementation                */

UTIL_SAFE_CAST_FUNCTION(ecl_grid , ECL_GRID_ID);

/**
   this function allocates the internal index_map and inv_index_map fields.
*/



static ecl_cell_type * ecl_grid_get_cell(const ecl_grid_type * grid , int global_index) {
#ifdef LARGE_CELL_MALLOC
  return &grid->cells[global_index];
#else
  return grid->cells[global_index];
#endif
}


/**
   this function uses heuristics (ahhh - i hate it) in an attempt to
   mark cells with fucked geometry - see further comments in the
   function ecl_cell_taint_cell() which actually does it.
*/

static void ecl_grid_taint_cells( ecl_grid_type * ecl_grid ) {
  int index;
  for (index = 0; index < ecl_grid->size; index++) {
    ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , index );
    ecl_cell_taint_cell( cell );
  }
}


static void ecl_grid_free_cells( ecl_grid_type * grid ) {
#ifndef LARGE_CELL_MALLOC
  int i;
  for (i=0; i < grid->size; i++) {
    ecl_cell_type * cell = ecl_grid_get_cell( grid , i );
    ecl_cell_free( cell );
  }
#endif
  free( grid->cells );
}

static void ecl_grid_alloc_cells( ecl_grid_type * grid , bool init_valid) {
  grid->cells           = util_calloc(grid->size , sizeof * grid->cells );
#ifndef LARGE_CELL_MALLOC
  {
    int i;
    for (i=0; i < grid->size; i++) 
      grid->cells[i] = ecl_cell_alloc();
  }
#endif
  {
    ecl_cell_type * cell0 = ecl_grid_get_cell( grid , 0 );
    ecl_cell_init( cell0 , init_valid );
    {
      int i;
      for (i=1; i < grid->size; i++) {
        ecl_cell_type * target_cell = ecl_grid_get_cell( grid , i );
        ecl_cell_memcpy( target_cell , cell0 );
      }
    }
  }
}

/**
   will create a new blank grid instance. if the global_grid argument
   is != NULL the newly created grid instance will copy the mapaxes
   transformations; and set the global_grid pointer of the new grid
   instance. apart from that no further lgr-relationsip initialisation
   is performed.
*/

static ecl_grid_type * ecl_grid_alloc_empty(ecl_grid_type * global_grid , int dualp_flag , int nx , int ny , int nz, int grid_nr , bool init_valid) {
  ecl_grid_type * grid = util_malloc(sizeof * grid );
  UTIL_TYPE_ID_INIT(grid , ECL_GRID_ID);
  grid->total_active   = 0;
  grid->total_active_fracture = 0;
  grid->nx                    = nx;
  grid->ny                    = ny;
  grid->nz                    = nz;
  grid->size                  = nx*ny*nz;
  grid->grid_nr               = grid_nr;
  grid->global_grid           = global_grid;
  grid->coarsening_active     = false;

  grid->dualp_flag            = dualp_flag;
  grid->coord_kw              = NULL;
  grid->visited               = NULL;
  grid->inv_index_map         = NULL;
  grid->index_map             = NULL; 
  grid->fracture_index_map    = NULL;
  grid->inv_fracture_index_map = NULL;
  ecl_grid_alloc_cells( grid , init_valid );


  if (global_grid != NULL) {
    /* this is an lgr instance, and we inherit the global grid
       transformations from the main grid. */
    grid->unit_x[0]     = global_grid->unit_x[0];  
    grid->unit_x[1]     = global_grid->unit_x[1];  
    grid->unit_y[0]     = global_grid->unit_y[0];  
    grid->unit_y[1]     = global_grid->unit_y[1];  
    grid->origo[0]      = global_grid->origo[0];   
    grid->origo[1]      = global_grid->origo[1];   
    grid->use_mapaxes   = global_grid->use_mapaxes;
  } else {
    grid->unit_x[0]     = 1;
    grid->unit_x[1]     = 0;
    grid->unit_y[0]     = 0;
    grid->unit_y[1]     = 1;
    grid->origo[0]      = 0;
    grid->origo[1]      = 0;
    grid->use_mapaxes   = false;
  }


  grid->block_dim      = 0;
  grid->values         = NULL;
  if (grid_nr == 0) {  /* this is the main grid */
    grid->LGR_list = vector_alloc_new(); 
    vector_append_ref( grid->LGR_list , grid ); /* adding a 'self' pointer as first inistance - without destructor! */
    grid->LGR_hash = hash_alloc();
  } else {
    grid->LGR_list     = NULL;
    grid->LGR_hash     = NULL;
  }
  grid->name         = NULL;
  grid->parent_name  = NULL;
  grid->parent_grid  = NULL;
  grid->children     = hash_alloc();
  grid->coarse_cells = vector_alloc_new();
  return grid;
}




static  int ecl_grid_get_global_index__(const ecl_grid_type * ecl_grid , int i , int j , int k) {
  return i + j * ecl_grid->nx + k * ecl_grid->nx * ecl_grid->ny;
}


static void ecl_grid_set_cell_EGRID(ecl_grid_type * ecl_grid , int i, int j , int k , 
                                    double x[4][2] , double y[4][2] , double z[4][2] , 
                                    const int * actnum, const int * corsnum) {
  
  const int global_index   = ecl_grid_get_global_index__(ecl_grid , i , j  , k );
  ecl_cell_type * cell     = ecl_grid_get_cell( ecl_grid , global_index );
  int ip , iz;

  for (iz = 0; iz < 2; iz++) {
    for (ip = 0; ip < 4; ip++) {
      int c = ip + iz * 4;
      point_set(&cell->corner_list[c] , x[ip][iz] , y[ip][iz] , z[ip][iz]);

      if (ecl_grid->use_mapaxes)
        point_mapaxes_transform( &cell->corner_list[c] , ecl_grid->origo , ecl_grid->unit_x , ecl_grid->unit_y );
    }
  }

    


  /*
    If actnum == NULL that is taken to mean active.

    for normal runs actnum will be 1 for active cells,
    for dual porosity models it can also be 2 and 3.
  */
  if (actnum == NULL)
    cell->active = ACTIVE;
  else 
    cell->active = actnum[global_index];
  
  if (corsnum != NULL)
    cell->coarse_group = corsnum[ global_index ] - 1;
}


static void ecl_grid_set_cell_GRID(ecl_grid_type * ecl_grid , int coords_size , const int * coords , const float * corners) {
  
  const int i  = coords[0] - 1; /* eclipse 1 offset */
  const int j  = coords[1] - 1;
  int k  = coords[2] - 1;
  int global_index;
  ecl_cell_type * cell;
  bool matrix_cell = true;
  int active_value = ACTIVE_MATRIX;

  /*
    This is the rather hysterical treatment of dual porosity in qGRID
    files. Cells with k >= nz consitute the fracture part of the
    grid. For these cell the cell properties are not recalculated, but
    the active flag is updated to include the active|inactive
    properties of the fracture.
  */

  if (k >= ecl_grid->nz) {
    k -= ecl_grid->nz;
    matrix_cell = false;
    active_value = ACTIVE_FRACTURE;
  }


  global_index = ecl_grid_get_global_index__(ecl_grid , i, j , k);
  cell = ecl_grid_get_cell( ecl_grid , global_index);
  
  /* the coords keyword can optionally contain 4,5 or 7 elements:

        coords[0..2] = i,j,k
        coords[3]    = global_cell number (not used here)
        ----
        coords[4]    = 1,0 for active/inactive cells. Does NOT differentiate between matrix and fracture cells.
        coords[5]    = 0 for normal cells, icell of host cell for lgr cell.
        coords[6]    = 0 for normal cells, coarsening group for coarsened cell [not treated yet].

        if coords[4] is not present it is assumed that the cell is active.

        Note about LGRs: The GRID file format has an additional
        keyword called LGRILG which maps out the parent box containing
        an LGR - this only duplicates the information which can be
        learned from the coords[5] element, and is not used in the
        current code - however in good Schlum tradition a file with
        4/5 element COORDS keywords and LGRs might come any day - then
        we are fucked.
  */

  {
    int c;


    /* 
       The grid_nr > 0 test essentially checks if this is a LGR; if
       this test applies we either have bug - or a GRID file with LGRs
       and only 4/5 elements in the coords keywords. In the latter
       case we must start using the LGRILG keyword.
    */
    if ((ecl_grid->grid_nr > 0) && (coords_size != 7)) 
      util_abort("%s: Need 7 element coords keywords for LGR - or reimplement to use LGRILG keyword.\n",__func__);
    
    switch(coords_size) {
    case(4):                /* all cells active */
      cell->active += active_value;
      break;
    case(5):                /* only spesific cells active - no lgr */
      cell->active  += coords[4] * active_value;
      break;
    case(7):
      cell->active      += coords[4] * active_value;
      cell->host_cell    = coords[5] - 1;
      cell->coarse_group = coords[6] - 1;
      if (cell->coarse_group >= 0)
        ecl_grid->coarsening_active = true;
      break;
    default:
      util_abort("%s: coord size:%d unrecognized - should 4,5 or 7.\n",__func__ , coords_size);
    }
    
    if (matrix_cell) {
      for (c = 0; c < 8; c++) {
        point_set(&cell->corner_list[c] , corners[3*c] , corners[3*c + 1] , corners[3*c + 2]);
        if (ecl_grid->use_mapaxes)
          point_mapaxes_transform( &cell->corner_list[c] , ecl_grid->origo , ecl_grid->unit_x , ecl_grid->unit_y );
      }
    }
  }
  SET_CELL_FLAG(cell , CELL_FLAG_VALID );
}


/**
   The function ecl_grid_set_active_index() must be called immediately
   prior to calling this function, to ensure that
   ecl_grid->total_active is correct.
*/

static void ecl_grid_init_index_map__( ecl_grid_type * ecl_grid , int * index_map , int * inv_index_map , int active_mask, int type_index) {
  int global_index;

  for (global_index = 0; global_index < ecl_grid->size; global_index++) {     
    const ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);
    if (cell->active & active_mask) {
      index_map[global_index] = cell->active_index[type_index];
      
      if (cell->coarse_group == COARSE_GROUP_NONE)
        inv_index_map[cell->active_index[type_index]] = global_index;
      //else: In the case of coarse groups the inv_index_map is set below.
    } else
      index_map[global_index] = -1;
  }

}


static void ecl_grid_realloc_index_map(ecl_grid_type * ecl_grid) {
  /* Creating the inverse mapping for the matrix cells. */
  {
    ecl_grid->index_map     = util_realloc(ecl_grid->index_map     , ecl_grid->size                * sizeof * ecl_grid->index_map     );
    ecl_grid->inv_index_map = util_realloc(ecl_grid->inv_index_map , ecl_grid->total_active * sizeof * ecl_grid->inv_index_map );
    ecl_grid_init_index_map__( ecl_grid , ecl_grid->index_map , ecl_grid->inv_index_map , ACTIVE_MATRIX , MATRIX_INDEX);
  }

  /* Create the inverse mapping for the fractures. */
  if (ecl_grid->dualp_flag != FILEHEAD_SINGLE_POROSITY) {
    ecl_grid->fracture_index_map     = util_realloc(ecl_grid->fracture_index_map     , ecl_grid->size                  * sizeof * ecl_grid->fracture_index_map     );
    ecl_grid->inv_fracture_index_map = util_realloc(ecl_grid->inv_fracture_index_map , ecl_grid->total_active_fracture * sizeof * ecl_grid->inv_fracture_index_map );
    ecl_grid_init_index_map__( ecl_grid , ecl_grid->fracture_index_map , ecl_grid->inv_fracture_index_map , ACTIVE_FRACTURE , FRACTURE_INDEX);
  }

  
  /* Update the inverse map in the case of coarse cells. Observe that
     in the case of coarse cells with more than one active cell in the
     main grid, the inverse active -> global mapping will map to the
     first active cell in the coarse cell. */
  {
    int coarse_group;
    for (coarse_group = 0; coarse_group < ecl_grid_get_num_coarse_groups( ecl_grid ); coarse_group++) {
      ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group( ecl_grid , coarse_group );
      if (ecl_coarse_cell_get_num_active( coarse_cell ) > 0) {
        int global_index          = ecl_coarse_cell_iget_active_cell_index( coarse_cell , 0 );
        int active_value          = ecl_coarse_cell_iget_active_value( coarse_cell , 0 );
        int active_index          = ecl_coarse_cell_get_active_index( coarse_cell );
        int active_fracture_index = ecl_coarse_cell_get_active_fracture_index( coarse_cell );
        
        if (active_value & ACTIVE_MATRIX) 
          ecl_grid->inv_index_map[ active_index ] = global_index;    // The active -> global mapping point to one "random" cell in the coarse group
          
        if (active_value & ACTIVE_FRACTURE) 
          ecl_grid->inv_fracture_index_map[ active_fracture_index ] = global_index;

        {
          int coarse_size = ecl_coarse_cell_get_size( coarse_cell );
          const int_vector_type * global_index_list = ecl_coarse_cell_get_index_vector( coarse_cell );
          int ic;
          for (ic =0; ic < coarse_size; ic++) {
            int gi = int_vector_iget( global_index_list , ic );
            
            if (active_value & ACTIVE_MATRIX) 
              ecl_grid->index_map[ gi ] = active_index;        // All the cells in the coarse group point to the same active index.
            
            if (active_value & ACTIVE_FRACTURE) 
              ecl_grid->fracture_index_map[ gi ] = active_fracture_index;
          }
        }  
      } // else the coarse cell does not have any active cells.
    }
  }
}
  


/*
  This function goes through the entire grid and sets the active_index
  of all the cells. The functione ecl_grid_realloc_index_map()
  subsequently reads this to create and initialize the index map.
*/

static void ecl_grid_set_active_index(ecl_grid_type * ecl_grid) {
  int global_index;
  int active_index = 0;
  int active_fracture_index = 0;
  
  if (!ecl_grid_have_coarse_cells( ecl_grid )) {
    /* Keeping a fast path for the 99% most common case of no coarse
       groups and single porosity. */
    {
      for (global_index = 0; global_index < ecl_grid->size; global_index++) {
        ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);
        
        if (cell->active & ACTIVE_MATRIX) {
          cell->active_index[MATRIX_INDEX] = active_index;
          active_index++;
        } 
      }
    }
    
    if (ecl_grid->dualp_flag != FILEHEAD_SINGLE_POROSITY) {
      for (global_index = 0; global_index < ecl_grid->size; global_index++) {
        ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);
        if (cell->active & ACTIVE_FRACTURE) {
          cell->active_index[FRACTURE_INDEX] = active_fracture_index;
          active_fracture_index++;
        } 
      }
    }
  } else {
    /* --- More involved path in the case of coarsening groups. --- */
    
    /* 1: Go through all the cells and set the active index. In the
          case of coarse cells we only set the common active index of
          the entire coarse cell.
    */
    for (global_index = 0; global_index < ecl_grid->size; global_index++) {
      ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);
      if (cell->active != INACTIVE) {
        if (cell->coarse_group == COARSE_GROUP_NONE) {
          
          if (cell->active & ACTIVE_MATRIX) {
            cell->active_index[MATRIX_INDEX] = active_index;
            active_index++;
          }

          if (cell->active & ACTIVE_FRACTURE) {
            cell->active_index[FRACTURE_INDEX] = active_fracture_index;
            active_fracture_index++;
          }

        } else {
          ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group( ecl_grid , cell->coarse_group );
          ecl_coarse_cell_update_index( coarse_cell , global_index , &active_index , &active_fracture_index , cell->active);
        }
      } 
    }

    
    /*
      2: Go through all the coarse cells and set the active index and
         active value of all the cells in the coarse cell to the
         common value for the coarse cell.
    */
    {
      int coarse_group;
      for (coarse_group = 0; coarse_group < ecl_grid_get_num_coarse_groups( ecl_grid ); coarse_group++) {
        ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group( ecl_grid , coarse_group );
        if (ecl_coarse_cell_get_num_active(coarse_cell) > 0) {
          int cell_active_index          = ecl_coarse_cell_get_active_index( coarse_cell );
          int cell_active_value          = ecl_coarse_cell_iget_active_value( coarse_cell , 0);
          int group_size                 = ecl_coarse_cell_get_size( coarse_cell );
          const int * coarse_cell_list   = ecl_coarse_cell_get_index_ptr( coarse_cell );
          int i;

          for (i=0; i < group_size; i++) {
            global_index = coarse_cell_list[i];
            {
              ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
              
              if (cell_active_value & ACTIVE_MATRIX)
                cell->active_index[MATRIX_INDEX] = cell_active_index;
              
              /* Coarse cell and dual porosity - that is probably close to zero measure. */
              if (cell_active_value & ACTIVE_FRACTURE) {
                int cell_active_fracture_index = ecl_coarse_cell_get_active_fracture_index( coarse_cell );
                cell->active_index[FRACTURE_INDEX] = cell_active_fracture_index;
              }
            }
          }

        }
      }
    }
  }
  ecl_grid->total_active = active_index;
  ecl_grid->total_active_fracture = active_fracture_index;
}


static void ecl_grid_update_index( ecl_grid_type * ecl_grid) {
  ecl_grid_set_active_index(ecl_grid);
  ecl_grid_realloc_index_map(ecl_grid);
}


/*****************************************************************/
/* Coarse cells */

static ecl_coarse_cell_type * ecl_grid_get_or_create_coarse_cell( ecl_grid_type * ecl_grid , int coarse_nr) {
  if (vector_safe_iget( ecl_grid->coarse_cells , coarse_nr ) == NULL) 
    vector_iset_owned_ref( ecl_grid->coarse_cells , coarse_nr , ecl_coarse_cell_alloc() , ecl_coarse_cell_free__);

  return vector_iget( ecl_grid->coarse_cells , coarse_nr );
}


static void ecl_grid_init_coarse_cells( ecl_grid_type * ecl_grid ) {
  if (ecl_grid->coarsening_active) {
    int global_index;
    for (global_index = 0; global_index < ecl_grid->size; global_index++) {
      ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
      if (cell->coarse_group != COARSE_GROUP_NONE) {
        ecl_coarse_cell_type * coarse_cell = ecl_grid_get_or_create_coarse_cell( ecl_grid , cell->coarse_group);
        int i,j,k;
        ecl_grid_get_ijk1( ecl_grid , global_index , &i , &j , &k);
        ecl_coarse_cell_update( coarse_cell , i , j , k , global_index );
      }
    }
  }
}


ecl_coarse_cell_type * ecl_grid_iget_coarse_group( const ecl_grid_type * ecl_grid , int coarse_nr ) {
  return vector_iget( ecl_grid->coarse_cells , coarse_nr );
}


ecl_coarse_cell_type * ecl_grid_get_cell_coarse_group1( const ecl_grid_type * ecl_grid , int global_index) {
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
  if (cell->coarse_group == COARSE_GROUP_NONE)
    return NULL;
  else
    return ecl_grid_iget_coarse_group( ecl_grid , cell->coarse_group );
}


ecl_coarse_cell_type * ecl_grid_get_cell_coarse_group3( const ecl_grid_type * ecl_grid , int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k );
  return ecl_grid_get_cell_coarse_group1( ecl_grid , global_index );
}


bool ecl_grid_cell_in_coarse_group1( const ecl_grid_type * main_grid , int global_index ) {
  ecl_cell_type * cell = ecl_grid_get_cell( main_grid , global_index );
  if (cell->coarse_group == COARSE_GROUP_NONE )
    return false;
  else
    return true;
}


bool ecl_grid_cell_in_coarse_group3( const ecl_grid_type * main_grid , int i , int j , int k) {
  return ecl_grid_cell_in_coarse_group1( main_grid , ecl_grid_get_global_index3( main_grid , i , j , k ));
}


/*****************************************************************/

static void ecl_grid_pillar_cross_planes(const point_type * p0, 
                                         double e_x , double e_y , double e_z , 
                                         const double *z , double *x , double *y) {
  int k;
  if (e_z != 0) {
    for (k=0; k < 2; k++) {
      double t = (z[k] -  p0->z) / e_z;
      x[k] = p0->x + t * e_x;
      y[k] = p0->y + t * e_y;
    }
  } else {
    for (k=0; k < 2; k++) {
      x[k] = p0->x;
      y[k] = p0->y;
    }
  }
}


/**
   This function must be run before the cell coordinates are
   calculated.  This function is only called for the main grid
   instance, and not for lgrs; the lgrs will inherit the mapaxes
   settings from the global grid.
 */


static void ecl_grid_init_mapaxes( ecl_grid_type * ecl_grid , const float * mapaxes) {
  if (ecl_grid->global_grid != NULL)
    util_abort("%s: hmmmm - this is a major fuck up; trying to grid transformation data from mapaxes for a subgrid(lgr)\n",__func__);
  {
    const double unit_y[2] = {mapaxes[0] - mapaxes[2] , mapaxes[1] - mapaxes[3]};
    const double unit_x[2] = {mapaxes[4] - mapaxes[2] , mapaxes[5] - mapaxes[3]};
    
    {
      double norm_x = 1.0/sqrt( unit_x[0]*unit_x[0] + unit_x[1]*unit_x[1] );
      double norm_y = 1.0/sqrt( unit_y[0]*unit_y[0] + unit_y[1]*unit_y[1] );
      
      ecl_grid->unit_x[0] = unit_x[0] * norm_x;
      ecl_grid->unit_x[1] = unit_x[1] * norm_x;
      ecl_grid->unit_y[0] = unit_y[0] * norm_y;
      ecl_grid->unit_y[1] = unit_y[1] * norm_y;
    }
    ecl_grid->origo[0] = mapaxes[2];
    ecl_grid->origo[1] = mapaxes[3];
    memcpy( ecl_grid->mapaxes , mapaxes , 6 * sizeof( float ));
    ecl_grid->use_mapaxes = true;
  }
}



/**
   this function will add a ecl_grid instance as a lgr to the main
   grid. the lgr grid as added to two different structures of the main
   grid:

    1. in the main_grid->lgr_list the lgr instances are inserted in
       order of occurence in the grid file. the following equalities
       should apply:

          occurence number in file == lgr_grid->grid_nr

       This 'mostly' agrees with the GRIDHEAD(4) item, but
       unfortunately not always. Cases have popped up where the series
       of GRIDHEAD(4) values from lgr to lgr have holes :-(
  
       when installed in the lgr_list vector the lgr grid is installed
       with a destructor, i.e. the grid is destroyed when the vector
       is destroyed.

    2. in the main->lgr_hash the lgr instance is installed with the
       lgrname as key. only a reference is installed in the hash
       table. 

    observe that this is in principle somewhat different from the
    install functions below; here the lgr is added to the top level
    grid (i.e. the main grid) which has the storage responsability of
    all the lgr instances. the cell->lgr relationship is established
    in the _install_egrid / install_grid functions further down.
*/


static void ecl_grid_add_lgr( ecl_grid_type * main_grid , ecl_grid_type * lgr_grid) {
  int next_grid_nr = vector_get_size( main_grid->LGR_list );
  if (next_grid_nr != lgr_grid->grid_nr) 
    util_abort("%s: index based insertion of lgr grid failed. next_grid_nr:%d  lgr->grid_nr:%d \n",__func__ , next_grid_nr , lgr_grid->grid_nr);
  {
    vector_append_owned_ref( main_grid->LGR_list , lgr_grid , ecl_grid_free__);
    hash_insert_ref( main_grid->LGR_hash , lgr_grid->name , lgr_grid);
  }
}


static void ecl_grid_install_lgr_common(ecl_grid_type * host_grid , ecl_grid_type * lgr_grid) {
  hash_insert_ref( host_grid->children , lgr_grid->name , lgr_grid);
  lgr_grid->parent_grid = host_grid;
}


/**
   this function will set the lgr pointer of the relevant cells in the
   host grid to point to the lgr_grid. observe that the ecl_cell_type
   instances do *not* own the lgr_grid - all lgr_grid instances are
   owned by the main grid.
*/

static void ecl_grid_install_lgr_EGRID(ecl_grid_type * host_grid , ecl_grid_type * lgr_grid , const int * hostnum) {
  int global_lgr_index;

  for (global_lgr_index = 0; global_lgr_index < lgr_grid->size; global_lgr_index++) {
    int host_index = hostnum[ global_lgr_index ] - 1;
    ecl_cell_type * lgr_cell  = ecl_grid_get_cell( lgr_grid , global_lgr_index);
    ecl_cell_type * host_cell = ecl_grid_get_cell( host_grid ,  host_index );
  
    ecl_cell_install_lgr( host_cell , lgr_grid );
    lgr_cell->host_cell = host_index;
  }
  ecl_grid_install_lgr_common( host_grid , lgr_grid );
}


/**
   similar to ecl_grid_install_lgr_egrid for grid based instances.
*/
static void ecl_grid_install_lgr_GRID(ecl_grid_type * host_grid , ecl_grid_type * lgr_grid) {
  int global_lgr_index;
  
  for (global_lgr_index = 0; global_lgr_index < lgr_grid->size; global_lgr_index++) {
    ecl_cell_type * lgr_cell = ecl_grid_get_cell( lgr_grid , global_lgr_index);
    ecl_cell_type * host_cell = ecl_grid_get_cell( host_grid , lgr_cell->host_cell );
    ecl_cell_install_lgr( host_cell , lgr_grid );
  }
  ecl_grid_install_lgr_common( host_grid , lgr_grid );
}



/**
   sets the name of the lgr and the name of the parent, if this is a
   nested lgr. for normal lgr descending directly from the coarse grid
   the parent_name is set to NULL.
*/
   

static void ecl_grid_set_lgr_name_EGRID(ecl_grid_type * lgr_grid , const ecl_file_type * ecl_file , int grid_nr) {
  ecl_kw_type * lgrname_kw = ecl_file_iget_named_kw( ecl_file , LGR_KW , grid_nr - 1);
  lgr_grid->name = util_alloc_strip_copy( ecl_kw_iget_ptr( lgrname_kw , 0) );  /* trailing zeros are stripped away. */
  if (ecl_file_has_kw( ecl_file , LGR_PARENT_KW)) {
    ecl_kw_type * parent_kw = ecl_file_iget_named_kw( ecl_file , LGR_PARENT_KW , grid_nr - 1);
    char * parent = util_alloc_strip_copy( ecl_kw_iget_ptr( parent_kw , 0));

    if (strlen( parent ) > 0) 
      lgr_grid->parent_name = parent;
    else  /* lgr_grid->parent has been initialized to NULL */
      free( parent );
  }
}

/**
   sets the name of the lgr and the name of the parent, if this is a
   nested lgr. for lgr descending directly from the parent eclipse
   will supply 'global' (whereas for egrid it will return '' -
   cool?). anyway global -> NULL.
*/

static void ecl_grid_set_lgr_name_GRID(ecl_grid_type * lgr_grid , const ecl_file_type * ecl_file , int grid_nr) {
  ecl_kw_type * lgr_kw = ecl_file_iget_named_kw( ecl_file , LGR_KW , grid_nr - 1);
  lgr_grid->name = util_alloc_strip_copy( ecl_kw_iget_ptr( lgr_kw , 0) );  /* trailing zeros are stripped away. */
  {
    /**
       the lgr keyword can have one or two elements; in the case of two elements
       the second element will be the name of the parent grid - in the case of
       only one element the current lgr is assumed to descend from the main grid
    */
    if (ecl_kw_get_size( lgr_kw ) == 2) {
      char * parent = util_alloc_strip_copy( ecl_kw_iget_ptr( lgr_kw , 1));
      
      if ((strlen(parent) == 0) || (strcmp(parent , GLOBAL_STRING ) == 0))
        free( parent );
      else
        lgr_grid->parent_name = parent;
    }
  }
}




static void ecl_grid_init_GRDECL_data_jslice(ecl_grid_type * ecl_grid ,  const float * zcorn , const float * coord , const int * actnum, const int * corsnum , int j) {
  const int nx = ecl_grid->nx;
  const int ny = ecl_grid->ny;
  const int nz = ecl_grid->nz;
  int i;

  
  for (i=0; i < nx; i++) {
    point_type pillars[4][2];
    int pillar_index[4];
    pillar_index[0] = 6 * ( j      * (nx + 1) + i    );
    pillar_index[1] = 6 * ( j      * (nx + 1) + i + 1);
    pillar_index[2] = 6 * ((j + 1) * (nx + 1) + i    );
    pillar_index[3] = 6 * ((j + 1) * (nx + 1) + i + 1);

    { 
      int ip;
      for (ip = 0; ip < 4; ip++) {
        int index = pillar_index[ip];
        point_set(&pillars[ip][0] , coord[index] , coord[index + 1] , coord[index + 2]);
        
        index += 3;
        point_set(&pillars[ip][1] , coord[index] , coord[index + 1] , coord[index + 2]);
      }
    }

    {
      double ex[4];
      double ey[4];
      double ez[4];
      int k;
      
      {
        int ip;
        for (ip = 0; ip <  4; ip++) {
          ex[ip] = pillars[ip][1].x - pillars[ip][0].x;
          ey[ip] = pillars[ip][1].y - pillars[ip][0].y;
          ez[ip] = pillars[ip][1].z - pillars[ip][0].z;
        }
      }
      
      
      for (k=0; k < nz; k++) {
        double x[4][2];
        double y[4][2];
        double z[4][2];

        {
          int c;
          for (c = 0; c < 2; c++) {
            z[0][c] = zcorn[k*8*nx*ny + j*4*nx + 2*i            + c*4*nx*ny];
            z[1][c] = zcorn[k*8*nx*ny + j*4*nx + 2*i  +  1      + c*4*nx*ny];
            z[2][c] = zcorn[k*8*nx*ny + j*4*nx + 2*nx + 2*i     + c*4*nx*ny];
            z[3][c] = zcorn[k*8*nx*ny + j*4*nx + 2*nx + 2*i + 1 + c*4*nx*ny];
          }
        }
        
        {
          int ip;
          for (ip = 0; ip <  4; ip++) 
            ecl_grid_pillar_cross_planes(&pillars[ip][0] , ex[ip], ey[ip] , ez[ip] , z[ip] , x[ip] , y[ip]);
        }
        
        ecl_grid_set_cell_EGRID(ecl_grid , i , j , k , x , y , z , actnum , corsnum);
      }
    }
  }
}


void ecl_grid_init_GRDECL_data(ecl_grid_type * ecl_grid ,  const float * zcorn , const float * coord , const int * actnum, const int * corsnum) {
  const int ny = ecl_grid->ny;
  int j;
#pragma omp parallel for
  for ( j=0; j < ny; j++) 
    ecl_grid_init_GRDECL_data_jslice( ecl_grid , zcorn, coord , actnum , corsnum , j );
}



/*
  2---3
  |   |
  0---1
*/

static ecl_grid_type * ecl_grid_alloc_GRDECL_data__(ecl_grid_type * global_grid , 
                                                    int dualp_flag , int nx , int ny , int nz , 
                                                    const float * zcorn , const float * coord , const int * actnum, const float * mapaxes, const int * corsnum, 
                                                    int grid_nr) {

  ecl_grid_type * ecl_grid = ecl_grid_alloc_empty(global_grid , dualp_flag , nx,ny,nz,grid_nr,true);
  
  if (mapaxes != NULL)
    ecl_grid_init_mapaxes( ecl_grid , mapaxes );

  if (corsnum != NULL)
    ecl_grid->coarsening_active = true;
  
  ecl_grid->coord_kw = ecl_kw_alloc_new("COORD" , 6*(nx + 1) * (ny + 1) , ECL_FLOAT_TYPE , coord );
  ecl_grid_init_GRDECL_data( ecl_grid , zcorn , coord , actnum , corsnum);

  ecl_grid_init_coarse_cells( ecl_grid );
  ecl_grid_update_index( ecl_grid );
  ecl_grid_taint_cells( ecl_grid );
  return ecl_grid;
}

/*
  If you create/load data for the various fields, this function can be
  used to create a GRID instance, without going through a GRID/EGRID
  file - currently the implementation does not support the creation of
  a lgr hierarchy - or cell coarsening.
*/

ecl_grid_type * ecl_grid_alloc_GRDECL_data(int nx , int ny , int nz , const float * zcorn , const float * coord , const int * actnum, const float * mapaxes) {
  return ecl_grid_alloc_GRDECL_data__(NULL , FILEHEAD_SINGLE_POROSITY , nx , ny , nz , zcorn , coord , actnum , mapaxes , NULL , 0);
}

static ecl_grid_type * ecl_grid_alloc_GRDECL_kw__(ecl_grid_type * global_grid ,  
                                                  int dualp_flag, 
                                                  const ecl_kw_type * gridhead_kw , 
                                                  const ecl_kw_type * zcorn_kw , 
                                                  const ecl_kw_type * coord_kw , 
                                                  const ecl_kw_type * actnum_kw ,    /* Can be NULL */ 
                                                  const ecl_kw_type * mapaxes_kw ,   /* Can be NULL */
                                                  const ecl_kw_type * corsnum_kw ,   /* Can be NULL */
                                                  int grid_nr) {
  
  int gtype, nx,ny,nz;
  
  gtype   = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_TYPE_INDEX);
  nx      = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_NX_INDEX);
  ny      = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_NY_INDEX);
  nz      = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_NZ_INDEX);

  /*
    The code used to have this test:

       if (grid_nr != ecl_kw_iget_int( gridhead_kw , GRIDHEAD_LGR_INDEX))
          util_abort("%s: internal error in grid loader - lgr index mismatch\n",__func__);
          
    But then suddenly a EGRID file where this did not apply appeared :-(
  */

  if (gtype != GRIDHEAD_GRIDTYPE_CORNERPOINT)
    util_abort("%s: gtype:%d fatal error when loading grid - must have corner point grid - aborting\n",__func__ , gtype );

  {
    const float * mapaxes_data = NULL;
    const int   * actnum_data  = NULL;
    const int   * corsnum_data = NULL;
    
    if (mapaxes_kw != NULL)
      mapaxes_data = ecl_kw_get_float_ptr( mapaxes_kw );

    if (actnum_kw != NULL)
      actnum_data = ecl_kw_get_int_ptr(actnum_kw);

    if (corsnum_kw != NULL)
      corsnum_data = ecl_kw_get_int_ptr( corsnum_kw );
    
    return ecl_grid_alloc_GRDECL_data__(global_grid , 
                                        dualp_flag , 
                                        nx , ny , nz , 
                                        ecl_kw_get_float_ptr(zcorn_kw) , 
                                        ecl_kw_get_float_ptr(coord_kw) , 
                                        actnum_data,
                                        mapaxes_data, 
                                        corsnum_data,
                                        grid_nr);
  }
}


/**
   If you create/load ecl_kw instances for the various fields, this
   function can be used to create a GRID instance, without going
   through a GRID/EGRID file. Does not support LGR or coarsening
   hierarchies.  
*/

ecl_grid_type * ecl_grid_alloc_GRDECL_kw( int nx, int ny , int nz , 
                                          const ecl_kw_type * zcorn_kw , 
                                          const ecl_kw_type * coord_kw , 
                                          const ecl_kw_type * actnum_kw ,      /* Can be NULL */
                                          const ecl_kw_type * mapaxes_kw ) {   /* Can be NULL */
                                         

  ecl_kw_type * gridhead_kw = ecl_grid_alloc_gridhead_kw( nx , ny , nz , 0);
  ecl_grid_type * ecl_grid = ecl_grid_alloc_GRDECL_kw__(NULL , FILEHEAD_SINGLE_POROSITY , gridhead_kw , zcorn_kw , coord_kw , actnum_kw , mapaxes_kw , NULL , 0);
  ecl_kw_free( gridhead_kw );
  return ecl_grid;

}



/**
   Creating a grid based on a EGRID file is a three step process:

   1. Load the file and extracte the keywords.
   2. Call xx_alloc_GRDECL_kw__() to build grid based on keywords.
   3. Call xx_alloc_GRDECL_data__() to build the grid based on keyword data.
    
   The point is that external scope can create grid based on both a
   list of keywords, and actual data - in addition to the normal
   loading of a full file. Currently the 'shortcuts'
   ecl_grid_alloc_grdecl_kw() and ecl_grid_alloc_grdecl_data() do not
   support LGRs.
*/


static ecl_grid_type * ecl_grid_alloc_EGRID__( ecl_grid_type * main_grid , const ecl_file_type * ecl_file , int grid_nr) {
  ecl_kw_type * gridhead_kw  = ecl_file_iget_named_kw( ecl_file , GRIDHEAD_KW  , grid_nr);
  ecl_kw_type * zcorn_kw     = ecl_file_iget_named_kw( ecl_file , ZCORN_KW     , grid_nr);
  ecl_kw_type * coord_kw     = ecl_file_iget_named_kw( ecl_file , COORD_KW     , grid_nr);
  ecl_kw_type * corsnum_kw   = NULL;
  ecl_kw_type * actnum_kw    = NULL;
  ecl_kw_type * mapaxes_kw   = NULL; 
  int dualp_flag;
  if (grid_nr == 0) {
    ecl_kw_type * filehead_kw  = ecl_file_iget_named_kw( ecl_file , FILEHEAD_KW  , grid_nr);
    dualp_flag                 = ecl_kw_iget_int( filehead_kw , FILEHEAD_DUALP_INDEX );
  } else
    dualp_flag = main_grid->dualp_flag;
  

  /** If ACTNUM is not present - that is is interpreted as - all active. */
  if (ecl_file_get_num_named_kw(ecl_file , ACTNUM_KW) > grid_nr)
    actnum_kw = ecl_file_iget_named_kw( ecl_file , ACTNUM_KW    , grid_nr);
  
  if (grid_nr == 0) {
    /* MAPAXES and COARSENING only apply to the global grid. */
    if (ecl_file_has_kw( ecl_file , MAPAXES_KW)) 
      mapaxes_kw   = ecl_file_iget_named_kw( ecl_file , MAPAXES_KW , 0);
    
    if (ecl_file_has_kw( ecl_file , CORSNUM_KW)) 
      corsnum_kw   = ecl_file_iget_named_kw( ecl_file , CORSNUM_KW , 0);
  }

  
  
  {
    ecl_grid_type * ecl_grid = ecl_grid_alloc_GRDECL_kw__( main_grid , 
                                                           dualp_flag , 
                                                           gridhead_kw , 
                                                           zcorn_kw , 
                                                           coord_kw , 
                                                           actnum_kw , 
                                                           mapaxes_kw , 
                                                           corsnum_kw ,
                                                           grid_nr );

    if (grid_nr > 0) ecl_grid_set_lgr_name_EGRID(ecl_grid , ecl_file , grid_nr);
    return ecl_grid;
  }
}



static ecl_grid_type * ecl_grid_alloc_EGRID(const char * grid_file) {
  ecl_file_enum   file_type;
  file_type = ecl_util_get_file_type(grid_file , NULL , NULL);
  if (file_type != ECL_EGRID_FILE)
    util_abort("%s: %s wrong file type - expected .EGRID file - aborting \n",__func__ , grid_file);
  {
    ecl_file_type * ecl_file   = ecl_file_open( grid_file , 0);
    int num_grid               = ecl_file_get_num_named_kw( ecl_file , GRIDHEAD_KW );
    ecl_grid_type * main_grid  = ecl_grid_alloc_EGRID__( NULL , ecl_file , 0 );
    int grid_nr;
    
    for ( grid_nr = 1; grid_nr < num_grid; grid_nr++) {
      ecl_grid_type * lgr_grid = ecl_grid_alloc_EGRID__( main_grid , ecl_file , grid_nr );
      ecl_grid_add_lgr( main_grid , lgr_grid );
      {
        ecl_grid_type * host_grid;
        ecl_kw_type   * hostnum_kw = ecl_file_iget_named_kw( ecl_file , HOSTNUM_KW , grid_nr - 1);
        if (lgr_grid->parent_name == NULL)
          host_grid = main_grid;
        else 
          host_grid = ecl_grid_get_lgr( main_grid , lgr_grid->parent_name );
        
        ecl_grid_install_lgr_EGRID( host_grid , lgr_grid , ecl_kw_get_int_ptr( hostnum_kw) );
      }
    }
    main_grid->name = util_alloc_string_copy( grid_file );
    ecl_file_close( ecl_file );
    return main_grid;
  }
}







static ecl_grid_type * ecl_grid_alloc_GRID_data__(ecl_grid_type * global_grid , int num_coords , int dualp_flag , int nx, int ny , int nz , int grid_nr , int coords_size , int ** coords , float ** corners , const float * mapaxes) {
  if (dualp_flag != FILEHEAD_SINGLE_POROSITY)
    nz = nz / 2;
  {
    ecl_grid_type * grid = ecl_grid_alloc_empty( global_grid , dualp_flag , nx , ny , nz , grid_nr , false);
    
    if (mapaxes != NULL)
      ecl_grid_init_mapaxes( grid , mapaxes );
    
    {
      int index;
      for ( index=0; index < num_coords; index++) 
        ecl_grid_set_cell_GRID(grid , coords_size , coords[index] , corners[index]);
    }
    
    ecl_grid_init_coarse_cells( grid );
    ecl_grid_update_index( grid );
    ecl_grid_taint_cells( grid );
    return grid;
  }
}

/*
  coords[num_coords][coords_size]
  corners[num_coords][24]
*/

ecl_grid_type * ecl_grid_alloc_GRID_data(int num_coords , int nx , int ny , int nz , int coords_size , int ** coords , float ** corners , const float * mapaxes) {
  return ecl_grid_alloc_GRID_data__( NULL , 
                                     num_coords , 
                                     FILEHEAD_SINGLE_POROSITY , /* Does currently not support to determine dualp_flag from inspection. */
                                     nx , ny , nz , 0 , coords_size , coords , corners , mapaxes);
}



static int ecl_grid_dual_porosity_GRID_check( ecl_file_type * ecl_file ) {
  ecl_kw_type * dimens_kw = ecl_file_iget_named_kw( ecl_file , DIMENS_KW , 0);
  int nx   = ecl_kw_iget_int(dimens_kw , DIMENS_NX_INDEX);
  int ny   = ecl_kw_iget_int(dimens_kw , DIMENS_NY_INDEX);
  int nz   = ecl_kw_iget_int(dimens_kw , DIMENS_NZ_INDEX);

  if ((nz % 2) == 1)
    return FILEHEAD_SINGLE_POROSITY;
  else {
    int dualp_flag = FILEHEAD_DUAL_POROSITY;
    int num_corners = ecl_file_get_num_named_kw( ecl_file , CORNERS_KW );
    int matrix_index = 0;
    int fracture_index;

    ecl_kw_type * matrix_kw;
    ecl_kw_type * fracture_kw;
    
    if (num_corners > nx*ny*nz)
      fracture_index = nx*ny*nz/2;
    else
      fracture_index = num_corners / 2;
    
    while (true) {
      matrix_kw   = ecl_file_iget_named_kw( ecl_file, CORNERS_KW , matrix_index );
      fracture_kw = ecl_file_iget_named_kw( ecl_file, CORNERS_KW , fracture_index );

      if (!ecl_kw_equal(matrix_kw , fracture_kw)) {
        dualp_flag = FILEHEAD_SINGLE_POROSITY;
        break;
      }
      
      matrix_index++;
      fracture_index++;
      if (fracture_index == nx*ny*nz)
        break;
    }

    return dualp_flag;
  }
}


static ecl_grid_type * ecl_grid_alloc_GRID__(ecl_grid_type * global_grid , const ecl_file_type * ecl_file , int cell_offset , int grid_nr, int dualp_flag) {
  int           nx,ny,nz;
  const float * mapaxes_data = NULL;
  ecl_grid_type * grid;

  // 1: Fetching header data from the DIMENS keyword.
  {
    ecl_kw_type * dimens_kw   = ecl_file_iget_named_kw( ecl_file , DIMENS_KW , grid_nr);
    nx   = ecl_kw_iget_int(dimens_kw , DIMENS_NX_INDEX);
    ny   = ecl_kw_iget_int(dimens_kw , DIMENS_NY_INDEX);
    nz   = ecl_kw_iget_int(dimens_kw , DIMENS_NZ_INDEX);
  }
  

  // 2: Fetching the mapaxes data from the MAPAXES keyword; the
  //    keyword is optional, and is only applicable to the global grid.
  {
    if ((grid_nr == 0) && (ecl_file_has_kw( ecl_file , MAPAXES_KW))) {
      const ecl_kw_type * mapaxes_kw = ecl_file_iget_named_kw( ecl_file , MAPAXES_KW , 0);
      mapaxes_data = ecl_kw_get_float_ptr( mapaxes_kw); 
    }
  }
  

  /*
    The number of COORDS/CORNERS blocks depends on the GRIDFILE option
    used in the ECLIPSE datafile when asking ECLIPSE to save a file in
    GRID format. If the GRIDFILE option is set to 1; only active
    cells are stored. In that case:

      1. We have absolutely no real-world coordinate information about
         the inactive cells.

      2. The GRID file only contains the global information, and can
         not contain LGR information.
    
    If the GRIDFILE option is set to 2 ECLIPSE will save coordinate
    information for all the cells in the grid, and also lgrs. To
    determine how many coords keywords we should read we use the
    following heuristics:

       1. If global_grid != NULL : this is an lgr, and num_coords =
          nx*ny*nz;
       
       2. If global_grid == NULL : this is the global grid. We check
          how many COORDS keywords there are in the file:
    
           a) If there are >= nx*ny*nz keywords we set num_coords ==
              nx*ny*nz.

           b) If there are < nx*ny*nz keywords we set num_coords ==
              #COORDS keywords in the file.

    Possible LGR cells will follow *AFTER* the first nx*ny*nz cells;
    the loop stops at nx*ny*nz. Additionally the LGR cells should be
    discarded (by checking coords[5]) in the ecl_grid_set_cell_GRID()
    function.
  */




  {
    int num_coords;
    
    if (global_grid == NULL) {
      /* This is the main grid - can be both nactive or nx*ny*nz coord elements. */
      int num_coords_kw = ecl_file_get_num_named_kw( ecl_file , COORDS_KW);
      if (num_coords_kw >= nx*ny*nz)
        num_coords = nx*ny*nz;
      else
        num_coords = num_coords_kw;
    } else
      /* This is an lgr - always nx*ny*nz elements. */
      num_coords = nx*ny*nz;

    
    // 3: Fetching the main chunk of cell data from the COORDS and
    //    CORNERS keywords.
    {
      int coords_size = -1;
      int index;
    
      int ** coords    = util_calloc( num_coords , sizeof * coords  );
      float ** corners = util_calloc( num_coords , sizeof * corners );
      
      for (index = 0; index < num_coords; index++) {
        const ecl_kw_type * coords_kw = ecl_file_iget_named_kw(ecl_file , COORDS_KW  , index + cell_offset);
        const ecl_kw_type * corners_kw = ecl_file_iget_named_kw(ecl_file , CORNERS_KW , index + cell_offset);
        
        coords[index]  = ecl_kw_get_int_ptr(coords_kw);
        corners[index] = ecl_kw_get_float_ptr(corners_kw);
        coords_size = ecl_kw_get_size( coords_kw );
      }
      // Create the grid:
      grid = ecl_grid_alloc_GRID_data__( global_grid , num_coords , dualp_flag , nx , ny , nz , grid_nr , coords_size , coords , corners , mapaxes_data );

      free( coords );
      free( corners );
    }
  }

  if (grid_nr > 0) ecl_grid_set_lgr_name_GRID(grid , ecl_file , grid_nr);
  return grid;
}





static ecl_grid_type * ecl_grid_alloc_GRID(const char * grid_file) {

  ecl_file_enum   file_type;
  file_type = ecl_util_get_file_type(grid_file , NULL , NULL);
  if (file_type != ECL_GRID_FILE)
    util_abort("%s: %s wrong file type - expected .GRID file - aborting \n",__func__ , grid_file);
  
  {
    int cell_offset = 0;
    ecl_file_type * ecl_file  = ecl_file_open( grid_file , 0);
    int num_grid              = ecl_file_get_num_named_kw( ecl_file , DIMENS_KW);
    ecl_grid_type * main_grid;
    int grid_nr;
    int dualp_flag;

    dualp_flag = ecl_grid_dual_porosity_GRID_check( ecl_file );
    main_grid  = ecl_grid_alloc_GRID__(NULL , ecl_file , cell_offset , 0,dualp_flag);
    cell_offset += ecl_grid_get_global_size( main_grid );

    for (grid_nr = 1; grid_nr < num_grid; grid_nr++) {
      ecl_grid_type * lgr_grid = ecl_grid_alloc_GRID__(main_grid , ecl_file , cell_offset , grid_nr , dualp_flag);
      cell_offset += ecl_grid_get_global_size( lgr_grid );
      ecl_grid_add_lgr( main_grid , lgr_grid );
      {
        ecl_grid_type * host_grid;
   
        if (lgr_grid->parent_name == NULL)
          host_grid = main_grid;
        else 
          host_grid = ecl_grid_get_lgr( main_grid , lgr_grid->parent_name );
          
        ecl_grid_install_lgr_GRID( host_grid , lgr_grid );
      }
    }
    main_grid->name = util_alloc_string_copy( grid_file );
    ecl_file_close( ecl_file );
    return main_grid;
  }
}
                                 


/**
   This function will allocate a new regular grid with dimensions nx x
   ny x nz. The cells in the grid are spanned by the the three unit
   vectors ivec, jvec and kvec.

   The actnum argument should be a pointer to an integer array of
   length nx*ny*nz where actnum[i + j*nx + k*nx*ny] == 1 for active
   cells and 0 for inactive cells. The actnum array can be NULL, in
   which case all cells will be active.
*/

ecl_grid_type * ecl_grid_alloc_regular( int nx, int ny , int nz , const double * ivec, const double * jvec , const double * kvec , const int * actnum) {
  ecl_grid_type * grid = ecl_grid_alloc_empty(NULL , FILEHEAD_SINGLE_POROSITY , nx , ny , nz , 0,true);
  const double offset[3] = {0,0,0};

  int k,j,i;
  for (k=0; k < nz; k++) {
    for (j=0; j< ny; j++) {
      for (i=0; i < nx; i++) {
        int global_index = i + j*nx + k*nx*ny;

        ecl_cell_type * cell = ecl_grid_get_cell(grid , global_index );
        
        ecl_cell_init_regular( cell , offset , i,j,k,global_index , ivec , jvec , kvec , actnum );
      }
    }
  }

  ecl_grid_update_index( grid );
  return grid;
}


/**
   This function will allocate a new rectangular grid with dimensions
   nx x ny x nz. The cells in the grid are rectangular with dimensions
   dx x dy x dz.

   The actnum argument should be a pointer to an integer array of
   length nx*ny*nz where actnum[i + j*nx + k*nx*ny] == 1 for active
   cells and 0 for inactive cells. The actnum array can be NULL, in
   which case all cells will be active.
*/

ecl_grid_type * ecl_grid_alloc_rectangular( int nx , int ny , int nz , double dx , double dy , double dz , const int * actnum) {
  const double ivec[3] = {dx , 0 , 0};
  const double jvec[3] = {0 , dy , 0};
  const double kvec[3] = {0 , 0 , dz};

  return ecl_grid_alloc_regular( nx , ny , nz , ivec , jvec , kvec , actnum);
}


/**
   This function will allocate a ecl_grid instance. As input it takes
   a filename, which can be both a GRID file and an EGRID file (both
   formatted and unformatted).

   When allocating based on an EGRID file the COORDS, ZCORN and ACTNUM
   keywords are extracted, and the ecl_grid_alloc_GRDECL() function is
   called with these keywords. This function can be called directly
   with these keywords.
*/

ecl_grid_type * ecl_grid_alloc(const char * grid_file ) {
  ecl_file_enum    file_type;
  ecl_grid_type  * ecl_grid = NULL;

  file_type = ecl_util_get_file_type(grid_file , NULL ,  NULL);
  if (file_type == ECL_GRID_FILE)
    ecl_grid = ecl_grid_alloc_GRID(grid_file);
  else if (file_type == ECL_EGRID_FILE)
    ecl_grid = ecl_grid_alloc_EGRID(grid_file);
  else
    util_abort("%s must have .GRID or .EGRID file - %s not recognized \n", __func__ , grid_file);
  
  return ecl_grid;
}


static void ecl_grid_file_nactive_dims( fortio_type * data_fortio , int * dims) {
  if (data_fortio) {
    if (ecl_kw_fseek_kw( INTEHEAD_KW , false , false , data_fortio )) {
      ecl_kw_type * intehead_kw = ecl_kw_fread_alloc( data_fortio );
      dims[3] = ecl_kw_iget_int( intehead_kw , INTEHEAD_NACTIVE_INDEX );
      ecl_kw_free( intehead_kw );
    }
  }
}


static bool ecl_grid_file_EGRID_dims( fortio_type * grid_fortio , fortio_type * data_fortio , int * dims ) {
  
  if (ecl_kw_fseek_kw( GRIDHEAD_KW , false , false , grid_fortio)) {
    {
      ecl_kw_type * gridhead_kw = ecl_kw_fread_alloc( grid_fortio );
      dims[0] = ecl_kw_iget_int( gridhead_kw , GRIDHEAD_NX_INDEX );
      dims[1] = ecl_kw_iget_int( gridhead_kw , GRIDHEAD_NY_INDEX );
      dims[2] = ecl_kw_iget_int( gridhead_kw , GRIDHEAD_NZ_INDEX );
      
      ecl_kw_free( gridhead_kw );
    }
    ecl_grid_file_nactive_dims( data_fortio , dims );
    return true;
  } else
    return false;
  
}

static bool ecl_grid_file_GRID_dims( fortio_type * grid_fortio , fortio_type * data_fortio , int * dims ) {
  
  if (ecl_kw_fseek_kw( DIMENS_KW , false , false , grid_fortio)) {
    {
      ecl_kw_type * dimens_kw = ecl_kw_fread_alloc( grid_fortio );
      dims[0] = ecl_kw_iget_int( dimens_kw , DIMENS_NX_INDEX );
      dims[1] = ecl_kw_iget_int( dimens_kw , DIMENS_NY_INDEX );
      dims[2] = ecl_kw_iget_int( dimens_kw , DIMENS_NZ_INDEX );
      
      ecl_kw_free( dimens_kw );
    }

    ecl_grid_file_nactive_dims( data_fortio , dims );
    return true;
  } else
    return false;
  
}

/**
   Will check the grid dimensions from the input grid file
   @grid_filename; the input file must be a GRID/EGRID file. On exit
   the dims array will be filled as:

          dims[0] = nx; 
          dims[1] = ny; 
          dims[2] = nz;

   Optionally you can in addition supply the name of a restart or INIT
   file in the second file argument - if-and-only-if, that filename
   points to an existing file the fourth element in the dims array
   will be set as:

          dims[3] = nactive;
          
   The function as a whole will return true if the grid dimensions
   (nx,ny,nz) are sucessfully set. If the dimensions are not set the
   dims vector is not touched.
*/



bool ecl_grid_file_dims( const char * grid_filename , const char * init_restart_filename , int * dims) {
  bool input_file_OK = false;
  bool grid_fmt_file;
  ecl_file_enum grid_file_type = ecl_util_get_file_type( grid_filename , &grid_fmt_file , NULL );

  if ((grid_file_type == ECL_GRID_FILE) || (grid_file_type == ECL_EGRID_FILE)) {
    fortio_type * grid_fortio = fortio_open_reader( grid_filename , grid_fmt_file , ECL_ENDIAN_FLIP );
    if (grid_fortio) {
      fortio_type * data_fortio = NULL;
      bool data_fmt_file;
      
      if (init_restart_filename) {
        ecl_util_get_file_type( init_restart_filename , &data_fmt_file , NULL );
        data_fortio = fortio_open_reader( init_restart_filename , data_fmt_file , ECL_ENDIAN_FLIP );
      }
      
      
      if (grid_file_type == ECL_GRID_FILE)
        input_file_OK = ecl_grid_file_GRID_dims( grid_fortio , data_fortio , dims );
      else
        input_file_OK = ecl_grid_file_EGRID_dims( grid_fortio , data_fortio , dims );
      
      if (data_fortio)
        fortio_fclose( data_fortio );
      
      fortio_fclose( grid_fortio );
    }
  }  
  
  return input_file_OK;
}



/**
   Will load the grid corresponding to the input @input_case;
   depending on the value of @input_case many different paths will be
   tried:

   1 case_input - an existing GRID/EGRID file: Just load the file -
     with no further ado.

   2 case_input - an existing ECLIPSE file which is not a grid file;
     if it has definite formatted/unformatted status look only for
     those GRID/EGRID with the same formatted/unformatted status.
    
   3 case_input is only an ECLIPSE base, look for
     formatted/unformatted files with the correct basename.


   For cases 2 & 3 the function will look for files in the following order:

      BASE.EGRID   BASE.GRID   BASE.FEGRID   BASE.FGRID

   and stop with the first success. Will return NULL if no GRID/EGRID
   files can be found.
*/




char * ecl_grid_alloc_case_filename( const char * case_input ) {
  ecl_file_enum    file_type;
  bool             fmt_file;
  file_type = ecl_util_get_file_type( case_input , &fmt_file ,  NULL);
  
  if (file_type == ECL_GRID_FILE)
    return util_alloc_string_copy( case_input ); /* Case 1 */
  else if (file_type == ECL_EGRID_FILE)
    return util_alloc_string_copy( case_input ); /* Case 1 */
  else {
    char * grid_file = NULL;
    char * path;
    char * basename;
    util_alloc_file_components( case_input , &path , &basename , NULL);
    if ((file_type == ECL_OTHER_FILE) || (file_type == ECL_DATA_FILE)) {          /* Case 3 - only basename recognized */
      char * EGRID  = ecl_util_alloc_filename( path , basename , ECL_EGRID_FILE , false , -1);
      char * GRID   = ecl_util_alloc_filename( path , basename , ECL_GRID_FILE  , false , -1);
      char * FEGRID = ecl_util_alloc_filename( path , basename , ECL_EGRID_FILE , true  , -1);
      char * FGRID  = ecl_util_alloc_filename( path , basename , ECL_GRID_FILE  , true  , -1);

      if (util_file_exists( EGRID ))
        grid_file = util_alloc_string_copy( EGRID );
      else if (util_file_exists( GRID ))
        grid_file = util_alloc_string_copy( GRID );
      else if (util_file_exists( FEGRID ))
        grid_file = util_alloc_string_copy( FEGRID );
      else if (util_file_exists( FGRID ))
        grid_file = util_alloc_string_copy( FGRID );
      /*
        else: could not find a GRID/EGRID. 
      */

      free( EGRID );
      free( FEGRID );
      free( GRID );
      free( FGRID );
    } else {                                                                      /* Case 2 - we know the formatted / unformatted status. */
      char * EGRID  = ecl_util_alloc_filename( path , basename , ECL_EGRID_FILE , fmt_file , -1);
      char * GRID   = ecl_util_alloc_filename( path , basename , ECL_GRID_FILE  , fmt_file , -1);
      
      if (util_file_exists( EGRID ))
        grid_file = util_alloc_string_copy( EGRID );
      else if (util_file_exists( GRID ))
        grid_file = util_alloc_string_copy( GRID );
      
      free( EGRID );
      free( GRID );
    }
    return grid_file;
  }
}



ecl_grid_type * ecl_grid_load_case( const char * case_input ) {
  ecl_grid_type * ecl_grid = NULL;
  char * grid_file = ecl_grid_alloc_case_filename( case_input );
  if (grid_file != NULL) {
    ecl_grid = ecl_grid_alloc( grid_file );
    free( grid_file );
  }
  return ecl_grid;
}



bool ecl_grid_exists( const char * case_input ) {
  bool exists = false;
  char * grid_file = ecl_grid_alloc_case_filename( case_input );
  if (grid_file != NULL) {
    exists = true;
    free( grid_file );
  }
  return exists;
}



/**
   Return true if grids g1 and g2 are equal, and false otherwise. To
   return true all cells must be identical. 
*/

static bool ecl_grid_compare__(const ecl_grid_type * g1 , const ecl_grid_type * g2, bool verbose) {

  bool equal = true;
  if (g1->size != g2->size)
    equal = false;
  else {
    int g;
    for (g = 0; g < g1->size; g++) {
      bool this_equal = true;
      ecl_cell_type *c1 = ecl_grid_get_cell( g1 , g );
      ecl_cell_type *c2 = ecl_grid_get_cell( g2 , g );
      ecl_cell_compare(c1 , c2 , &this_equal);
      
      if (!this_equal) {
        if (verbose) {
          int i,j,k;
          ecl_grid_get_ijk1( g1 , g , &i , &j , &k);
          if (i == 207 && j == 63) {
            printf("Difference in cell: %d : %d,%d,%d  Volume:%g \n",g,i,j,k , ecl_cell_get_volume( c1 ));
            printf("-----------------------------------------------------------------\n");
            ecl_cell_dump_ascii( c1 , i , j , k , stdout );
            printf("-----------------------------------------------------------------\n");
            ecl_cell_dump_ascii( c2 , i , j , k , stdout );
            printf("-----------------------------------------------------------------\n");
            //break;
          }
        }
        equal = false;
        //break;
      }

    }
  }
  
  return equal;
}


bool ecl_grid_compare(const ecl_grid_type * g1 , const ecl_grid_type * g2 , bool include_lgr, bool verbose) {
  if (!include_lgr) 
    return ecl_grid_compare__(g1 , g2 , verbose);
  else {
    if (vector_get_size( g1->LGR_list ) == vector_get_size( g2->LGR_list )) {
      bool equal;
      int grid_nr;
      for (grid_nr = 0; grid_nr < vector_get_size( g1->LGR_list ); grid_nr++) {
        const ecl_grid_type * lgr1 = vector_iget_const( g1->LGR_list , grid_nr);
        const ecl_grid_type * lgr2 = vector_iget_const( g2->LGR_list , grid_nr);

        equal = ecl_grid_compare__(lgr1 , lgr2 , verbose);
        if (!equal) 
          break;
      }
      return equal;
    } else
      return false;
  }
}



/*****************************************************************/

bool ecl_grid_cell_contains_xyz1( const ecl_grid_type * ecl_grid , int global_index , double x , double y , double z) {
  point_type p;
  point_set( &p , x , y , z);
  
  return ecl_cell_contains_point( ecl_grid_get_cell( ecl_grid , global_index) , &p);
}


bool ecl_grid_cell_contains_xyz3( const ecl_grid_type * ecl_grid , int i , int j , int k, double x , double y , double z) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k );
  return ecl_grid_cell_contains_xyz1( ecl_grid , global_index , x ,y  , z);
}



/**
   This function returns the global index for the cell (in layer 'k')
   which contains the point x,y. Observe that if you are looking for
   (i,j) you must call the function ecl_grid_get_ijk1() on the return value.
*/

int ecl_grid_get_global_index_from_xy( const ecl_grid_type * ecl_grid , int k , bool lower_layer , double x , double y) {

  int i,j;
  for (j=0; j < ecl_grid->ny; j++)
    for (i=0; i < ecl_grid->nx; i++) {
      int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k );
      if (ecl_cell_layer_contains_xy( ecl_grid_get_cell( ecl_grid , global_index ) , lower_layer , x , y))
        return global_index;  
    }
  return -1; /* Did not find x,y */
}



int ecl_grid_get_global_index_from_xy_top( const ecl_grid_type * ecl_grid , double x , double y) {
  return ecl_grid_get_global_index_from_xy( ecl_grid , ecl_grid->nz - 1 , false , x , y );
}

int ecl_grid_get_global_index_from_xy_bottom( const ecl_grid_type * ecl_grid , double x , double y) {
  return ecl_grid_get_global_index_from_xy( ecl_grid , 0 , true , x , y );
}


static void ecl_grid_clear_visited( ecl_grid_type * grid ) {
  if (grid->visited == NULL)
    grid->visited = util_calloc( grid->size , sizeof * grid->visited );
  
  { 
    int i;
    for (i=0; i < grid->size; i++)
      grid->visited[i] = false;
  }
}


/* 
   Box coordinates are not inclusive, i.e. [i1,i2) 
*/
static int ecl_grid_box_contains_xyz( const ecl_grid_type * grid , int i1, int i2 , int j1 , int j2 , int k1 , int k2 , const point_type * p) {

  int i,j,k;
  int global_index = -1;
  for (k=k1; k < k2; k++)
    for (j=j1; j < j2; j++)
      for (i=i1; i < i2; i++) {
        global_index = ecl_grid_get_global_index3( grid , i , j , k);
        if (!grid->visited[ global_index ]) {
          grid->visited[ global_index ] = true;
          if (ecl_cell_contains_point( ecl_grid_get_cell( grid , global_index ) , p )) {
            return global_index;
          }
        }
      }
  return -1;  /* Returning -1; did not find xyz. */
}


/**
   This function will find the global index of the cell containing the
   world coordinates (x,y,z), if no cell can be found the function
   will return -1.

   The function is basically based on scanning through the cells in
   natural (i fastest) order and querying whether the cell[i,j,k]
   contains the (x,y,z) point; not very elegant :-(

   The last argument - 'start_index' - can be used to speed things up
   a bit if you have reasonable guess of where the the (x,y,z) is
   located. The start_index value is used as this:


     start_index == 0: I do not have a clue, start from the beginning
        and scan through the grid linearly.


     start_index != 0: 
        1. Check the cell 'start_index'.
        2. Check the neighbours (i +/- 1, j +/- 1, k +/- 1 ).
        3. Give up and do a linear search starting from start_index.

*/



int ecl_grid_get_global_index_from_xyz(ecl_grid_type * grid , double x , double y , double z , int start_index) {
  int global_index;
  point_type p;
  point_set( &p , x , y , z);
  ecl_grid_clear_visited( grid );
  
  if (start_index >= 0) {
    /* Try start index */
    if (ecl_cell_contains_point( ecl_grid_get_cell( grid , start_index) , &p ))
      return start_index;
    else {
      /* Try neighbours */
      int i,j,k;
      int i1,i2,j1,j2,k1,k2;
      int nx,ny,nz;
      ecl_grid_get_dims( grid , &nx , &ny , &nz , NULL);
      ecl_grid_get_ijk1( grid , start_index , &i , &j , &k);

      i1 = util_int_max( 0 , i - 1 );
      j1 = util_int_max( 0 , j - 1 );
      k1 = util_int_max( 0 , k - 1 );
      
      i2 = util_int_min( nx , i + 1 );
      j2 = util_int_min( ny , j + 1 );
      k2 = util_int_min( nz , k + 1 );
      
      global_index = ecl_grid_box_contains_xyz( grid , i1 , i2 , j1 , j2 , k1 , k2 , &p);
      if (global_index >= 0)
        return global_index;


      /* Try a bigger box */
      i1 = util_int_max( 0 , i - 2 );
      j1 = util_int_max( 0 , j - 2 );
      k1 = util_int_max( 0 , k - 2 );
      
      i2 = util_int_min( nx , i + 2 );
      j2 = util_int_min( ny , j + 2 );
      k2 = util_int_min( nz , k + 2 );
      
      global_index = ecl_grid_box_contains_xyz( grid , i1 , i2 , j1 , j2 , k1 , k2 , &p);
      if (global_index >= 0)
        return global_index;


    }
  } 
  
  /* 
     OK - the attempted shortcuts did not pay off. We start on the
     full linear search starting from start_index.
  */
  
  {
    int index    = 0;
    global_index = -1;

    while (true) {
      int current_index = ((index + start_index) % grid->size);
      bool cell_contains;
      cell_contains = ecl_cell_contains_point( ecl_grid_get_cell( grid , current_index) , &p );
      
      if (cell_contains) {
        global_index = current_index;
        break;
      }
      index++;
      if (index == grid->size)
        break;
    } 
  }
  return global_index;
}







void ecl_grid_alloc_blocking_variables(ecl_grid_type * grid, int block_dim) {
  int index;
  grid->block_dim = block_dim;
  if (block_dim == 2)
    grid->block_size = grid->nx* grid->ny; // Not supported
  else if (block_dim == 3)
    grid->block_size = grid->size;
  else
    util_abort("%: valid values are two and three. Value:%d invaid \n",__func__ , block_dim);

  grid->values         = util_calloc( grid->block_size , sizeof * grid->values );
  for (index = 0; index < grid->block_size; index++)
    grid->values[index] = double_vector_alloc( 0 , 0.0 );
}



void ecl_grid_init_blocking(ecl_grid_type * grid) {
  int index;
  for (index = 0; index < grid->block_size; index++)
    double_vector_reset(grid->values[index]);
  grid->last_block_index = 0;
}




bool ecl_grid_block_value_3d(ecl_grid_type * grid, double x , double y , double z , double value) {
  if (grid->block_dim != 3)
    util_abort("%s: Wrong blocking dimension \n",__func__);
  {
    int global_index = ecl_grid_get_global_index_from_xyz( grid , x , y , z , grid->last_block_index);
    if (global_index >= 0) {
      double_vector_append( grid->values[global_index] , value);
      grid->last_block_index = global_index;
      return true;
    } else
      return false;
  }
}




double ecl_grid_block_eval3d(ecl_grid_type * grid , int i, int j , int k ,block_function_ftype * blockf ) {
  int global_index = ecl_grid_get_global_index3(grid , i,j,k);
  return blockf( grid->values[global_index]);
}


int ecl_grid_get_block_count3d(const ecl_grid_type * grid , int i , int j, int k) {
  int global_index = ecl_grid_get_global_index3(grid , i,j,k);
  return double_vector_size( grid->values[global_index]);
}

/* End of blocking functions                                     */
/*****************************************************************/

void ecl_grid_free(ecl_grid_type * grid) {
  ecl_grid_free_cells( grid );
  util_safe_free(grid->index_map);
  util_safe_free(grid->inv_index_map);
  
  util_safe_free(grid->fracture_index_map);
  util_safe_free(grid->inv_fracture_index_map);
  
  if (grid->values != NULL) {
    int i;
    for (i=0; i < grid->block_size; i++)
      double_vector_free( grid->values[i] );
    free( grid->values );
  }
  if (grid->grid_nr == 0) { /* This is the main grid. */
    vector_free( grid->LGR_list );
    hash_free( grid->LGR_hash );
  }
  if (grid->coord_kw != NULL)
    ecl_kw_free( grid->coord_kw );
  
  vector_free( grid->coarse_cells );
  hash_free( grid->children );
  util_safe_free( grid->parent_name );
  util_safe_free( grid->visited );
  util_safe_free( grid->name );
  free( grid );
}


void ecl_grid_free__( void * arg ) {
  ecl_grid_type * ecl_grid = ecl_grid_safe_cast( arg );
  ecl_grid_free( ecl_grid );
}




void ecl_grid_get_distance(const ecl_grid_type * grid , int global_index1, int global_index2 , double *dx , double *dy , double *dz) {
  ecl_cell_type * cell1 = ecl_grid_get_cell( grid , global_index1);
  ecl_cell_type * cell2 = ecl_grid_get_cell( grid , global_index2);
  ecl_cell_assert_center( cell1 );
  ecl_cell_assert_center( cell2 );
  {
    *dx = cell1->center.x - cell2->center.x;
    *dy = cell1->center.y - cell2->center.y;
    *dz = cell1->center.z - cell2->center.z;
  }
}



/*****************************************************************/
/* Index based query functions */
/*****************************************************************/



/**
   Only checks that i,j,k are in the required intervals:
  
      0 <= i < nx
      0 <= j < ny
      0 <= k < nz

*/
   
bool ecl_grid_ijk_valid(const ecl_grid_type * grid , int i , int j , int k) {
  bool OK = false;

  if (i >= 0 && i < grid->nx)
    if (j >= 0 && j < grid->ny)
      if (k >= 0 && k < grid->nz)
        OK = true;

  return OK;
}


void ecl_grid_get_dims(const ecl_grid_type * grid , int *nx , int * ny , int * nz , int * active_size) {
  if (nx != NULL) *nx                   = grid->nx;
  if (ny != NULL) *ny                   = grid->ny;
  if (nz != NULL) *nz                   = grid->nz;
  if (active_size != NULL) *active_size = grid->total_active;
}


int ecl_grid_get_nz( const ecl_grid_type * grid ) {
  return grid->nz;
}

int ecl_grid_get_nx( const ecl_grid_type * grid ) {
  return grid->nx;
}

int ecl_grid_get_ny( const ecl_grid_type * grid ) {
  return grid->ny;
}

int ecl_grid_get_nactive( const ecl_grid_type * grid ) {
  return grid->total_active;
}


grid_dims_type  ecl_grid_iget_dims( const ecl_grid_type * grid , int grid_nr) {
  grid_dims_type dims;
  const ecl_grid_type * lgr;

  if (grid_nr == 0) 
    lgr = grid;
  else
    lgr = ecl_grid_iget_lgr( grid , grid_nr - 1 );
  
  dims.nx = lgr->nx;
  dims.ny = lgr->ny;
  dims.nz = lgr->nz;
  dims.nactive = lgr->total_active;
  
  return dims;
}



int ecl_grid_get_nactive_fracture( const ecl_grid_type * grid ) {
  return grid->total_active_fracture;
}


int ecl_grid_get_parent_cell1( const ecl_grid_type * grid , int global_index ) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid, global_index );
  return cell->host_cell;
}


int ecl_grid_get_parent_cell3( const ecl_grid_type * grid , int i , int j , int k) {
  int global_index = ecl_grid_get_global_index__(grid , i , j , k);
  return ecl_grid_get_parent_cell1( grid , global_index );
}



/*****************************************************************/
/* Functions for converting between the different index types. */

/**
   Converts: (i,j,k) -> global_index. i,j,k are zero offset.
*/

int ecl_grid_get_global_index3(const ecl_grid_type * ecl_grid , int i , int j , int k) {
  if (ecl_grid_ijk_valid(ecl_grid , i , j , k))
    return ecl_grid_get_global_index__(ecl_grid , i , j , k);
  else {
    util_abort("%s: i,j,k = (%d,%d,%d) is invalid:\n\n  nx: [0,%d>\n  ny: [0,%d>\n  nz: [0,%d>\n",__func__ , i,j,k,ecl_grid->nx,ecl_grid->ny,ecl_grid->nz);
    return -1; /* Compiler shut up. */
  }
}


/**
   Converts: active_index -> global_index
*/

int ecl_grid_get_global_index1A(const ecl_grid_type * ecl_grid , int active_index) {
  return ecl_grid->inv_index_map[active_index];
}


int ecl_grid_get_global_index1F(const ecl_grid_type * ecl_grid , int active_fracture_index) {
  return ecl_grid->inv_fracture_index_map[active_fracture_index];
}



/**
   Converts: (i,j,k) -> active_index
   (i,j,k ) are zero offset.
   
   Will return -1 if the cell is not active.
*/

int ecl_grid_get_active_index3(const ecl_grid_type * ecl_grid , int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3(ecl_grid , i,j,k);  /* In range: [0,nx*ny*nz) */
  return ecl_grid_get_active_index1(ecl_grid , global_index);
}


/**
   Converts: global_index -> active_index.
   
   Will return -1 if the cell is not active.
*/

int ecl_grid_get_active_index1(const ecl_grid_type * ecl_grid , int global_index) {
  return ecl_grid->index_map[global_index];
}


int ecl_grid_get_active_fracture_index3(const ecl_grid_type * ecl_grid , int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3(ecl_grid , i,j,k);  /* In range: [0,nx*ny*nz) */
  return ecl_grid_get_active_fracture_index1(ecl_grid , global_index);
}


/**
   Converts: global_index -> active_index.
   
   Will return -1 if the cell is not active.
*/

int ecl_grid_get_active_fracture_index1(const ecl_grid_type * ecl_grid , int global_index) {
  if (ecl_grid->fracture_index_map == NULL)
    return -1; 
  else
    return ecl_grid->fracture_index_map[global_index];
}



/*
  Converts global_index -> (i,j,k)
  
  This function returns C-based zero offset indices. cell_
*/

void ecl_grid_get_ijk1(const ecl_grid_type * grid , int global_index, int *i, int *j , int *k) {
  *k = global_index / (grid->nx * grid->ny); global_index -= (*k) * (grid->nx * grid->ny);
  *j = global_index / grid->nx;              global_index -= (*j) *  grid->nx;
  *i = global_index;
}

/*
  Converts active_index -> (i,j,k)
*/

void ecl_grid_get_ijk1A(const ecl_grid_type *ecl_grid , int active_index , int *i, int * j, int * k) {
  if (active_index >= 0 && active_index < ecl_grid->total_active) {
    int global_index = ecl_grid_get_global_index1A( ecl_grid , active_index );
    ecl_grid_get_ijk1(ecl_grid , global_index , i,j,k);
  } else
    util_abort("%s: error active_index:%d invalid - grid has only:%d active cells. \n",__func__ , active_index , ecl_grid->total_active);
}


/******************************************************************/
/*
  Functions to get the 'true' (i.e. UTM or whatever) position (x,y,z).
*/

/*
  ijk are C-based zero offset.
*/

void ecl_grid_get_xyz1(const ecl_grid_type * grid , int global_index , double *xpos , double *ypos , double *zpos) {
  ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  ecl_cell_assert_center( cell );
  {
    *xpos = cell->center.x;
    *ypos = cell->center.y;
    *zpos = cell->center.z;
  }
}



void ecl_grid_get_xyz3(const ecl_grid_type * grid , int i, int j , int k, double *xpos , double *ypos , double *zpos) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , k );
  ecl_grid_get_xyz1( grid , global_index , xpos , ypos , zpos);
}





/**
   This function will return (by reference) the x,y,z values of corner
   nr 'corner_nr' in cell 'global_index'. See the documentation of
   tetraheder decomposition for the numbering of the corners.
*/


void ecl_grid_get_corner_xyz1(const ecl_grid_type * grid , int global_index , int corner_nr , double * xpos , double * ypos , double * zpos ) {
  if ((corner_nr >= 0) &&  (corner_nr <= 7)) {
    const ecl_cell_type * cell  = ecl_grid_get_cell( grid , global_index );
    const point_type      point = cell->corner_list[ corner_nr ];
    *xpos = point.x;
    *ypos = point.y;
    *zpos = point.z;
  }
}


void ecl_grid_get_corner_xyz3(const ecl_grid_type * grid , int i , int j , int k, int corner_nr , double * xpos , double * ypos , double * zpos ) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , k );
  ecl_grid_get_corner_xyz1( grid , global_index , corner_nr , xpos , ypos , zpos);
}



void ecl_grid_get_xyz1A(const ecl_grid_type * grid , int active_index , double *xpos , double *ypos , double *zpos) {
  const int global_index = ecl_grid_get_global_index1A( grid , active_index );
  ecl_grid_get_xyz1( grid , global_index , xpos , ypos , zpos );
}



double ecl_grid_get_cdepth1(const ecl_grid_type * grid , int global_index) {
  ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  ecl_cell_assert_center( cell );
  return cell->center.z;
}


double ecl_grid_get_cdepth3(const ecl_grid_type * grid , int i, int j , int k) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , k );
  return ecl_grid_get_cdepth1( grid , global_index );
}


int ecl_grid_locate_depth( const ecl_grid_type * grid , double depth , int i , int j ) {
  if (depth < ecl_grid_get_top2( grid , i , j))
    return -1;
  else if (depth >= ecl_grid_get_bottom2( grid , i , j ))
    return -1 * grid->nz;
  else {
    int k=0;
    double bottom = ecl_grid_get_top3( grid , i , j , k);

    while (true) {
      double top = bottom;
      bottom = ecl_grid_get_bottom3( grid , i , j , k );

      if ((depth >= top) && (depth < bottom)) 
        return k;
      
      k++;
      if (k == grid->nz)
        util_abort("%s: internal error when scanning for depth:%g \n",__func__ , depth);
    }
  }
}



/**
   Returns the depth of the top surface of the cell. 
*/

double ecl_grid_get_top1(const ecl_grid_type * grid , int global_index) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index );
  double depth = 0;
  int ij;

  for (ij = 0; ij < 4; ij++) 
    depth += cell->corner_list[ij].z;
  
  return depth * 0.25;
}



double ecl_grid_get_top3(const ecl_grid_type * grid , int i, int j , int k) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , k );
  return ecl_grid_get_top1( grid , global_index );
}


double ecl_grid_get_top2(const ecl_grid_type * grid , int i, int j) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , 0);
  return ecl_grid_get_top1( grid , global_index );
}


double ecl_grid_get_bottom2(const ecl_grid_type * grid , int i, int j) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , grid->nz - 1);
  return ecl_grid_get_bottom1( grid , global_index );
}



double ecl_grid_get_top1A(const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
  return ecl_grid_get_top1( grid , global_index );
}


/**
   Returns the depth of the bottom surface of the cell. 
*/

double ecl_grid_get_bottom1(const ecl_grid_type * grid , int global_index) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  double depth = 0;
  int ij;

  for (ij = 0; ij < 4; ij++) 
    depth += cell->corner_list[ij + 4].z;
  
  return depth * 0.25;
}


double ecl_grid_get_bottom3(const ecl_grid_type * grid , int i, int j , int k) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , k );
  return ecl_grid_get_bottom1( grid , global_index );
}



double ecl_grid_get_bottom1A(const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
  return ecl_grid_get_bottom1( grid , global_index );
}


  
double ecl_grid_get_cell_thickness1( const ecl_grid_type * grid , int global_index ) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  double thickness = 0;
  int ij;
  
  for (ij = 0; ij < 4; ij++) 
    thickness += (cell->corner_list[ij + 4].z - cell->corner_list[ij].z);
  
  return thickness * 0.25;
}


double ecl_grid_get_cell_thickness3( const ecl_grid_type * grid , int i , int j , int k) {
  const int global_index = ecl_grid_get_global_index3(grid , i,j,k);
  return ecl_grid_get_cell_thickness1( grid , global_index );
}


/*****************************************************************/
/* Functions to query whether a cell is active or not.           */

/*
   Global index in [0,...,nx*ny*nz)
*/

bool ecl_grid_cell_active1(const ecl_grid_type * ecl_grid , int global_index) {
  if (ecl_grid->index_map[global_index] >= 0)
    return true;
  else
    return false;
}



bool ecl_grid_cell_active3(const ecl_grid_type * ecl_grid, int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k);
  return ecl_grid_cell_active1( ecl_grid , global_index );
}

/*****************************************************************/

bool ecl_grid_cell_invalid1(const ecl_grid_type * ecl_grid , int global_index) {
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);
  return GET_CELL_FLAG(cell , CELL_FLAG_TAINTED);
}

bool ecl_grid_cell_invalid3(const ecl_grid_type * ecl_grid , int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k);
  return ecl_grid_cell_invalid1( ecl_grid , global_index );
}

double ecl_grid_cell_invalid1A(const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
  return ecl_grid_cell_invalid1( grid , global_index );
}



/*****************************************************************/
/* Functions for LGR query/lookup/... */

static void __assert_main_grid(const ecl_grid_type * ecl_grid) {
  if (ecl_grid->grid_nr != 0) 
    util_abort("%s: tried to get LGR grid from another LGR_grid - only main grid can be used as first input \n",__func__);
}


/**
   This functon will return a a ecl_grid instance corresponding to the
   lgr with name lgr_name. The function will fail HARD if no lgr with
   this name is installed under the present main grid; check first
   with ecl_grid_has_lgr() if you are whimp.
   
   Leading/trailing spaces on lgr_name are stripped prior to the hash lookup.
*/


ecl_grid_type * ecl_grid_get_lgr(const ecl_grid_type * main_grid, const char * __lgr_name) {
  __assert_main_grid( main_grid );
  {
    char * lgr_name          = util_alloc_strip_copy( __lgr_name );
    ecl_grid_type * lgr_grid = hash_get(main_grid->LGR_hash , lgr_name);
    free(lgr_name);
    return lgr_grid;
  }
}


/**
   Returns true/false if the main grid has a a lgr with name
   __lgr_name. Leading/trailing spaces are stripped before checking.
*/

bool ecl_grid_has_lgr(const ecl_grid_type * main_grid, const char * __lgr_name) {
  __assert_main_grid( main_grid );
  {
    char * lgr_name          = util_alloc_strip_copy( __lgr_name );
    bool has_lgr             = hash_has_key( main_grid->LGR_hash , lgr_name );
    free(lgr_name);
    return has_lgr;
  }
}


int ecl_grid_get_num_coarse_groups( const ecl_grid_type * main_grid ) {
  return vector_get_size( main_grid->coarse_cells );
}

bool ecl_grid_have_coarse_cells( const ecl_grid_type * main_grid ) {
  if (vector_get_size( main_grid->coarse_cells ) == 0)
    return false;
  else
    return true;
}

/**
   Return the number of LGR's associated with this main grid
   instance. The main grid is not counted.
*/
int ecl_grid_get_num_lgr(const ecl_grid_type * main_grid ) {
  __assert_main_grid( main_grid );
  return vector_get_size( main_grid->LGR_list ) - 1;  
}

/**
   The lgr_nr has zero offset, not counting the main grid, i.e.

      ecl_grid_iget_lgr( ecl_grid , 0);
   
   will return the first LGR - and fail HARD if there are no LGR's.
*/

ecl_grid_type * ecl_grid_iget_lgr(const ecl_grid_type * main_grid, int lgr_nr) {
  __assert_main_grid( main_grid );
  return vector_iget(  main_grid->LGR_list , lgr_nr + 1);
}


/**
   The following functions will return the LGR subgrid referenced by
   the coordinates given. Observe the following:

   1. The functions will happily return NULL if no LGR is associated
      with the cell indicated - in fact that is (currently) the only
      way to query whether a particular cell has a LGR.
      
   2. If a certain cell is refined in several levels this function
      will return a pointer to the first level of refinement. The
      return value can can be used for repeated calls to descend
      deeper into the refinement hierarchy.  
*/


const ecl_grid_type * ecl_grid_get_cell_lgr1(const ecl_grid_type * grid , int global_index ) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  return cell->lgr;
}


const ecl_grid_type * ecl_grid_get_cell_lgr3(const ecl_grid_type * grid , int i, int j , int k) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , k );
  return ecl_grid_get_cell_lgr1( grid , global_index );
}



const ecl_grid_type * ecl_grid_get_cell_lgr1A(const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A( grid , active_index );
  return ecl_grid_get_cell_lgr1( grid , global_index );
}


/**
   Will return the global grid for a lgr. If the input grid is indeed
   a global grid itself the function will return NULL.
*/
const ecl_grid_type * ecl_grid_get_global_grid( const ecl_grid_type * grid ) {
  return grid->global_grid;
}


/*****************************************************************/

/** 
    Allocates a stringlist instance with the lookup names of the lgr names in this grid.
*/

stringlist_type * ecl_grid_alloc_lgr_name_list(const ecl_grid_type * ecl_grid) {
  __assert_main_grid( ecl_grid );
  {
    return hash_alloc_stringlist( ecl_grid->LGR_hash );
  }
}



/*****************************************************************/

/**
   This function returns the grid_nr field of the field; this is just
   the occurence number in the grid file. Starting with 0 at the main
   grid, and then increasing consecutively through the lgr sections.

   Observe that there is A MAJOR POTENTIAL for confusion with the
   ecl_grid_iget_lgr() function, the latter does not refer to the main
   grid and returns the first lgr section (which has grid_nr == 1) for
   input argument 0.
*/


int ecl_grid_get_grid_nr( const ecl_grid_type * ecl_grid ) { 
  return ecl_grid->grid_nr; 
}


const char * ecl_grid_get_name( const ecl_grid_type * ecl_grid ) {
  return ecl_grid->name;
}


int ecl_grid_get_global_size( const ecl_grid_type * ecl_grid ) {
  return ecl_grid->nx * ecl_grid->ny * ecl_grid->nz;
}

int ecl_grid_get_active_size( const ecl_grid_type * ecl_grid ) {
  return ecl_grid_get_nactive( ecl_grid );
}

double ecl_grid_get_cell_volume1( const ecl_grid_type * ecl_grid, int global_index ) {
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
  return ecl_cell_get_volume( cell );
}


double ecl_grid_get_cell_volume3( const ecl_grid_type * ecl_grid, int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k);
  return ecl_grid_get_cell_volume1( ecl_grid , global_index );
}


void ecl_grid_summarize(const ecl_grid_type * ecl_grid) {
  int             active_cells , nx,ny,nz;
  ecl_grid_get_dims(ecl_grid , &nx , &ny , &nz , &active_cells);
  printf("      Name ..................: %s  \n",ecl_grid->name);
  printf("      Active cells ..........: %d \n",active_cells);
  printf("      Active fracture cells..: %d \n",ecl_grid_get_nactive_fracture( ecl_grid ));
  printf("      nx ....................: %d \n",nx);
  printf("      ny ....................: %d \n",ny);
  printf("      nz ....................: %d \n",nz);
  printf("      Volume ................: %d \n",nx*ny*nz);
  printf("      Origo X................: %10.2f \n",ecl_grid->origo[0]);
  printf("      Origo Y................: %10.2f \n",ecl_grid->origo[1]);
  
  if (ecl_grid->grid_nr == 0) {
    int grid_nr;
    for (grid_nr=1; grid_nr < vector_get_size( ecl_grid->LGR_list ); grid_nr++) {
      printf("\n");
      ecl_grid_summarize( vector_iget_const( ecl_grid->LGR_list , grid_nr ));
    }
  }
  ecl_grid_test_lgr_consistency( ecl_grid );
}

/*****************************************************************/
/**
   
   This function is used to translate (with the help of the ecl_grid
   functionality) i,j,k to an index which can be used to look up an
   element in the ecl_kw instance. It is just a minor convenience
   function.

   * If the ecl_kw instance has nx*ny*nz (i,j,k) are translated to a
     global index with ecl_grid_get_global_index3(). This is typically
     the case when the ecl_kw instance represents a petrophysical
     property which is e.g. loaded from a INIT file.

   * If the ecl_kw instance has nactive elements the (i,j,k) indices
     are converted to an active index with
     ecl_grid_get_active_index3(). This is typically the case if the
     ecl_kw instance is a solution vector which has been loaded from a
     restart file. If you ask for an inactive cell the function will
     return -1.

   * If the ecl_kw instance has neither nx*ny*nz nor nactive elements
     the function will fail HARD.

   * The return value is double, irrespective of the type of the
     underlying datatype of the ecl_kw instance - the function will
     fail HARD if the underlying type can not be safely converted to
     double, i.e. if it is not in the set [ecl_float_type ,
     ecl_int_type , ecl_double_type].

   * i,j,k: C-based zero offset grid coordinates.

*/

static int ecl_grid_get_property_index__(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k) {
  int kw_size        = ecl_kw_get_size( ecl_kw );
  int lookup_index   = -1;

  if (kw_size == ecl_grid->nx * ecl_grid->ny * ecl_grid->nz) 
    lookup_index = ecl_grid_get_global_index3(ecl_grid , i , j , k);
  else if (kw_size == ecl_grid->total_active) 
    /* Will be set to -1 if the cell is not active. */ 
    lookup_index = ecl_grid_get_active_index3(ecl_grid , i , j , k);
  else 
    util_abort("%s: incommensurable size ... \n",__func__);

  return lookup_index;
}



static bool ecl_grid_get_property__(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k, void * value) {
  ecl_type_enum ecl_type = ecl_kw_get_type( ecl_kw );
  if ((ecl_type == ECL_FLOAT_TYPE) || (ecl_type == ECL_INT_TYPE) || (ecl_type == ECL_DOUBLE_TYPE)) {
    int lookup_index   = ecl_grid_get_property_index__( ecl_grid , ecl_kw , i , j , k );

    if (lookup_index >= 0) {
      ecl_kw_iget( ecl_kw , lookup_index , value );
      return true;
    } else 
      return false;
      
  } else {
    util_abort("%s: sorry - can not lookup ECLIPSE type:%s with %s.\n",__func__ , ecl_util_get_type_name( ecl_type ) , __func__);
    return false;
  }
}


double ecl_grid_get_double_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k) {
  ecl_type_enum ecl_type = ecl_kw_get_type( ecl_kw );
  if (ecl_type == ECL_DOUBLE_TYPE) {
    double value;
    if (ecl_grid_get_property__( ecl_grid , ecl_kw , i , j , k , &value))
      return value;
    else
      return -1;   // (i,j,k) Points to an inactive cell.
  } else {
    util_abort("%s: Wrong type \n" , __func__);
    return -1;
  }
}


int ecl_grid_get_int_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k) {
  ecl_type_enum ecl_type = ecl_kw_get_type( ecl_kw );
  if (ecl_type == ECL_INT_TYPE) {
    int value;

    if (ecl_grid_get_property__( ecl_grid , ecl_kw , i , j , k , &value))
      return value;
    else
      return -1;    // (i,j,k) Points to an inactive cell.

  } else {
    util_abort("%s: Wrong type \n" , __func__);
    return -1;
  }
}


float ecl_grid_get_float_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k) {
  ecl_type_enum ecl_type = ecl_kw_get_type( ecl_kw );
  if (ecl_type == ECL_FLOAT_TYPE) {
    float value;
    
    if (ecl_grid_get_property__( ecl_grid , ecl_kw , i , j , k , &value))
      return value;
    else
      return -1;    // (i,j,k) Points to an inactive cell.
    
  } else {
    util_abort("%s: Wrong type \n" , __func__);
    return -1;
  }
}

double ecl_grid_get_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k) {
  ecl_type_enum ecl_type = ecl_kw_get_type( ecl_kw );
  if ((ecl_type == ECL_FLOAT_TYPE) || (ecl_type == ECL_INT_TYPE) || (ecl_type == ECL_DOUBLE_TYPE)) {
    int lookup_index   = ecl_grid_get_property_index__( ecl_grid , ecl_kw , i , j , k );
    
    if (lookup_index >= 0)
      return ecl_kw_iget_as_double( ecl_kw , lookup_index );
    else
      return -1;   /* Tried to lookup an inactive cell. */

  } else {
    util_abort("%s: sorry - can not lookup ECLIPSE type:%s with %s.\n",__func__ , ecl_util_get_type_name( ecl_type ) , __func__);
    return -1;
  }
}






/**
   Will fill the double_vector instance @column with values from
   ecl_kw from the column given by (i,j). If @ecl_kw has size nactive
   the inactive k values will not be set, i.e. you should make sure
   that the default value of the @column instance has been properly
   set beforehand.

   The column vector will be filled with double values, the content of
   ecl_kw will be converted to double in the case INTE,REAL and DOUB
   types, otherwsie it is crash and burn.
*/


void ecl_grid_get_column_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j, double_vector_type * column) {
  ecl_type_enum ecl_type = ecl_kw_get_type( ecl_kw );
  if ((ecl_type == ECL_FLOAT_TYPE) || (ecl_type == ECL_INT_TYPE) || (ecl_type == ECL_DOUBLE_TYPE)) {
    int kw_size           = ecl_kw_get_size( ecl_kw );
    bool use_global_index = false;

    if (kw_size == ecl_grid->nx * ecl_grid->ny * ecl_grid->nz) 
      use_global_index = true;
    else if (kw_size == ecl_grid->total_active) 
      use_global_index = false;
    else 
      util_abort("%s: incommensurable sizes: nx*ny*nz = %d  nactive=%d  kw_size:%d \n",__func__ , ecl_grid->size , ecl_grid->total_active , ecl_kw_get_size( ecl_kw ));

    double_vector_reset( column );
    {
      int k;
      for (k=0; k < ecl_grid->nz; k++) {
        if (use_global_index) {
          int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k );
          double_vector_iset( column , k , ecl_kw_iget_as_double( ecl_kw , global_index ));
        } else {
          int active_index = ecl_grid_get_active_index3( ecl_grid , i , j , k );
          if (active_index >= 0)
            double_vector_iset( column, k , ecl_kw_iget_as_double( ecl_kw , active_index ));
        }
      }
    }
  } else 
    util_abort("%s: sorry - can not lookup ECLIPSE type:%s with %s.\n",__func__ , ecl_util_get_type_name( ecl_type ) , __func__);
}


/*****************************************************************/
/**
   This function will look up all the indices in the grid where the
   region_kw has a certain value (region_value). The ecl_kw instance
   must be loaded beforehand, typically with the functions
   ecl_kw_grdecl_fseek_kw / ecl_kw_fscanf_alloc_grdecl_data.

   The two boolean flags active_only and export_active_index determine
   how active/inactive indieces should be handled:

     active_only: Means that only cells which match the required
        region_value AND are also active are stored. If active_only is
        set to false, ALL cells matching region value are stored in
        index_list.

     export_active_index: if this value is true the the index of the
        cell is in the space of active cells, otherwise it is in terms
        of the global indexing.

   Observe the following about the ecl_kw instance wth region data:

    * It must be of type integer - otherwise we blow up hard.  The
    * size must be the total number of cells (should handle boxes and
      so on ...)

   Observe that there is no way to get ijk from this function, then
   you must call ecl_grid_get_ijk() afterwards. the return value is
   the number of cells found.
*/

int ecl_grid_get_region_cells(const ecl_grid_type * ecl_grid , const ecl_kw_type * region_kw , int region_value , bool active_only, bool export_active_index , int_vector_type * index_list) {
  int cells_found = 0;
  if (ecl_kw_get_size( region_kw ) == ecl_grid->size) {
    if (ecl_kw_get_type( region_kw ) == ECL_INT_TYPE) {
                const int * region_ptr = ecl_kw_iget_ptr( region_kw , 0);
                int_vector_reset( index_list );
      

      {
        int global_index;
        for (global_index = 0; global_index < ecl_grid->size; global_index++) {
          if (region_ptr[global_index] == region_value) {
             if (!active_only || (ecl_grid->index_map[global_index] >= 0)) {
              /* Okay - this index should be included */
              if (export_active_index)
                int_vector_iset(index_list , cells_found , ecl_grid->index_map[global_index]);
              else
                int_vector_iset(index_list , cells_found , global_index);
              cells_found++;
            }
           }
        }
      }
    }  else
      util_abort("%s: type mismatch - regions_kw must be of type integer \n",__func__);

  } else
    util_abort("%s: size mismatch grid has %d cells - region specifier:%d \n",__func__ , ecl_grid->size , ecl_kw_get_size( region_kw ));
  return cells_found;
}



/*****************************************************************/



void ecl_grid_grdecl_fprintf_kw( const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , const char * special_header , FILE * stream , double double_default) {
  int src_size = ecl_kw_get_size( ecl_kw );
  if (src_size == ecl_grid->size)
    ecl_kw_fprintf_grdecl__( ecl_kw , special_header , stream );
  else if (src_size == ecl_grid->total_active) {
    void  * default_ptr = NULL;
    float   float_default;
    int     int_default;
    int     bool_default;
    ecl_type_enum ecl_type = ecl_kw_get_type( ecl_kw );
    
    if (ecl_type == ECL_FLOAT_TYPE) {
      float_default = (float) double_default;
      default_ptr = &float_default;
    } else if (ecl_type == ECL_INT_TYPE) {
      int_default = (int) double_default;
      default_ptr = &int_default;
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      default_ptr = &double_default;
    } else if (ecl_type == ECL_BOOL_TYPE) {
      int tmp = (int) double_default;
      if (tmp == 1)
        bool_default = ECL_BOOL_TRUE_INT;
      else if (tmp == 0)
        bool_default = ECL_BOOL_FALSE_INT;
      else
        util_abort("%s: only 0 and 1 are allowed for bool interpolation\n",__func__);
      default_ptr = &bool_default;
    }
    
    if (default_ptr == NULL) 
      util_abort("%s: invalid type \n",__func__);
    
    {
      ecl_kw_type * tmp_kw = ecl_kw_alloc_scatter_copy( ecl_kw , ecl_grid->size , ecl_grid->inv_index_map , default_ptr );
      ecl_kw_fprintf_grdecl__( tmp_kw , special_header , stream );
      ecl_kw_free( tmp_kw );
    }
  } else 
    util_abort("%s: size mismatch. ecl_kw must have either nx*ny*ny elements or nactive elements\n",__func__);

}

/*****************************************************************/


static bool ecl_grid_test_lgr_consistency2( const ecl_grid_type * parent , const ecl_grid_type * child) {
  bool consistent = true;
  int child_size = ecl_grid_get_global_size( child );
  int child_index;
  for (child_index = 0; child_index < child_size; child_index++) {
    int parent_cell = ecl_grid_get_parent_cell1( child , child_index );
    if (parent_cell >= 0) {
      const ecl_grid_type * child_test = ecl_grid_get_cell_lgr1( parent , parent_cell );
      if (child != child_test) {
        fprintf(stderr , "Child parent mapping failure : ptr difference.\n");
        consistent = false;
      }
    } else {
      fprintf(stderr , "Child parent mapping failure : lgr has no parent cell.\n");
      consistent = false;
    }
  }
  return consistent;
}



bool ecl_grid_test_lgr_consistency( const ecl_grid_type * ecl_grid ) {
  hash_iter_type * lgr_iter = hash_iter_alloc( ecl_grid->children );
  bool consistent = true;
  while (!hash_iter_is_complete( lgr_iter )) {
    const ecl_grid_type * lgr = hash_iter_get_next_value( lgr_iter );
    consistent &= ecl_grid_test_lgr_consistency2( ecl_grid , lgr );
    consistent &= ecl_grid_test_lgr_consistency( lgr );
  }
  hash_iter_free( lgr_iter ); 
  return consistent;
}


static void ecl_grid_dump__(const ecl_grid_type * grid , FILE * stream) {
  util_fwrite_int( grid->grid_nr , stream );
  util_fwrite_string( grid->name , stream );
  util_fwrite_int( grid->nx   , stream );
  util_fwrite_int( grid->nz   , stream );
  util_fwrite_int( grid->ny   , stream );
  util_fwrite_int( grid->size , stream );
  util_fwrite_int( grid->total_active , stream );
  util_fwrite_int_vector( grid->index_map , grid->size , stream , __func__ );
  util_fwrite_int_vector( grid->inv_index_map , grid->total_active , stream , __func__ );
  
  {
    int i;
    for (i=0; i < grid->size; i++) {
      const ecl_cell_type * cell = ecl_grid_get_cell( grid , i );
      ecl_cell_dump( cell , stream );
    }
  }
}


static void ecl_grid_dump_ascii__(const ecl_grid_type * grid , bool active_only , FILE * stream) {
  fprintf(stream , "Grid nr           : %d\n",grid->grid_nr);
  fprintf(stream , "Grid name         : %s\n",grid->name);
  fprintf(stream , "nx                : %6d\n",grid->nx);
  fprintf(stream , "ny                : %6d\n",grid->ny);
  fprintf(stream , "nz                : %6d\n",grid->nz);
  fprintf(stream , "nactive           : %6d\n",grid->total_active);
  fprintf(stream , "nactive fracture  : %6d\n",grid->total_active_fracture);
  
  {
    int l;
    for (l=0; l < grid->size; l++) {
      const ecl_cell_type * cell = ecl_grid_get_cell( grid , l );
      if (cell->active_index[MATRIX_INDEX] >= 0 || !active_only) {
        int i,j,k;
        ecl_grid_get_ijk1( grid , l , &i , &j , &k);
        ecl_cell_dump_ascii( cell , i,j,k , stream );
      }
    }
  }
}


/**
   The dump function will dump a binary image of the internal grid
   representation. The purpose of these dumps is to be able to test
   the internal representation of the grid. No metadata is dumped, and
   apart from byte-by-byte comparisons, the dump files are *not* meant
   to be read. 
*/


void ecl_grid_dump(const ecl_grid_type * grid , FILE * stream) {
  int i;
  for (i = 0; i < vector_get_size( grid->LGR_list ); i++) 
    ecl_grid_dump__( vector_iget_const( grid->LGR_list , i) , stream ); 
}

void ecl_grid_dump_ascii(const ecl_grid_type * grid , bool active_only , FILE * stream) {
  int i;
  for (i = 0; i < vector_get_size( grid->LGR_list ); i++) 
    ecl_grid_dump_ascii__( vector_iget_const( grid->LGR_list , i) , active_only , stream ); 
}



/*****************************************************************/

/*
 'MAPUNITS'           1 'CHAR'
 'METRES  '

'GRIDUNIT'           2 'CHAR'
 'METRES  ' '        '

*/



/**
   DIMENS
   MAPUNITS
   MAPAXES
   GRIDUNIT
   RADIAL
   COORDS
   CORNERS
   COORDS
   CORNERS
   ....
   DIMENS
   RADIAL
   COORDS
   CORNERS
   COORDS
   CORNERS
   ...
*/


static const float * ecl_grid_get_mapaxes( const ecl_grid_type * grid ) {
  if (grid->use_mapaxes)
    return grid->mapaxes;
  else
    return NULL;
}

static ecl_kw_type * ecl_grid_alloc_mapaxes_kw( const float * mapaxes ) {
  return ecl_kw_alloc_new( MAPAXES_KW , 6 , ECL_FLOAT_TYPE , mapaxes);
}

static ecl_kw_type * ecl_grid_alloc_mapunits_kw( ) {
  ecl_kw_type * mapunits_kw = ecl_kw_alloc( MAPUNITS_KW , 1 , ECL_CHAR_TYPE);
  ecl_kw_iset_string8( mapunits_kw , 0 , "METRES" );
  return mapunits_kw;
}

static ecl_kw_type * ecl_grid_alloc_gridunits_kw( ) {
  ecl_kw_type * gridunits_kw = ecl_kw_alloc( GRIDUNIT_KW , 2 , ECL_CHAR_TYPE);
  ecl_kw_iset_string8( gridunits_kw , 0 , "METRES" );
  ecl_kw_iset_string8( gridunits_kw , 1 , "" );
  return gridunits_kw;
}

/*****************************************************************/

static void ecl_grid_fwrite_mapaxes( const float * mapaxes , fortio_type * fortio) {
  ecl_kw_type * mapaxes_kw = ecl_grid_alloc_mapaxes_kw( mapaxes );
  ecl_kw_fwrite( mapaxes_kw , fortio );
  ecl_kw_free( mapaxes_kw );
}

static void ecl_grid_fwrite_mapunits( fortio_type * fortio ) {
  ecl_kw_type * mapunits_kw = ecl_grid_alloc_mapunits_kw(  );
  ecl_kw_fwrite( mapunits_kw , fortio );
  ecl_kw_free( mapunits_kw );
}


static void ecl_grid_fwrite_gridunits( fortio_type * fortio)  {
  ecl_kw_type * gridunits_kw = ecl_grid_alloc_gridunits_kw( );
  ecl_kw_fwrite( gridunits_kw , fortio );
  ecl_kw_free( gridunits_kw );
}

static void ecl_grid_fwrite_main_GRID_headers( const ecl_grid_type * ecl_grid , fortio_type * fortio ) {
  ecl_grid_fwrite_mapunits( fortio );
  if (ecl_grid->use_mapaxes) 
    ecl_grid_fwrite_mapaxes( ecl_grid->mapaxes , fortio );
  ecl_grid_fwrite_gridunits( fortio );
}


static void ecl_grid_fwrite_GRID__( const ecl_grid_type * grid , int coords_size , fortio_type * fortio) {
  if (grid->parent_grid != NULL) {
    ecl_kw_type * lgr_kw = ecl_kw_alloc(LGR_KW , 1 , ECL_CHAR_TYPE );
    ecl_kw_iset_string8( lgr_kw , 0 , grid->name );
    ecl_kw_fwrite( lgr_kw , fortio );
    ecl_kw_free( lgr_kw );
  }

  {
    ecl_kw_type * dimens_kw = ecl_kw_alloc(DIMENS_KW , 3 , ECL_INT_TYPE );
    ecl_kw_iset_int( dimens_kw , 0 , grid->nx );
    ecl_kw_iset_int( dimens_kw , 1 , grid->ny );
    if (grid->dualp_flag == FILEHEAD_SINGLE_POROSITY)
      ecl_kw_iset_int( dimens_kw , 2 , grid->nz );
    else
      ecl_kw_iset_int( dimens_kw , 2 , 2*grid->nz );

    ecl_kw_fwrite( dimens_kw , fortio );
    ecl_kw_free( dimens_kw );
  }

  if (grid->parent_grid == NULL) 
    ecl_grid_fwrite_main_GRID_headers( grid , fortio );
  
  {
    ecl_kw_type * radial_kw = ecl_kw_alloc( RADIAL_KW , 1 , ECL_CHAR_TYPE);
    ecl_kw_iset_string8( radial_kw , 0 , "FALSE" );
    ecl_kw_fwrite( radial_kw , fortio );
    ecl_kw_free( radial_kw );
  }

  {
    ecl_kw_type * coords_kw  = ecl_kw_alloc( COORDS_KW  , coords_size , ECL_INT_TYPE );
    ecl_kw_type * corners_kw = ecl_kw_alloc( CORNERS_KW , 24 , ECL_FLOAT_TYPE );
    int i,j,k;
    for (k=0; k < grid->nz; k++) {
      for (j=0; j < grid->ny; j++) {
        for (i=0; i < grid->nx; i++) {
          int global_index = ecl_grid_get_global_index__(grid , i , j , k );
          const ecl_cell_type * cell = ecl_grid_get_cell( grid ,  global_index );
          
          ecl_cell_fwrite_GRID( grid , cell , false , coords_size , i,j,k,global_index,coords_kw , corners_kw , fortio );
        }
      }
    }

    if (grid->dualp_flag != FILEHEAD_SINGLE_POROSITY) {
      for (k=grid->nz; k < 2*grid->nz; k++) {
        for (j=0; j < grid->ny; j++) {
          for (i=0; i < grid->nx; i++) {
            int global_index = ecl_grid_get_global_index__(grid , i , j , k - grid->nz );
            const ecl_cell_type * cell = ecl_grid_get_cell( grid ,  global_index );
            
            ecl_cell_fwrite_GRID( grid , cell , true , coords_size , i,j,k,global_index ,  coords_kw , corners_kw , fortio );
          }
        }
      }
    }
  }
}


void ecl_grid_fwrite_GRID( const ecl_grid_type * grid , const char * filename) {  
  int coords_size = 5;
  bool fmt_file   = false;

  fortio_type * fortio = fortio_open_writer( filename , fmt_file , ECL_ENDIAN_FLIP );
  if (hash_get_size( grid->children ) > 0)
    coords_size = 7;
  
  if (grid->coarsening_active)
    coords_size = 7;
  
  {
    int grid_nr; 
    for (grid_nr = 0; grid_nr < vector_get_size( grid->LGR_list ); grid_nr++) {
      const ecl_grid_type * igrid = vector_iget_const( grid->LGR_list , grid_nr );
      ecl_grid_fwrite_GRID__( igrid , coords_size , fortio );
    }
  }
  fortio_fclose( fortio );
}

/*****************************************************************/

/*
FILEHEAD          100:INTE
MAPUNITS            1:CHAR
MAPAXES             6:REAL
GRIDUNIT            2:CHAR
GRIDHEAD          100:INTE
COORD           15990:REAL
ZCORN          286720:REAL
ACTNUM          35840:INTE
ENDGRID             0:INTE
*/


/**
   This function does by construction not take a grid instance as
   argument, so that it can be called from an external scope to create
   a standard EGRID header without creating a grid instance first. 
*/

static void ecl_grid_fwrite_main_EGRID_header( const float * mapaxes, int dualp_flag , fortio_type * fortio ) {
  int EGRID_VERSION  = 3;
  int RELEASE_YEAR   = 2007;
  int COMPAT_VERSION = 0;
  
  {
    ecl_kw_type * filehead_kw = ecl_kw_alloc( FILEHEAD_KW , 100 , ECL_INT_TYPE );
    ecl_kw_scalar_set_int( filehead_kw , 0 );

    ecl_kw_iset_int( filehead_kw , FILEHEAD_VERSION_INDEX   , EGRID_VERSION );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_YEAR_INDEX      , RELEASE_YEAR );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_COMPAT_INDEX    , COMPAT_VERSION );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_TYPE_INDEX      , FILEHEAD_GRIDTYPE_CORNERPOINT );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_DUALP_INDEX     , dualp_flag );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_ORGFORMAT_INDEX , FILEHEAD_ORGTYPE_CORNERPOINT );

    ecl_kw_fwrite( filehead_kw , fortio );
    ecl_kw_free( filehead_kw );
  }

  ecl_grid_fwrite_mapunits( fortio );
  if (mapaxes != NULL) 
    ecl_grid_fwrite_mapaxes( mapaxes , fortio );

  ecl_grid_fwrite_gridunits( fortio );
}


static void ecl_grid_fwrite_gridhead_kw( int nx, int ny , int nz, int grid_nr, fortio_type * fortio ) {
  ecl_kw_type * gridhead_kw = ecl_grid_alloc_gridhead_kw( nx , ny , nz , grid_nr);
  ecl_kw_fwrite( gridhead_kw , fortio );
  ecl_kw_free( gridhead_kw );
}




void ecl_grid_fwrite_EGRID_header__( int dims[3] , const float mapaxes[6], int dualp_flag , fortio_type * fortio) {
  ecl_grid_fwrite_main_EGRID_header( mapaxes , dualp_flag , fortio );
  ecl_grid_fwrite_gridhead_kw( dims[0] , dims[1] , dims[2] , 0 , fortio);
}

void ecl_grid_fwrite_EGRID_header( int dims[3] , const float mapaxes[6], fortio_type * fortio) {
  ecl_grid_fwrite_EGRID_header__(dims , mapaxes , FILEHEAD_SINGLE_POROSITY , fortio );
}


/*****************************************************************/


/*
  Will scan the halfopen k-interval [k1,k2) to find a cell which has
  valid geometry. If no cell is found the function will return -1.  
*/

static int ecl_grid_get_valid_index( const ecl_grid_type * grid , int i , int j , int k1 , int k2) {
  int global_index;
  int k = k1;
  int delta = (k1 < k2) ? 1 : -1 ;

  while (true) {
    ecl_cell_type * cell;
    global_index = ecl_grid_get_global_index3( grid , i , j , k );
    
    cell = ecl_grid_get_cell( grid ,  global_index );
    if (GET_CELL_FLAG(cell , CELL_FLAG_VALID))
      return global_index;
    else {
      k += delta;
      if (k == k2)
        return -1;
    }
  }
  
}


static int ecl_grid_get_bottom_valid_index( const ecl_grid_type * grid , int i , int j ) {
  return ecl_grid_get_valid_index( grid , i , j ,  grid->nz - 1 , -1);
}

static int ecl_grid_get_top_valid_index( const ecl_grid_type * grid , int i , int j ) {
  return ecl_grid_get_valid_index( grid , i , j ,  0 , grid->nz);
}



static bool ecl_grid_init_coord_section__( const ecl_grid_type * grid , int i, int j , int corner_index , bool force_set , float * coord ) {
  
  const int top_index    = ecl_grid_get_top_valid_index( grid , i , j );
  const int bottom_index = ecl_grid_get_bottom_valid_index( grid , i , j );
  
  if (top_index == -1) 
    util_exit("% : no cell with a valid geometry description found in (i,j) = %d,%d - then what? \n",__func__ , i,j);
  
  {
    point_type top_point;
    point_type bottom_point;
  
    const ecl_cell_type * bottom_cell = ecl_grid_get_cell( grid , bottom_index );
    const ecl_cell_type * top_cell    = ecl_grid_get_cell( grid , top_index);
    
    /*
      2---3
      |   |
      0---1
    */
    int coord_offset = 6 * ( j * (grid->nx + 1) + i );
    {
      point_copy_values( &top_point    , &top_cell->corner_list[corner_index]);
      point_copy_values( &bottom_point , &bottom_cell->corner_list[ corner_index + 4]);
      
      
      if ((top_point.z == bottom_point.z) && (force_set == false)) {
        return false;
      } else {
        point_mapaxes_invtransform( &top_point    , grid->origo , grid->unit_x , grid->unit_y );
        point_mapaxes_invtransform( &bottom_point , grid->origo , grid->unit_x , grid->unit_y );
        
        coord[coord_offset]     = top_point.x;
        coord[coord_offset + 1] = top_point.y;
        coord[coord_offset + 2] = top_point.z;
        
        coord[coord_offset + 3] = bottom_point.x;
        coord[coord_offset + 4] = bottom_point.y;
        coord[coord_offset + 5] = bottom_point.z;
        
        return true;
      }
    }
  }
}


static void ecl_grid_init_coord_section( const ecl_grid_type * grid , int i, int j , float * coord ) {
  int corner_index = 0;
  
  if (i == grid->nx) {
    i -= 1;
    corner_index += 1;
  }
  
  if (j == grid->ny) {
    j -= 1;
    corner_index += 2;
  }
    
  ecl_grid_init_coord_section__( grid , i , j , corner_index, true, coord);
}




static void ecl_grid_init_coord_data( const ecl_grid_type * grid , float * coord ) {
  /*
    The coord vector contains the points defining the top and bottom
    of the pillars. The vector contains (nx + 1) * (ny + 1) 6 element
    chunks of data, where each chunk contains the coordinates (x,y,z)
    f the top and the bottom of the pillar.
  */
  int i,j;
  for (j=0; j <= grid->ny; j++) {
    for (i=0; i <= grid->nx; i++) 
      ecl_grid_init_coord_section( grid , i , j , coord );
    
  }
}



float * ecl_grid_alloc_coord_data( const ecl_grid_type * grid ) {
  float * coord = util_calloc( (grid->nx + 1) * (grid->ny + 1) * 6 , sizeof * coord );
  ecl_grid_init_coord_data( grid , coord );
  return coord;
}

void ecl_grid_assert_coord_kw( ecl_grid_type * grid ) {
  if (grid->coord_kw == NULL) {
    grid->coord_kw = ecl_kw_alloc( COORD_KW , (grid->nx + 1) * (grid->ny + 1) * 6 , ECL_FLOAT_TYPE );
    ecl_grid_init_coord_data( grid , ecl_kw_get_void_ptr( grid->coord_kw ));
  }
}





/*****************************************************************/

static void ecl_grid_init_zcorn_data( const ecl_grid_type * grid , float * zcorn ) {
  int nx = grid->nx;
  int ny = grid->ny;
  int nz = grid->nz;
  int i,j,k;
  for (j=0; j < ny; j++) {
    for (i=0; i < nx; i++) {
      for (k=0; k < nz; k++) {
        const int cell_index   = ecl_grid_get_global_index3( grid , i,j,k);
        const ecl_cell_type * cell = ecl_grid_get_cell( grid , cell_index );
        int l;

        for (l=0; l < 2; l++) {
          point_type p0 = cell->corner_list[ 4*l];
          point_type p1 = cell->corner_list[ 4*l + 1];
          point_type p2 = cell->corner_list[ 4*l + 2];
          point_type p3 = cell->corner_list[ 4*l + 3];

          zcorn[k*8*nx*ny + j*4*nx + 2*i            + l*4*nx*ny] = p0.z;
          zcorn[k*8*nx*ny + j*4*nx + 2*i  +  1      + l*4*nx*ny] = p1.z;
          zcorn[k*8*nx*ny + j*4*nx + 2*nx + 2*i     + l*4*nx*ny] = p2.z;
          zcorn[k*8*nx*ny + j*4*nx + 2*nx + 2*i + 1 + l*4*nx*ny] = p3.z;
        }
      }
    }
  }
}


float * ecl_grid_alloc_zcorn_data( const ecl_grid_type * grid ) {
  float * zcorn = util_calloc( 8 * grid->size , sizeof * zcorn );
  ecl_grid_init_zcorn_data( grid , zcorn );
  return zcorn;
}



ecl_kw_type * ecl_grid_alloc_zcorn_kw( const ecl_grid_type * grid ) {
  ecl_kw_type * zcorn_kw = ecl_kw_alloc( ZCORN_KW , 8 * grid->size , ECL_FLOAT_TYPE );
  ecl_grid_init_zcorn_data( grid , ecl_kw_get_void_ptr( zcorn_kw ));
  return zcorn_kw;
}

/*****************************************************************/

static void ecl_grid_init_actnum_data( const ecl_grid_type * grid , int * actnum ) {
  int i;
  for (i=0; i < grid->size; i++) {
    const ecl_cell_type * cell = ecl_grid_get_cell( grid , i );
    if (cell->coarse_group == COARSE_GROUP_NONE)
      actnum[i] = cell->active;
    else {
      /* In the case of coarse cells we must query the coarse cell for
         the original, uncoarsened distribution of actnum values. */
      ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group( grid , cell->coarse_group );
      
      /* 1: Set all the elements in the coarse group to inactive. */
      {
        int group_size = ecl_coarse_cell_get_size( coarse_cell );
        const int * index_ptr = ecl_coarse_cell_get_index_ptr( coarse_cell );
        int j;

        for (j=0; j < group_size; j++) 
          actnum[index_ptr[j]] = INACTIVE; 
      }
      
      /* 2: Explicitly pick up the active cells from the coarse group
            and mark them correctly. */
      {  
        int num_active = ecl_coarse_cell_get_num_active( coarse_cell );
        int j;

        for (j=0; j < num_active; j++) {
          int global_index = ecl_coarse_cell_iget_active_cell_index( coarse_cell , j );
          int active_value = ecl_coarse_cell_iget_active_value( coarse_cell , j );

          actnum[global_index] = active_value;
        }
      }
    }
  }
}


int * ecl_grid_alloc_actnum_data( const ecl_grid_type * grid ) {
  int * actnum = util_calloc( grid->size , sizeof * actnum);
  ecl_grid_init_actnum_data( grid , actnum );
  return actnum;
}



ecl_kw_type * ecl_grid_alloc_actnum_kw( const ecl_grid_type * grid ) {
  ecl_kw_type * actnum_kw = ecl_kw_alloc( ACTNUM_KW , grid->size  , ECL_INT_TYPE );
  ecl_grid_init_actnum_data( grid , ecl_kw_get_void_ptr( actnum_kw ));
  return actnum_kw;
}

/*****************************************************************/

static void ecl_grid_init_hostnum_data( const ecl_grid_type * grid , int * hostnum ) {
  int i;
  for (i=0; i < grid->size; i++) {
    const ecl_cell_type * cell = ecl_grid_get_cell(grid , i );
    hostnum[i] = cell->host_cell;
  }
}

int * ecl_grid_alloc_hostnum_data( const ecl_grid_type * grid ) {
  int * hostnum = util_calloc( grid->size , sizeof * hostnum );
  ecl_grid_init_hostnum_data( grid , hostnum );
  return hostnum;
}


ecl_kw_type * ecl_grid_alloc_hostnum_kw( const ecl_grid_type * grid ) {
  ecl_kw_type * hostnum_kw = ecl_kw_alloc( HOSTNUM_KW , grid->size  , ECL_INT_TYPE );
  ecl_grid_init_hostnum_data( grid , ecl_kw_get_void_ptr( hostnum_kw ));
  return hostnum_kw;
}

/*****************************************************************/

static void ecl_grid_init_corsnum_data( const ecl_grid_type * grid , int * corsnum ) {
  int i;
  for (i=0; i < grid->size; i++) {
    const ecl_cell_type * cell = ecl_grid_get_cell(grid , i );
    corsnum[i] = cell->coarse_group + 1;
  }
}

int * ecl_grid_alloc_corsnum_data( const ecl_grid_type * grid ) {
  int * corsnum = util_calloc( grid->size , sizeof * corsnum );
  ecl_grid_init_corsnum_data( grid , corsnum );
  return corsnum;
}


ecl_kw_type * ecl_grid_alloc_corsnum_kw( const ecl_grid_type * grid ) {
  ecl_kw_type * corsnum_kw = ecl_kw_alloc( CORSNUM_KW , grid->size  , ECL_INT_TYPE );
  ecl_grid_init_corsnum_data( grid , ecl_kw_get_void_ptr( corsnum_kw ));
  return corsnum_kw;
}

/*****************************************************************/


ecl_kw_type * ecl_grid_alloc_gridhead_kw( int nx, int ny , int nz , int grid_nr) {
  ecl_kw_type * gridhead_kw = ecl_kw_alloc( GRIDHEAD_KW , GRIDHEAD_SIZE , ECL_INT_TYPE );
  ecl_kw_scalar_set_int( gridhead_kw , 0 );
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_TYPE_INDEX , GRIDHEAD_GRIDTYPE_CORNERPOINT );
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_NX_INDEX , nx);
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_NY_INDEX , ny);
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_NZ_INDEX , nz);
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_LGR_INDEX , grid_nr );
  return gridhead_kw;
}


/*****************************************************************/



static void ecl_grid_fwrite_EGRID__( ecl_grid_type * grid , fortio_type * fortio) {  
  bool is_lgr = true;
  if (grid->parent_grid == NULL) 
    is_lgr = false;

  /* Writing header */
  if (!is_lgr)
    ecl_grid_fwrite_main_EGRID_header( ecl_grid_get_mapaxes( grid ) , grid->dualp_flag , fortio );
  else {
    {
      ecl_kw_type * lgr_kw = ecl_kw_alloc(LGR_KW , 1 , ECL_CHAR_TYPE );
      ecl_kw_iset_string8( lgr_kw , 0 , grid->name );
      ecl_kw_fwrite( lgr_kw , fortio );
      ecl_kw_free( lgr_kw );
    }

    {
      ecl_kw_type * lgr_parent_kw = ecl_kw_alloc(LGR_PARENT_KW , 1 , ECL_CHAR_TYPE );
      if (grid->parent_name != NULL)
        ecl_kw_iset_string8( lgr_parent_kw , 0 , grid->parent_name );
      else
        ecl_kw_iset_string8( lgr_parent_kw , 0 , "");
      
      ecl_kw_fwrite( lgr_parent_kw , fortio );
      ecl_kw_free( lgr_parent_kw );
    }
  }
  
  ecl_grid_fwrite_gridhead_kw( grid->nx , grid->ny , grid->nz , grid->grid_nr , fortio);
  /* Writing main grid data */
  {
    {
      ecl_grid_assert_coord_kw( grid );
      ecl_kw_fwrite( grid->coord_kw , fortio );
    }
    {
      ecl_kw_type * zcorn_kw = ecl_grid_alloc_zcorn_kw( grid );
      ecl_kw_fwrite( zcorn_kw , fortio );
      ecl_kw_free( zcorn_kw );
    }
    {
      ecl_kw_type * actnum_kw = ecl_grid_alloc_actnum_kw( grid );
      ecl_kw_fwrite( actnum_kw , fortio );
      ecl_kw_free( actnum_kw );
    }
    if (is_lgr) {
      ecl_kw_type * hostnum_kw = ecl_grid_alloc_hostnum_kw( grid );
      ecl_kw_fwrite( hostnum_kw , fortio );
      ecl_kw_free( hostnum_kw );
    }
    if (grid->coarsening_active) {
      ecl_kw_type * corsnum_kw = ecl_grid_alloc_corsnum_kw( grid );
      ecl_kw_fwrite( corsnum_kw , fortio );
      ecl_kw_free( corsnum_kw );
    }
    
    {
      ecl_kw_type * endgrid_kw = ecl_kw_alloc( ENDGRID_KW , 0 , ECL_INT_TYPE );
      ecl_kw_fwrite( endgrid_kw , fortio );
      ecl_kw_free( endgrid_kw );
    }
  }
  
  if (is_lgr) {
    ecl_kw_type * endlgr_kw = ecl_kw_alloc( ENDLGR_KW , 0 , ECL_INT_TYPE );
    ecl_kw_fwrite( endlgr_kw , fortio );
    ecl_kw_free( endlgr_kw );
  }
}


void ecl_grid_fwrite_EGRID( ecl_grid_type * grid , const char * filename) {  
  bool fmt_file        = false;
  fortio_type * fortio = fortio_open_writer( filename , fmt_file , ECL_ENDIAN_FLIP );
  {
    int grid_nr; 
    for (grid_nr = 0; grid_nr < vector_get_size( grid->LGR_list ); grid_nr++) {
      ecl_grid_type * igrid = vector_iget( grid->LGR_list , grid_nr );
      ecl_grid_fwrite_EGRID__( igrid , fortio );
    }
  }
  fortio_fclose( fortio );
}


/**
   Writes the current grid as grdecl keywords suitable to be read by
   ECLIPSE. This function will only write the main grid and not
   possible LGRs which are attached.
*/

void ecl_grid_fprintf_grdecl(ecl_grid_type * grid , FILE * stream ) {
  {
    ecl_kw_type * mapunits_kw = ecl_grid_alloc_mapunits_kw( grid );
    ecl_kw_fprintf_grdecl( mapunits_kw , stream );
    ecl_kw_free( mapunits_kw );
    fprintf(stream , "\n");
  }
  
  if (grid->use_mapaxes) {
    ecl_kw_type * mapaxes_kw = ecl_grid_alloc_mapaxes_kw( grid->mapaxes );
    ecl_kw_fprintf_grdecl( mapaxes_kw , stream );
    ecl_kw_free( mapaxes_kw );
  }

  {
    ecl_kw_type * gridunits_kw = ecl_grid_alloc_gridunits_kw( grid );
    ecl_kw_fprintf_grdecl( gridunits_kw , stream );
    ecl_kw_free( gridunits_kw );
    fprintf(stream , "\n");
  }
  
  /* 
     The specgrid header is not properly internalized; the fourth and
     fifth elements are just set to hardcoded values.
  */
  {
    int numres      = 1;
    char coord_type = 'F';
    fprintf(stream , "%s\n" , SPECGRID_KW);
    fprintf(stream , "  %d  %d  %d  %d  %c / \n\n",grid->nx , grid->ny , grid->nz , numres , coord_type);
  }
  
  {
    ecl_grid_assert_coord_kw( grid );
    ecl_kw_fprintf_grdecl( grid->coord_kw , stream );
    fprintf(stream , "\n");
  }

  {
    ecl_kw_type * zcorn_kw = ecl_grid_alloc_zcorn_kw( grid );
    ecl_kw_fprintf_grdecl( zcorn_kw , stream );
    ecl_kw_free( zcorn_kw );
    fprintf(stream , "\n");
  }

  {
    ecl_kw_type * actnum_kw = ecl_grid_alloc_actnum_kw( grid );
    ecl_kw_fprintf_grdecl( actnum_kw , stream );
    ecl_kw_free( actnum_kw );
    fprintf(stream , "\n");
  }
}  


/*****************************************************************/

/**
   The ri_points pointer should point to the base address of the
   points data; this function will calculate the correct offset based on
   global_index.  
*/

void ecl_grid_cell_ri_export( const ecl_grid_type * ecl_grid , int global_index , double * ri_points) {
  const ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
  int offset = global_index * 8 * 3;
  ecl_cell_ri_export( cell , &ri_points[ offset ] );
}


void ecl_grid_ri_export( const ecl_grid_type * ecl_grid , double * ri_points) {
  int global_index;
  for (global_index = 0; global_index < ecl_grid->size; global_index++) 
    ecl_grid_cell_ri_export( ecl_grid , global_index , ri_points );
}

/*****************************************************************/


bool ecl_grid_dual_grid( const ecl_grid_type * ecl_grid ) {
  if (ecl_grid->dualp_flag == FILEHEAD_SINGLE_POROSITY)
    return false;
  else
    return true;
}
