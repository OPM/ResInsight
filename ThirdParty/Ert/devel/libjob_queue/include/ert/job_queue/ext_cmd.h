/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ext_cmd.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __EXT_CMD_H__
#define __EXT_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/config/config.h>


  typedef struct ext_cmd_struct ext_cmd_type;

  bool           ext_cmd_internal( const ext_cmd_type * ext_cmd );
  config_type  * ext_cmd_alloc_config();
  ext_cmd_type * ext_cmd_alloc(bool internal);
  void           ext_cmd_free( ext_cmd_type * ext_cmd );
  void           ext_cmd_free__( void * arg);
  void           ext_cmd_set_executable( ext_cmd_type * ext_cmd , const char * executable );
  ext_cmd_type * ext_cmd_config_alloc( config_type * config , const char * config_file);

  void           ext_cmd_set_executable( ext_cmd_type * ext_cmd , const char * executable);
  void           ext_cmd_set_function( ext_cmd_type * ext_cmd , const char * function);
  void           ext_cmd_set_module( ext_cmd_type * ext_cmd , const char * module);
  
#ifdef __cplusplus
}
#endif

#endif
