/*
   Copyright (C) 2011  Equinor ASA, Norway.

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

#ifndef ERT_ECL_GRID_H
#define ERT_ECL_GRID_H

#include <stdbool.h>

#include <ert/util/double_vector.hpp>
#include <ert/util/int_vector.hpp>
#include <ert/util/stringlist.hpp>

#include <ert/ecl/ecl_coarse_cell.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/grid_dims.hpp>
#include <ert/ecl/nnc_info.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#define ECL_GRID_COORD_SIZE(nx,ny)    (((nx) + 1) * ((ny) + 1) * 6)
#define ECL_GRID_ZCORN_SIZE(nx,ny,nz) (((nx) * (ny) * (nz) * 8))

#define ECL_GRID_GLOBAL_GRID   "Global"  // used as key in hash tables over grids.
#define  ECL_GRID_MAINGRID_LGR_NR 0

  typedef double (block_function_ftype) ( const double_vector_type *);
  typedef struct ecl_grid_struct ecl_grid_type;

  bool                         ecl_grid_have_coarse_cells( const ecl_grid_type * main_grid );
  bool                         ecl_grid_cell_in_coarse_group1( const ecl_grid_type * main_grid , int global_index );
  bool                         ecl_grid_cell_in_coarse_group3( const ecl_grid_type * main_grid , int i , int j , int k);
  int                          ecl_grid_get_num_coarse_groups( const ecl_grid_type * main_grid );
  ecl_coarse_cell_type       * ecl_grid_iget_coarse_group( const ecl_grid_type * ecl_grid , int coarse_nr );
  ecl_coarse_cell_type       * ecl_grid_get_cell_coarse_group1( const ecl_grid_type * ecl_grid , int global_index);
  ecl_coarse_cell_type       * ecl_grid_get_cell_coarse_group3( const ecl_grid_type * ecl_grid , int i , int j , int k);

  int ecl_grid_get_cell_twist1( const ecl_grid_type * ecl_grid, int global_index );
  int ecl_grid_get_cell_twist3( const ecl_grid_type * ecl_grid, int i , int j , int k);

  void            ecl_grid_get_column_property(const ecl_grid_type * ecl_grid , const ecl_kw_type * ecl_kw , int i , int j, double_vector_type * column);
  int             ecl_grid_get_global_index_from_xy_top( const ecl_grid_type * ecl_grid , double x , double y);
  int             ecl_grid_get_global_index_from_xy_bottom( const ecl_grid_type * ecl_grid , double x , double y);
  ecl_grid_type * ecl_grid_alloc_dx_dy_dz_tops( int nx, int ny , int nz , const double * dx , const double * dy , const double * dz , const double * tops , const int * actnum);

  void            ecl_grid_get_cell_corner_xyz3(const ecl_grid_type * grid , int i , int j , int k, int corner_nr , double * xpos , double * ypos , double * zpos );
  void            ecl_grid_get_cell_corner_xyz1(const ecl_grid_type * grid , int global_index , int corner_nr , double * xpos , double * ypos , double * zpos );
  void            ecl_grid_get_corner_xyz(const ecl_grid_type * grid , int i , int j , int k, double * xpos , double * ypos , double * zpos );

  double          ecl_grid_get_cell_dx1A( const ecl_grid_type * grid , int active_index);
  double          ecl_grid_get_cell_dy1A( const ecl_grid_type * grid , int active_index);
  double          ecl_grid_get_cell_dz1A( const ecl_grid_type * grid , int active_index );
  double          ecl_grid_get_cell_thickness1A( const ecl_grid_type * grid , int active_index );

  double          ecl_grid_get_cell_dx1( const ecl_grid_type * grid , int global_index );
  double          ecl_grid_get_cell_dy1( const ecl_grid_type * grid , int global_index );
  double          ecl_grid_get_cell_dz1( const ecl_grid_type * grid , int global_index );
  double          ecl_grid_get_cell_thickness1( const ecl_grid_type * grid , int global_index );

  double          ecl_grid_get_cell_dx3( const ecl_grid_type * grid , int i , int j , int k);
  double          ecl_grid_get_cell_dy3( const ecl_grid_type * grid , int i , int j , int k);
  double          ecl_grid_get_cell_dz3( const ecl_grid_type * grid , int i , int j , int k);
  double          ecl_grid_get_cell_thickness3( const ecl_grid_type * grid , int i , int j , int k);

  void            ecl_grid_get_distance(const ecl_grid_type * grid , int global_index1, int global_index2 , double *dx , double *dy , double *dz);
  double          ecl_grid_get_cdepth1A(const ecl_grid_type * grid , int active_index);
  double          ecl_grid_get_cdepth1(const ecl_grid_type * grid , int global_index);
  double          ecl_grid_get_cdepth3(const ecl_grid_type * grid , int i, int j , int k);
  int             ecl_grid_get_global_index_from_xy( const ecl_grid_type * ecl_grid , int k , bool lower_layer , double x , double y);
  bool            ecl_grid_cell_contains_xyz1( const ecl_grid_type * ecl_grid , int global_index , double x , double y , double z);
  bool            ecl_grid_cell_contains_xyz3( const ecl_grid_type * ecl_grid , int i , int j , int k, double x , double y , double z );
  double          ecl_grid_get_cell_volume1( const ecl_grid_type * ecl_grid, int global_index );
  double          ecl_grid_get_cell_volume3( const ecl_grid_type * ecl_grid, int i , int j , int k);
  double          ecl_grid_get_cell_volume1A( const ecl_grid_type * ecl_grid, int active_index );
  bool            ecl_grid_cell_contains1(const ecl_grid_type * grid , int global_index , double x , double y , double z);
  bool            ecl_grid_cell_contains3(const ecl_grid_type * grid , int i , int j ,int k , double x , double y , double z);
  int             ecl_grid_get_global_index_from_xyz(ecl_grid_type * grid , double x , double y , double z , int start_index);
  bool            ecl_grid_get_ijk_from_xyz(ecl_grid_type * grid , double x , double y , double z , int start_index, int *i, int *j, int *k );
  bool            ecl_grid_get_ij_from_xy( const ecl_grid_type * grid , double x , double y , int k , int* i, int* j);
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

  const nnc_info_type * ecl_grid_get_cell_nnc_info3( const ecl_grid_type * grid , int i , int j , int k);
  const nnc_info_type * ecl_grid_get_cell_nnc_info1( const ecl_grid_type * grid , int global_index);
  void                  ecl_grid_add_self_nnc( ecl_grid_type * grid1, int g1, int g2, int nnc_index);
  void                  ecl_grid_add_self_nnc_list( ecl_grid_type * grid, const int * g1_list , const int * g2_list , int num_nnc );

  ecl_grid_type * ecl_grid_alloc_GRDECL_kw( int nx, int ny , int nz , const ecl_kw_type * zcorn_kw , const ecl_kw_type * coord_kw , const ecl_kw_type * actnum_kw , const ecl_kw_type * mapaxes_kw );
  ecl_grid_type * ecl_grid_alloc_GRDECL_data(int , int , int , const float *  , const float *  , const int * , bool apply_mapaxes , const float * mapaxes);
  ecl_grid_type * ecl_grid_alloc_GRID_data(int num_coords , int nx, int ny , int nz , int coords_size , int ** coords , float ** corners , bool apply_mapaxes, const float * mapaxes);
  ecl_grid_type * ecl_grid_alloc(const char * );
  ecl_grid_type * ecl_grid_alloc_ext_actnum(const char * , const int * ext_actnum);
  ecl_grid_type * ecl_grid_load_case( const char * case_input );
  ecl_grid_type * ecl_grid_load_case__( const char * case_input , bool apply_mapaxes);
  ecl_grid_type * ecl_grid_alloc_rectangular( int nx , int ny , int nz , double dx , double dy , double dz , const int * actnum);
  ecl_grid_type * ecl_grid_alloc_regular( int nx, int ny , int nz , const double * ivec, const double * jvec , const double * kvec , const int * actnum);
  ecl_grid_type * ecl_grid_alloc_dxv_dyv_dzv( int nx, int ny , int nz , const double * dxv , const double * dyv , const double * dzv , const int * actnum);
  ecl_grid_type * ecl_grid_alloc_dxv_dyv_dzv_depthz( int nx, int ny , int nz , const double * dxv , const double * dyv , const double * dzv , const double * depthz , const int * actnum);
  ecl_kw_type   * ecl_grid_alloc_volume_kw( const ecl_grid_type * grid , bool active_size);
  ecl_kw_type   * ecl_grid_alloc_mapaxes_kw( const ecl_grid_type * grid );
  ecl_kw_type   * ecl_grid_alloc_coord_kw( const ecl_grid_type * grid);

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

  bool            ecl_grid_get_xyz_inside1(const ecl_grid_type * grid , int global_index , double *xpos , double *ypos , double *zpos);
  bool            ecl_grid_get_xyz_inside3(const ecl_grid_type * grid , int i , int j , int k , double *xpos , double *ypos , double *zpos);

  int             ecl_grid_get_global_size( const ecl_grid_type * ecl_grid );
  bool            ecl_grid_compare(const ecl_grid_type * g1 , const ecl_grid_type * g2 , bool include_lgr, bool include_nnc , bool verbose);
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

  bool            ecl_grid_cell_valid1(const ecl_grid_type * ecl_grid , int global_index);
  bool            ecl_grid_cell_valid3(const ecl_grid_type * ecl_grid , int i , int j , int k);
  double          ecl_grid_cell_valid1A(const ecl_grid_type * grid , int active_index);

  void            ecl_grid_dump(const ecl_grid_type * grid , FILE * stream);
  void            ecl_grid_dump_ascii(ecl_grid_type * grid , bool active_only , FILE * stream);
  void ecl_grid_dump_ascii_cell1(ecl_grid_type * grid , int global_index , FILE * stream, const double * offset);
  void ecl_grid_dump_ascii_cell3(ecl_grid_type * grid , int i , int j , int k , FILE * stream , const double * offset);

  /* lgr related functions */
  const ecl_grid_type   * ecl_grid_get_cell_lgr3(const ecl_grid_type * grid , int i, int j , int k);
  const ecl_grid_type   * ecl_grid_get_cell_lgr1A(const ecl_grid_type * grid , int active_index);
  const ecl_grid_type   * ecl_grid_get_cell_lgr1(const ecl_grid_type * grid , int global_index );
  int                     ecl_grid_get_num_lgr(const ecl_grid_type * main_grid );
  int                     ecl_grid_get_lgr_nr( const ecl_grid_type * ecl_grid );
  int                     ecl_grid_get_lgr_nr_from_name( const ecl_grid_type * grid , const char * name);
  ecl_grid_type         * ecl_grid_iget_lgr(const ecl_grid_type * main_grid , int lgr_index);
  ecl_grid_type         * ecl_grid_get_lgr_from_lgr_nr(const ecl_grid_type * main_grid, int lgr_nr);
  ecl_grid_type         * ecl_grid_get_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);
  bool                    ecl_grid_has_lgr(const ecl_grid_type * main_grid, const char * __lgr_name);
  bool                    ecl_grid_has_lgr_nr(const ecl_grid_type * main_grid, int lgr_nr);
  const char            * ecl_grid_iget_lgr_name( const ecl_grid_type * ecl_grid , int lgr_index);
  const char            * ecl_grid_get_lgr_name( const ecl_grid_type * ecl_grid , int lgr_nr);
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

  void                    ecl_grid_fwrite_dims( const ecl_grid_type * grid , fortio_type * init_file,  ert_ecl_unit_enum output_unit);
  void                    ecl_grid_fwrite_depth( const ecl_grid_type * grid , fortio_type * init_file , ert_ecl_unit_enum ouput_unit);

  void                    ecl_grid_fwrite_EGRID(  ecl_grid_type * grid , const char * filename, bool metric_output);
  void                    ecl_grid_fwrite_EGRID2( ecl_grid_type * grid , const char * filename, ert_ecl_unit_enum output_unit);

  void                    ecl_grid_fwrite_GRID( const ecl_grid_type * grid , const char * filename);
  void                    ecl_grid_fwrite_GRID2( const ecl_grid_type * grid , const char * filename, ert_ecl_unit_enum output_unit);

  void                    ecl_grid_fprintf_grdecl(  ecl_grid_type * grid , FILE * stream );
  void                    ecl_grid_fprintf_grdecl2(  ecl_grid_type * grid , FILE * stream , ert_ecl_unit_enum output_unit);

  int              ecl_grid_zcorn_index__(int nx, int ny , int i, int j , int k , int c);
  int              ecl_grid_zcorn_index(const ecl_grid_type * grid , int i, int j , int k , int c);
  ecl_grid_type * ecl_grid_alloc_EGRID(const char * grid_file, bool apply_mapaxes );
  ecl_grid_type * ecl_grid_alloc_GRID(const char * grid_file, bool apply_mapaxes );

  float          * ecl_grid_alloc_zcorn_data( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_zcorn_kw( const ecl_grid_type * grid );
  int            * ecl_grid_alloc_actnum_data( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_actnum_kw( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_hostnum_kw( const ecl_grid_type * grid );
  ecl_kw_type    * ecl_grid_alloc_gridhead_kw( int nx, int ny , int nz , int grid_nr);
  ecl_grid_type  * ecl_grid_alloc_copy( const ecl_grid_type * src_grid );
  ecl_grid_type  * ecl_grid_alloc_processed_copy( const ecl_grid_type * src_grid , const double * zcorn , const int * actnum);

  void             ecl_grid_ri_export( const ecl_grid_type * ecl_grid , double * ri_points);
  void             ecl_grid_cell_ri_export( const ecl_grid_type * ecl_grid , int global_index , double * ri_points);

  bool             ecl_grid_dual_grid( const ecl_grid_type * ecl_grid );
  int              ecl_grid_get_num_nnc( const ecl_grid_type * grid );

  bool ecl_grid_cell_regular3( const ecl_grid_type * ecl_grid, int i,int j,int k);
  bool ecl_grid_cell_regular1( const ecl_grid_type * ecl_grid, int global_index);

  void ecl_grid_init_zcorn_data( const ecl_grid_type * grid , float * zcorn );
  void ecl_grid_init_zcorn_data_double( const ecl_grid_type * grid , double * zcorn );
  int ecl_grid_get_zcorn_size( const ecl_grid_type * grid );

  void ecl_grid_init_coord_data( const ecl_grid_type * grid , float * coord );
  void ecl_grid_init_coord_data_double( const ecl_grid_type * grid , double * coord );
  int  ecl_grid_get_coord_size( const ecl_grid_type * ecl_grid);

  void ecl_grid_init_actnum_data( const ecl_grid_type * grid , int * actnum );
  bool ecl_grid_use_mapaxes( const ecl_grid_type * grid );
  void ecl_grid_init_mapaxes_data_double( const ecl_grid_type * grid , double * mapaxes);
  void ecl_grid_reset_actnum( ecl_grid_type * grid , const int * actnum );
  void ecl_grid_compressed_kw_copy( const ecl_grid_type * grid , ecl_kw_type * target_kw , const ecl_kw_type * src_kw);
  void ecl_grid_global_kw_copy( const ecl_grid_type * grid , ecl_kw_type * target_kw , const ecl_kw_type * src_kw);
  void ecl_grid_export_cell_corners1(const ecl_grid_type * grid, int global_index, double *x, double *y, double *z);

  ert_ecl_unit_enum ecl_grid_get_unit_system(const ecl_grid_type * grid);
  void ecl_grid_export_index(const ecl_grid_type * grid, int * global_index, int * index_data , bool active_only);
  void ecl_grid_export_data_as_int( int index_size, const int * global_index, const ecl_kw_type * kw, int * output);
  void ecl_grid_export_data_as_double( int index_size, const int * data_index, const ecl_kw_type * kw, double * output);
  void ecl_grid_export_volume( const ecl_grid_type * grid, int index_size, const int * global_index, double * output );
  void ecl_grid_export_position( const ecl_grid_type * grid, int index_size, const int * global_index, double * output);
  void export_corners( const ecl_grid_type * grid, int index_size, const int * global_index, double * output);

  UTIL_IS_INSTANCE_HEADER( ecl_grid );
  UTIL_SAFE_CAST_HEADER( ecl_grid );

#ifdef __cplusplus
}
namespace ecl {

  ecl_grid_type * ecl_grid_alloc_GRDECL_data(int nx,
                                             int ny,
                                             int nz,
                                             const double * zcorn,
                                             const double * coord,
                                             const int * actnum,
                                             bool apply_mapaxes,
                                             const float * mapaxes);

}

#ifdef __cplusplus
#endif

#endif
#endif
