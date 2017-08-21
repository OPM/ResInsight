/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'ert_version.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_VERSION
#define ERT_VERSION

#include <stdbool.h>

#ifdef __cplusplus
extern"C" {
#endif

char * 	     version_get_git_commit();
char * 	     version_get_git_commit_short();
char * 	     version_get_build_time();
int    	     version_get_major_ert_version();
int    	     version_get_minor_ert_version();
const char * version_get_micro_ert_version();
bool         version_is_ert_devel_version();

#ifdef __cplusplus
}
#endif

#endif
