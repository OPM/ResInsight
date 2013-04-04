/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_refcase_list.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef __ECL_REFCASE_LIST_H__
#define __ECL_REFCASE_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/ecl/ecl_sum.h>
  
  
  typedef struct ecl_refcase_list_struct ecl_refcase_list_type;

  ecl_refcase_list_type * ecl_refcase_list_alloc( );
  void                    ecl_refcase_list_free( ecl_refcase_list_type * refcase_list );

  bool                    ecl_refcase_list_has_default( ecl_refcase_list_type * refcase_list );
  const ecl_sum_type    * ecl_refcase_list_get_default( ecl_refcase_list_type * refcase_list ); 
  bool                    ecl_refcase_list_set_default( ecl_refcase_list_type * refcase_list , const char * default_case);
  int                     ecl_refcase_list_get_size(ecl_refcase_list_type * refcase_list );
  int                     ecl_refcase_list_add_matching( ecl_refcase_list_type * refcase_list , const char * glob_string);
  int                     ecl_refcase_list_add_case( ecl_refcase_list_type * refcase_list , const char * case_name);
  const char            * ecl_refcase_list_iget_pathcase( ecl_refcase_list_type * refcase_list , int index);
  const ecl_sum_type    * ecl_refcase_list_iget_case( ecl_refcase_list_type * refcase_list , int index);
  const ecl_sum_type    * ecl_refcase_list_get_case( ecl_refcase_list_type * refcase_list , const char * case_name);
  bool                    ecl_refcase_list_has_case( ecl_refcase_list_type * refcase_list , const char * case_name);

#ifdef __cplusplus
}
#endif
#endif
