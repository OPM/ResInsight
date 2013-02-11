/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_report_list.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __ERT_REPORT_LIST_H__
#define __ERT_REPORT_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/ecl/ecl_sum.h>  

#include <ert/config/config.h>
  
  typedef struct ert_report_list_struct ert_report_list_type;

  ert_report_list_type  * ert_report_list_alloc(const char * target_path, const char * plot_path );
  bool                    ert_report_list_add_report( ert_report_list_type * report_list , const char * template_path);
  void                    ert_report_list_free( ert_report_list_type * report_list );
  bool                    ert_report_list_add_path( ert_report_list_type * report_list , const char * path );
  void                    ert_report_list_set_target_path( ert_report_list_type * report_list , const char * target_path );
  void                    ert_report_list_set_plot_path( ert_report_list_type * report_list , const char * plot_path );
  int                     ert_report_list_get_num( const ert_report_list_type * report_list );
  void                    ert_report_list_add_groups( ert_report_list_type * report_list , const ecl_sum_type * ecl_sum , const char * group_pattern );
  void                    ert_report_list_add_wells( ert_report_list_type * report_list , const ecl_sum_type * ecl_sum , const char * well_pattern );
  void                    ert_report_list_add_global_context( ert_report_list_type * report_list , const char * key , const char * value);
  void                    ert_report_list_site_init( ert_report_list_type * report_list , config_type * config );
  void                    ert_report_list_init( ert_report_list_type * report_list , config_type * config , const ecl_sum_type * refcase);
  void                    ert_report_list_create( const ert_report_list_type * report_list , const char * current_case , bool verbose );
  
#ifdef __cplusplus
}
#endif
#endif
