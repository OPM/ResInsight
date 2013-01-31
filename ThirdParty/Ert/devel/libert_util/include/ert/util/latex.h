/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'latex.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

 
#ifndef __LATEX_H__
#define __LATEX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define LATEX_EXTENSION  "tex"

  typedef struct latex_struct latex_type;
  

  latex_type * latex_alloc( const char * input_file , bool in_place);
  void         latex_free( latex_type * latex );
  void         latex_set_target_file( latex_type * latex , const char * target_file );
  bool         latex_compile( latex_type * latex , bool ignore_errors , bool with_xref); 
  void         latex_set_timeout( latex_type * latex , int timeout);
  int          latex_get_timeout( const latex_type * latex );
  const char * latex_get_runpath( const latex_type * latex );
  const char * latex_get_target_file( const latex_type * latex );

  void         latex_link_path( const latex_type * latex , const char * path);
  void         latex_link_directory_content(const latex_type * latex , const char * path);

#ifdef __cplusplus
}
#endif

#endif
