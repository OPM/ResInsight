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

#include <ert/geometry/geo_util.h>
#include <ert/geometry/geo_polygon.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_type.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_coarse_cell.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/grid_dims.h>
#include <ert/ecl/nnc_info.h>


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
  About nnc
  ---------

  When loading a grid file the various NNC related keywords are read
  in to assemble information of the NNC. The NNC information is
  organized as follows:

       For cells with NNC's attached the information is kept in a
       nnc_info_type structure. For a particular cell the nnc_info
       structure keeps track of which other cells this particular cell
       is connected to, on a per grid (i.e. LGR) basis.

       In the nnc_info structure the different grids are identified
       through the lgr_nr.



  Example usage:
  --------------

     ecl_grid_type * grid = ecl_grid_alloc("FILE.EGRID");

     // Get a int_vector instance with all the cells which have nnc info
     // attached.
     const int_vector_type * cells_with_nnc = ecl_grid_get_nnc_index_list( grid );

     // Iterate over all the cells with nnc info:
     for (int i=0; i < int_vector_size( cells_with_nnc ); i++) {
         int cell_index =  int_vector_iget( cells_with_nnc , i);
         const nnc_info_type * nnc_info = ecl_grid_get_nnc_info1( grid , cell_index);

         // Get all the nnc connections from @cell_index to other cells in the same grid
         {
            const int_vector_type * nnc_list = nnc_info_get_self_index_list( nnc_info );
            for (int j=0; j < int_vector_size( nnc_list ); j++)
               printf("Cell[%d] -> %d  in the same grid \n",cell_index , int_vector_iget(nnc_list , j));
         }


         {
             for (int lgr_index=0; lgr_index < nnc_info_get_size( nnc_info ); lgr_index++) {
                nnc_vector_type * nnc_vector = nnc_info_iget_vector( nnc_info , lgr_index );
                int lgr_nr = nnc_vector_get_lgr_nr( nnc_vector );
                if (lgr_nr != nnc_info_get_lgr_nr( nnc_info )) {
                   const int_vector_type * nnc_list = nnc_vector_get_index_list( nnc_vector );
                   for (int j=0; j < int_vector_size( nnc_list ); j++)
                       printf("Cell[%d] -> %d  in lgr:%d/%s \n",cell_index , int_vector_iget(nnc_list , j) , lgr_nr  , ecl_grid_get_lgr_name(ecl_grid , lgr_nr));
                }
             }
         }
     }


  Dual porosity and nnc: In ECLIPSE the connection between the matrix
  properties and the fracture properties in a cell is implemented as a
  nnc where the fracture cell has global index in the range [nx*ny*nz,
  2*nz*ny*nz). In ert we we have not implemented this double covering
  in the case of dual porosity models so NNC involving
  fracture cells are not considered.
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


/*
About ordering of the corners in the cell
-----------------------------------------

This code reads and builds the grid structure from the ECLIPSE
GRID/EGRID files without really considering the question about where
the cells are located in "the real world", the format is quite general
and it should(?) be possible to formulate different conventions
(i.e. handedness and direction of z-axis) with the same format.

The corners in a cell are numbered 0 - 7, where corners 0-3 constitute
one layer and the corners 4-7 consitute the other layer. Observe the
numbering does not follow a consistent rotation around the face:


                                  j
  6---7                        /|\
  |   |                         |
  4---5                         |
                                |
                                o---------->  i
  2---3
  |   |
  0---1

Many grids are left-handed, i.e. the direction of increasing z will
point down towards the center of the earth. Hence in the figure above
the layer 4-7 will be deeper down in the reservoir than layer 0-3, and
also have higher z-value.

Warning: The main author of this code suspects that the coordinate
system can be right-handed as well, giving a z axis which will
increase 'towards the sky'; the safest is probaly to check this
explicitly if it matters for the case at hand.

Method 0 corresponds to a tetrahedron decomposition which will split
the lower layer along the 1-2 diagonal and the upper layer along the
4-7 diagonal, method 1 corresponds to the alternative decomposition
which splits the lower face along the 0-3 diagnoal and the upper face
along the 5-6 diagonal.
*/


static const int tetrahedron_permutations[2][12][3] = {{
                                                        // K-
                                                        {0,1,2},
                                                        {3,2,1},
                                                        // J+
                                                        {6,2,7},
                                                        {3,7,2},
                                                        // I-
                                                        {0,2,4},
                                                        {6,4,2},
                                                        // I+
                                                        {3,1,7},
                                                        {5,7,1},
                                                        // J-
                                                        {0,4,1},
                                                        {5,1,4},
                                                        // K+
                                                        {5,4,7},
                                                        {6,7,4}
                                                        },
                                                       {
                                                        // K-
                                                        {1,3,0},
                                                        {2,0,3},
                                                        // J+
                                                        {2,3,6},
                                                        {7,6,3},
                                                        // I-
                                                        {2,6,0},
                                                        {4,0,6},
                                                        // I+
                                                        {7,3,5},
                                                        {1,5,3},
                                                        // J-
                                                        {1,0,5},
                                                        {4,5,0},
                                                        // K+
                                                        {7,5,6},
                                                        {4,6,5}
                                                       }};



/*

  the implementation is based on a hierarchy of three datatypes:

   1. ecl_grid   - this is the only exported datatype
   2. ecl_cell   - internal
   3. point      - internal

*/

typedef struct point_struct  point_type;

struct point_struct {
    double x;
    double y;
    double z;
};

static void point_mapaxes_transform( point_type * p , const double origo[2], const double unit_x[2] , const double unit_y[2]) {
  double new_x =  origo[0] + p->x*unit_x[0] + p->y*unit_y[0];
  double new_y =  origo[1] + p->x*unit_x[1] + p->y*unit_y[1];

  p->x = new_x;
  p->y = new_y;
}

static void point_mapaxes_invtransform( point_type * p , const double origo[2], const double unit_x[2] , const double unit_y[2]) {
  double norm   =  1.0 / (unit_x[0]*unit_y[1] - unit_x[1] * unit_y[0]);
  double dx     = p->x - origo[0];
  double dy     = p->y - origo[1];


  double org_x  =  ( dx*unit_y[1] - dy*unit_y[0]) * norm;
  double org_y  =  (-dx*unit_x[1] + dy*unit_x[0]) * norm;

  p->x = org_x;
  p->y = org_y;
}

static void point_inplace_add(point_type * point , const point_type * add) {
  point->x += add->x;
  point->y += add->y;
  point->z += add->z;
}

static void point_inplace_sub(point_type * point , const point_type * sub) {
  point->x -= sub->x;
  point->y -= sub->y;
  point->z -= sub->z;
}

static void point_inplace_scale(point_type * point , double scale_factor) {
  point->x *= scale_factor;
  point->y *= scale_factor;
  point->z *= scale_factor;
}

/**
   Will compute the vector cross product B x C and store the result in A.
*/

static void point_vector_cross(point_type * A , const point_type * B , const point_type * C) {
  A->x =   B->y*C->z - B->z*C->y;
  A->y = -(B->x*C->z - B->z*C->x);
  A->z =   B->x*C->y - B->y*C->x;
}

static double point_dot_product( const point_type * v1 , const point_type * v2) {
  return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}


static void point_compare( const point_type *p1 , const point_type * p2, bool * equal) {
  const double tolerance = 0.001;

  double diff_x = (fabs(p1->x - p2->x) / fabs(p1->x + p2->x + 1));
  double diff_y = (fabs(p1->y - p2->y) / fabs(p1->y + p2->y + 1));
  double diff_z = (fabs(p1->z - p2->z) / fabs(p1->z + p2->z + 1));

  if (diff_x + diff_y + diff_z > tolerance)
    *equal = false;
}

static void point_dump( const point_type * p , FILE * stream) {
  util_fwrite_double( p->x , stream );
  util_fwrite_double( p->y , stream );
  util_fwrite_double( p->z , stream );
}

static void point_dump_ascii( const point_type * p , FILE * stream , const double * offset) {
  if (offset)
    fprintf(stream , "(%7.2f, %7.2f, %7.2f) " , p->x - offset[0], p->y - offset[1] , p->z - offset[2]);
  else
    fprintf(stream , "(%7.2f, %7.2f, %7.2f) " , p->x , p->y , p->z);

}

static void point_set(point_type * p , double x , double y , double z) {
  p->x = x;
  p->y = y;
  p->z = z;
}


static void point_shift(point_type * p , double dx , double dy , double dz) {
  p->x += dx;
  p->y += dy;
  p->z += dz;
}


static void point_copy_values(point_type * p , const point_type * src) {
  point_set(p , src->x , src->y , src->z);
}

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
#define CELL_FLAG_VOLUME 8

typedef struct ecl_cell_struct           ecl_cell_type;

#define GET_CELL_FLAG(cell,flag) (((cell->cell_flags & (flag)) == 0) ? false : true)
#define SET_CELL_FLAG(cell,flag) ((cell->cell_flags |= (flag)))
#define METER_TO_FEET_SCALE_FACTOR   3.28084
#define METER_TO_CM_SCALE_FACTOR   100.0

struct ecl_cell_struct {
  point_type center;
  point_type corner_list[8];

  double                 volume;             /* Cache volume - whether it is initialized or not is handled by a cell_flags. */
  int                    active;
  int                    active_index[2];    /* [0]: The active matrix index; [1]: the active fracture index */
  const ecl_grid_type   *lgr;                /* if this cell is part of an lgr; this will point to a grid instance for that lgr; NULL if not part of lgr. */
  int                    host_cell;          /* the global index of the host cell for an lgr cell, set to -1 for normal cells. */
  int                    coarse_group;       /* The index of the coarse group holding this cell -1 for non-coarsened cells. */
  int                    cell_flags;
  nnc_info_type        * nnc_info;           /* Non-neighbour connection info*/
};



static void          ecl_grid_init_mapaxes_data_float( const ecl_grid_type * grid , float * mapaxes);
float *              ecl_grid_alloc_coord_data( const ecl_grid_type * grid );
static const float * ecl_grid_get_mapaxes( const ecl_grid_type * grid );

#define ECL_GRID_ID       991010

struct ecl_grid_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                   lgr_nr;        /* EGRID files: corresponds to item 4 in gridhead - 0 for the main grid.
                                          GRID files: 0 for the main grid, then 1 -> number of LGRs in order read from file*/
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

  ecl_cell_type      *  cells;

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
    lgr_nr > 0.
  */
  vector_type         * LGR_list;      /* a vector of ecl_grid instances for LGRs - the index corresponds to the order LGRs are read from file*/
  int_vector_type     * lgr_index_map; /* a vector that maps LGR-nr for EGRID files to index into the LGR_list.*/
  hash_type           * LGR_hash;      /* a hash of pointers to ecl_grid instances - for name based lookup of lgr. */
  int                   parent_box[6]; /* integers i1,i2, j1,j2, k1,k2 of the parent grid region containing this lgr. the indices are inclusive - zero offset */
                                       /* not used yet .. */

  int                    dualp_flag;
  bool                   use_mapaxes;
  double                 unit_x[2];
  double                 unit_y[2];
  double                 origo[2];
  float                * mapaxes;
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

  ert_ecl_unit_enum     unit_system;
  int                   eclipse_version;
};

static void ecl_cell_compare(const ecl_cell_type * c1 , const ecl_cell_type * c2,  bool include_nnc , bool * equal) {
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

  if (include_nnc) {
    if (*equal)
      *equal = nnc_info_equal( c1->nnc_info , c2->nnc_info );
  }

}


static void ecl_cell_dump( const ecl_cell_type * cell , FILE * stream) {
  int i;
  for (i=0; i < 8; i++)
    point_dump( &cell->corner_list[i] , stream );
}


static void ecl_cell_assert_center( ecl_cell_type * cell);

static void ecl_cell_dump_ascii( ecl_cell_type * cell , int i , int j , int k , FILE * stream , const double * offset) {
  fprintf(stream, "Cell: i:%3d  j:%3d    k:%3d   host_cell:%d  CoarseGroup:%4d active_nr:%6d  active:%d \nCorners:\n",
          i, j, k,
          cell->host_cell, cell->coarse_group,
          cell->active_index[MATRIX_INDEX],
          cell->active);

  ecl_cell_assert_center( cell );
  fprintf(stream , "Center   : ");
  point_dump_ascii( &cell->center , stream , offset);
  fprintf(stream , "\n");

  {
    int l;
    for (l=0; l < 8; l++) {
      fprintf(stream , "Corner %d : ",l);
      point_dump_ascii( &cell->corner_list[l] , stream , offset);
      fprintf(stream , "\n");
    }
  }
  fprintf(stream , "\n");
}


