/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'util_env.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_UTIL_ENV_H
#define ERT_UTIL_ENV_H



#ifdef __cplusplus
extern"C" {
#endif
  
  char       * util_alloc_PATH_executable(const char * executable );
  void         util_setenv( const char * variable , const char * value);
  const char * util_interp_setenv( const char * variable , const char * value);
  void         util_unsetenv( const char * variable);
  char       * util_alloc_envvar( const char * value );

#ifdef __cplusplus
}
#endif
#endif
