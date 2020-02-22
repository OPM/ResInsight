/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_io_config.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_IO_CONFIG_H
#define ERT_ECL_IO_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif


typedef struct ecl_io_config_struct ecl_io_config_type;

/* Modifiers */
void 		     ecl_io_config_set_formatted(ecl_io_config_type *, bool );
void 		     ecl_io_config_set_unified_restart(ecl_io_config_type *, bool );
void 		     ecl_io_config_set_unified_summary(ecl_io_config_type *, bool );


/* Accesors */
bool 		     ecl_io_config_get_formatted(ecl_io_config_type *);
bool 		     ecl_io_config_get_unified_restart(ecl_io_config_type *);
bool 		     ecl_io_config_get_unified_summary(ecl_io_config_type *);


/* Allocater & destructor */
ecl_io_config_type * ecl_io_config_alloc(bool ,bool ,bool);
void                 ecl_io_config_free(ecl_io_config_type * );

#ifdef __cplusplus
}
#endif
#endif