static void ecl_cell_fwrite_GRID(const ecl_grid_type * grid,
                                 const ecl_cell_type * cell,
                                 bool fracture_cell,
                                 int coords_size,
                                 int i,
                                 int j,
                                 int k,
                                 int global_index,
                                 ecl_kw_type * coords_kw,
                                 ecl_kw_type * corners_kw,
                                 fortio_type * fortio) {
  ecl_kw_iset_int( coords_kw , 0 , i + 1);
  ecl_kw_iset_int( coords_kw , 1 , j + 1);
  ecl_kw_iset_int( coords_kw , 2 , k + 1);
  ecl_kw_iset_int( coords_kw , 3 , global_index + 1);

  ecl_kw_iset_int( coords_kw , 4 , 0);
  if (fracture_cell) {
    if (cell->active & CELL_ACTIVE_FRACTURE)
      ecl_kw_iset_int( coords_kw , 4 , 1);
  } else {
    if (cell->active & CELL_ACTIVE_MATRIX)
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
   completely. We therefore try to invalidate such cells here. The
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
  if (cell->active == CELL_NOT_ACTIVE) {
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



static int ecl_cell_get_twist( const ecl_cell_type * cell ) {
  int twist_count = 0;

  for (int c = 0; c < 4; c++) {
    const point_type * p1 = &cell->corner_list[c];
    const point_type * p2 = &cell->corner_list[c + 4];
    if ((p2->z - p1->z) < 0)
      twist_count += 1;
  }
  return twist_count;
}



/*****************************************************************/




/**
   Observe that when allocating based on a grid file not all cells are
   necessarily accessed beyond this function. In general not all cells
   will have a coords/corners section in the grid file.
*/

static void ecl_cell_init( ecl_cell_type * cell , bool init_valid) {
  cell->active                = CELL_NOT_ACTIVE;
  cell->lgr                   = NULL;
  cell->host_cell             = HOST_CELL_NONE;
  cell->coarse_group          = COARSE_GROUP_NONE;
  cell->cell_flags            = 0;
  cell->active_index[MATRIX_INDEX]   = -1;
  cell->active_index[FRACTURE_INDEX] = -1;
  if (init_valid)
    cell->cell_flags = CELL_FLAG_VALID;

  cell->nnc_info = NULL;
}


/*
#define mod(x,n) ((x) % (n))
static int ecl_cell_get_tetrahedron_method( int i , int j , int k) {
  return (1 + (1 - 2*mod(i,2)) * (1 - 2 * mod(j,2)) * (1 - 2*mod(k,2))) / 2;
}
#undef mod
*/

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

static double C(double *r,int f1,int f2,int f3){
  if (f1 == 0) {
    if (f2 == 0) {
      if (f3 == 0)
        return r[0];                        // 000
      else
        return r[4] - r[0];                 // 001
    } else {
      if (f3 == 0)
        return r[2] - r[0];                 // 010
      else
        return r[6] + r[0] - r[4] - r[2];   // 011
    }
  } else {
    if (f2 == 0) {
      if (f3 == 0)
        return r[1] - r[0];                 // 100
      else
        return r[5]+r[0]-r[4]-r[1];         // 101
    } else {
      if (f3 == 0)
        return r[3]+r[0]-r[2]-r[1];         // 110
      else
        return r[7]+r[4]+r[2]+r[1]-r[6]-r[5]-r[3]-r[0];   // 111
    }
  }
}


static double ecl_cell_get_volume_tskille( ecl_cell_type * cell ) {
  double volume = 0;
  int pb,pg,qa,qg,ra,rb;
  double X[8];
  double Y[8];
  double Z[8];
  {
    int c;
    for (c = 0; c < 8; c++) {
      X[c] = cell->corner_list[c].x;
      Y[c] = cell->corner_list[c].y;
      Z[c] = cell->corner_list[c].z;
    }
  }

  for (pb=0;pb<=1;pb++)
    for (pg=0;pg<=1;pg++)
      for (qa=0;qa<=1;qa++)
        for (qg=0;qg<=1;qg++)
          for (ra=0;ra<=1;ra++)
            for (rb=0;rb<=1;rb++) {
              int divisor = (qa+ra+1)*(pb+rb+1)*(pg+qg+1);
              double dV = C(X,1,pb,pg) * C(Y,qa,1,qg) * C(Z,ra,rb,1) -
                          C(X,1,pb,pg) * C(Z,qa,1,qg) * C(Y,ra,rb,1) -
                          C(Y,1,pb,pg) * C(X,qa,1,qg) * C(Z,ra,rb,1) +
                          C(Y,1,pb,pg) * C(Z,qa,1,qg) * C(X,ra,rb,1) +
                          C(Z,1,pb,pg) * C(X,qa,1,qg) * C(Y,ra,rb,1) -
                          C(Z,1,pb,pg) * C(Y,qa,1,qg) * C(X,ra,rb,1);

              volume += dV / divisor;
            }

  return fabs(volume);
}

typedef struct tetrahedron_struct tetrahedron_type;

struct tetrahedron_struct {
    point_type p0;
    point_type p1;
    point_type p2;
    point_type p3;
};

/*
  Calculates the volume of a tetrahedron, a normalization of 1/6 is
  omitted.
*/
static inline double tetrahedron_volume6( tetrahedron_type tet ) {
  point_type bxc;

  /* vector subtraction */
  tet.p0.x -= tet.p3.x;
  tet.p0.y -= tet.p3.y;
  tet.p0.z -= tet.p3.z;

  tet.p1.x -= tet.p3.x;
  tet.p1.y -= tet.p3.y;
  tet.p1.z -= tet.p3.z;

  tet.p2.x -= tet.p3.x;
  tet.p2.y -= tet.p3.y;
  tet.p2.z -= tet.p3.z;

  /* cross product */
  bxc.x = tet.p1.y*tet.p2.z - tet.p1.z*tet.p2.y;
  bxc.y = -(tet.p1.x*tet.p2.z - tet.p1.z*tet.p2.x);
  bxc.z = tet.p1.x*tet.p2.y - tet.p1.y*tet.p2.x;

  /* dot product */
  return tet.p0.x*bxc.x + tet.p0.y*bxc.y + tet.p0.z*bxc.z;
}

/*
 Returns true if and only if the point p is inside the tetrahedron tet.
*/
static bool tetrahedron_contains(tetrahedron_type tet, const point_type p) {
  const double epsilon = 1e-9;
  double tetra_volume = fabs(tetrahedron_volume6(tet));

  if(tetra_volume < epsilon)
    return false;

  // Decomposes tetrahedron into 4 new tetrahedrons
  point_type tetra_points[4] = {tet.p0, tet.p1, tet.p2, tet.p3};
  double decomposition_volume = 0;
  for(int i = 0; i < 4; ++i) {
      const point_type tmp = tetra_points[i];
      tetra_points[i] = p;

      // Compute volum of decomposition tetrahedron
      tetrahedron_type dec_tet;
      dec_tet.p0 = tetra_points[0]; dec_tet.p1 = tetra_points[1];
      dec_tet.p2 = tetra_points[2]; dec_tet.p3 = tetra_points[3];
      decomposition_volume += fabs(tetrahedron_volume6(dec_tet));

      tetra_points[i] = tmp;
  }

  return (fabs(tetra_volume - decomposition_volume) < epsilon);
}

/*
 * This function used to account for a significant amount of execution time
 * when used in opm-parser and has been optimised significantly. This means
 * inlining several operations, e.g. vector operations, and other tricks.
 */
static double ecl_cell_get_signed_volume( ecl_cell_type * cell) {
  if (GET_CELL_FLAG(cell , CELL_FLAG_VOLUME))
    return cell->volume;

  ecl_cell_assert_center( cell );
  {
    /*
     * We make an activation record local copy of the cell's corners for less
     * jumping in memory and better cache performance.
     */
    point_type center = cell->center;
    point_type corners[ 8 ];
    memcpy( corners, cell->corner_list, sizeof( point_type ) * 8 );

    tetrahedron_type tet = { .p0 = center };
    double           volume = 0;
    /*
      using both tetrahedron decompositions - gives good agreement
      with porv from eclipse init files.
    */

    /*
     * The order of these loops is intentional and guided by profiling. It's much
     * faster to access method, then the number, rather than the other way
     * around. If you are to change this, please measure performance impact.
     */
    for( int method = 0; method < 2; ++method ) {
      for( int itet = 0; itet < 12; ++itet  ) {
        const int point0 = tetrahedron_permutations[ method ][ itet ][ 0 ];
        const int point1 = tetrahedron_permutations[ method ][ itet ][ 1 ];
        const int point2 = tetrahedron_permutations[ method ][ itet ][ 2 ];

        tet.p1 = corners[ point0 ];
        tet.p2 = corners[ point1 ];
        tet.p3 = corners[ point2 ];
        volume += tetrahedron_volume6( tet ) / 6;
      }
    }

    /* The volume of a tetrahedron is
     *        |a·(b x c)|
     *  V  =  -----------
     *             6
     * Since sum( |a·(b x c)| ) / 6 is equal to
     * sum( |a·(b x c)| / 6 ) we can do the (rather expensive) division only once
     * and still get the correct result. We multiply by 0.5 because we've now
     * considered two decompositions of the tetrahedron, and want their average.
     *
     *
     * Note added: these volume calculations are used to calculate pore
     * volumes in OPM, it turns out that opm is very sensitive to these
     * volumes. Extracting the divison by 6.0 was actually enough to
     * induce a regression test failure in flow, this has therefore been
     * reverted.
     */

    cell->volume = volume * 0.5;
    SET_CELL_FLAG( cell , CELL_FLAG_VOLUME );
  }
  return cell->volume;
}


static double ecl_cell_get_volume( ecl_cell_type * cell ) {
  return fabs( ecl_cell_get_signed_volume(cell));
}




static double triangle_area(double x1, double y1,
                            double x2, double y2,
                            double x3, double y3) {
  return fabs(x1*y2 + x2*y3 + x3*y1 - x1*y3 - x3*y2 - x2*y1)*0.5;
}


static bool triangle_contains(const point_type *p0,
                              const point_type *p1,
                              const point_type *p2,
                              double x, double y) {
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

static double parallelogram_area3d(const point_type * p0, const point_type * p1, const point_type * p2) {
  point_type a = *p1;
  point_type b = *p2;
  point_inplace_sub(&a, p0);
  point_inplace_sub(&b, p0);

  point_type c;
  point_vector_cross(&c, &a, &b);
  return sqrt(point_dot_product(&c, &c));
}

/*
 * Returns true if and only if point p is contained in the triangle denoted by
 * p0, p1 and p2. Note that if the triangle is a line, the function will still
 * return true if the point lies on the line segment.
 */
static bool triangle_contains3d(const point_type *p0,
                                const point_type *p1,
                                const point_type *p2,
                                const point_type *p) {
  double epsilon = 1e-10;
  double vt = parallelogram_area3d(p0, p1, p2);

  double v1 = parallelogram_area3d(p0, p1, p);
  double v2 = parallelogram_area3d(p0, p2, p);
  double v3 = parallelogram_area3d(p1, p2, p);

  // p0, p1, p2 represents a line segment and
  // p lies on the line this segment represents
  if(vt < epsilon && fabs(v1+v2+v3) < epsilon) {
    double x_min = util_double_min(p0->x, util_double_min(p1->x, p2->x));
    double x_max = util_double_max(p0->x, util_double_max(p1->x, p2->x));

    double y_min = util_double_min(p0->y, util_double_min(p1->y, p2->y));
    double y_max = util_double_max(p0->y, util_double_max(p1->y, p2->y));

    double z_min = util_double_min(p0->z, util_double_min(p1->z, p2->z));
    double z_max = util_double_max(p0->z, util_double_max(p1->z, p2->z));

    return (
            x_min-epsilon <= p->x && p->x <= x_max+epsilon &&
            y_min-epsilon <= p->y && p->y <= y_max+epsilon &&
            z_min-epsilon <= p->z && p->z <= z_max+epsilon
           );
  }

  return (fabs( vt - (v1 + v2 + v3 )) < epsilon);
}




/*
   if the layer defined by the cell corners 0-1-2-3 (lower == true) or
   4-5-6-7 (lower == false) contain the point (x,y) the function will
   return true - otehrwise false.

   the function works by dividing the cell face into two triangles,
   which are checked one at a time with the function
   triangle_contains().
*/


static bool ecl_cell_layer_contains_xy(const ecl_cell_type * cell,
                                       bool lower_layer,
                                       double x,
                                       double y) {
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



/**
       lower layer:   upper layer

         2---3           6---7
         |   |           |   |
         0---1           4---5
*/
static void ecl_cell_init_regular(ecl_cell_type * cell,
                                  const double * offset,
                                  int i,
                                  int j,
                                  int k,
                                  int global_index,
                                  const double * ivec,
                                  const double * jvec,
                                  const double * kvec,
                                  const int * actnum) {
  point_set(&cell->corner_list[0] , offset[0] , offset[1] , offset[2] ); // Point 0

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
    cell->active = CELL_ACTIVE;
}

/* end of cell implementation                                    */
/*****************************************************************/
/* starting on the ecl_grid proper implementation                */

UTIL_SAFE_CAST_FUNCTION(ecl_grid , ECL_GRID_ID);
UTIL_IS_INSTANCE_FUNCTION( ecl_grid , ECL_GRID_ID);

/**
   this function allocates the internal index_map and inv_index_map fields.
*/



static ecl_cell_type * ecl_grid_get_cell(const ecl_grid_type * grid,
                                         int global_index) {
  return &grid->cells[global_index];
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
  if (!grid->cells)
    return;

  for (int i=0; i < grid->size; i++) {
    ecl_cell_type * cell = ecl_grid_get_cell( grid , i );
    if (cell->nnc_info)
      nnc_info_free(cell->nnc_info);
  }
  free( grid->cells );

}

static bool ecl_grid_alloc_cells( ecl_grid_type * grid , bool init_valid) {
  grid->cells = malloc(grid->size * sizeof * grid->cells );
  if (!grid->cells)
    return false;

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
    return true;
  }
}

/**
   will create a new blank grid instance. if the global_grid argument
   is != NULL the newly created grid instance will copy the mapaxes
   transformations; and set the global_grid pointer of the new grid
   instance. apart from that no further lgr-relationsip initialisation
   is performed.
*/

static ecl_grid_type * ecl_grid_alloc_empty(ecl_grid_type * global_grid,
                                            int dualp_flag,
                                            int nx,
                                            int ny,
                                            int nz,
                                            int lgr_nr,
                                            bool init_valid) {
  ecl_grid_type * grid = util_malloc(sizeof * grid );
  UTIL_TYPE_ID_INIT(grid , ECL_GRID_ID);
  grid->total_active   = 0;
  grid->total_active_fracture = 0;
  grid->nx                    = nx;
  grid->ny                    = ny;
  grid->nz                    = nz;
  grid->size                  = nx*ny*nz;
  grid->lgr_nr                = lgr_nr;
  grid->global_grid           = global_grid;
  grid->coarsening_active     = false;
  grid->mapaxes               = NULL;

  grid->dualp_flag            = dualp_flag;
  grid->coord_kw              = NULL;
  grid->visited               = NULL;
  grid->inv_index_map         = NULL;
  grid->index_map             = NULL;
  grid->fracture_index_map    = NULL;
  grid->inv_fracture_index_map = NULL;
  grid->unit_system            = ECL_METRIC_UNITS;


  if (global_grid != NULL) {
    /*
       this is an lgr instance, and we inherit the global grid
       transformations from the main grid. The underlying mapaxes
       pointer is jest left at NULL for lgr instances.
    */
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


  grid->block_dim       = 0;
  grid->values          = NULL;
  if (ECL_GRID_MAINGRID_LGR_NR == lgr_nr) {  /* this is the main grid */
    grid->LGR_list      = vector_alloc_new();
    grid->lgr_index_map = int_vector_alloc(0,0);
    grid->LGR_hash      = hash_alloc();
  } else {
    grid->LGR_list      = NULL;
    grid->lgr_index_map = NULL;
    grid->LGR_hash      = NULL;
  }
  grid->name            = NULL;
  grid->parent_name     = NULL;
  grid->parent_grid     = NULL;
  grid->children        = hash_alloc();
  grid->coarse_cells    = vector_alloc_new();
  grid->eclipse_version = 0;

  /* This is the large allocation - which can potentially fail. */
  if (!ecl_grid_alloc_cells( grid , init_valid )) {
    ecl_grid_free( grid );
    grid = NULL;
  }
  return grid;
}




static int ecl_grid_get_global_index__(const ecl_grid_type * ecl_grid,
                                       int i,
                                       int j,
                                       int k) {
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
    cell->active = CELL_ACTIVE;
  else
    cell->active = actnum[global_index];

  if (corsnum != NULL)
    cell->coarse_group = corsnum[ global_index ] - 1;
}


static void ecl_grid_set_cell_GRID(ecl_grid_type * ecl_grid,
                                   int coords_size,
                                   const int * coords,
                                   const float * corners) {

  const int i  = coords[0] - 1; /* eclipse 1 offset */
  const int j  = coords[1] - 1;
  int k  = coords[2] - 1;
  int global_index;
  ecl_cell_type * cell;
  bool matrix_cell = true;
  int active_value = CELL_ACTIVE_MATRIX;

  /*
    This is the rather hysterical treatment of dual porosity in GRID
    files. Cells with k >= nz consitute the fracture part of the
    grid. For these cell the cell properties are not recalculated, but
    the active flag is updated to include the active|inactive
    properties of the fracture.
  */

  if (k >= ecl_grid->nz) {
    k -= ecl_grid->nz;
    matrix_cell = false;
    active_value = CELL_ACTIVE_FRACTURE;
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
       The ECL_GRID_MAINGRID_LGR_NR != ecl_grid->lgr_nr test checks if
       this is a LGR; if this test applies we either have bug - or a
       GRID file with LGRs and only 4/5 elements in the coords keywords.
       In the latter case we must start using the LGRILG keyword.
    */
    if ((ECL_GRID_MAINGRID_LGR_NR != ecl_grid->lgr_nr) && (coords_size != 7))
      util_abort("%s: Need 7 element coords keywords for LGR - or reimplement to use LGRILG keyword.\n",__func__);

    switch(coords_size) {
    case 4:                /* all cells active */
      cell->active += active_value;
      break;
    case 5:                /* only spesific cells active - no lgr */
      cell->active  += coords[4] * active_value;
      break;
    case 7:
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

static void ecl_grid_init_index_map__(ecl_grid_type * ecl_grid,
                                      int * index_map,
                                      int * inv_index_map,
                                      int active_mask,
                                      int type_index) {
  for (int global_index = 0; global_index < ecl_grid->size; global_index++) {
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
  ecl_grid->index_map     = util_realloc(ecl_grid->index_map,
                                         ecl_grid->size * sizeof * ecl_grid->index_map);
  ecl_grid->inv_index_map = util_realloc(ecl_grid->inv_index_map,
                                         ecl_grid->total_active * sizeof * ecl_grid->inv_index_map);
  ecl_grid_init_index_map__(ecl_grid,
                            ecl_grid->index_map,
                            ecl_grid->inv_index_map,
                            CELL_ACTIVE_MATRIX,
                            MATRIX_INDEX);


  /* Create the inverse mapping for the fractures. */
  if (ecl_grid->dualp_flag != FILEHEAD_SINGLE_POROSITY) {
    ecl_grid->fracture_index_map     = util_realloc(ecl_grid->fracture_index_map,
                                                    ecl_grid->size * sizeof * ecl_grid->fracture_index_map);
    ecl_grid->inv_fracture_index_map = util_realloc(ecl_grid->inv_fracture_index_map,
                                                    ecl_grid->total_active_fracture * sizeof * ecl_grid->inv_fracture_index_map);
    ecl_grid_init_index_map__(ecl_grid,
                              ecl_grid->fracture_index_map,
                              ecl_grid->inv_fracture_index_map,
                              CELL_ACTIVE_FRACTURE,
                              FRACTURE_INDEX);
  }


  /* Update the inverse map in the case of coarse cells. Observe that
     in the case of coarse cells with more than one active cell in the
     main grid, the inverse active -> global mapping will map to the
     first active cell in the coarse cell.
  */
  int size = ecl_grid_get_num_coarse_groups(ecl_grid);
  for (int coarse_group = 0; coarse_group < size; coarse_group++) {
    ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group(ecl_grid, coarse_group);
    if (ecl_coarse_cell_get_num_active(coarse_cell) > 0) {
      int global_index          = ecl_coarse_cell_iget_active_cell_index(coarse_cell, 0);
      int active_value          = ecl_coarse_cell_iget_active_value(coarse_cell, 0);
      int active_index          = ecl_coarse_cell_get_active_index(coarse_cell);
      int active_fracture_index = ecl_coarse_cell_get_active_fracture_index(coarse_cell);

      if (active_value & CELL_ACTIVE_MATRIX) // active->global mapping point to one "random" cell in the coarse group
        ecl_grid->inv_index_map[active_index] = global_index;

      if (active_value & CELL_ACTIVE_FRACTURE)
        ecl_grid->inv_fracture_index_map[active_fracture_index] = global_index;

      int coarse_size = ecl_coarse_cell_get_size(coarse_cell);
      const int_vector_type * global_index_list = ecl_coarse_cell_get_index_vector(coarse_cell);
      for (int ic = 0; ic < coarse_size; ic++) {
        int gi = int_vector_iget(global_index_list, ic);

        if (active_value & CELL_ACTIVE_MATRIX) // All cells in the coarse group point to the same active index.
          ecl_grid->index_map[gi] = active_index;

        if (active_value & CELL_ACTIVE_FRACTURE)
          ecl_grid->fracture_index_map[gi] = active_fracture_index;
      }
    } // else the coarse cell does not have any active cells.
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
    for (global_index = 0; global_index < ecl_grid->size; global_index++) {
      ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);

      if (cell->active & CELL_ACTIVE_MATRIX) {
        cell->active_index[MATRIX_INDEX] = active_index;
        active_index++;
      }
    }

    if (ecl_grid->dualp_flag != FILEHEAD_SINGLE_POROSITY) {
      for (global_index = 0; global_index < ecl_grid->size; global_index++) {
        ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);
        if (cell->active & CELL_ACTIVE_FRACTURE) {
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
      if (cell->active != CELL_NOT_ACTIVE) {
        if (cell->coarse_group == COARSE_GROUP_NONE) {

          if (cell->active & CELL_ACTIVE_MATRIX) {
            cell->active_index[MATRIX_INDEX] = active_index;
            active_index++;
          }

          if (cell->active & CELL_ACTIVE_FRACTURE) {
            cell->active_index[FRACTURE_INDEX] = active_fracture_index;
            active_fracture_index++;
          }

        } else {
          ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group( ecl_grid , cell->coarse_group );
          ecl_coarse_cell_update_index(coarse_cell,
                                       global_index,
                                       &active_index,
                                       &active_fracture_index,
                                       cell->active);
        }
      }
    }


    /*
      2: Go through all the coarse cells and set the active index and
         active value of all the cells in the coarse cell to the
         common value for the coarse cell.
    */

    int size = ecl_grid_get_num_coarse_groups(ecl_grid);
    for (int coarse_group = 0; coarse_group < size; coarse_group++) {
      ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group( ecl_grid , coarse_group );
      if (ecl_coarse_cell_get_num_active(coarse_cell) > 0) {
        int cell_active_index          = ecl_coarse_cell_get_active_index( coarse_cell );
        int cell_active_value          = ecl_coarse_cell_iget_active_value( coarse_cell , 0);
        int group_size                 = ecl_coarse_cell_get_size( coarse_cell );
        const int * coarse_cell_list   = ecl_coarse_cell_get_index_ptr( coarse_cell );

        for (int i=0; i < group_size; i++) {
          global_index = coarse_cell_list[i];

          ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );

          if (cell_active_value & CELL_ACTIVE_MATRIX)
            cell->active_index[MATRIX_INDEX] = cell_active_index;

          /* Coarse cell and dual porosity - that is probably close to zero measure. */
          if (cell_active_value & CELL_ACTIVE_FRACTURE) {
            int cell_active_fracture_index = ecl_coarse_cell_get_active_fracture_index( coarse_cell );
            cell->active_index[FRACTURE_INDEX] = cell_active_fracture_index;
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


static void ecl_grid_init_mapaxes( ecl_grid_type * ecl_grid , bool apply_mapaxes, const float * mapaxes) {
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
    ecl_grid->mapaxes = util_malloc( 6 * sizeof * ecl_grid->mapaxes );
    memcpy( ecl_grid->mapaxes , mapaxes , 6 * sizeof( float ));

    /*
      If the apply_mapaxes variable is false we will not apply the
      transformation; however we will still internalize the mapaxes
      information, and also output that if the file is saved again.
    */
    ecl_grid->use_mapaxes = apply_mapaxes;
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
  vector_append_owned_ref( main_grid->LGR_list , lgr_grid , ecl_grid_free__);
  int_vector_iset(main_grid->lgr_index_map, lgr_grid->lgr_nr, vector_get_size(main_grid->LGR_list)-1);
  hash_insert_ref( main_grid->LGR_hash , lgr_grid->name , lgr_grid);
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
    ecl_kw_type * parent_kw = ecl_file_iget_named_kw( ecl_file , LGR_PARENT_KW , grid_nr -1);
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


/*
  This function will calculate the index of a corner in the the zcorn
  array.  The corner is given with i,j,k indices of the cell, and
  corner number [0...8). The function is mainly a convenience function
  for testing, and should not be used in a tight inner loop unrolling
  the zcorn vector.
*/

int ecl_grid_zcorn_index__(int nx, int ny , int i, int j , int k , int c) {
  int zcorn_index =  k*8*nx*ny + j*4*nx + 2*i;
  if ((c % 2) == 1)
    zcorn_index += 1;

  if ((c % 4) == 2)
    zcorn_index += 2*nx;

  if ((c % 4) == 3)
    zcorn_index += 2*nx;

  if (c >= 4)
    zcorn_index += 4*nx*ny;

  return zcorn_index;
}

int ecl_grid_zcorn_index(const ecl_grid_type * grid , int i, int j , int k , int c) {
  return ecl_grid_zcorn_index__( grid->nx, grid->ny , i , j , k , c );
}


static void ecl_grid_init_GRDECL_data_jslice(ecl_grid_type * ecl_grid,
                                             const float * zcorn,
                                             const float * coord,
                                             const int * actnum,
                                             const int * corsnum,
                                             int j) {
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


void ecl_grid_init_GRDECL_data(ecl_grid_type * ecl_grid,
                               const float * zcorn,
                               const float * coord,
                               const int * actnum,
                               const int * corsnum) {
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

static ecl_grid_type * ecl_grid_alloc_GRDECL_data__(ecl_grid_type * global_grid,
                                                    int dualp_flag,
                                                    bool apply_mapaxes,
                                                    int nx,
                                                    int ny,
                                                    int nz,
                                                    const float * zcorn,
                                                    const float * coord,
                                                    const int * actnum,
                                                    const float * mapaxes,
                                                    const int * corsnum,
                                                    int lgr_nr) {

  ecl_grid_type * ecl_grid = ecl_grid_alloc_empty(global_grid , dualp_flag , nx,ny,nz,lgr_nr,true);
  if (ecl_grid) {
    if (mapaxes != NULL)
      ecl_grid_init_mapaxes( ecl_grid , apply_mapaxes, mapaxes );

    if (corsnum != NULL)
      ecl_grid->coarsening_active = true;

    ecl_grid->coord_kw = ecl_kw_alloc_new("COORD" , 6*(nx + 1) * (ny + 1) , ECL_FLOAT , coord );
    ecl_grid_init_GRDECL_data( ecl_grid , zcorn , coord , actnum , corsnum);

    ecl_grid_init_coarse_cells( ecl_grid );
    ecl_grid_update_index( ecl_grid );
    ecl_grid_taint_cells( ecl_grid );
  }
  return ecl_grid;
}


static void ecl_grid_copy_mapaxes( ecl_grid_type * target_grid , const ecl_grid_type * src_grid ) {
  target_grid->use_mapaxes = src_grid->use_mapaxes;
  if (src_grid->mapaxes)
    target_grid->mapaxes = util_realloc_copy(target_grid->mapaxes , src_grid->mapaxes , 6 * sizeof * src_grid->mapaxes );
  else
    target_grid->mapaxes = util_realloc_copy(target_grid->mapaxes , NULL , 0 );

  for (int i=0; i < 2; i++) {
    target_grid->unit_x[i] = src_grid->unit_x[i];
    target_grid->unit_y[i] = src_grid->unit_y[i];
    target_grid->origo[i] = src_grid->origo[i];
  }
}


static void ecl_grid_copy_content( ecl_grid_type * target_grid , const ecl_grid_type * src_grid ) {
  int global_index;
  for (global_index = 0; global_index  < src_grid->size; global_index++) {
    ecl_cell_type * target_cell = ecl_grid_get_cell( target_grid , global_index);
    const ecl_cell_type * src_cell = ecl_grid_get_cell( src_grid , global_index );

    ecl_cell_memcpy( target_cell , src_cell );
    if (src_cell->nnc_info)
      target_cell->nnc_info = nnc_info_alloc_copy( src_cell->nnc_info );
  }
  ecl_grid_copy_mapaxes( target_grid , src_grid );

  target_grid->parent_name = util_alloc_string_copy( src_grid->parent_name );
  target_grid->name = util_alloc_string_copy( src_grid->name );

  target_grid->coarsening_active = src_grid->coarsening_active;
  ecl_grid_init_coarse_cells( target_grid );
}

static ecl_grid_type * ecl_grid_alloc_copy__( const ecl_grid_type * src_grid,  ecl_grid_type * main_grid ) {
  ecl_grid_type * copy_grid = ecl_grid_alloc_empty( main_grid ,
                                                    src_grid->dualp_flag ,
                                                    ecl_grid_get_nx( src_grid ) ,
                                                    ecl_grid_get_ny( src_grid ) ,
                                                    ecl_grid_get_nz( src_grid ) ,
                                                    0 ,
                                                    false );
  if (copy_grid) {
    ecl_grid_copy_content( copy_grid , src_grid );  // This will handle everything except LGR relationships which is established in the calling routine
    ecl_grid_update_index( copy_grid );
  }
  return copy_grid;
}




ecl_grid_type * ecl_grid_alloc_copy( const ecl_grid_type * src_grid ) {
  ecl_grid_type * copy_grid = ecl_grid_alloc_copy__( src_grid , NULL );

  {
    int grid_nr;
    for (grid_nr = 0; grid_nr < vector_get_size( src_grid->LGR_list ); grid_nr++) {
      const ecl_grid_type * src_lgr = vector_iget_const( src_grid->LGR_list , grid_nr);
      ecl_grid_type * copy_lgr = ecl_grid_alloc_copy__( src_lgr , copy_grid );
      ecl_grid_type * host_grid;

      ecl_grid_add_lgr( copy_grid , copy_lgr );   // This handles the storage ownership of the LGR.
      if (copy_lgr->parent_name == NULL)
        host_grid = copy_grid;
      else
        host_grid = ecl_grid_get_lgr( copy_grid , copy_lgr->parent_name );

      {
        int global_lgr_index;

        for (global_lgr_index = 0; global_lgr_index < copy_lgr->size; global_lgr_index++) {
          ecl_cell_type * lgr_cell  = ecl_grid_get_cell( copy_lgr , global_lgr_index);
          ecl_cell_type * host_cell = ecl_grid_get_cell( host_grid , lgr_cell->host_cell );

          ecl_cell_install_lgr( host_cell , copy_lgr );
        }
        ecl_grid_install_lgr_common( host_grid , copy_lgr );

      }

    }
  }

  return copy_grid;
}

/*
  Does not handle LGR
*/
ecl_grid_type * ecl_grid_alloc_processed_copy( const ecl_grid_type * src_grid , const double * zcorn , const int * actnum)
{
  ecl_grid_type * grid;

  if (zcorn == NULL) {
    grid = ecl_grid_alloc_copy__( src_grid , NULL );
    if (actnum)
      ecl_grid_reset_actnum( grid , actnum );
  } else {
    int nx,ny,nz,na;
    int zcorn_size = ecl_grid_get_zcorn_size( src_grid );
    float * zcorn_float = util_malloc( zcorn_size * sizeof * zcorn_float );
    float * coord = ecl_grid_alloc_coord_data( src_grid );
    float * mapaxes = NULL;

    if (ecl_grid_get_mapaxes( src_grid )) {
      mapaxes = util_malloc( 6 * sizeof * mapaxes );
      ecl_grid_init_mapaxes_data_float(src_grid, mapaxes);
    }
    ecl_grid_get_dims( src_grid , &nx, &ny , &nz , &na);
    for (int i=0; i < zcorn_size; i++)
      zcorn_float[i] = zcorn[i];

    grid = ecl_grid_alloc_GRDECL_data( nx , ny , nz , zcorn_float , coord , actnum , src_grid->use_mapaxes, mapaxes );

    free( mapaxes );
    free( coord );
    free( zcorn_float );
  }

  return grid;
}



/*
  If you create/load data for the various fields, this function can be
  used to create a GRID instance, without going through a GRID/EGRID
  file - currently the implementation does not support the creation of
  a lgr hierarchy - or cell coarsening.
*/

ecl_grid_type * ecl_grid_alloc_GRDECL_data(int nx,
                                           int ny,
                                           int nz,
                                           const float * zcorn,
                                           const float * coord,
                                           const int * actnum,
                                           bool apply_mapaxes,
                                           const float * mapaxes) {
  return ecl_grid_alloc_GRDECL_data__(NULL,
                                      FILEHEAD_SINGLE_POROSITY,
                                      apply_mapaxes,
                                      nx,
                                      ny,
                                      nz,
                                      zcorn,
                                      coord,
                                      actnum,
                                      mapaxes,
                                      NULL,
                                      0);
}


const float * ecl_grid_get_mapaxes_from_kw__(const ecl_kw_type * mapaxes_kw) {
    const float * mapaxes_data = ecl_kw_get_float_ptr(mapaxes_kw);

    float x1 = mapaxes_data[2];
    float y1 = mapaxes_data[3];
    float x2 = mapaxes_data[4];
    float y2 = mapaxes_data[5];

    float norm_denominator = x1 * y2 - x2 * y1;

    if(norm_denominator == 0.0) {
        mapaxes_data = NULL;
    }

    return mapaxes_data;
}

static ecl_grid_type * ecl_grid_alloc_GRDECL_kw__(ecl_grid_type * global_grid ,
                                                  int dualp_flag,
                                                  bool apply_mapaxes,
                                                  const ecl_kw_type * gridhead_kw ,
                                                  const ecl_kw_type * zcorn_kw ,
                                                  const ecl_kw_type * coord_kw ,
                                                  const ecl_kw_type * actnum_kw ,    /* Can be NULL */
                                                  const ecl_kw_type * mapaxes_kw ,   /* Can be NULL */
                                                  const ecl_kw_type * corsnum_kw) {   /* Can be NULL */
   int gtype, nx,ny,nz, lgr_nr;

  gtype   = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_TYPE_INDEX);
  nx      = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_NX_INDEX);
  ny      = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_NY_INDEX);
  nz      = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_NZ_INDEX);
  lgr_nr  = ecl_kw_iget_int(gridhead_kw , GRIDHEAD_LGR_INDEX);

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
      mapaxes_data = ecl_grid_get_mapaxes_from_kw__(mapaxes_kw);

    if (actnum_kw != NULL)
      actnum_data = ecl_kw_get_int_ptr(actnum_kw);

    if (corsnum_kw != NULL)
      corsnum_data = ecl_kw_get_int_ptr( corsnum_kw );

    return ecl_grid_alloc_GRDECL_data__(global_grid ,
                                        dualp_flag ,
                                        apply_mapaxes,
                                        nx , ny , nz ,
                                        ecl_kw_get_float_ptr(zcorn_kw) ,
                                        ecl_kw_get_float_ptr(coord_kw) ,
                                        actnum_data,
                                        mapaxes_data,
                                        corsnum_data,
                                        lgr_nr);
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

  bool apply_mapaxes = true;
  ecl_kw_type * gridhead_kw = ecl_grid_alloc_gridhead_kw( nx, ny, nz, 0);
  ecl_grid_type * ecl_grid = ecl_grid_alloc_GRDECL_kw__(NULL,
                                                        FILEHEAD_SINGLE_POROSITY,
                                                        apply_mapaxes,
                                                        gridhead_kw,
                                                        zcorn_kw,
                                                        coord_kw,
                                                        actnum_kw,
                                                        mapaxes_kw,
                                                        NULL);
  ecl_kw_free( gridhead_kw );
  return ecl_grid;

}






static void ecl_grid_init_cell_nnc_info(ecl_grid_type * ecl_grid, int global_index) {
  ecl_cell_type * grid_cell = ecl_grid_get_cell(ecl_grid, global_index);

  if (!grid_cell->nnc_info)
    grid_cell->nnc_info = nnc_info_alloc(ecl_grid->lgr_nr);
}

/*
  The function ecl_grid_add_self_nnc() will add a NNC connection
  between two cells in the same grid. Observe that there are two
  peculiarities with this implementation:

   1. In the ecl_grid structure the nnc information is distributed
      among the cells. The main purpose of adding the nnc information
      like this is to include the NNC information in the EGRID files
      when writing to disk. Before being written to disk the NNC
      information is serialized into vectors NNC1 and NNC2. It is the
      ordering in the NNC1 and NNC2 vectors which must be correct, and
      that is goverened by the nnc_index argument - i.e. the nnc_index
      serves as an 'ID' for the NNC connections.

      After all NCC information has been entered you should be certain
      that all nnc_index values in the range [0,num_nnc] have been
      set.


   2. To get valid NNC information to load in e.g. Resinsight the
      corresponding TRANNNC keyword must be added to the INIT file,
      i.e. the calling scope must create a ecl_kw with
      transmissibility values in parallell with adding NNC information
      to the grid:

         fortio_type * init_file = fortio_open_writer( "CASE.INIT" , ...
         ecl_grid_type * grid ...
         ecl_kw_type * trannnc_kw = ecl_kw_alloc( "TRANNNC" , num_nnc , ECL_FLOAT_TYPE );

         for (int i = 0; i < num_nnc; i++) {
             int   g1 = ...
             int   g2 = ..
             float T  = ..

             ecl_grid_add_self_nnc( grid , g1 , g2 , i );
             ecl_kw_iset( trannnc_kw , i , T );
         }
         ...
         ecl_grid_fwrite_EGRID( grid , ... );
         ecl_kw_fwrite( trannnc_kw , init_file );

*/

void ecl_grid_add_self_nnc( ecl_grid_type * grid, int cell_index1, int cell_index2, int nnc_index) {
  ecl_cell_type * grid_cell = ecl_grid_get_cell(grid, cell_index1);
  ecl_grid_init_cell_nnc_info(grid, cell_index1);
  nnc_info_add_nnc(grid_cell->nnc_info, grid->lgr_nr, cell_index2, nnc_index);
}

/*
  This function will add all the nnc connections given by the g1_list
  and g2_list arrays. The ncc connections will be added with
  consecutively running nnc_index = [0,num_nnc).
*/

void ecl_grid_add_self_nnc_list( ecl_grid_type * grid, const int * g1_list , const int * g2_list , int num_nnc ) {
  int i;
  for (i = 0; i < num_nnc; i++)
    ecl_grid_add_self_nnc( grid , g1_list[i] , g2_list[i] , i );
}

/*
  This function populates nnc_info for cells with non neighbour
  connections. For cells C1 and C2 the function will only add the directed link:

      C1 -> C2

  i.e. it is impossible to go from C2 and back to C1. The functions
  will add links as:

    NNC1 -> NNC2   For nnc between cells in the same grid.
    NNCG -> NNCL   For global -> lgr connection
    NNA1 -> NNA2   For links between different LGRs
*/

static void ecl_grid_init_nnc_cells( ecl_grid_type * grid1, ecl_grid_type * grid2, const ecl_kw_type * keyword1, const ecl_kw_type * keyword2) {

  int * grid1_nnc_cells = ecl_kw_get_int_ptr(keyword1);
  int * grid2_nnc_cells = ecl_kw_get_int_ptr(keyword2);
  int nnc_count = ecl_kw_get_size(keyword2);

  int nnc_index;
  for (nnc_index = 0; nnc_index < nnc_count; nnc_index++) {
    int grid1_cell_index = grid1_nnc_cells[nnc_index] -1;
    int grid2_cell_index = grid2_nnc_cells[nnc_index] -1;


  /*
    In the ECLIPSE output format grids with dual porosity are (to some
    extent ...) modeled as two independent grids stacked on top of
    eachother, where the fracture cells have global index in the range
    [nx*ny*nz, 2*nx*ny*nz).

    The physical connection between the matrix and the fractures in
    cell nr c is modelled as an nnc: cell[c] -> cell[c + nx*ny*nz]. In
    the ert ecl library we only have cells in the range [0,nx*ny*nz),
    and fracture is a property of a cell. We therefore do not include
    nnc connections involving fracture cells (i.e. cell_index >=
    nx*ny*nz).
  */
    if ((FILEHEAD_SINGLE_POROSITY != grid1->dualp_flag) &&
        ((grid1_cell_index >= grid1->size) ||
         (grid2_cell_index >= grid2->size)))
      break;



    {
      ecl_cell_type * grid1_cell = ecl_grid_get_cell(grid1, grid1_cell_index);
      ecl_grid_init_cell_nnc_info(grid1, grid1_cell_index);
      nnc_info_add_nnc(grid1_cell->nnc_info, grid2->lgr_nr, grid2_cell_index , nnc_index);
    }
  }
}


/*
  This function reads the non-neighbour connection data from file and initializes the grid structure with the the nnc data
*/
static void ecl_grid_init_nnc(ecl_grid_type * main_grid, ecl_file_type * ecl_file) {
  int num_nnchead_kw = ecl_file_get_num_named_kw( ecl_file , NNCHEAD_KW );
  int i;

  /*
    NB: There is a bug in Eclipse version 2015.1, for MPI runs with
        six or more processors (I think ...) the NNC datastructures
        are in an internally inconsistent state; and will lead to a
        hard crash. The issue has been fixed in version 2015.2, but
        unfortunately it is not possible to test for micro version.

        if(num_nnchead_kw > 0 && main_grid->eclipse_version == 2015)
           return;
  */

  for (i = 0; i < num_nnchead_kw; i++) {
    ecl_file_view_type * lgr_view = ecl_file_alloc_global_blockview(ecl_file , NNCHEAD_KW , i);
    ecl_kw_type * nnchead_kw = ecl_file_view_iget_named_kw(lgr_view, NNCHEAD_KW, 0);
    int lgr_nr = ecl_kw_iget_int(nnchead_kw, NNCHEAD_LGR_INDEX);

    if (ecl_file_view_has_kw(lgr_view , NNC1_KW)) {
      const ecl_kw_type * nnc1 = ecl_file_view_iget_named_kw(lgr_view, NNC1_KW, 0);
      const ecl_kw_type * nnc2 = ecl_file_view_iget_named_kw(lgr_view, NNC2_KW, 0);

      {
        ecl_grid_type * grid = (lgr_nr > 0) ? ecl_grid_get_lgr_from_lgr_nr(main_grid, lgr_nr) : main_grid;
        ecl_grid_init_nnc_cells(grid, grid, nnc1, nnc2);
      }
    }

    if (ecl_file_view_has_kw(lgr_view , NNCL_KW)) {
      const ecl_kw_type * nncl = ecl_file_view_iget_named_kw(lgr_view, NNCL_KW, 0);
      const ecl_kw_type * nncg = ecl_file_view_iget_named_kw(lgr_view, NNCG_KW, 0);
      {
        ecl_grid_type * grid = (lgr_nr > 0) ? ecl_grid_get_lgr_from_lgr_nr(main_grid, lgr_nr) : main_grid;
        ecl_grid_init_nnc_cells(main_grid, grid , nncg, nncl);
      }
    }

    ecl_file_view_free( lgr_view );
  }
}

/*
  This function reads the non-neighbour connection data for
  amalgamated LGRs (that is, non-neighbour connections between two
  LGRs) and initializes the grid structure with the nnc data.
*/
static void ecl_grid_init_nnc_amalgamated(ecl_grid_type * main_grid, ecl_file_type * ecl_file) {
  int num_nncheada_kw   = ecl_file_get_num_named_kw( ecl_file , NNCHEADA_KW );

  int i;
  for (i = 0; i < num_nncheada_kw; i++) {
    ecl_kw_type * nncheada_kw = ecl_file_iget_named_kw( ecl_file , NNCHEADA_KW , i);
    int lgr_nr1 = ecl_kw_iget_int(nncheada_kw, NNCHEADA_ILOC1_INDEX);
    int lgr_nr2 = ecl_kw_iget_int(nncheada_kw, NNCHEADA_ILOC2_INDEX);

    ecl_grid_type * lgr_grid1 = ecl_grid_get_lgr_from_lgr_nr(main_grid, lgr_nr1);
    ecl_grid_type * lgr_grid2 = ecl_grid_get_lgr_from_lgr_nr(main_grid, lgr_nr2);

    ecl_kw_type * nna1_kw = ecl_file_iget_named_kw( ecl_file , NNA1_KW , i);
    ecl_kw_type * nna2_kw = ecl_file_iget_named_kw( ecl_file , NNA2_KW , i);

    ecl_grid_init_nnc_cells( lgr_grid1 ,lgr_grid2,  nna1_kw, nna2_kw);

  }
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


static ecl_grid_type * ecl_grid_alloc_EGRID__( ecl_grid_type * main_grid , const ecl_file_type * ecl_file , int grid_nr, bool apply_mapaxes) {
  ecl_kw_type * gridhead_kw  = ecl_file_iget_named_kw( ecl_file , GRIDHEAD_KW  , grid_nr);
  ecl_kw_type * zcorn_kw     = ecl_file_iget_named_kw( ecl_file , ZCORN_KW     , grid_nr);
  ecl_kw_type * coord_kw     = ecl_file_iget_named_kw( ecl_file , COORD_KW     , grid_nr);
  ecl_kw_type * corsnum_kw   = NULL;
  ecl_kw_type * actnum_kw    = NULL;
  ecl_kw_type * mapaxes_kw   = NULL;
  int dualp_flag;
  int eclipse_version;
  if (grid_nr == 0) {
    ecl_kw_type * filehead_kw  = ecl_file_iget_named_kw( ecl_file , FILEHEAD_KW  , grid_nr);
    dualp_flag                 = ecl_kw_iget_int( filehead_kw , FILEHEAD_DUALP_INDEX );
    eclipse_version = ecl_kw_iget_int( filehead_kw, FILEHEAD_YEAR_INDEX);
  } else{
    dualp_flag = main_grid->dualp_flag;
    eclipse_version = main_grid->eclipse_version;
  }


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
                                                           apply_mapaxes,
                                                           gridhead_kw ,
                                                           zcorn_kw ,
                                                           coord_kw ,
                                                           actnum_kw ,
                                                           mapaxes_kw ,
                                                           corsnum_kw );

    if (ECL_GRID_MAINGRID_LGR_NR != grid_nr) ecl_grid_set_lgr_name_EGRID(ecl_grid , ecl_file , grid_nr);
    ecl_grid->eclipse_version = eclipse_version;
    return ecl_grid;
  }
}



ecl_grid_type * ecl_grid_alloc_EGRID(const char * grid_file, bool apply_mapaxes) {
  ecl_file_enum   file_type;
  file_type = ecl_util_get_file_type(grid_file , NULL , NULL);
  if (file_type != ECL_EGRID_FILE)
    util_abort("%s: %s wrong file type - expected .EGRID file - aborting \n",__func__ , grid_file);
  {
    ecl_file_type * ecl_file   = ecl_file_open( grid_file , 0);
    if (ecl_file) {
      int num_grid               = ecl_file_get_num_named_kw( ecl_file , GRIDHEAD_KW );
      ecl_grid_type * main_grid  = ecl_grid_alloc_EGRID__( NULL , ecl_file , 0 , apply_mapaxes);
      int grid_nr;

      for ( grid_nr = 1; grid_nr < num_grid; grid_nr++) {
        ecl_grid_type * lgr_grid = ecl_grid_alloc_EGRID__( main_grid , ecl_file , grid_nr , false);  /* The apply_mapaxes argument is ignored for LGR - it inherits from parent anyway. */
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
      ecl_grid_init_nnc(main_grid, ecl_file);
      ecl_grid_init_nnc_amalgamated(main_grid, ecl_file);

      ecl_file_close( ecl_file );
      return main_grid;
    } else
      return NULL;
  }
}





static ecl_grid_type * ecl_grid_alloc_GRID_data__(ecl_grid_type * global_grid , int num_coords , int dualp_flag , bool apply_mapaxes, int nx, int ny , int nz , int grid_nr , int coords_size , int ** coords , float ** corners , const float * mapaxes) {
  if (dualp_flag != FILEHEAD_SINGLE_POROSITY)
    nz = nz / 2;
  {
    ecl_grid_type * grid = ecl_grid_alloc_empty( global_grid , dualp_flag , nx , ny , nz , grid_nr, false);
    if (grid) {
      if (mapaxes != NULL)
        ecl_grid_init_mapaxes( grid , apply_mapaxes , mapaxes);

      {
        int index;
        for ( index=0; index < num_coords; index++)
          ecl_grid_set_cell_GRID(grid , coords_size , coords[index] , corners[index]);
      }

      ecl_grid_init_coarse_cells( grid );
      ecl_grid_update_index( grid );
      ecl_grid_taint_cells( grid );

    }
    return grid;
  }
}

/*
  coords[num_coords][coords_size]
  corners[num_coords][24]
*/

ecl_grid_type * ecl_grid_alloc_GRID_data(int num_coords , int nx , int ny , int nz , int coords_size , int ** coords , float ** corners , bool apply_mapaxes, const float * mapaxes) {
  return ecl_grid_alloc_GRID_data__( NULL ,
                                     num_coords ,
                                     FILEHEAD_SINGLE_POROSITY , /* Does currently not support to determine dualp_flag from inspection. */
                                     apply_mapaxes,
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


static ecl_grid_type * ecl_grid_alloc_GRID__(ecl_grid_type * global_grid , const ecl_file_type * ecl_file , int cell_offset , int grid_nr, int dualp_flag, bool apply_mapaxes) {
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
      mapaxes_data = ecl_grid_get_mapaxes_from_kw__(mapaxes_kw);
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
      grid = ecl_grid_alloc_GRID_data__( global_grid , num_coords , dualp_flag , apply_mapaxes, nx , ny , nz , grid_nr , coords_size , coords , corners , mapaxes_data );

      free( coords );
      free( corners );
    }
  }

  if (grid_nr > 0) ecl_grid_set_lgr_name_GRID(grid , ecl_file , grid_nr);
  return grid;
}





ecl_grid_type * ecl_grid_alloc_GRID(const char * grid_file, bool apply_mapaxes) {

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
    main_grid  = ecl_grid_alloc_GRID__(NULL , ecl_file , cell_offset , 0,dualp_flag , apply_mapaxes);
    cell_offset += ecl_grid_get_global_size( main_grid );

    for (grid_nr = 1; grid_nr < num_grid; grid_nr++) {
      ecl_grid_type * lgr_grid = ecl_grid_alloc_GRID__(main_grid , ecl_file , cell_offset , grid_nr , dualp_flag, false);
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
  ecl_grid_type * grid = ecl_grid_alloc_empty(NULL , FILEHEAD_SINGLE_POROSITY , nx , ny , nz , 0, true);
  if (grid) {
    const double grid_offset[3] = {0,0,0};

    int k,j,i;
    for (k=0; k < nz; k++) {
      for (j=0; j< ny; j++) {
        for (i=0; i < nx; i++) {
          int global_index = i + j*nx + k*nx*ny;
          double offset[3] = {
            grid_offset[0] + i*ivec[0] + j*jvec[0] + k*kvec[0],
            grid_offset[1] + i*ivec[1] + j*jvec[1] + k*kvec[1],
            grid_offset[2] + i*ivec[2] + j*jvec[2] + k*kvec[2]
          };

          ecl_cell_type * cell = ecl_grid_get_cell(grid , global_index );
          ecl_cell_init_regular( cell , offset , i,j,k,global_index , ivec , jvec , kvec , actnum );
        }
      }
    }
    ecl_grid_update_index( grid );
  }

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
   This function will allocate a new grid with logical dimensions nx x
   ny x nz. The cells in the grid are spanned by the dxv, dyv and dzv
   vectors which are of size nx, ny and nz and specify the thickness
   of a cell in x, y, and in z direction.

   The actnum argument is a pointer to an integer array of length
   nx*ny*nz where actnum[i + j*nx + k*nx*ny] == 1 for active cells and
   0 for inactive cells. The actnum array can be NULL, in which case
   all cells will be active.  */
ecl_grid_type * ecl_grid_alloc_dxv_dyv_dzv( int nx, int ny , int nz , const double * dxv , const double * dyv , const double * dzv , const int * actnum)
{
    ecl_grid_type* grid = ecl_grid_alloc_empty(NULL,
                                               FILEHEAD_SINGLE_POROSITY,
                                               nx, ny, nz,
                                               /*lgr_nr=*/0, /*init_valid=*/true);
    if (grid) {
      double ivec[3] = { 0, 0, 0 };
      double jvec[3] = { 0, 0, 0 };
      double kvec[3] = { 0, 0, 0 };

      const double grid_offset[3] = {0,0,0};
      double offset[3];
      int i, j, k;
      offset[2] = grid_offset[2];
      for (k=0; k < nz; k++) {
        kvec[2] = dzv[k];
        offset[1] = grid_offset[1];
        for (j=0; j < ny; j++) {
          jvec[1] = dyv[j];
          offset[0] = grid_offset[0];
          for (i=0; i < nx; i++) {
            int global_index = i + j*nx + k*nx*ny;
            ecl_cell_type* cell = ecl_grid_get_cell(grid, global_index);
            ivec[0] = dxv[i];

            ecl_cell_init_regular(cell, offset,
                                  i,j,k,global_index,
                                  ivec,jvec,kvec,
                                  actnum);
            offset[0] += dxv[i];
          }
          offset[1] += dyv[j];
        }
        offset[2] += dzv[k];
      }
      ecl_grid_update_index(grid);
    }

    return grid;
}


ecl_grid_type * ecl_grid_alloc_dxv_dyv_dzv_depthz( int nx, int ny , int nz , const double * dxv , const double * dyv , const double * dzv , const double * depthz , const int * actnum)
{
    ecl_grid_type* grid = ecl_grid_alloc_empty(NULL,
                                               FILEHEAD_SINGLE_POROSITY,
                                               nx, ny, nz,
                                               /*lgr_nr=*/0, /*init_valid=*/true);


    /* First layer - where the DEPTHZ keyword applies. */
    if (grid) {
      int i,j;
      int k = 0;
      double y0 = 0;
      for (j=0; j < ny; j++) {
        double x0 = 0;
        for (i = 0; i < nx; i++) {
          int global_index = i + j*nx + k*nx*ny;
          ecl_cell_type* cell = ecl_grid_get_cell(grid, global_index);
          double z0 = depthz[ i     + j*(nx + 1)];
          double z1 = depthz[ i + 1 + j*(nx + 1)];
          double z2 = depthz[ i +     (j + 1)*(nx + 1)];
          double z3 = depthz[ i + 1 + (j + 1)*(nx + 1)];


          point_set(&cell->corner_list[0] , x0 , y0 , z0);
          point_set(&cell->corner_list[1] , x0 + dxv[i] , y0 , z1);
          point_set(&cell->corner_list[2] , x0          , y0 + dyv[j] , z2);
          point_set(&cell->corner_list[3] , x0 + dxv[i] , y0 + dyv[j] , z3);
          {
            int c;
            for (c = 0; c < 4; c++) {
              cell->corner_list[c + 4] = cell->corner_list[c];
              point_shift(&cell->corner_list[c + 4] , 0 , 0 , dzv[0]);
            }
          }
          x0 += dxv[i];
        }
        y0 += dyv[j];
      }
    }

    /* Remaining layers */
    if (grid) {
      int i,j,k;
      for (k=1; k < nz; k++) {
        for (j=0; j <ny; j++) {
          for (i=0; i < nx; i++) {
            int g2 = i + j*nx + k*nx*ny;
            int g1 = i + j*nx + (k - 1)*nx*ny;
            ecl_cell_type* cell2 = ecl_grid_get_cell(grid, g2);
            ecl_cell_type* cell1 = ecl_grid_get_cell(grid, g1);
            int c;

            for (c = 0; c < 4; c++) {
              cell2->corner_list[c] = cell1->corner_list[c + 4];
              cell2->corner_list[c + 4] = cell1->corner_list[c + 4];
              point_shift( &cell2->corner_list[c + 4] , 0 , 0 , dzv[k]);
            }
          }
        }
      }
    }

    if (grid) {
      int i,j,k;
      for (k=0; k < nz; k++) {
        for (j=0; j <ny; j++) {
          for (i=0; i < nx; i++) {
            int global_index = i + j*nx + k*nx*ny;
            ecl_cell_type* cell = ecl_grid_get_cell(grid, global_index);

            if (actnum)
              cell->active = actnum[global_index];
            else
              cell->active = CELL_ACTIVE;
          }
        }
      }
    }

    if (grid)
      ecl_grid_update_index(grid);

    return grid;
}


/**
   This is a really broken function which is here only to support
   creating rectangualar grids from OPM. The vectors dx,dy,dz and tops
   are all of length nx*ny*nz. In principle all values of these four
   arrays are independent - but taking that all out will create some
   really weird looking grids. For physical correctness there should
   be many constraints among the different values.

   The x and y position of a cell is found by adding the increments
   from dx and dy, whereas the vertical position is read dircetly out
   of the tops array.

   The ECLIPSE input format only requires size(dz) >= nx*ny and the
   same for tops. The remaining layers will then be extrapolated; such
   trickery must be performed before calling this function.
*/

ecl_grid_type * ecl_grid_alloc_dx_dy_dz_tops( int nx, int ny , int nz , const double * dx , const double * dy , const double * dz , const double * tops , const int * actnum) {

  ecl_grid_type* grid = ecl_grid_alloc_empty(NULL,
                                             FILEHEAD_SINGLE_POROSITY,
                                             nx, ny, nz,
                                             0, true);
  if (grid) {
    int i, j, k;
    double * y0 = util_calloc( nx, sizeof * y0 );

    for (k=0; k < nz; k++) {
      for (i=0; i < nx; i++) {
        y0[i] = 0;
      }
      for (j=0; j < ny; j++) {
        double x0 = 0;
        for (i=0; i < nx; i++) {
          int g = i + j*nx + k*nx*ny;
          ecl_cell_type* cell = ecl_grid_get_cell(grid, g);
          double z0 = tops[ g ];

          point_set(&cell->corner_list[0] , x0         , y0[i]         , z0);
          point_set(&cell->corner_list[1] , x0 + dx[g] , y0[i]         , z0);
          point_set(&cell->corner_list[2] , x0         , y0[i] + dy[g] , z0);
          point_set(&cell->corner_list[3] , x0 + dx[g] , y0[i] + dy[g] , z0);

          point_set(&cell->corner_list[4] , x0         , y0[i]         , z0 + dz[g]);
          point_set(&cell->corner_list[5] , x0 + dx[g] , y0[i]         , z0 + dz[g]);
          point_set(&cell->corner_list[6] , x0         , y0[i] + dy[g] , z0 + dz[g]);
          point_set(&cell->corner_list[7] , x0 + dx[g] , y0[i] + dy[g] , z0 + dz[g]);

          x0    += dx[g];
          y0[i] += dy[g];

          if (actnum != NULL)
            cell->active = actnum[g];
          else
            cell->active = CELL_ACTIVE;
        }
      }
    }
    free( y0 );

    ecl_grid_update_index(grid);
  }
  return grid;
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

ecl_grid_type * ecl_grid_alloc__(const char * grid_file , bool apply_mapaxes) {
  ecl_file_enum    file_type;
  ecl_grid_type  * ecl_grid = NULL;

  file_type = ecl_util_get_file_type(grid_file , NULL ,  NULL);
  if (file_type == ECL_GRID_FILE)
    ecl_grid = ecl_grid_alloc_GRID(grid_file, apply_mapaxes);
  else if (file_type == ECL_EGRID_FILE)
    ecl_grid = ecl_grid_alloc_EGRID(grid_file, apply_mapaxes);
  else
    util_abort("%s must have .GRID or .EGRID file - %s not recognized \n", __func__ , grid_file);

  return ecl_grid;
}


ecl_grid_type * ecl_grid_alloc(const char * grid_file ) {
  bool apply_mapaxes = true;
  return ecl_grid_alloc__( grid_file , apply_mapaxes );
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
   (nx,ny,nz) are successfully set. If the dimensions are not set the
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

ecl_grid_type * ecl_grid_load_case__( const char * case_input , bool apply_mapaxes) {
  ecl_grid_type * ecl_grid = NULL;
  char * grid_file = ecl_grid_alloc_case_filename( case_input );
  if (grid_file != NULL) {

    if (util_file_exists( grid_file ))
      ecl_grid = ecl_grid_alloc__( grid_file , apply_mapaxes);

    free( grid_file );
  }
  return ecl_grid;
}

ecl_grid_type * ecl_grid_load_case( const char * case_input ) {
  bool apply_mapaxes = true;
  return ecl_grid_load_case__( case_input , apply_mapaxes );
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


static bool ecl_grid_compare_coarse_cells(const ecl_grid_type * g1 , const ecl_grid_type * g2, bool verbose) {
  if (vector_get_size( g1->coarse_cells ) == vector_get_size( g2->coarse_cells )) {
    bool equal = true;
    int c;

    for (c = 0; c < vector_get_size( g1->coarse_cells ); c++) {
      const ecl_coarse_cell_type * coarse_cell1 = vector_iget_const( g1->coarse_cells , c);
      const ecl_coarse_cell_type * coarse_cell2 = vector_iget_const( g2->coarse_cells , c);

      equal = ecl_coarse_cell_equal( coarse_cell1 , coarse_cell2 );
      if (!equal)
        if (verbose) fprintf(stderr,"Difference in coarse cell:%d \n",c );

    }
    return equal;
  } else
    return false;
}


static bool ecl_grid_compare_cells(const ecl_grid_type * g1 , const ecl_grid_type * g2, bool include_nnc , bool verbose) {
  int g;
  bool equal = true;
  for (g = 0; g < g1->size; g++) {
    bool this_equal = true;
    ecl_cell_type *c1 = ecl_grid_get_cell( g1 , g );
    ecl_cell_type *c2 = ecl_grid_get_cell( g2 , g );
    ecl_cell_compare(c1 , c2 ,  include_nnc , &this_equal);

    if (!this_equal) {
      if (verbose) {
        int i,j,k;
        ecl_grid_get_ijk1( g1 , g , &i , &j , &k);

        printf("Difference in cell: %d : %d,%d,%d  nnc_equal:%d Volume:%g \n",g,i,j,k , nnc_info_equal( c1->nnc_info , c2->nnc_info) , ecl_cell_get_volume( c1 ));
        printf("-----------------------------------------------------------------\n");
        ecl_cell_dump_ascii( c1 , i , j , k , stdout , NULL);
        printf("-----------------------------------------------------------------\n");
        ecl_cell_dump_ascii( c2 , i , j , k , stdout , NULL );
        printf("-----------------------------------------------------------------\n");

      }
      equal = false;
      break;
    }
  }
  return equal;
}

static bool ecl_grid_compare_index(const ecl_grid_type * g1 , const ecl_grid_type * g2, bool verbose) {
  bool equal = true;

  if (g1->total_active != g2->total_active) {
    if (verbose)
      fprintf(stderr,"Difference in total active:%d / %d\n",g1->total_active , g2->total_active);
    equal = false;
  }

  if (equal) {
    if (memcmp( g1->index_map , g2->index_map , g1->size * sizeof * g1->index_map ) != 0) {
      equal = false;
      if (verbose)
        fprintf(stderr,"Difference in index map \n");
    }
  }

  if (equal) {
    if (memcmp( g1->inv_index_map , g2->inv_index_map , g1->total_active * sizeof * g1->inv_index_map ) != 0) {
      equal = false;
      if (verbose)
        fprintf(stderr,"Difference in inverse index map \n");
    }
  }

  if (equal && (g1->dualp_flag != FILEHEAD_SINGLE_POROSITY)) {
    if (g1->total_active_fracture != g2->total_active_fracture) {
      if (verbose)
        fprintf(stderr,"Difference in toal_active_fracture %d / %d \n",g1->total_active_fracture , g2->total_active_fracture);
      equal = false;
    }

    if (equal) {
      if (memcmp( g1->fracture_index_map , g2->fracture_index_map , g1->size * sizeof * g1->fracture_index_map ) != 0) {
        equal = false;
        if (verbose)
          fprintf(stderr,"Difference in fracture_index_map \n");
      }
    }

    if (equal) {
      if (memcmp( g1->inv_fracture_index_map , g2->inv_fracture_index_map , g1->total_active_fracture * sizeof * g1->inv_fracture_index_map ) != 0) {
        equal = false;
        if (verbose)
          fprintf(stderr,"Difference in inv_fracture_index_map \n");
      }
    }

  }
  return equal;
}


static bool ecl_grid_compare_mapaxes(const ecl_grid_type * g1 , const ecl_grid_type * g2, bool verbose) {
  bool equal = true;
  if (g1->use_mapaxes == g2->use_mapaxes) {
    if (g1->mapaxes) {
      if (memcmp( g1->mapaxes , g2->mapaxes , 6 * sizeof * g1->mapaxes ) != 0)
        equal = false;
    }
  } else
    equal = false;

  if (!equal && verbose)
    fprintf(stderr,"Difference in mapaxes \n" );

  return equal;
}


/**
   Return true if grids g1 and g2 are equal, and false otherwise. To
   return true all cells must be identical.
*/

static bool ecl_grid_compare__(const ecl_grid_type * g1 , const ecl_grid_type * g2, bool include_nnc , bool verbose) {

  bool equal = true;
  if (g1->size != g2->size)
    equal = false;

  // The name of the parent grid corresponds to a filename; they can be different.
  if (equal && g1->parent_grid) {
    if (!util_string_equal( g1->name , g2->name )) {
      equal = false;
      if (verbose)
        fprintf(stderr,"Difference in name %s <-> %s \n" , g1->name , g2->name);
    }
  }

  /*
    When .GRID files are involved this is hardwired to FILEHEAD_SINGLE_POROSITY.
  */
  if (g1->dualp_flag != g2->dualp_flag) {
    equal = false;
    if (verbose)
      fprintf(stderr,"Dual porosity flags differ: %d / %d \n" , g1->dualp_flag , g2->dualp_flag);
  }

  if (equal)
    equal = ecl_grid_compare_cells(g1 , g2 , include_nnc , verbose);

  if (equal)
    equal = ecl_grid_compare_index( g1 , g2 , true /*verbose*/);

  if (equal)
    equal = ecl_grid_compare_coarse_cells( g1 , g2 , verbose );

  if (equal)
    equal = ecl_grid_compare_mapaxes( g1 , g2 , verbose );

  return equal;
}


bool ecl_grid_compare(const ecl_grid_type * g1 , const ecl_grid_type * g2 , bool include_lgr, bool include_nnc , bool verbose) {
  bool equal = ecl_grid_compare__(g1 , g2 , include_nnc , verbose);

  if (equal && include_lgr) {
    if (vector_get_size( g1->LGR_list ) == vector_get_size( g2->LGR_list )) {
      int grid_nr;
      for (grid_nr = 0; grid_nr < vector_get_size( g1->LGR_list ); grid_nr++) {
        const ecl_grid_type * lgr1 = vector_iget_const( g1->LGR_list , grid_nr);
        const ecl_grid_type * lgr2 = vector_iget_const( g2->LGR_list , grid_nr);

        printf("Comparing lgr grid:%d \n",grid_nr);
        equal = ecl_grid_compare__(lgr1 , lgr2 , include_nnc , verbose);
        if (!equal)
          break;
      }
    } else
      equal = false;
  }
  return equal;
}

/*****************************************************************/

typedef enum {NOT_ON_FACE, BELONGS_TO_CELL, BELONGS_TO_OTHER} face_status_enum;

/*
    Returns whether the given point is contained within the minimal cube
    encapsulating the cell that has all faces parallel to a coordinate plane.
*/
static bool ecl_grid_cube_contains(const ecl_cell_type * cell, const point_type * p) {
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

  return true;
}

/*
   Returns true if and only if p is on plane "plane" of cell when decomposed by "method".
*/
static bool ecl_grid_on_plane(const ecl_cell_type * cell, const int method,
        const int plane, const point_type * p) {
  const point_type * p0 = &cell->corner_list[ tetrahedron_permutations[method][plane][0] ];
  const point_type * p1 = &cell->corner_list[ tetrahedron_permutations[method][plane][1] ];
  const point_type * p2 = &cell->corner_list[ tetrahedron_permutations[method][plane][2] ];
  return triangle_contains3d(p0, p1, p2, p);
}

/*
   Returns true if and only if p is on one of the cells faces and
   "belongs" to this cell. This is done such that every point is contained in at most
   one point.

   Known caveats when using this function:
     - if a point is on the surface of a/many cells, but for all of these cells
       the point is contained on two opposite sides of the cell. Imagine a cake
       being cut as a cake should be cut. To which of the slices does the center
       point of the cake belong? This is a somewhat obscure situation and it is
       not possible to circumvent by only considering the grid cell by cell.
    - if there is a fault and this cell is on the border of the grid.
    - if a cells projection to the xy-plane is concave, this method might give
      false positives.

   Note: The correctness of this function relies *HEAVILY* on the permutation of the
   tetrahedrons in the decompositions.
*/
static face_status_enum ecl_grid_on_cell_face(const ecl_cell_type * cell, const int method,
        const point_type * p,
        const bool max_i, const bool max_j, const bool max_k) {

  int k_minus = 0, j_pluss = 1, i_minus = 2, i_pluss = 3, j_minus = 4, k_pluss = 5;
  bool on[6];
  for(int i = 0; i < 6; ++i) {
    on[i] = (
        ecl_grid_on_plane(cell, method, 2*i, p) ||
        ecl_grid_on_plane(cell, method, 2*i+1, p)
            );
  }

  // Not on any of the cell sides
  if(!on[k_minus] && !on[k_pluss] && !on[j_pluss] && !on[j_minus] && !on[i_minus] && !on[i_pluss])
    return NOT_ON_FACE;

  // Handles side collapses, i.e. the point is contained on opposite sides.
  // Cell passes on the responsibility if not on border of grid.
  bool i_collapse = (on[i_minus] && on[i_pluss]);
  bool j_collapse = (on[j_minus] && on[j_pluss]);
  bool k_collapse = (on[k_minus] && on[k_pluss]);

  for(int i = 0; i < 6; ++i)
    on[i] &= (!i_collapse || max_i) && (!j_collapse || max_j) && (!k_collapse || max_k);

  on[i_minus] &= !on[i_pluss];
  on[j_minus] &= !on[j_pluss];
  on[k_minus] &= !on[k_pluss];

  // Removed from all sides
  if(!on[k_minus] && !on[k_pluss] && !on[j_pluss] && !on[j_minus] && !on[i_minus] && !on[i_pluss])
    return BELONGS_TO_OTHER;

  // Not on any of the lower priority sides
  if(!on[k_pluss] && !on[j_pluss] && !on[i_pluss])
    return BELONGS_TO_CELL;

  // Contained in cell due to border conditions
  // NOTE: One should read X <= Y as X "implies" Y
  if((on[i_pluss] <= max_i) && (on[j_pluss] <= max_j) && (on[k_pluss] <= max_k))
    return BELONGS_TO_CELL;

  return BELONGS_TO_OTHER;
}

/*
 Returns true if and only if the tetrahedron defined by p0, p1, p2, p3
 contains p.

 The sole purpose of this functions is to make concave_cell_contains
 more readable.
*/
static bool tetrahedron_by_points_contains(const point_type * p0,
                                           const point_type * p1,
                                           const point_type * p2,
                                           const point_type * p3,
                                           const point_type * p) {

  tetrahedron_type pro_tet;
  pro_tet.p0 = *p0;
  pro_tet.p1 = *p1;
  pro_tet.p2 = *p2;
  pro_tet.p3 = *p3;

  return tetrahedron_contains(pro_tet, *p);
}

/*
 Returns true if and only if the cell "cell" decomposed by "method" contains the point "p".
 This is done by decomposing the cell into 5 tetrahedrons according to the decomposition
 method for the faces.

 Assumes the cell to not be self-intersecting!

 Note: This function relies *HEAVILY* on the permutation of tetrahedron_permutations.
*/
static bool concave_cell_contains( const ecl_cell_type * cell, int method, const point_type * p) {

  const point_type * dia[2][2] = {
      {
          &cell->corner_list[tetrahedron_permutations[method][0][1]],
          &cell->corner_list[tetrahedron_permutations[method][0][2]]
      },
      {
          &cell->corner_list[tetrahedron_permutations[method][10][1]],
          &cell->corner_list[tetrahedron_permutations[method][10][2]]
      }
  };

  const point_type * extra[2][2] = {
      {
          &cell->corner_list[tetrahedron_permutations[method][0][0]],
          &cell->corner_list[tetrahedron_permutations[method][1][0]]
      },
      {
          &cell->corner_list[tetrahedron_permutations[method][10][0]],
          &cell->corner_list[tetrahedron_permutations[method][11][0]]
      }
  };

  // Test for containment in cell core
  if (tetrahedron_by_points_contains(dia[0][0], dia[1][0], dia[0][1], dia[1][1], p))
    return true;

  // Test for containment in protrusions
  for(int i = 0; i < 2; ++i) {
    if(tetrahedron_by_points_contains(dia[i][0], dia[i][1], dia[(i+1)%2][0], extra[i][0], p))
      return true;

    if(tetrahedron_by_points_contains(dia[i][0], dia[(i+1)%2][1], dia[i][1], extra[i][1], p))
      return true;
  }

  return false;
}

/*
  Observe the following quirks with this functions:

  - It is quite simple to create a cell where the center point is
    actually *not* inside the cell - that might come as a surprise!

  - Cells with nonzero twist are completely discarded from the search,
    if the point (x,y,z) "should" have been found on the inside of a
    twisted cell the algorithm will incorrectly return false; a
    warning will be printed on stderr if a cell is discarded due to
    twist.

  - See the documentation of ecl_grid_on_cell_face for caveats regarding
    containtment of points of cell faces.
*/
bool ecl_grid_cell_contains_xyz3( const ecl_grid_type * ecl_grid , int i, int j , int k, double x , double y , double z) {
  point_type p;
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , ecl_grid_get_global_index3( ecl_grid , i, j , k ));
  point_set( &p , x , y , z);
  int method = (i + j + k) % 2; // Chooses the approperiate decomposition method for the cell

  if (GET_CELL_FLAG(cell , CELL_FLAG_TAINTED))
    return false;

  // Pruning
  if (!ecl_grid_cube_contains(cell, &p))
    return false;

  // Checks if point is on one of the faces of the cell, and if so whether it
  // "belongs" to this cell.
  bool max_i = (i == ecl_grid->nx-1);
  bool max_j = (j == ecl_grid->ny-1);
  bool max_k = (k == ecl_grid->nz-1);
  face_status_enum face_status = ecl_grid_on_cell_face(cell, method, &p, max_i, max_j, max_k);

  if(face_status != NOT_ON_FACE)
    return face_status == BELONGS_TO_CELL;

  // Twisted cells
  if (ecl_cell_get_twist(cell) > 0) {
    fprintf(stderr, "** Warning: Point (%g,%g,%g) is in vicinity of twisted cell: (%d,%d,%d) - function:%s might be mistaken.\n", x,y,z,i,j,k, __func__);
    return false;
  }

  // We now check whether the point is strictly inside the cell
  return concave_cell_contains(cell, method, &p);
}


bool ecl_grid_cell_contains_xyz1( const ecl_grid_type * ecl_grid , int global_index, double x , double y , double z) {
  int i,j,k;
  ecl_grid_get_ijk1( ecl_grid , global_index , &i , &j , &k);
  return ecl_grid_cell_contains_xyz3( ecl_grid , i,j,k,x ,y  , z);
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
          if (ecl_grid_cell_contains_xyz1( grid , global_index , p->x , p->y , p->z )) {
            return global_index;
          }
        }
      }
  return -1;  /* Returning -1; did not find xyz. */
}


/**
 * Search for given xyz coordinate around global start_index in a box of size bx
 */
static int ecl_grid_get_global_index_from_xyz_around_box(ecl_grid_type * grid , double x , double y , double z , int start_index, int bx, point_type * p) {
  /* Try neighbours */
    int i,j,k;
    int i1,i2,j1,j2,k1,k2;
    int nx,ny,nz;
    ecl_grid_get_dims( grid , &nx , &ny , &nz , NULL);
    ecl_grid_get_ijk1( grid , start_index , &i , &j , &k);

    i1 = util_int_max( 0 , i - bx );
    j1 = util_int_max( 0 , j - bx );
    k1 = util_int_max( 0 , k - bx );

    i2 = util_int_min( nx , i + bx );
    j2 = util_int_min( ny , j + bx );
    k2 = util_int_min( nz , k + bx );

    int global_index = ecl_grid_box_contains_xyz( grid , i1 , i2 , j1 , j2 , k1 , k2 , p);
    return global_index;
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
    if (ecl_grid_cell_contains_xyz1( grid , start_index , x,y,z))
      return start_index;
    else {
      /* Try boxes 2, 4, 8, ..., 64  */
      for (int bx = 1; bx <= 6; bx++) {
        global_index = ecl_grid_get_global_index_from_xyz_around_box(grid, x, y, z,
                                                                     start_index,
                                                                     1<<bx, &p);
        if (global_index >= 0)
          return global_index;
      }
    }
  }

  /*
    OK - the attempted shortcuts did not pay off. Perform full linear search.
  */

  global_index = -1;

  for (int index = 0; index < grid->size; index++) {
    if (ecl_grid_cell_contains_xyz1( grid , index , x , y , z))
      return index;
  }
  return -1;
}

bool ecl_grid_get_ijk_from_xyz(ecl_grid_type * grid , double x , double y , double z , int start_index, int *i, int *j, int *k ) {
  int g = ecl_grid_get_global_index_from_xyz(grid, x, y, z, start_index);
  if (g < 0)
    return false;

  ecl_grid_get_ijk1( grid , g , i,j,k);
  return true;
}


static bool ecl_grid_sublayer_contanins_xy__(const ecl_grid_type * grid , double x , double y , int k , int i1 , int i2 , int j1 , int j2, geo_polygon_type * polygon) {
  int i,j;

  geo_polygon_reset( polygon );

  /* Bottom edge */
  for (i=i1; i < i2; i++) {
    double corner_pos[3];
    ecl_grid_get_corner_xyz( grid , i , j1 , k , &corner_pos[0] , &corner_pos[1] , &corner_pos[2]);
    geo_polygon_add_point( polygon , corner_pos[0] , corner_pos[1]);
  }

  /* Right edge */
  for (j=j1; j < j2; j++) {
    double corner_pos[3];
    ecl_grid_get_corner_xyz( grid , i2 , j , k , &corner_pos[0] , &corner_pos[1] , &corner_pos[2]);
    geo_polygon_add_point( polygon , corner_pos[0] , corner_pos[1]);
  }

  /* Top edge */
  for (i=i2; i > i1; i--) {
    double corner_pos[3];
    ecl_grid_get_corner_xyz( grid , i , j2 , k , &corner_pos[0] , &corner_pos[1] , &corner_pos[2]);
    geo_polygon_add_point( polygon , corner_pos[0] , corner_pos[1]);
  }

  /* Left edge */
  for (j=j2; j > j1; j--) {
    double corner_pos[3];
    ecl_grid_get_corner_xyz( grid , i1 , j , k , &corner_pos[0] , &corner_pos[1] , &corner_pos[2]);
    geo_polygon_add_point( polygon , corner_pos[0] , corner_pos[1]);
  }
  geo_polygon_close( polygon );
  return geo_polygon_contains_point__( polygon , x  , y , true );
}



bool ecl_grid_get_ij_from_xy( const ecl_grid_type * grid , double x , double y , int k , int* i, int* j) {
  geo_polygon_type * polygon = geo_polygon_alloc( NULL );
  int nx = ecl_grid_get_nx( grid );
  int ny = ecl_grid_get_ny( grid );
  bool inside = ecl_grid_sublayer_contanins_xy__(grid , x , y , k , 0 , nx , 0 , ny , polygon);
  if (inside) {
    int i1 = 0;
    int i2 = nx;
    int j1 = 0;
    int j2 = ny;

    while (true) {
      if ((i2 - i1) > 1) {
        int ic = (i1 + i2) / 2;
        if (ecl_grid_sublayer_contanins_xy__(grid , x , y , k , i1 , ic , j1 , j2 , polygon))
          i2 = ic;
        else {
          if (!ecl_grid_sublayer_contanins_xy__(grid , x , y , k , ic , i2 , j1 , j2 , polygon))
            util_abort("%s: point nowhere to be found ... \n",__func__);
          i1 = ic;
        }
      }

      if ((j2 - j1) > 1) {
        int jc = (j1 + j2) / 2;
        if (ecl_grid_sublayer_contanins_xy__(grid , x , y , k , i1 , i2 , j1 , jc , polygon))
          j2 = jc;
        else {
          if (!ecl_grid_sublayer_contanins_xy__(grid , x , y , k , i1 , i2 , jc , j2 , polygon))
            util_abort("%s: point nowhere to be found ... \n",__func__);
          j1 = jc;
        }
      }

      if ((i2 - i1) == 1 && (j2 - j1) == 1) {
        *i = i1;
        *j = j1;
        break;
      }
    }
  }

  geo_polygon_free( polygon );
  return inside;
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
  util_safe_free(grid->mapaxes);

  if (grid->values != NULL) {
    int i;
    for (i=0; i < grid->block_size; i++)
      double_vector_free( grid->values[i] );
    free( grid->values );
  }
  if (ECL_GRID_MAINGRID_LGR_NR == grid->lgr_nr) { /* This is the main grid. */
    vector_free( grid->LGR_list );
    int_vector_free( grid->lgr_index_map);
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
    lgr = ecl_grid_iget_lgr(grid, grid_nr - 1);

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

  The cell center is calculated as the plain average of the eight
  corner positions, it is quite simple to construct cells where this
  average position is on the outside of the cell - hence there is no
  guarantee that the (x,y,z) position returned from this function
  actually is on the inside of the cell.
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


void ecl_grid_get_cell_corner_xyz1(const ecl_grid_type * grid , int global_index , int corner_nr , double * xpos , double * ypos , double * zpos ) {
  if ((corner_nr >= 0) &&  (corner_nr <= 7)) {
    const ecl_cell_type * cell  = ecl_grid_get_cell( grid , global_index );
    const point_type      point = cell->corner_list[ corner_nr ];
    *xpos = point.x;
    *ypos = point.y;
    *zpos = point.z;
  }
}


void ecl_grid_export_cell_corners1(const ecl_grid_type * grid, int global_index, double *x, double *y, double *z) {
  const ecl_cell_type * cell = ecl_grid_get_cell(grid, global_index);
  for (int i=0; i<8; i++) {
    const point_type point = cell->corner_list[i];
    x[i] = point.x;
    y[i] = point.y;
    z[i] = point.z;
  }
}


void ecl_grid_get_cell_corner_xyz3(const ecl_grid_type * grid , int i , int j , int k, int corner_nr , double * xpos , double * ypos , double * zpos ) {
  const int global_index = ecl_grid_get_global_index__(grid , i , j , k );
  ecl_grid_get_cell_corner_xyz1( grid , global_index , corner_nr , xpos , ypos , zpos);
}


void ecl_grid_get_corner_xyz(const ecl_grid_type * grid , int i , int j , int k, double * xpos , double * ypos , double * zpos ) {
  if (i < 0 || i > grid->nx)
    util_abort("%s: invalid i value:%d  Valid range: [0,%d] \n",__func__ , i,grid->nx);

  if (j < 0 || j > grid->ny)
    util_abort("%s: invalid j value:%d  Valid range: [0,%d] \n",__func__ , j,grid->ny);

  if (k < 0 || k > grid->nz)
    util_abort("%s: invalid k value:%d  Valid range: [0,%d] \n",__func__ , k,grid->nz);

  {
    int corner_nr = 0;
    if (i == grid->nx) {
      i -= 1;
      corner_nr += 1;
    }

    if (j == grid->ny) {
      j -= 1;
      corner_nr += 2;
    }

    if (k == grid->nz) {
      k -= 1;
      corner_nr += 4;
    }

    ecl_grid_get_cell_corner_xyz3( grid , i , j , k , corner_nr , xpos , ypos , zpos);
  }
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


double ecl_grid_get_cdepth1A(const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
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



double ecl_grid_get_cell_dz1( const ecl_grid_type * grid , int global_index ) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  double dz = 0;
  int ij;

  for (ij = 0; ij < 4; ij++)
    dz += (cell->corner_list[ij + 4].z - cell->corner_list[ij].z);

  return dz * 0.25;
}


double ecl_grid_get_cell_dz3( const ecl_grid_type * grid , int i , int j , int k) {
  const int global_index = ecl_grid_get_global_index3(grid , i,j,k);
  return ecl_grid_get_cell_dz1( grid , global_index );
}


double ecl_grid_get_cell_dz1A( const ecl_grid_type * grid , int active_index ) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
  return ecl_grid_get_cell_dz1( grid , global_index );
}


double ecl_grid_get_cell_thickness1( const ecl_grid_type * grid , int global_index ) {
  return ecl_grid_get_cell_dz1( grid, global_index );
}


double ecl_grid_get_cell_thickness3( const ecl_grid_type * grid , int i , int j , int k) {
  return ecl_grid_get_cell_dz3( grid, i,j,k);
}


double ecl_grid_get_cell_thickness1A( const ecl_grid_type * grid , int active_index ) {
  return ecl_grid_get_cell_dz1A( grid, active_index);
}



double ecl_grid_get_cell_dx1( const ecl_grid_type * grid , int global_index ) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  double dx = 0;
  double dy = 0;
  int c;

  for (c = 1; c < 8; c += 2) {
    dx += cell->corner_list[c].x - cell->corner_list[c - 1].x;
    dy += cell->corner_list[c].y - cell->corner_list[c - 1].y;
  }
  dx *= 0.25;
  dy *= 0.25;

  return sqrt( dx * dx + dy * dy );
}


double ecl_grid_get_cell_dx3( const ecl_grid_type * grid , int i , int j , int k) {
  const int global_index = ecl_grid_get_global_index3(grid , i,j,k);
  return ecl_grid_get_cell_dx1( grid , global_index );
}


double ecl_grid_get_cell_dx1A( const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
  return ecl_grid_get_cell_dx1( grid , global_index );
}


/*
  The current algorithm for calculating the cell dimensions DX,DY and
  DZ reproduces the Eclipse results from the INIT file, but we are in
  general *not* guaranteed to satisfy the relationship:

     DX * DY * DZ = V

*/

double ecl_grid_get_cell_dy1( const ecl_grid_type * grid , int global_index ) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  double dx = 0;
  double dy = 0;

  for (int k = 0; k < 2; k++) {
    for (int i = 0; i < 2; i++) {
      int c1 = i + k*4;
      int c2 = c1 + 2;
      dx += cell->corner_list[c2].x - cell->corner_list[c1].x;
      dy += cell->corner_list[c2].y - cell->corner_list[c1].y;
    }
  }
  dx *= 0.25;
  dy *= 0.25;

  return sqrt( dx * dx + dy * dy );
}


double ecl_grid_get_cell_dy3( const ecl_grid_type * grid , int i , int j , int k) {
  const int global_index = ecl_grid_get_global_index3(grid , i,j,k);
  return ecl_grid_get_cell_dy1( grid , global_index );
}

double ecl_grid_get_cell_dy1A( const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
  return ecl_grid_get_cell_dy1( grid , global_index );
}


const nnc_info_type * ecl_grid_get_cell_nnc_info1( const ecl_grid_type * grid , int global_index) {
  const ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index);
  return cell->nnc_info;
}

const nnc_info_type * ecl_grid_get_cell_nnc_info3( const ecl_grid_type * grid , int i , int j , int k) {
  const int global_index = ecl_grid_get_global_index3(grid , i,j,k);
  return ecl_grid_get_cell_nnc_info1(grid, global_index);
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


bool ecl_grid_cell_valid1(const ecl_grid_type * ecl_grid , int global_index) {
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index);
  if (GET_CELL_FLAG(cell , CELL_FLAG_TAINTED))
    return false;
  else
    return (GET_CELL_FLAG(cell , CELL_FLAG_VALID));
}

bool ecl_grid_cell_valid3(const ecl_grid_type * ecl_grid , int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k);
  return ecl_grid_cell_valid1( ecl_grid , global_index );
}

double ecl_grid_cell_valid1A(const ecl_grid_type * grid , int active_index) {
  const int global_index = ecl_grid_get_global_index1A(grid , active_index);
  return ecl_grid_cell_valid1( grid , global_index );
}



/*****************************************************************/
/* Functions for LGR query/lookup/... */

static void __assert_main_grid(const ecl_grid_type * ecl_grid) {
  if (ecl_grid->lgr_nr != ECL_GRID_MAINGRID_LGR_NR)
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
  if(!__lgr_name)
    return false;

  __assert_main_grid( main_grid );
  {
    char * lgr_name          = util_alloc_strip_copy( __lgr_name );
    bool has_lgr             = hash_has_key( main_grid->LGR_hash , lgr_name );
    free(lgr_name);
    return has_lgr;
  }
}

bool ecl_grid_has_lgr_nr(const ecl_grid_type * main_grid, int lgr_nr) {
  __assert_main_grid( main_grid );
  {
    if (int_vector_size( main_grid->lgr_index_map ) > lgr_nr)
      return true;
    else
      return false;
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
  return vector_get_size( main_grid->LGR_list );
}

/**
   The lgr_index has zero offset
   ecl_grid_iget_lgr( ecl_grid , 0); will return the first lgr
   ecl_grid_iget_lgr( ecl_grid , 1); will return the second lgr
   The method will fail HARD if lgr_index is out of bounds.
*/

ecl_grid_type * ecl_grid_iget_lgr(const ecl_grid_type * main_grid, int lgr_index) {
  __assert_main_grid( main_grid );
  return vector_iget(  main_grid->LGR_list , lgr_index);
}

/*
   This function returns the lgr with the given lgr_nr. The lgr_nr is
   the fourth element in the GRIDHEAD for EGRID files.  The lgr nr is
   equal to the grid nr if the grid's are consecutive numbered and
   read from file in increasing lgr nr order.

   This method can only be used for EGRID files. For GRID files the
   lgr_nr is 0 for all grids.
*/

ecl_grid_type * ecl_grid_get_lgr_from_lgr_nr(const ecl_grid_type * main_grid, int lgr_nr) {
  __assert_main_grid( main_grid );
  {
    int lgr_index = int_vector_iget( main_grid->lgr_index_map , lgr_nr );
    return vector_iget(  main_grid->LGR_list , lgr_index);
  }
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

const char * ecl_grid_iget_lgr_name( const ecl_grid_type * ecl_grid , int lgr_index) {
  __assert_main_grid( ecl_grid );
  if (lgr_index < (vector_get_size( ecl_grid->LGR_list ))) {
    const ecl_grid_type * lgr = vector_iget( ecl_grid->LGR_list , lgr_index);
    return lgr->name;
  } else
    return NULL;
}


const char * ecl_grid_get_lgr_name( const ecl_grid_type * ecl_grid , int lgr_nr) {
  __assert_main_grid( ecl_grid );
  if (lgr_nr == 0)
    return ecl_grid->name;
  {
    int lgr_index = int_vector_iget( ecl_grid->lgr_index_map , lgr_nr );
    return ecl_grid_iget_lgr_name( ecl_grid , lgr_index );
  }
}


int ecl_grid_get_lgr_nr_from_name( const ecl_grid_type * grid , const char * name) {
  __assert_main_grid( grid );
  if (strcmp( name , grid->name) == 0)
    return 0;
  else {
    const ecl_grid_type * lgr = ecl_grid_get_lgr( grid , name );
    return lgr->lgr_nr;
  }
}


/*****************************************************************/

/**
   This function returns the lgr_nr field of the grid; for GRID files, this
   is just the occurence number in the grid file. Starting with 0 at the main
   grid, and then increasing consecutively through the lgr sections.
   For EGRID files, this is the LGR number (fourth element in the
   gridhead).
*/


int ecl_grid_get_lgr_nr( const ecl_grid_type * ecl_grid ) {
  return ecl_grid->lgr_nr;
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


bool ecl_grid_cell_regular1( const ecl_grid_type * ecl_grid, int global_index ) {
  double x,y,z;
  ecl_grid_get_xyz1( ecl_grid , global_index , &x,&y,&z);
  return ecl_grid_cell_contains_xyz1( ecl_grid , global_index , x , y , z );
}


bool ecl_grid_cell_regular3( const ecl_grid_type * ecl_grid, int i,int j,int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i,j,k);
  return ecl_grid_cell_regular1( ecl_grid , global_index );
}

/*
  The function ecl_grid_get_cell_twist() is an attempt to measure how
  twisted or deformed a cell is. For a 'normal' cell the corners
  [0..3] will z value <= the corners [4..7]. This function will count
  the number of times the z value from the [4..7] is lower than the
  corresponding z value from the [0..3] layer.

  The purpose of the function is to detect twisted cells before
  embarking on cell contains calculation. The current
  ecl_cell_contains_xyz( ) implementation will fail badly for twisted
  cells.

  If the function return 4 you probably have an inverted z-axis!
*/

int ecl_grid_get_cell_twist1( const ecl_grid_type * ecl_grid, int global_index ) {
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
  return ecl_cell_get_twist( cell );
}


int ecl_grid_get_cell_twist3(const ecl_grid_type * ecl_grid, int i, int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k);
  return ecl_grid_get_cell_twist1( ecl_grid , global_index );
}


double ecl_grid_get_cell_volume1( const ecl_grid_type * ecl_grid, int global_index ) {
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
  int i,j,k;
  ecl_grid_get_ijk1( ecl_grid , global_index, &i , &j , &k);
  return ecl_cell_get_volume( cell );
}


double ecl_grid_get_cell_volume1A( const ecl_grid_type * ecl_grid, int active_index ) {
  int global_index = ecl_grid_get_global_index1A( ecl_grid , active_index );
  return ecl_grid_get_cell_volume1( ecl_grid , global_index );
}



double ecl_grid_get_cell_volume1_tskille( const ecl_grid_type * ecl_grid, int global_index ) {
  ecl_cell_type * cell = ecl_grid_get_cell( ecl_grid , global_index );
  return ecl_cell_get_volume_tskille( cell );
}





double ecl_grid_get_cell_volume3( const ecl_grid_type * ecl_grid, int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_grid , i , j , k);
  return ecl_grid_get_cell_volume1( ecl_grid , global_index );
}


void ecl_grid_summarize(const ecl_grid_type * ecl_grid) {
  int             active_cells , nx,ny,nz;
  ecl_grid_get_dims(ecl_grid , &nx , &ny , &nz , &active_cells);
  printf("      Name ..................: %s  \n",ecl_grid->name);
  printf("      Grid nr ...............: %d  \n",ecl_grid->lgr_nr );
  printf("      Active cells ..........: %d \n",active_cells);
  printf("      Active fracture cells..: %d \n",ecl_grid_get_nactive_fracture( ecl_grid ));
  printf("      nx ....................: %d \n",nx);
  printf("      ny ....................: %d \n",ny);
  printf("      nz ....................: %d \n",nz);
  printf("      Volume ................: %d \n",nx*ny*nz);
  printf("      Origo X................: %10.2f \n",ecl_grid->origo[0]);
  printf("      Origo Y................: %10.2f \n",ecl_grid->origo[1]);

  if (ECL_GRID_MAINGRID_LGR_NR == ecl_grid->lgr_nr) {
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
  ecl_data_type data_type = ecl_kw_get_data_type( ecl_kw );
  if (ecl_type_is_numeric(data_type)) {
    int lookup_index   = ecl_grid_get_property_index__( ecl_grid , ecl_kw , i , j , k );

    if (lookup_index >= 0) {
      ecl_kw_iget( ecl_kw , lookup_index , value );
      return true;
    } else
      return false;

  } else {
    util_abort("%s: sorry - can not lookup ECLIPSE type:%s with %s.\n",__func__ , ecl_type_alloc_name( data_type ) , __func__);
    return false;
  }
}


double ecl_grid_get_double_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k) {
  ecl_data_type data_type = ecl_kw_get_data_type( ecl_kw );
  if (ecl_type_is_double(data_type)) {
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
  ecl_data_type data_type = ecl_kw_get_data_type( ecl_kw );
  if (ecl_type_is_int(data_type)) {
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
  ecl_data_type data_type = ecl_kw_get_data_type( ecl_kw );
  if (ecl_type_is_float(data_type)) {
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
  ecl_data_type data_type = ecl_kw_get_data_type( ecl_kw );
  if (ecl_type_is_numeric(data_type)) {
    int lookup_index   = ecl_grid_get_property_index__( ecl_grid , ecl_kw , i , j , k );

    if (lookup_index >= 0)
      return ecl_kw_iget_as_double( ecl_kw , lookup_index );
    else
      return -1;   /* Tried to lookup an inactive cell. */

  } else {
    util_abort("%s: sorry - can not lookup ECLIPSE type:%s with %s.\n",__func__ , ecl_type_alloc_name( data_type ) , __func__);
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
  ecl_data_type data_type = ecl_kw_get_data_type(ecl_kw);
  if (ecl_type_is_numeric(data_type)) {
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
    util_abort("%s: sorry - can not lookup ECLIPSE type:%s with %s.\n",__func__ , ecl_type_alloc_name( data_type ) , __func__);
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
    if (ecl_type_is_int(ecl_kw_get_data_type( region_kw ))) {
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
    ecl_type_enum ecl_type = ecl_type_get_type(ecl_kw_get_data_type( ecl_kw ));

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
  util_fwrite_int( grid->lgr_nr , stream );
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


static void ecl_grid_dump_ascii__(ecl_grid_type * grid , bool active_only , FILE * stream) {
  fprintf(stream , "Grid nr           : %d\n",grid->lgr_nr);
  fprintf(stream , "Grid name         : %s\n",grid->name);
  fprintf(stream , "nx                : %6d\n",grid->nx);
  fprintf(stream , "ny                : %6d\n",grid->ny);
  fprintf(stream , "nz                : %6d\n",grid->nz);
  fprintf(stream , "nactive           : %6d\n",grid->total_active);
  fprintf(stream , "nactive fracture  : %6d\n",grid->total_active_fracture);

  {
    int l;
    for (l=0; l < grid->size; l++) {
      ecl_cell_type * cell = ecl_grid_get_cell( grid , l );
      if (cell->active_index[MATRIX_INDEX] >= 0 || !active_only) {
        int i,j,k;
        ecl_grid_get_ijk1( grid , l , &i , &j , &k);
        ecl_cell_dump_ascii( cell , i,j,k , stream , NULL);
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
  ecl_grid_dump__(grid, stream );
  {
    int i;
    for (i = 0; i < vector_get_size( grid->LGR_list ); i++)
      ecl_grid_dump__( vector_iget_const( grid->LGR_list , i) , stream );
  }
}

void ecl_grid_dump_ascii(ecl_grid_type * grid , bool active_only , FILE * stream) {
  ecl_grid_dump_ascii__( grid , active_only , stream );
  {
    int i;
    for (i = 0; i < vector_get_size( grid->LGR_list ); i++)
      ecl_grid_dump_ascii__( vector_iget( grid->LGR_list , i) , active_only , stream );
  }
}


void ecl_grid_dump_ascii_cell1(ecl_grid_type * grid , int global_index , FILE * stream , const double * offset) {
  ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index );
  int i,j,k;
  ecl_grid_get_ijk1( grid , global_index , &i , &j , &k);
  ecl_cell_dump_ascii(cell , i,j,k, stream , offset);
}


void ecl_grid_dump_ascii_cell3(ecl_grid_type * grid , int i , int j , int k , FILE * stream , const double * offset) {
  int global_index  = ecl_grid_get_global_index3(grid , i,j,k);
  ecl_cell_type * cell = ecl_grid_get_cell( grid , global_index );
  ecl_cell_dump_ascii(cell , i,j,k, stream , offset);
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


bool ecl_grid_use_mapaxes( const ecl_grid_type * grid ) {
  return grid->use_mapaxes;
}

void ecl_grid_init_mapaxes_data_double( const ecl_grid_type * grid , double * mapaxes) {
  int i;
  for (i = 0; i < 6; i++)
    mapaxes[i] = grid->mapaxes[i];
}


static void ecl_grid_init_mapaxes_data_float( const ecl_grid_type * grid , float * mapaxes) {
  int i;
  for (i = 0; i < 6; i++)
    mapaxes[i] = grid->mapaxes[i];
}



static const float * ecl_grid_get_mapaxes( const ecl_grid_type * grid ) {
  return grid->mapaxes;
}

ecl_kw_type * ecl_grid_alloc_mapaxes_kw( const ecl_grid_type * grid ) {
  return ecl_kw_alloc_new( MAPAXES_KW , 6 , ECL_FLOAT , grid->mapaxes);
}

static ecl_kw_type * ecl_grid_alloc_mapunits_kw( ert_ecl_unit_enum output_unit ) {
  ecl_kw_type * mapunits_kw = ecl_kw_alloc( MAPUNITS_KW , 1 , ECL_CHAR);

  if (output_unit == ECL_FIELD_UNITS)
    ecl_kw_iset_string8( mapunits_kw , 0 , "FEET" );

  if (output_unit == ECL_METRIC_UNITS)
    ecl_kw_iset_string8( mapunits_kw , 0 , "METRES" );

  if (output_unit == ECL_LAB_UNITS)
    ecl_kw_iset_string8( mapunits_kw , 0 , "CM" );

  return mapunits_kw;
}

static ecl_kw_type * ecl_grid_alloc_gridunits_kw( ert_ecl_unit_enum output_unit ) {
  ecl_kw_type * gridunits_kw = ecl_kw_alloc( GRIDUNIT_KW , 2 , ECL_CHAR);

  if (output_unit == ECL_FIELD_UNITS)
    ecl_kw_iset_string8( gridunits_kw , 0 , "FEET" );

  if (output_unit == ECL_METRIC_UNITS)
    ecl_kw_iset_string8( gridunits_kw , 0 , "METRES" );

  if (output_unit == ECL_LAB_UNITS)
    ecl_kw_iset_string8( gridunits_kw , 0 , "CM" );

  ecl_kw_iset_string8( gridunits_kw , 1 , "" );
  return gridunits_kw;
}

/*****************************************************************/

static float ecl_grid_output_scaling( const ecl_grid_type * grid , ert_ecl_unit_enum output_unit) {
  if (grid->unit_system == output_unit)
      return 1.0;
  else {
    double scale_factor = 1;

    if (grid->unit_system == ECL_FIELD_UNITS)
      scale_factor = 1.0 / METER_TO_FEET_SCALE_FACTOR;

    if (grid->unit_system == ECL_LAB_UNITS)
      scale_factor = 1.0 / METER_TO_CM_SCALE_FACTOR;

    if (output_unit == ECL_FIELD_UNITS)
      scale_factor *= METER_TO_FEET_SCALE_FACTOR;

    if (output_unit == ECL_LAB_UNITS)
      scale_factor *= METER_TO_CM_SCALE_FACTOR;

    return scale_factor;
  }
}


static void ecl_grid_fwrite_mapaxes( const ecl_grid_type * grid , fortio_type * fortio) {
  ecl_kw_type * mapaxes_kw = ecl_grid_alloc_mapaxes_kw( grid );
  ecl_kw_fwrite( mapaxes_kw , fortio );
  ecl_kw_free( mapaxes_kw );
}

static void ecl_grid_fwrite_mapunits( fortio_type * fortio , ert_ecl_unit_enum output_unit) {
  ecl_kw_type * mapunits_kw = ecl_grid_alloc_mapunits_kw( output_unit );
  ecl_kw_fwrite( mapunits_kw , fortio );
  ecl_kw_free( mapunits_kw );
}


static void ecl_grid_fwrite_gridunits( fortio_type * fortio, ert_ecl_unit_enum output_unit)  {
  ecl_kw_type * gridunits_kw = ecl_grid_alloc_gridunits_kw( output_unit );
  ecl_kw_fwrite( gridunits_kw , fortio );
  ecl_kw_free( gridunits_kw );
}


static void ecl_grid_fwrite_main_GRID_headers( const ecl_grid_type * ecl_grid , fortio_type * fortio , ert_ecl_unit_enum output_unit) {
  ecl_grid_fwrite_mapunits( fortio , output_unit );

  if (ecl_grid->use_mapaxes)
    ecl_grid_fwrite_mapaxes( ecl_grid , fortio );

  ecl_grid_fwrite_gridunits( fortio , output_unit );
}


static void ecl_grid_fwrite_GRID__( const ecl_grid_type * grid , int coords_size , fortio_type * fortio, ert_ecl_unit_enum output_unit) {
  if (grid->parent_grid != NULL) {
    ecl_kw_type * lgr_kw = ecl_kw_alloc(LGR_KW , 1 , ECL_CHAR);
    ecl_kw_iset_string8( lgr_kw , 0 , grid->name );
    ecl_kw_fwrite( lgr_kw , fortio );
    ecl_kw_free( lgr_kw );
  }

  {
    ecl_kw_type * dimens_kw = ecl_kw_alloc(DIMENS_KW , 3 , ECL_INT);
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
    ecl_grid_fwrite_main_GRID_headers( grid , fortio , output_unit);

  {
    ecl_kw_type * radial_kw = ecl_kw_alloc( RADIAL_KW , 1 , ECL_CHAR);
    ecl_kw_iset_string8( radial_kw , 0 , "FALSE" );
    ecl_kw_fwrite( radial_kw , fortio );
    ecl_kw_free( radial_kw );
  }

  {
    ecl_kw_type * coords_kw  = ecl_kw_alloc( COORDS_KW  , coords_size , ECL_INT);
    ecl_kw_type * corners_kw = ecl_kw_alloc( CORNERS_KW , 24 , ECL_FLOAT);
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


void ecl_grid_fwrite_GRID2( const ecl_grid_type * grid , const char * filename, ert_ecl_unit_enum output_unit) {
  int coords_size = 5;
  bool fmt_file   = false;

  fortio_type * fortio = fortio_open_writer( filename , fmt_file , ECL_ENDIAN_FLIP );
  if (hash_get_size( grid->children ) > 0)
    coords_size = 7;

  if (grid->coarsening_active)
    coords_size = 7;

  ecl_grid_fwrite_GRID__( grid , coords_size , fortio , output_unit);

  {
    int grid_nr;
    for (grid_nr = 0; grid_nr < vector_get_size( grid->LGR_list ); grid_nr++) {
      const ecl_grid_type * igrid = vector_iget_const( grid->LGR_list , grid_nr );
      ecl_grid_fwrite_GRID__( igrid , coords_size , fortio , output_unit );
    }
  }
  fortio_fclose( fortio );
}

void ecl_grid_fwrite_GRID( const ecl_grid_type * grid , const char * filename) {
  ecl_grid_fwrite_GRID2( grid , filename , ECL_METRIC_UNITS );
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

static void ecl_grid_fwrite_main_EGRID_header( const ecl_grid_type * grid , fortio_type * fortio , ert_ecl_unit_enum output_unit) {
  int EGRID_VERSION  = 3;
  int RELEASE_YEAR   = 2007;
  int COMPAT_VERSION = 0;
  const float * mapaxes = ecl_grid_get_mapaxes( grid );

  {
    ecl_kw_type * filehead_kw = ecl_kw_alloc( FILEHEAD_KW , 100 , ECL_INT);
    ecl_kw_scalar_set_int( filehead_kw , 0 );

    ecl_kw_iset_int( filehead_kw , FILEHEAD_VERSION_INDEX   , EGRID_VERSION );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_YEAR_INDEX      , RELEASE_YEAR );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_COMPAT_INDEX    , COMPAT_VERSION );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_TYPE_INDEX      , FILEHEAD_GRIDTYPE_CORNERPOINT );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_DUALP_INDEX     , grid->dualp_flag );
    ecl_kw_iset_int( filehead_kw , FILEHEAD_ORGFORMAT_INDEX , FILEHEAD_ORGTYPE_CORNERPOINT );

    ecl_kw_fwrite( filehead_kw , fortio );
    ecl_kw_free( filehead_kw );
  }

  ecl_grid_fwrite_mapunits( fortio , output_unit );
  if (mapaxes != NULL)
    ecl_grid_fwrite_mapaxes( grid , fortio );

  ecl_grid_fwrite_gridunits( fortio , output_unit);
}


static void ecl_grid_fwrite_gridhead_kw( int nx, int ny , int nz, int grid_nr, fortio_type * fortio ) {
  ecl_kw_type * gridhead_kw = ecl_grid_alloc_gridhead_kw( nx , ny , nz , grid_nr);
  ecl_kw_fwrite( gridhead_kw , fortio );
  ecl_kw_free( gridhead_kw );
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



static bool ecl_grid_init_coord_section__( const ecl_grid_type * grid , int i, int j , int i_corner, int j_corner , bool force_set , float * coord_float , double * coord_double ) {

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
    int corner_index = j_corner*2 + i_corner;
    int coord_offset = 6 * ( (j + j_corner) * (grid->nx + 1) + (i + i_corner) );
    {
      point_copy_values( &top_point    , &top_cell->corner_list[corner_index]);
      point_copy_values( &bottom_point , &bottom_cell->corner_list[ corner_index + 4]);


      if ((top_point.z == bottom_point.z) && (force_set == false)) {
        return false;
      } else {
        if (grid->use_mapaxes) {
          point_mapaxes_invtransform( &top_point    , grid->origo , grid->unit_x , grid->unit_y );
          point_mapaxes_invtransform( &bottom_point , grid->origo , grid->unit_x , grid->unit_y );
        }

        if (coord_float) {
          coord_float[coord_offset]     = top_point.x;

          coord_float[coord_offset + 1] = top_point.y;
          coord_float[coord_offset + 2] = top_point.z;
          coord_float[coord_offset + 3] = bottom_point.x;
          coord_float[coord_offset + 4] = bottom_point.y;
          coord_float[coord_offset + 5] = bottom_point.z;
        }


        if (coord_double) {
          coord_double[coord_offset]     = top_point.x;
          coord_double[coord_offset + 1] = top_point.y;
          coord_double[coord_offset + 2] = top_point.z;

          coord_double[coord_offset + 3] = bottom_point.x;
          coord_double[coord_offset + 4] = bottom_point.y;
          coord_double[coord_offset + 5] = bottom_point.z;
        }


        return true;
      }
    }
  }
}


static void ecl_grid_init_coord_section( const ecl_grid_type * grid , int i, int j , float * coord_float , double * coord_double ) {
  int i_corner = 0;
  int j_corner = 0;

  if (i == grid->nx) {
    i -= 1;
    i_corner = 1;
  }

  if (j == grid->ny) {
    j -= 1;
    j_corner = 1;
  }

  ecl_grid_init_coord_section__( grid , i,j, i_corner,j_corner, /*force_set=*/true, coord_float , coord_double);
}




void ecl_grid_init_coord_data( const ecl_grid_type * grid , float * coord ) {
  /*
    The coord vector contains the points defining the top and bottom
    of the pillars. The vector contains (nx + 1) * (ny + 1) 6 element
    chunks of data, where each chunk contains the coordinates (x,y,z)
    of the top and the bottom of the pillar.
  */
  int i,j;
  for (j=0; j <= grid->ny; j++) {
    for (i=0; i <= grid->nx; i++)
      ecl_grid_init_coord_section( grid , i , j , coord , NULL);

  }
}


void ecl_grid_init_coord_data_double( const ecl_grid_type * grid , double * coord ) {
  /*
    The coord vector contains the points defining the top and bottom
    of the pillars. The vector contains (nx + 1) * (ny + 1) 6 element
    chunks of data, where each chunk contains the coordinates (x,y,z)
    f the top and the bottom of the pillar.
  */
  int i,j;
  for (j=0; j <= grid->ny; j++) {
    for (i=0; i <= grid->nx; i++)
      ecl_grid_init_coord_section( grid , i , j , NULL , coord);

  }
}



float * ecl_grid_alloc_coord_data( const ecl_grid_type * grid ) {
  float * coord = util_calloc( ecl_grid_get_coord_size(grid) , sizeof * coord );
  ecl_grid_init_coord_data( grid , coord );
  return coord;
}

void ecl_grid_assert_coord_kw( ecl_grid_type * grid ) {
  if (grid->coord_kw == NULL) {
    grid->coord_kw = ecl_kw_alloc( COORD_KW , ecl_grid_get_coord_size( grid ) , ECL_FLOAT);
    ecl_grid_init_coord_data( grid , ecl_kw_get_void_ptr( grid->coord_kw ));
  }
}





/*****************************************************************/

static void ecl_grid_init_zcorn_data__( const ecl_grid_type * grid , float * zcorn_float , double * zcorn_double ) {
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

          int z1 = k*8*nx*ny + j*4*nx + 2*i            + l*4*nx*ny;
          int z2 = k*8*nx*ny + j*4*nx + 2*i  +  1      + l*4*nx*ny;
          int z3 = k*8*nx*ny + j*4*nx + 2*nx + 2*i     + l*4*nx*ny;
          int z4 = k*8*nx*ny + j*4*nx + 2*nx + 2*i + 1 + l*4*nx*ny;

          if (zcorn_float) {
            zcorn_float[z1] = p0.z;
            zcorn_float[z2] = p1.z;
            zcorn_float[z3] = p2.z;
            zcorn_float[z4] = p3.z;
          }

          if (zcorn_double) {
            zcorn_double[z1] = p0.z;
            zcorn_double[z2] = p1.z;
            zcorn_double[z3] = p2.z;
            zcorn_double[z4] = p3.z;
          }
        }
      }
    }
  }
}

void ecl_grid_init_zcorn_data( const ecl_grid_type * grid , float * zcorn ) {
  ecl_grid_init_zcorn_data__( grid , zcorn , NULL );
}

void ecl_grid_init_zcorn_data_double( const ecl_grid_type * grid , double * zcorn ) {
  ecl_grid_init_zcorn_data__( grid , NULL , zcorn );
}


float * ecl_grid_alloc_zcorn_data( const ecl_grid_type * grid ) {
  float * zcorn = util_calloc( 8 * grid->size , sizeof * zcorn );
  ecl_grid_init_zcorn_data( grid , zcorn );
  return zcorn;
}



ecl_kw_type * ecl_grid_alloc_zcorn_kw( const ecl_grid_type * grid ) {
  ecl_kw_type * zcorn_kw = ecl_kw_alloc( ZCORN_KW , ecl_grid_get_zcorn_size(grid), ECL_FLOAT);
  ecl_grid_init_zcorn_data(grid , ecl_kw_get_void_ptr(zcorn_kw));
  return zcorn_kw;
}

ecl_kw_type * ecl_grid_alloc_coord_kw( const ecl_grid_type * grid) {
  if(grid->coord_kw)
    return ecl_kw_alloc_copy(grid->coord_kw);

  ecl_kw_type * coord_kw = ecl_kw_alloc(
          COORD_KW,
          ECL_GRID_COORD_SIZE(grid->nx, grid->ny),
          ECL_FLOAT
          );
  ecl_grid_init_coord_data(grid, ecl_kw_get_float_ptr(coord_kw));

  return coord_kw;
}


int ecl_grid_get_coord_size( const ecl_grid_type * grid) {
  return ECL_GRID_COORD_SIZE( grid->nx , grid->ny );
}


int ecl_grid_get_zcorn_size( const ecl_grid_type * grid ) {
  return ECL_GRID_ZCORN_SIZE( grid->nx , grid->ny, grid->nz );
}

/*****************************************************************/

void ecl_grid_init_actnum_data( const ecl_grid_type * grid , int * actnum ) {
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
          actnum[index_ptr[j]] = CELL_NOT_ACTIVE;
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
  ecl_kw_type * actnum_kw = ecl_kw_alloc( ACTNUM_KW , grid->size  , ECL_INT);
  ecl_grid_init_actnum_data( grid , ecl_kw_get_void_ptr( actnum_kw ));
  return actnum_kw;
}


void ecl_grid_compressed_kw_copy( const ecl_grid_type * grid , ecl_kw_type * target_kw , const ecl_kw_type * src_kw) {
  if ((ecl_kw_get_size( target_kw ) == ecl_grid_get_nactive(grid)) && (ecl_kw_get_size( src_kw ) == ecl_grid_get_global_size(grid))) {
    int active_index = 0;
    int global_index;
    for (global_index = 0; global_index < ecl_grid_get_global_size( grid ); global_index++) {
      if (ecl_grid_cell_active1(grid, global_index)) {
        ecl_kw_iset( target_kw , active_index , ecl_kw_iget_ptr(src_kw , global_index));
        active_index++;
      }
    }
  } else
    util_abort("%s: size mismatch target:%d  src:%d  expected %d,%d \n",ecl_kw_get_size( target_kw ), ecl_kw_get_size( src_kw ) , ecl_grid_get_nactive(grid) , ecl_grid_get_global_size(grid));
}


void ecl_grid_global_kw_copy( const ecl_grid_type * grid , ecl_kw_type * target_kw , const ecl_kw_type * src_kw) {
  if ((ecl_kw_get_size( src_kw ) == ecl_grid_get_nactive(grid)) && (ecl_kw_get_size( target_kw ) == ecl_grid_get_global_size(grid))) {
    int active_index = 0;
    int global_index;
    for (global_index = 0; global_index < ecl_grid_get_global_size( grid ); global_index++) {
      if (ecl_grid_cell_active1(grid, global_index)) {
        ecl_kw_iset( target_kw , global_index , ecl_kw_iget_ptr(src_kw , active_index));
        active_index++;
      }
    }
  } else
    util_abort("%s: size mismatch target:%d  src:%d  expected %d,%d \n",ecl_kw_get_size( target_kw ), ecl_kw_get_size( src_kw ) , ecl_grid_get_global_size(grid), ecl_grid_get_nactive(grid));
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
  ecl_kw_type * hostnum_kw = ecl_kw_alloc( HOSTNUM_KW , grid->size  , ECL_INT);
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
  ecl_kw_type * corsnum_kw = ecl_kw_alloc( CORSNUM_KW , grid->size  , ECL_INT);
  ecl_grid_init_corsnum_data( grid , ecl_kw_get_void_ptr( corsnum_kw ));
  return corsnum_kw;
}

/*****************************************************************/


ecl_kw_type * ecl_grid_alloc_gridhead_kw( int nx, int ny , int nz , int grid_nr) {
  ecl_kw_type * gridhead_kw = ecl_kw_alloc( GRIDHEAD_KW , GRIDHEAD_SIZE , ECL_INT);
  ecl_kw_scalar_set_int( gridhead_kw , 0 );
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_TYPE_INDEX , GRIDHEAD_GRIDTYPE_CORNERPOINT );
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_NX_INDEX , nx);
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_NY_INDEX , ny);
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_NZ_INDEX , nz);
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_NUMRES_INDEX , 1);
  ecl_kw_iset_int( gridhead_kw , GRIDHEAD_LGR_INDEX , grid_nr );
  return gridhead_kw;
}


/*****************************************************************/


void ecl_grid_reset_actnum( ecl_grid_type * grid , const int * actnum ) {
  const int global_size = ecl_grid_get_global_size( grid );
  int g;
  for (g=0; g < global_size; g++) {
    ecl_cell_type * cell = ecl_grid_get_cell( grid , g );
    if (actnum)
      cell->active = actnum[g];
    else
      cell->active = 1;
  }
  ecl_grid_update_index( grid );
}


static void  ecl_grid_fwrite_self_nnc( const ecl_grid_type * grid , fortio_type * fortio ) {
  const int default_index = 1;
  int_vector_type * g1 = int_vector_alloc(0 , default_index );
  int_vector_type * g2 = int_vector_alloc(0 , default_index );
  int g;

  for (g=0; g < ecl_grid_get_global_size(grid); g++) {
    ecl_cell_type * cell = ecl_grid_get_cell( grid , g );
    const nnc_info_type * nnc_info = cell->nnc_info;
    if (nnc_info) {
      const nnc_vector_type * nnc_vector = nnc_info_get_self_vector(nnc_info);
      int i;
      for (i = 0; i < nnc_vector_get_size( nnc_vector ); i++) {
        int nnc_index = nnc_vector_iget_nnc_index( nnc_vector , i );
        int_vector_iset( g1 , nnc_index , 1 + g );
        int_vector_iset( g2 , nnc_index , 1 + nnc_vector_iget_grid_index( nnc_vector , i ));
      }
    }
  }
  {
    int num_nnc = int_vector_size( g1 );
    ecl_kw_type * nnc1_kw = ecl_kw_alloc_new_shared( NNC1_KW , num_nnc , ECL_INT , int_vector_get_ptr( g1 ));
    ecl_kw_type * nnc2_kw = ecl_kw_alloc_new_shared( NNC2_KW , num_nnc , ECL_INT , int_vector_get_ptr( g2 ));
    ecl_kw_type * nnchead_kw = ecl_kw_alloc( NNCHEAD_KW , NNCHEAD_SIZE , ECL_INT);

    ecl_kw_scalar_set_int( nnchead_kw , 0 );
    ecl_kw_iset_int( nnchead_kw , NNCHEAD_NUMNNC_INDEX , num_nnc );
    ecl_kw_iset_int( nnchead_kw , NNCHEAD_LGR_INDEX , grid->lgr_nr );

    ecl_kw_fwrite( nnchead_kw , fortio);
    ecl_kw_fwrite( nnc1_kw , fortio);
    ecl_kw_fwrite( nnc2_kw , fortio);

    ecl_kw_free( nnchead_kw );
    ecl_kw_free( nnc2_kw );
    ecl_kw_free( nnc1_kw );
  }

  int_vector_free( g1 );
  int_vector_free( g2 );
}


static void ecl_grid_fwrite_EGRID__( ecl_grid_type * grid , fortio_type * fortio, ert_ecl_unit_enum output_unit) {
  bool is_lgr = true;
  if (grid->parent_grid == NULL)
    is_lgr = false;

  /* Writing header */
  if (!is_lgr) {
    ecl_grid_fwrite_main_EGRID_header( grid , fortio , output_unit );
  } else {
    {
      ecl_kw_type * lgr_kw = ecl_kw_alloc(LGR_KW , 1 , ECL_CHAR);
      ecl_kw_iset_string8( lgr_kw , 0 , grid->name );
      ecl_kw_fwrite( lgr_kw , fortio );
      ecl_kw_free( lgr_kw );
    }

    {
      ecl_kw_type * lgr_parent_kw = ecl_kw_alloc(LGR_PARENT_KW , 1 , ECL_CHAR);
      if (grid->parent_name != NULL)
        ecl_kw_iset_string8( lgr_parent_kw , 0 , grid->parent_name );
      else
        ecl_kw_iset_string8( lgr_parent_kw , 0 , "");

      ecl_kw_fwrite( lgr_parent_kw , fortio );
      ecl_kw_free( lgr_parent_kw );
    }
  }

  ecl_grid_fwrite_gridhead_kw( grid->nx , grid->ny , grid->nz , grid->lgr_nr , fortio);
  /* Writing main grid data */
  {
    ecl_grid_assert_coord_kw( grid );
    {
      ecl_kw_type * coord_kw = ecl_kw_alloc_copy(grid->coord_kw);
      ecl_kw_type * zcorn_kw = ecl_grid_alloc_zcorn_kw( grid );

      if (output_unit != grid->unit_system) {
        double scale_factor = ecl_grid_output_scaling( grid , output_unit );
        ecl_kw_scale_float(coord_kw, scale_factor);
        ecl_kw_scale_float(zcorn_kw, scale_factor);
      }
      ecl_kw_fwrite(coord_kw, fortio);
      ecl_kw_fwrite(zcorn_kw, fortio);
      ecl_kw_free( zcorn_kw );
      ecl_kw_free(coord_kw);
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
      ecl_kw_type * endgrid_kw = ecl_kw_alloc( ENDGRID_KW , 0 , ECL_INT);
      ecl_kw_fwrite( endgrid_kw , fortio );
      ecl_kw_free( endgrid_kw );
    }
  }

  if (is_lgr) {
    ecl_kw_type * endlgr_kw = ecl_kw_alloc( ENDLGR_KW , 0 , ECL_INT);
    ecl_kw_fwrite( endlgr_kw , fortio );
    ecl_kw_free( endlgr_kw );
  }
  ecl_grid_fwrite_self_nnc( grid , fortio );
}




void ecl_grid_fwrite_EGRID2( ecl_grid_type * grid , const char * filename, ert_ecl_unit_enum output_unit) {
  bool fmt_file        = false;
  fortio_type * fortio = fortio_open_writer( filename , fmt_file , ECL_ENDIAN_FLIP );

  ecl_grid_fwrite_EGRID__( grid , fortio, output_unit);
  {
    int grid_nr;
    for (grid_nr = 0; grid_nr < vector_get_size( grid->LGR_list ); grid_nr++) {
      ecl_grid_type * igrid = vector_iget( grid->LGR_list , grid_nr );
      ecl_grid_fwrite_EGRID__( igrid , fortio, output_unit );
    }
  }
  fortio_fclose( fortio );
}


/*
   The construction with ecl_grid_fwrite_EGRID() and
   ecl_grid_fwrite_EGRID2() is an attempt to create API stability. New
   code should use the ecl_grid_fwrite_EGRID2() function.
*/

void ecl_grid_fwrite_EGRID( ecl_grid_type * grid , const char * filename, bool output_metric) {
  ert_ecl_unit_enum output_unit = ECL_METRIC_UNITS;

  if (!output_metric)
    output_unit = ECL_FIELD_UNITS;

  ecl_grid_fwrite_EGRID2( grid , filename , output_unit );
}



void ecl_grid_fwrite_depth( const ecl_grid_type * grid , fortio_type * init_file , ert_ecl_unit_enum output_unit) {
  ecl_kw_type * depth_kw = ecl_kw_alloc("DEPTH" , ecl_grid_get_nactive(grid) , ECL_FLOAT);
  {
    float * depth_ptr = ecl_kw_get_ptr(depth_kw);
    for (int i = 0; i < ecl_grid_get_nactive( grid ); i++)
      depth_ptr[i] = ecl_grid_get_cdepth1A( grid , i );
  }
  ecl_kw_scale_float( depth_kw , ecl_grid_output_scaling( grid , output_unit ));
  ecl_kw_fwrite( depth_kw , init_file );
  ecl_kw_free( depth_kw );
}


void ecl_grid_fwrite_dims( const ecl_grid_type * grid , fortio_type * init_file,  ert_ecl_unit_enum output_unit) {
  ecl_kw_type * dx = ecl_kw_alloc("DX" , ecl_grid_get_nactive(grid) , ECL_FLOAT);
  ecl_kw_type * dy = ecl_kw_alloc("DY" , ecl_grid_get_nactive(grid) , ECL_FLOAT);
  ecl_kw_type * dz = ecl_kw_alloc("DZ" , ecl_grid_get_nactive(grid) , ECL_FLOAT);
  {
    {
      float * dx_ptr = ecl_kw_get_ptr(dx);
      float * dy_ptr = ecl_kw_get_ptr(dy);
      float * dz_ptr = ecl_kw_get_ptr(dz);

      for (int i = 0; i < ecl_grid_get_nactive( grid ); i++) {
        dx_ptr[i] = ecl_grid_get_cell_dx1A( grid , i );
        dy_ptr[i] = ecl_grid_get_cell_dy1A( grid , i );
        dz_ptr[i] = ecl_grid_get_cell_dz1A( grid , i );
      }
    }

    {
      float scale_factor = ecl_grid_output_scaling( grid , output_unit );
      ecl_kw_scale_float( dx , scale_factor );
      ecl_kw_scale_float( dy , scale_factor );
      ecl_kw_scale_float( dz , scale_factor );
    }
  }
  ecl_kw_fwrite( dx , init_file );
  ecl_kw_fwrite( dy , init_file );
  ecl_kw_fwrite( dz , init_file );
  ecl_kw_free( dx );
  ecl_kw_free( dy );
  ecl_kw_free( dz );
}


/**
   Writes the current grid as grdecl keywords suitable to be read by
   ECLIPSE. This function will only write the main grid and not
   possible LGRs which are attached.
*/

void ecl_grid_fprintf_grdecl2(ecl_grid_type * grid , FILE * stream , ert_ecl_unit_enum output_unit) {
  {
    ecl_kw_type * mapunits_kw = ecl_grid_alloc_mapunits_kw( output_unit );
    ecl_kw_fprintf_grdecl( mapunits_kw , stream );
    ecl_kw_free( mapunits_kw );
    fprintf(stream , "\n");
  }

  if (grid->use_mapaxes) {
    ecl_kw_type * mapaxes_kw = ecl_grid_alloc_mapaxes_kw( grid );
    ecl_kw_fprintf_grdecl( mapaxes_kw , stream );
    ecl_kw_free( mapaxes_kw );
  }

  {
    ecl_kw_type * gridunits_kw = ecl_grid_alloc_gridunits_kw( output_unit  );
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


void ecl_grid_fprintf_grdecl(ecl_grid_type * grid , FILE * stream ) {
  ecl_grid_fprintf_grdecl2( grid , stream , ECL_METRIC_UNITS);
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

static int ecl_grid_get_num_nnc__( const ecl_grid_type * grid ) {
  int g;
  int num_nnc = 0;
  for (g = 0; g < grid->size; g++) {
    const nnc_info_type * nnc_info = ecl_grid_get_cell_nnc_info1( grid , g );
    if (nnc_info)
      num_nnc += nnc_info_get_total_size( nnc_info );
  }
  return num_nnc;
}



int ecl_grid_get_num_nnc( const ecl_grid_type * grid ) {
  int num_nnc = ecl_grid_get_num_nnc__( grid );
  {
    int grid_nr;
    for (grid_nr = 0; grid_nr < vector_get_size( grid->LGR_list ); grid_nr++) {
      ecl_grid_type * igrid = vector_iget( grid->LGR_list , grid_nr );
      num_nnc += ecl_grid_get_num_nnc__( igrid );
    }
  }
  return num_nnc;
}


static ecl_kw_type * ecl_grid_alloc_volume_kw_active( const ecl_grid_type * grid) {
  ecl_kw_type * volume_kw = ecl_kw_alloc("VOLUME" , ecl_grid_get_active_size(grid) , ECL_DOUBLE);
  {
    double * volume_data = ecl_kw_get_ptr( volume_kw );
    int active_index;
    for (active_index = 0; active_index < ecl_grid_get_active_size(grid); active_index++) {
      double cell_volume = ecl_grid_get_cell_volume1A(grid , active_index);
      volume_data[ active_index] = cell_volume;
    }
  }
  return volume_kw;
}


static ecl_kw_type * ecl_grid_alloc_volume_kw_global( const ecl_grid_type * grid) {
  ecl_kw_type * volume_kw = ecl_kw_alloc("VOLUME" , ecl_grid_get_global_size(grid) , ECL_DOUBLE);
  {
    double * volume_data = ecl_kw_get_ptr( volume_kw );
    int global_index;
    for (global_index = 0; global_index < ecl_grid_get_global_size(grid); global_index++) {
      double cell_volume = ecl_grid_get_cell_volume1(grid , global_index);
      volume_data[ global_index] = cell_volume;
    }
  }
  return volume_kw;
}



ecl_kw_type * ecl_grid_alloc_volume_kw( const ecl_grid_type * grid , bool active_size) {
  if (active_size)
    return ecl_grid_alloc_volume_kw_active( grid );
  else
    return ecl_grid_alloc_volume_kw_global( grid );
}
//
