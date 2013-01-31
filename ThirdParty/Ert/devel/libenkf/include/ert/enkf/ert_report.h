/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_report.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __ERT_REPORT_H__
#define __ERT_REPORT_H__

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct ert_report_struct ert_report_type;
  
  ert_report_type * ert_report_alloc( const char * source_file , const char * target_file );
  void              ert_report_free( ert_report_type * ert_report );
  void              ert_report_free__(void * arg);
  bool              ert_report_create( ert_report_type * ert_report , const subst_list_type * context , const char * plot_path , const char * target_path );
  const char      * ert_report_get_basename( const ert_report_type * ert_report );
  const char      * ert_report_get_work_path( const ert_report_type * ert_report );

#ifdef __cplusplus
}
#endif
#endif
