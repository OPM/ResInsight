/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_grid.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ECL_GRID_H__
#define __ECL_GRID_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/double_vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_coarse_cell.h>
#include <ert/ecl/ecl_kw.h>  
#include <ert/ecl/grid_dims.h>


  typedef double (block_function_ftype) ( const double_vector_type *); 
  typedef struct ecl_grid_struct ecl_grid_type;

  bool                         ecl_grid_have_coarse_cells( const ecl_grid_type * main_grid );
  bool                         ecl_grid_cell_in_coarse_group1( const ecl_grid_type * main_grid , int global_index );   
  bool                         ecl_grid_cell_in_coarse_group3( const ecl_grid_type * main_grid , int i , int j , int k);   
  int                          ecl_grid_get_num_coarse_groups( const ecl_grid_type * main_grid );
  ecl_coarse_cell_type       * ecl_grid_iget_coarse_group( const ecl_grid_type * ecl_grid , int coarse_nr );
  ecl_coarse_cell_type       * ecl_grid_get_cell_coarse_group1( const ecl_grid_type * ecl_grid , int global_index);
  ecl_coarse_cell_type       * ecl_grid_get_cell_coarse_group3( const ecl_grid_type * ecl_grid , int i , int j , int k);
  

  void            ecl_grid_get_column_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j, double_vector_type * column);
  int             ecl_grid_get_global_index_from_xy_top( const ecl_grid_type * ecl_grid , double x , double y);
  int             ecl_grid_get_global_index_from_xy_bottom( const ecl_grid_type * ecl_grid , double x , double y);
  
  void            ecl_grid_get_corner_xyz3(const ecl_grid_type * grid , int i , int j , int k, int corner_nr , double * xpos , double * ypos , double * zpos );
  void            ecl_grid_get_corner_xyz1(const ecl_grid_type * grid , int global_index , int corner_nr , double * xpos , double * ypos , double * zpos );
  
  double          ecl_grid_get_cell_thickness3( const ecl_grid_type * grid , int i , int j , int k);
  double          ecl_grid_get_cell_thickness1( const ecl_grid_type * grid , int global_index );
  double          ecl_grid_get_cdepth1(const ecl_grid_type * grid , int global_index);
  double          ecl_grid_get_cdepth3(const ecl_grid_type * grid , int i, int j , int k);
  double          ecl_grid_get_depth3(const ecl_grid_type * grid , int i, int j , int k);
  int             ecl_grid_get_global_index_from_xy( const ecl_grid_type * ecl_grid , int k , bool lower_layer , double x , double y);
  bool            ecl_grid_cell_contains_xyz1( const ecl_grid_type * ecl_grid , int global_index , double x , double y , double z);
  bool            ecl_grid_cell_contains_xyz3( const ecl_grid_type * ecl_grid , int i , int j , int k, double x , double y , double z );
  double          ecl_grid_get_cell_volume1( const ecl_grid_type * ecl_grid, int global_index );
  double          ecl_grid_get_cell_volume3( const ecl_grid_type * ecl_grid, int i , int j , int k);
  bool            ecl_grid_cell_contains1(const ecl_grid_type * grid , int global_index , double x , double y , double z);
  bool            ecl_grid_cell_contains3(const ecl_grid_type * grid , int i , int j ,int k , double x , double y , double z);
  int             ecl_grid_get_global_index_from_xyz(ecl_grid_type * grid , double x , double y , double z , int start_index);
  const  char   * ecl_grid_get_name( const ecl_grid_type * );
  int             ecl_grid_get_active_index3(const ecl_grid_type * ecl_grid , int i , int j , int k);
  int             ecl_grid_get_active_index1(const ecl_grid_type * ecl_grid , int global_index);
  int             ecl_grid_get_active_fracture_index3(const ecl_grid_type * ecl_grid , int i , int j , int k);
  int             ecl_grid_get_active_fracture_index1(const ecl_grid_type * ecl_grid , int global_index);
  bool            ecl_grid_cell_active3(const ecl_grid_type * , int  , int  , int );
  bool            ecl_grid_cell_active1(const ecl_grid_type * , int);
  bool            ecl_grid_ijk_valid(const ecl_grid_type * , int  , int , int ); 
  int             ecl_grid_get_global_index3(const ecl_grid_type * , int  , int , int );
  int             ecl_grid_get_global_index1A(const ecl_grid_type * ecl_grid , int active_index);
  int             ecl_grid_get_global_index1F(const ecl_grid_type * ecl_grid , int active_fracture_index);
  
  ecl_grid_type * ecl_grid_alloc_GRDECL_kw( int nx, int ny , int nz , const ecl_kw_type * zcorn_kw , const ecl_kw_type * coord_kw , const ecl_kw_type * actnum_kw , const ecl_kw_type * mapaxes_kw );
  ecl_grid_type * ecl_grid_alloc_GRDECL_data(int , int , int , const float *  , const float *  , const int * , const float * mapaxes);
  ecl_grid_type * ecl_grid_alloc_GRID_data(int num_coords , int nx, int ny , int nz , int coords_size , int ** coords , float ** corners , const float * mapaxes);
  ecl_grid_type * ecl_grid_alloc(const char * );
  ecl_grid_type * ecl_grid_load_case( const char * case_input );
  ecl_grid_type * ecl_grid_alloc_rectangular( int nx , int ny , int nz , double dx , double dy , double dz , const int * actnum);
  ecl_grid_type * ecl_grid_alloc_regular( int nx, int ny , int nz , const double * ivec, const double * jvec , const double * kvec , const int * actnum);

  bool            ecl_grid_exists( const char * case_input );
  char          * ecl_grid_alloc_case_filename( const char * case_input );
  
  void            ecl_grid_free(ecl_grid_type * );
  void            ecl_grid_free__( void * arg );
  grid_dims_type  ecl_grid_iget_dims( const ecl_grid_type * grid , int grid_nr);
  void            ecl_grid_get_dims(const ecl_grid_type * , int *, int * , int * , int *);
  int             ecl_grid_get_nz( const ecl_grid_type * grid );
  int             ecl_grid_get_nx( const ecl_grid_type * grid );
  int             ecl_grid_get_ny( const ecl_grid_type * grid );
  int             ecl_grid_get_nactive( const ecl_grid_type * grid );
  int             ecl_grid_get_nactive_fracture( const ecl_grid_type * grid );
  int             ecl_grid_get_active_index(const ecl_grid_type *  , int  , int  , int );
  void            ecl_grid_summarize(const ecl_grid_type * );
  void            ecl_grid_get_ijk1(const ecl_grid_type * , int global_index , int *, int * , int *);
  void            ecl_grid_get_ijk1A(const ecl_grid_type * , int active_index, int *, int * , int *);
  void            ecl_grid_get_ijk_from_active_index(const ecl_grid_type *, int , int *, int * , int * );
  void            ecl_grid_get_xyz3(const ecl_grid_type * , int , int , int , double * , double * , double *);
  void            ecl_grid_get_xyz1(const ecl_grid_type * grid , int global_index , double *xpos , double *ypos , double *zpos);
  void            ecl_grid_get_xyz1A(const ecl_grid_type * grid , int active_index , double *xpos , double *ypos , double *zpos);
  int             ecl_grid_get_global_size( const ecl_grid_type * ecl_grid );
  bool            ecl_grid_compare(const ecl_grid_type * g1 , const ecl_grid_type * g2 , bool include_lgr, bool verbose);
  int             ecl_grid_get_active_size( const ecl_grid_type * ecl_grid );
  
  double          ecl_grid_get_bottom1(const ecl_grid_type * grid , int global_index);
  double          ecl_grid_get_bottom3(const ecl_grid_type * grid , int i, int j , int k);
  double          ecl_grid_get_bottom1A(const ecl_grid_type * grid , int active_index);
  double          ecl_grid_get_top1(const ecl_grid_type * grid , int global_index);
  double          ecl_grid_get_top3(const ecl_grid_type * grid , int i, int j , int k);
  double          ecl_grid_get_top1A(const ecl_grid_type * grid , int active_index);
  double          ecl_grid_get_top2(const ecl_grid_type * grid , int i, int j);
  double          ecl_grid_get_bottom2(const ecl_grid_type * grid , int i, int j);
  int             ecl_grid_locate_depth( const ecl_grid_type * grid , double depth , int i , int j );
  
  void            ecl_grid_alloc_blocking_variables(ecl_grid_type * , int );
  void            ecl_grid_init_blocking(ecl_grid_type * );
  double          ecl_grid_block_eval3d(ecl_grid_type * grid , int i, int j , int k ,block_function_ftype * blockf );
  int             ecl_grid_get_block_count3d(const ecl_grid_type * ecl_grid , int i , int j, int k);
  bool            ecl_grid_block_value_3d(ecl_grid_type * , double  , double  ,double , double);
  
  bool            ecl_grid_cell_invalid1(const ecl_grid_type * ecl_grid , int global_index);
  bool            ecl_grid_cell_invalid3(const ecl_grid_type * ecl_grid , int i , int j , int k);
  double          ecl_grid_cell_invalid1A(const ecl_grid_type * grid , int active_index);
  
  void            ecl_grid_dump(const ecl_grid_type * grid , FILE * stream);
  void            ecl_grid_dump_ascii(const ecl_grid_type * grid , bool active_only , FILE * stream);
  
  /* lgr related functions */
  const ecl_grid_type   * ecl_grid_get_cell_lgr3(const ecl_grid_type * grid , int i, int j , int k);
  const ecl_grid_type   * ecl_grid_get_cell_lgr1A(const ecl_grid_type * grid , int active_index);
  const ecl_grid_type   * ecl_grid_get_cell_lgr1(const ecl_grid_type * grid , int global_index );
  int                     ecl_grid_get_num_lgr(const ecl_grid_type * main_grid );
  int                     ecl_grid_get_grid_nr( const ecl_grid_type * ecl_grid );
  ecl_grid_type         * ecl_grid_iget_lgr(const ecl_grid_type * main_grid , int lgr_nr);
  ecl_grid_type         * ecl_grid_get_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);
  bool                    ecl_grid_has_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);
  stringlist_type       * ecl_grid_alloc_lgr_name_list(const ecl_grid_type * ecl_grid);
  int                     ecl_grid_get_parent_cell1( const ecl_grid_type * grid , int global_index);
  int                     ecl_grid_get_parent_cell3( const ecl_grid_type * grid , int i , int j , int k);
  const ecl_grid_type   * ecl_grid_get_global_grid( const ecl_grid_type * grid );
  bool                    ecl_grid_is_lgr( const ecl_grid_type * ecl_grid );
  
  double                ecl_grid_get_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k);
  float                 ecl_grid_get_float_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k);
  double                ecl_grid_get_double_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k);
  int                   ecl_grid_get_int_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j , int k);
  
  void                    ecl_grid_grdecl_fprintf_kw( const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , const char * special_header , FILE * stream , double double_default);
  bool                    ecl_grid_test_lgr_consistency( const ecl_grid_type * ecl_grid );
  
  void                    ecl_grid_fwrite_EGRID(  ecl_grid_type * grid , const char * filename);
  void                    ecl_grid_fwrite_GRID( const ecl_grid_type * grid , const char * filename);
  void                    ecl_grid_fprintf_grdecl(  ecl_grid_type * grid , FILE * stream );
  void                    ecl_grid_fwrite_EGRID_header__( int dims[3] , const float mapaxes[6], int dualp_flag , fortio_type * fortio);
  void                    ecl_grid_fwrite_EGRID_header( int dims[3] , const float mapaxes[6], fortio_type * fortio);
  
  float          * ecl_grid_alloc_zcorn_data( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_zcorn_kw( const ecl_grid_type * grid );
  int            * ecl_grid_alloc_actnum_data( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_actnum_kw( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_hostnum_kw( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_gridhead_kw( int nx, int ny , int nz , int grid_nr);
  
  void             ecl_grid_ri_export( const ecl_grid_type * ecl_grid , double * ri_points);
  void             ecl_grid_cell_ri_export( const ecl_grid_type * ecl_grid , int global_index , double * ri_points);

  bool             ecl_grid_dual_grid( const ecl_grid_type * ecl_grid );
  
#ifdef __cplusplus
}
#endif
#endif
