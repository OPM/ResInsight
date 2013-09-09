/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_file_kw.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ECL_FILE_KW_H__
#define __ECL_FILE_KW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fortio.h>

typedef struct ecl_file_kw_struct ecl_file_kw_type;
typedef struct inv_map_struct inv_map_type;

  inv_map_type     * inv_map_alloc();
  ecl_file_kw_type * inv_map_get_file_kw( inv_map_type * inv_map , const ecl_kw_type * ecl_kw );
  void               inv_map_free( inv_map_type * map );

  ecl_file_kw_type * ecl_file_kw_alloc( const ecl_kw_type * ecl_kw , offset_type offset);
  void               ecl_file_kw_free( ecl_file_kw_type * file_kw );
  void               ecl_file_kw_free__( void * arg );
  ecl_kw_type      * ecl_file_kw_get_kw( ecl_file_kw_type * file_kw , fortio_type * fortio, inv_map_type * inv_map);
  ecl_kw_type      * ecl_file_kw_get_kw_ptr( ecl_file_kw_type * file_kw , fortio_type * fortio , inv_map_type * inv_map );
  ecl_file_kw_type * ecl_file_kw_alloc_copy( const ecl_file_kw_type * src );
  const char       * ecl_file_kw_get_header( const ecl_file_kw_type * file_kw );
  int                ecl_file_kw_get_size( const ecl_file_kw_type * file_kw );
  ecl_type_enum      ecl_file_kw_get_type( const ecl_file_kw_type * file_kw);
  bool               ecl_file_kw_ptr_eq( const ecl_file_kw_type * file_kw , const ecl_kw_type * ecl_kw);
  void               ecl_file_kw_replace_kw( ecl_file_kw_type * file_kw , fortio_type * target , ecl_kw_type * new_kw );
  void               ecl_file_kw_fskip_data( const ecl_file_kw_type * file_kw , fortio_type * fortio);
  void               ecl_file_kw_inplace_fwrite( ecl_file_kw_type * file_kw , fortio_type * fortio);
 
#ifdef __cplusplus
}
#endif

#endif
