/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   The file 'runpath_list.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_RUNPATH_LIST_H
#define ERT_RUNPATH_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define RUNPATH_LIST_DEFAULT_LINE_FMT "%03d  %s  %s  %03d\n"


  typedef struct runpath_list_struct runpath_list_type;

  void                runpath_list_free( runpath_list_type * list );
  runpath_list_type * runpath_list_alloc(const char * export_file );
  int                 runpath_list_size( const runpath_list_type * list );
  void                runpath_list_add( runpath_list_type * list , int iens , int iter, const char * runpath , const char * basename);
  void                runpath_list_clear( runpath_list_type * list );
  int                 runpath_list_iget_iens( runpath_list_type * list , int index);
  int                 runpath_list_iget_iter( runpath_list_type * list , int index);
  char *              runpath_list_iget_runpath( runpath_list_type * list , int index);
  char *              runpath_list_iget_basename( runpath_list_type * list , int index);
  void                runpath_list_set_line_fmt( runpath_list_type * list , const char * line_fmt );
  const char        * runpath_list_get_line_fmt( const runpath_list_type * list );
  void                runpath_list_fprintf( runpath_list_type * list);
  const char *        runpath_list_get_export_file( const runpath_list_type * list );
  void                runpath_list_set_export_file( runpath_list_type * list , const char * export_file );

  

#ifdef __cplusplus
}
#endif

#endif
