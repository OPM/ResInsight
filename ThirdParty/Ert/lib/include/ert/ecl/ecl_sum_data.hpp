/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_sum_data.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_SUM_DATA_H
#define ERT_ECL_SUM_DATA_H


#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <ert/util/time_t_vector.hpp>
#include <ert/util/double_vector.hpp>
#include <ert/util/stringlist.hpp>

#include <ert/ecl/ecl_sum_tstep.hpp>
#include <ert/ecl/smspec_node.hpp>
#include <ert/ecl/ecl_sum_vector.hpp>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecl_sum_data_struct ecl_sum_data_type ;

  void                     ecl_sum_data_reset_self_map( ecl_sum_data_type * data );
  void                     ecl_sum_data_add_case(ecl_sum_data_type * self, const ecl_sum_data_type * other);
  void                     ecl_sum_data_fwrite_step( const ecl_sum_data_type * data , const char * ecl_case , bool fmt_case , bool unified, int report_step);
  void                     ecl_sum_data_fwrite( const ecl_sum_data_type * data , const char * ecl_case , bool fmt_case , bool unified);
  bool                     ecl_sum_data_can_write(const ecl_sum_data_type * data);
  bool                     ecl_sum_data_fread( ecl_sum_data_type * data , const stringlist_type * filelist, bool lazy_load, int file_options);
  ecl_sum_data_type      * ecl_sum_data_alloc_writer( ecl_smspec_type * smspec );
  ecl_sum_data_type      * ecl_sum_data_alloc( ecl_smspec_type * smspec);
  double                   ecl_sum_data_time2days( const ecl_sum_data_type * data , time_t sim_time);
  int                      ecl_sum_data_get_report_step_from_time(const ecl_sum_data_type * data , time_t sim_time);
  int                      ecl_sum_data_get_report_step_from_days(const ecl_sum_data_type * data , double days);
  bool                     ecl_sum_data_check_sim_time( const ecl_sum_data_type * data , time_t sim_time);
  bool                     ecl_sum_data_check_sim_days( const ecl_sum_data_type * data , double sim_days);
  int                      ecl_sum_data_get_num_ministep( const ecl_sum_data_type * data );
  double_vector_type     * ecl_sum_data_alloc_data_vector( const ecl_sum_data_type * data , int data_index , bool report_only);
  void                     ecl_sum_data_init_time_vector( const ecl_sum_data_type * data , time_t_vector_type * time_vector , bool report_only);
  time_t_vector_type     * ecl_sum_data_alloc_time_vector( const ecl_sum_data_type * data , bool report_only);
  time_t                   ecl_sum_data_get_data_start( const ecl_sum_data_type * data );
  time_t                   ecl_sum_data_get_report_time( const ecl_sum_data_type * data , int report_step);
  double                   ecl_sum_data_get_first_day( const ecl_sum_data_type * data);
  time_t                   ecl_sum_data_get_sim_start ( const ecl_sum_data_type * data );
  time_t                   ecl_sum_data_get_sim_end   ( const ecl_sum_data_type * data );
  double                   ecl_sum_data_get_sim_length( const ecl_sum_data_type * data );
  void                     ecl_sum_data_summarize(const ecl_sum_data_type * data , FILE * stream);
  double                   ecl_sum_data_iget( const ecl_sum_data_type * data , int internal_index , int params_index );

  double                   ecl_sum_data_iget_sim_days( const ecl_sum_data_type *  , int );
  time_t                   ecl_sum_data_iget_sim_time( const ecl_sum_data_type *  , int );
  void                     ecl_sum_data_get_interp_vector( const ecl_sum_data_type * data , time_t sim_time, const ecl_sum_vector_type * keylist, double_vector_type * results);

  bool                     ecl_sum_data_has_report_step(const ecl_sum_data_type *  , int );

  ecl_sum_data_type      * ecl_sum_data_fread_alloc( ecl_smspec_type *  , const stringlist_type * filelist , bool include_restart, bool lazy_load);
  void                     ecl_sum_data_free( ecl_sum_data_type * );
  int                      ecl_sum_data_get_last_report_step( const ecl_sum_data_type * data );
  int                      ecl_sum_data_get_first_report_step( const ecl_sum_data_type * data );

  double                   ecl_sum_data_get_from_sim_time( const ecl_sum_data_type * data , time_t sim_time , const ecl::smspec_node& smspec_node);
  double                   ecl_sum_data_get_from_sim_days( const ecl_sum_data_type * data , double sim_days , const ecl::smspec_node& smspec_node);

  int                      ecl_sum_data_get_length( const ecl_sum_data_type * data );
  int                      ecl_sum_data_iget_report_step(const ecl_sum_data_type * data , int internal_index);
  int                      ecl_sum_data_iget_report_end( const ecl_sum_data_type * data , int report_step );
  ecl_sum_tstep_type     * ecl_sum_data_add_new_tstep( ecl_sum_data_type * data , int report_step , double sim_seconds);
  bool                     ecl_sum_data_report_step_equal( const ecl_sum_data_type * data1 , const ecl_sum_data_type * data2);
  bool                     ecl_sum_data_report_step_compatible( const ecl_sum_data_type * data1 , const ecl_sum_data_type * data2);
  void                     ecl_sum_data_fwrite_interp_csv_line(const ecl_sum_data_type * data , time_t sim_time, const ecl_sum_vector_type * keylist, FILE *fp);
  double                   ecl_sum_data_get_last_value(const ecl_sum_data_type * data, int param_index);
  double                   ecl_sum_data_iget_last_value(const ecl_sum_data_type * data, int param_index);
  double                   ecl_sum_data_iget_first_value(const ecl_sum_data_type * data, int param_index);
  void                     ecl_sum_data_init_double_vector(const ecl_sum_data_type * data, int params_index, double * output_data);
  void                     ecl_sum_data_init_datetime64_vector(const ecl_sum_data_type * data, int64_t * output_data, int multiplier);

  void                     ecl_sum_data_init_double_frame(const ecl_sum_data_type * data, const ecl_sum_vector_type * keywords, double *output_data);
  double_vector_type     * ecl_sum_data_alloc_seconds_solution( const ecl_sum_data_type * data , const ecl::smspec_node& node , double value, bool rates_clamp_lower);
  void                     ecl_sum_data_init_double_frame_interp(const ecl_sum_data_type * data,
                                                                 const ecl_sum_vector_type * keywords,
                                                                 const time_t_vector_type * time_points,
                                                                 double * output_data);

  void ecl_sum_data_init_double_vector_interp(const ecl_sum_data_type * data,
                                              const ecl::smspec_node& smspec_node,
                                              const time_t_vector_type * time_points,
                                              double * output_data);


#ifdef __cplusplus
}
#endif
#endif
